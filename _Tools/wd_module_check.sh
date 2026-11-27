#!/bin/bash
# =====================================================================================================
#  WD Library module isolation check
#
#  For every Driver / Component module : compile the module folder with ONLY
#    wd config headers + _WD_Core + the module folder + the folders of its declared dependencies
#  on the include path, then check with nm that every symbol the module needs is provided by
#  core / its dependencies / HAL / RTOS / libc / board handles (no hidden dependency).
#
#  usage : _Tools/wd_module_check.sh -r <ref_cubemx_project> [-c <config_dir>] [-o <out_dir>] [module ...]
#  toolchain : arm-none-eabi-gcc in PATH, or ARM_GCC_BIN=<dir>
#
#  The manifest below is the dependency table published in README.md (keep both in sync).
# =====================================================================================================
set -u

# name | module folder (relative to WD_Library) | dependency module names (space separated)
MANIFEST='
softspi  | _WD_Drivers/_Driver_Soft_SPI                  |
softi2c  | _WD_Drivers/_Driver_Soft_I2C                  |
gpiofunc | _WD_Drivers/_Driver_Gpio                      |
blockdev | _WD_Drivers/_Driver_BlockDevice               |
adc      | _WD_Component/_Component_ADC                  | softspi
dds      | _WD_Component/_Component_DDS                  | softspi
digipot  | _WD_Component/_Component_DigiPOT              | softspi
ioexp    | _WD_Component/_Component_IO_Expander          | softi2c
iflash   | _WD_Component/_Component_Flash_IN             | blockdev
nvm      | _WD_Component/_Component_NVM_NOR              | blockdev
sdram    | _WD_Component/_Component_SDRAM                |
thermo   | _WD_Component/_Component_Thermometer          |
fdcan    | _WD_Component/_Component_CAN                  |
eth      | _WD_Component/_Component_Ethernet             | w6100
vio      | _WD_Component/_Component_Virtual_IO           |
w6100    | _WD_Third_Party/WIZnet_W6100                  |
'

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LIB="$ROOT/WD_Library"
REF=""; CFG="$LIB/_WD_Config_Template"; OUT="$ROOT/_build_check/modules"; ONLY=""

while [ $# -gt 0 ]; do
    case "$1" in
        -r) REF="$2"; shift 2 ;;
        -c) CFG="$2"; shift 2 ;;
        -o) OUT="$2"; shift 2 ;;
        *) ONLY="$ONLY $1"; shift ;;
    esac
done
if [ -z "$REF" ] || [ ! -d "$REF/Drivers" ]; then
    echo "error : reference CubeMX project (-r) is required"; exit 2
fi

BIN="${ARM_GCC_BIN:-}"
GCC="arm-none-eabi-gcc"; NM="arm-none-eabi-nm"
if [ -n "$BIN" ]; then GCC="$BIN/arm-none-eabi-gcc"; NM="$BIN/arm-none-eabi-nm"; fi

FLAGS="-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -std=gnu11 -O0 -ffunction-sections -fdata-sections \
-DDEBUG -DARM_MATH_CM4 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY -Wall"
BASE_INC="-I$CFG -I$LIB/_WD_Core"
for d in Core/Inc Drivers/STM32H7xx_HAL_Driver/Inc Drivers/STM32H7xx_HAL_Driver/Inc/Legacy \
         Middlewares/Third_Party/FreeRTOS/Source/include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 \
         Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
         Drivers/CMSIS/Device/ST/STM32H7xx/Include Drivers/CMSIS/Include Drivers/CMSIS/DSP/Include; do
    BASE_INC="$BASE_INC -I$REF/$d"
done

