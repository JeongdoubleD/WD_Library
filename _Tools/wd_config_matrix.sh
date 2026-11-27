#!/bin/bash
# =====================================================================================================
#  WD Library configuration matrix check
#
#  Builds WD_Library (+ Template/Core/_System) once per RTE switch combination listed in VARIANTS.
#  Each variant = template config (or -c dir) with some "#define RTE_xxx <value>" lines replaced.
#  A variant whose name starts with "!" must fail : a known defect (COMPLIANCE.md 6) or a negative
#  test of wd_config_check.h (cfg_check_xxx). The check
#  reports it as XFAIL, and as XPASS when it starts to build (then remove the '!').
#
#  usage : _Tools/wd_config_matrix.sh -r <ref_cubemx_project> [-c <config_dir>] [-o <out_dir>] [-j <jobs>] [--strict] [--werror] [variant ...]
#  toolchain : arm-none-eabi-gcc in PATH, or ARM_GCC_BIN=<dir>
# =====================================================================================================
set -u

# name | switches (NAME=value, space separated). Switch must exist in wd_config.h (checked).
VARIANTS='
default             |
no_print_serial     | RTE_PLATFORM_PRINT_SERIAL=0
no_telnet           | RTE_PLATFORM_PRINT_TELNET=0
no_fft              | RTE_PLATFORM_FFT=0
no_adc              | RTE_COMPONENT_ADC=0 RTE_COMPONENT_DDS=0
adc_internal_only   | RTE_COMPONENT_ADC_AD4134=0 RTE_COMPONENT_DDS=0
# legacy board (ADS127 / DDS without AD4134) : needs the previous board CubeMX project (htim1, TIM5, pins)
# that is not available - expected to fail against the reference project (COMPLIANCE.md known defects)
!dds_without_ad4134 | RTE_COMPONENT_ADC_AD4134=0
no_dds              | RTE_COMPONENT_DDS=0 RTE_COMPONENT_ADC_AD4134=0
dds_main_only       | RTE_COMPONENT_DDS_AD9837_SUB=0
no_digipot          | RTE_COMPONENT_DIGIPOT=0
no_thermometer      | RTE_COMPONENT_THERMOMETER=0
no_vio              | RTE_COMPONENT_VIO=0
no_internal_flash   | RTE_COMPONENT_FLASH_INTERNAL=0 RTE_COMPONENT_I_FLASH=0 RTE_SERVICE_FIRMWARE=0
nand_instead_of_nor | RTE_COMPONENT_NVM_NOR_FLASH=0 RTE_COMPONENT_NVM_NAND_FLASH=1
no_external_nvm     | RTE_COMPONENT_NVM_NOR_FLASH=0
no_sdram            | RTE_COMPONENT_SDRAM=0
no_can              | RTE_COMPONENT_CAN=0 RTE_SERVICE_NET_CAN=0
no_network          | RTE_COMPONENT_NETWORK=0 RTE_SERVICE_NET_ETHERNET=0 RTE_PLATFORM_PRINT_TELNET=0
no_net_services     | RTE_SERVICE_NET_CAN=0 RTE_SERVICE_NET_ETHERNET=0 RTE_COMPONENT_PROTOCOL_FACTORY=0
no_firmware         | RTE_SERVICE_FIRMWARE=0
no_mmu              | RTE_SERVICE_MMU=0 RTE_SERVICE_FIRMWARE=0
minimal             | RTE_PLATFORM_PRINT_SERIAL=0 RTE_PLATFORM_PRINT_TELNET=0 RTE_PLATFORM_FFT=0 RTE_COMPONENT_ADC=0 RTE_COMPONENT_DDS=0 RTE_COMPONENT_DIGIPOT=0 RTE_COMPONENT_THERMOMETER=0 RTE_COMPONENT_VIO=0 RTE_COMPONENT_NVM_NOR_FLASH=0 RTE_COMPONENT_SDRAM=0 RTE_COMPONENT_CAN=0 RTE_COMPONENT_NETWORK=0 RTE_SERVICE_NET_CAN=0 RTE_SERVICE_NET_ETHERNET=0 RTE_SERVICE_MMU=0 RTE_SERVICE_FIRMWARE=0
ds18b20             | RTE_COMPONENT_THERMOMETER_DS18B20=1 RTE_COMPONENT_THERMOMETER_NTC=0
mcp23017            | RTE_COMPONENT_IO_EXPANDER=1 RTE_COMPONENT_IO_EXPANDER_MCP23017=1
!ads127             | RTE_COMPONENT_ADC_ADS127=1 RTE_COMPONENT_ADC_AD4134=0
!cfg_check_ad4134   | RTE_COMPONENT_DDS=0
'

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REF=""; CFG="$ROOT/WD_Library/_WD_Config_Template"; OUT="$ROOT/_build_check/matrix"; JOBS=8; ONLY=""; STRICT=0; WERROR=0

