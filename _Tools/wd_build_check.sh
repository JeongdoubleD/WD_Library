#!/bin/bash
# =====================================================================================================
#  WD Library build check  (SW-STD-C-001 section 7 : compiler diagnostics / dependency check)
#
#  Compiles WD_Library (+ Template/Core/_System) against a reference STM32CubeMX project that provides
#  HAL / CMSIS / FreeRTOS / Core/Inc headers. Nothing is written into the reference project.
#
#  usage : _Tools/wd_build_check.sh -r <ref_cubemx_project> [-c <config_dir>] [-o <out_dir>]
#                                   [--strict] [--werror] [--no-template] [--deps] [--core] [--headers]
#    -r  reference CubeMX project root (contains Core/Inc, Drivers/, Middlewares/)
#    -c  directory with wd config headers (default : WD_Library/_WD_Config_Template)
#    -o  output directory for objects / logs (default : ./_build_check)
#    --strict      target warning set of COMPLIANCE.md (-Wextra -Wconversion ...)
#    --werror      treat warnings as errors
#    --no-template compile WD_Library only
#    --deps        list external symbols required by the library (nm)
#    --core        compile the core (_WD_Core) alone with only the core include path (isolation check)
#    --headers     compile every header alone, twice (self-contained + include guard, GEN-M-021)
#
#  toolchain : arm-none-eabi-gcc in PATH, or ARM_GCC_BIN=<dir containing arm-none-eabi-gcc>
# =====================================================================================================
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REF=""; CFG="$ROOT/WD_Library/_WD_Config_Template"; OUT="$ROOT/_build_check"
STRICT=0; WERROR=0; TEMPLATE=1; DEPS=0; CORE=0; HEADERS=0

while [ $# -gt 0 ]; do
    case "$1" in
        -r) REF="$2"; shift 2 ;;
        -c) CFG="$2"; shift 2 ;;
        -o) OUT="$2"; shift 2 ;;
        --strict) STRICT=1; shift ;;
        --werror) WERROR=1; shift ;;
        --no-template) TEMPLATE=0; shift ;;
        --deps) DEPS=1; shift ;;
        --core) CORE=1; TEMPLATE=0; shift ;;
        --headers) HEADERS=1; shift ;;
        *) echo "unknown option : $1"; exit 2 ;;
    esac
done

if [ -z "$REF" ] || [ ! -d "$REF/Drivers" ]; then
    echo "error : reference CubeMX project (-r) is required"; exit 2
fi

BIN="${ARM_GCC_BIN:-}"
GCC="arm-none-eabi-gcc"; NM="arm-none-eabi-nm"
if [ -n "$BIN" ]; then GCC="$BIN/arm-none-eabi-gcc"; NM="$BIN/arm-none-eabi-nm"; fi
command -v "$GCC" >/dev/null 2>&1 || [ -x "$GCC" ] || [ -x "$GCC.exe" ] || { echo "error : $GCC not found"; exit 2; }

CPU="-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb"
DEFS="-DDEBUG -DARM_MATH_CM4 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY"
WARN="-Wall"
[ $STRICT -eq 1 ] && WARN="$WARN -Wextra -Wconversion -Wsign-conversion -Wshadow -Wstrict-prototypes -Wmissing-prototypes"
[ $WERROR -eq 1 ] && WARN="$WARN -Werror"
FLAGS="$CPU -std=gnu11 -O0 -ffunction-sections -fdata-sections $DEFS $WARN"
# third party sources (WIZnet upstream) : not covered by the warning policy (COMPLIANCE.md 4)
FLAGS_TP="$CPU -std=gnu11 -O0 -ffunction-sections -fdata-sections $DEFS -Wall"

INC="-I$CFG"
[ $TEMPLATE -eq 1 ] && INC="$INC -I$ROOT/Template/Core/_System"
if [ $CORE -eq 1 ]; then
    INC="$INC -I$ROOT/WD_Library/_WD_Core"