# symbols that may stay unresolved : HAL / LL / CMSIS / RTOS / libc / compiler runtime / board HAL handles
EXTERNAL_OK='^(HAL_|LL_|os|v[A-Z]|x[A-Z]|pv[A-Z]|ux[A-Z]|e[A-Z]|__aeabi|__assert|_ctype_|_impure_ptr|SystemCoreClock|uwTick|mem|str|v?s?n?printf|puts|putc|putchar|getchar|sscanf|ato|rand|srand|sqrt|sin|cos|tan|atan|asin|acos|log|exp|pow|fmod|modf|round|floor|ceil|fabs|abs|mktime|localtime|gmtime|asctime|strftime|time|qsort|bsearch|fflush|fwrite|fputc|__errno|arm_|h[a-z]+[0-9]*$|Error_Handler$|MX_[A-Za-z0-9_]+_Init$)'

module_field() { echo "$MANIFEST" | awk -F'|' -v m="$1" -v f="$2" '{gsub(/^[ \t]+|[ \t]+$/, "", $1)} $1==m {gsub(/^[ \t]+|[ \t]+$/, "", $f); print $f}'; }

# dependency closure (depth-first, unique)
deps_closure() {
    local seen=" " stack="$1" m d
    while [ -n "$stack" ]; do
        m="${stack%% *}"; stack="${stack#"$m"}"; stack="${stack# }"
        for d in $(module_field "$m" 3); do
            case "$seen" in *" $d "*) ;; *) seen="$seen$d "; stack="$stack $d"; stack="${stack# }";; esac
        done
    done
    echo $seen
}

compile_dir() { # $1 = out dir, $2 = include args, $3 = source dir
    local f ok=0
    mkdir -p "$1"
    while IFS= read -r f; do
        "$GCC" $FLAGS $2 -c "$f" -o "$1/$(basename "$f").o" 2>>"$1/build.log" || { echo "    compile FAIL : ${f#$LIB/}"; ok=1; }
    done < <(find "$3" -name '*.c')
    return $ok
}

rm -rf "$OUT"; mkdir -p "$OUT"
compile_dir "$OUT/_core" "$BASE_INC" "$LIB/_WD_Core" || { echo "core does not compile"; exit 1; }

total=0; failed=0
MODULES=$(echo "$MANIFEST" | awk -F'|' 'NF>=2 {gsub(/^[ \t]+|[ \t]+$/, "", $1); if ($1 != "") print $1}')
[ -n "$ONLY" ] && MODULES="$ONLY"

for m in $MODULES; do
    total=$((total + 1))
    dir="$LIB/$(module_field "$m" 2)"
    deps="$(deps_closure "$m")"
    inc="$BASE_INC"
    for d in $m $deps; do
        while IFS= read -r sub; do inc="$inc -I$sub"; done < <(find "$LIB/$(module_field "$d" 2)" -type d)
    done

    status="OK"
    compile_dir "$OUT/$m" "$inc" "$dir" || status="COMPILE-FAIL"
    for d in $deps; do
        [ -d "$OUT/$d" ] || compile_dir "$OUT/$d" "$inc" "$LIB/$(module_field "$d" 2)" >/dev/null
    done

    if [ "$status" = "OK" ]; then
        objs_def="$OUT/_core/*.o $OUT/$m/*.o"; for d in $deps; do objs_def="$objs_def $OUT/$d/*.o"; done
        "$NM" -g --defined-only $objs_def 2>/dev/null | awk 'NF==3{print $3}' | sort -u > "$OUT/$m.def"
        "$NM" -u "$OUT"/$m/*.o 2>/dev/null | awk '{print $NF}' | grep -v ':' | sort -u > "$OUT/$m.undef"
        comm -23 "$OUT/$m.undef" "$OUT/$m.def" | grep -v "^$" | grep -vE "$EXTERNAL_OK" > "$OUT/$m.hidden" || true
        if [ -s "$OUT/$m.hidden" ]; then status="HIDDEN-DEP : $(tr '\n' ' ' < "$OUT/$m.hidden")"; fi
    fi
    [ "$status" = "OK" ] || failed=$((failed + 1))
    printf '%-9s deps=[%s]  %s\n' "$m" "$deps" "$status"
done

echo "modules : $total, failed : $failed   (logs : $OUT/<module>/build.log)"
[ $failed -eq 0 ]