while [ $# -gt 0 ]; do
    case "$1" in
        -r) REF="$2"; shift 2 ;;
        -c) CFG="$2"; shift 2 ;;
        -o) OUT="$2"; shift 2 ;;
        -j) JOBS="$2"; shift 2 ;;
        --strict) STRICT=1; shift ;;
        --werror) WERROR=1; shift ;;
        *) ONLY="$ONLY $1"; shift ;;
    esac
done
if [ -z "$REF" ] || [ ! -d "$REF/Drivers" ]; then
    echo "error : reference CubeMX project (-r) is required"; exit 2
fi

BIN="${ARM_GCC_BIN:-}"
GCC="arm-none-eabi-gcc"; [ -n "$BIN" ] && GCC="$BIN/arm-none-eabi-gcc"
export GCC

FLAGS="-mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -std=gnu11 -O0 -ffunction-sections -fdata-sections \
-DDEBUG -DARM_MATH_CM4 -DUSE_HAL_DRIVER -DSTM32H743xx -DUSE_PWR_LDO_SUPPLY"
WARN="-Wall"
[ $STRICT -eq 1 ] && WARN="$WARN -Wextra -Wconversion -Wsign-conversion -Wshadow -Wstrict-prototypes -Wmissing-prototypes"
[ $WERROR -eq 1 ] && WARN="$WARN -Werror"
export FLAGS_LIB="$FLAGS $WARN" FLAGS_TP="$FLAGS -Wall"   # third party sources : -Wall only (COMPLIANCE.md 4)
LIBINC="-I$ROOT/Template/Core/_System"
while IFS= read -r d; do
    case "$d" in *_WD_Third_Party*) LIBINC="$LIBINC -isystem $d" ;; *) LIBINC="$LIBINC -I$d" ;; esac
done < <(find "$ROOT/WD_Library" -type d ! -path '*_WD_Config_Template*')
for d in Core/Inc Drivers/STM32H7xx_HAL_Driver/Inc Drivers/STM32H7xx_HAL_Driver/Inc/Legacy \
         Middlewares/Third_Party/FreeRTOS/Source/include Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 \
         Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
         Drivers/CMSIS/Device/ST/STM32H7xx/Include Drivers/CMSIS/Include Drivers/CMSIS/DSP/Include; do
    LIBINC="$LIBINC -isystem $REF/$d"
done
SOURCES="$(find "$ROOT/WD_Library" "$ROOT/Template/Core/_System" -name '*.c' ! -path '*_WD_Config_Template*')"

rm -rf "$OUT"; mkdir -p "$OUT"
total=0; bad=0
while IFS='|' read -r name switches; do
    name="$(echo "$name" | tr -d ' \t')"; [ -z "$name" ] && continue
    case "$name" in '#'*) continue ;; esac
    expect_fail=0; case "$name" in '!'*) expect_fail=1; name="${name#!}";; esac
    if [ -n "$ONLY" ]; then case " $ONLY " in *" $name "*) ;; *) continue;; esac; fi
    total=$((total + 1))

    dir="$OUT/$name"; mkdir -p "$dir/cfg" "$dir/obj"
    cp "$CFG"/*.h "$dir/cfg/"
    status=""
    for sw in $switches; do
        key="${sw%%=*}"; val="${sw#*=}"
        if ! grep -qE "^#define[[:space:]]+$key[[:space:]]" "$dir/cfg/wd_config.h"; then status="BAD-SWITCH($key)"; break; fi
        sed -i -E "s/^(#define[[:space:]]+$key[[:space:]]+)[^[:space:]]+/\1$val/" "$dir/cfg/wd_config.h"
    done

    if [ -z "$status" ]; then
        echo "$SOURCES" | xargs -P "$JOBS" -I{} sh -c \
            'fl="$FLAGS_LIB"; case "$2" in *_WD_Third_Party*) case "$2" in */Port/*) ;; *) fl="$FLAGS_TP" ;; esac ;; esac; "$GCC" $fl -I"$1" '"$LIBINC"' -c "$2" -o "$3/$(basename "$2").o" 2>>"$3/$(basename "$2").log" || echo "$2" >> "$3/../fail.txt"' \
            _ "$dir/cfg" {} "$dir/obj"
        cat "$dir"/obj/*.log > "$dir/build.log" 2>/dev/null; rm -f "$dir"/obj/*.log
        nfail=$( [ -f "$dir/fail.txt" ] && wc -l < "$dir/fail.txt" || echo 0 )
        nwarn=$(grep -c 'warning:' "$dir/build.log" || true)
        if [ "$nfail" -eq 0 ]; then
            [ $expect_fail -eq 1 ] && status="XPASS (builds now : remove '!')" || status="OK"
        else
            [ $expect_fail -eq 1 ] && status="XFAIL ($nfail files, expected)" || status="FAIL ($nfail files)"
        fi
        status="$status  warnings=$nwarn"
    fi
    case "$status" in OK*|XFAIL*) ;; *) bad=$((bad + 1));; esac
    printf '%-20s %s\n' "$name" "$status"
done <<< "$VARIANTS"

echo "variants : $total, unexpected : $bad   (logs : $OUT/<variant>/build.log, fail.txt)"
[ $bad -eq 0 ]