else
    # third party / SDK headers as system headers : their warnings are not ours (strict / werror builds)
    while IFS= read -r d; do
        case "$d" in *_WD_Third_Party*) INC="$INC -isystem $d" ;; *) INC="$INC -I$d" ;; esac
    done < <(find "$ROOT/WD_Library" -type d ! -path '*_WD_Config_Template*')
fi
for d in Core/Inc Drivers/STM32H7xx_HAL_Driver/Inc Drivers/STM32H7xx_HAL_Driver/Inc/Legacy \
         Middlewares/Third_Party/FreeRTOS/Source/include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 \
         Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
         Drivers/CMSIS/Device/ST/STM32H7xx/Include Drivers/CMSIS/Include Drivers/CMSIS/DSP/Include; do
    INC="$INC -isystem $REF/$d"
done

rm -rf "$OUT"; mkdir -p "$OUT/obj"
SRCDIRS="$ROOT/WD_Library"; [ $TEMPLATE -eq 1 ] && SRCDIRS="$SRCDIRS $ROOT/Template/Core/_System"
[ $CORE -eq 1 ] && SRCDIRS="$ROOT/WD_Library/_WD_Core"

ok=0; fail=0
while IFS= read -r f; do
    o="$OUT/obj/$(basename "$f").o"
    fl="$FLAGS"; case "$f" in *_WD_Third_Party*) case "$f" in */Port/*) ;; *) fl="$FLAGS_TP" ;; esac ;; esac
    if "$GCC" $fl $INC -c "$f" -o "$o" 2>>"$OUT/build.log"; then ok=$((ok + 1)); else fail=$((fail + 1)); echo "FAIL $f" >> "$OUT/fail.log"; fi
done < <(find $SRCDIRS -name '*.c' ! -path '*_WD_Config_Template*')

warn_all=$(grep -c 'warning:' "$OUT/build.log" 2>/dev/null || true)
warn_lib=$(grep 'warning:' "$OUT/build.log" 2>/dev/null | grep -v '_WD_Third_Party' | grep -c 'WD_Library' || true)
echo "compiled : ok=$ok fail=$fail  warnings=$warn_all (WD_Library excl. third party : $warn_lib)"
echo "log      : $OUT/build.log"

# ----------------------------------------------------------------------------- header check (GEN-M-021)
# every header compiles alone and twice in a row (self-contained + include guard)
if [ $HEADERS -eq 1 ]; then
    hok=0; hbad=""
    mkdir -p "$OUT/headers"
    while IFS= read -r h; do
        b="$(basename "$h")"
        printf '#include "%s"\n#include "%s"\n' "$b" "$b" > "$OUT/headers/t.c"
        if "$GCC" $FLAGS $INC -fsyntax-only "$OUT/headers/t.c" 2>"$OUT/headers/$b.log"; then hok=$((hok + 1)); else hbad="$hbad $b"; fi
    done < <(find $SRCDIRS -name '*.h' ! -path '*_WD_Config_Template*' ! -path '*_WD_Third_Party*')
    echo "headers  : ok=$hok fail=$(echo $hbad | wc -w)${hbad:+ :$hbad}  (logs : $OUT/headers/)"
    [ -z "$hbad" ] || fail=$((fail + 1))
fi

if [ $DEPS -eq 1 ] && [ $fail -eq 0 ]; then
    "$NM" -g --defined-only "$OUT"/obj/*.o | awk 'NF==3{print $3}' | sort -u > "$OUT/defined.txt"
    "$NM" -u "$OUT"/obj/*.o | awk '{print $NF}' | grep -v ':' | sort -u > "$OUT/undefined.txt"
    comm -23 "$OUT/undefined.txt" "$OUT/defined.txt" > "$OUT/external.txt"
    echo "external symbols required ($(wc -l < "$OUT/external.txt")) : $OUT/external.txt"
    grep -E '^(h[a-z]+[0-9]*|Error_Handler)$' "$OUT/external.txt" | tr '\n' ' '; echo
fi

[ $fail -eq 0 ] || exit 1
