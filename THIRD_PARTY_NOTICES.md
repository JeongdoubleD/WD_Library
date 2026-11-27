# Third party notices

The WD Library is MIT licensed (see `LICENSE`, Copyright (c) 2025-2026 DoubleD).
It contains or is derived in part from the third party code below. Every affected source file states the
license expression in its first line (`SPDX-License-Identifier`) and names the original author.

| Component | Files | License | Copyright | Origin |
|---|---|---|---|---|
| WIZnet io6Library (W6100 driver, socket, DHCPv4/v6, DNS) | `WD_Library/_WD_Third_Party/WIZnet_W6100/` (except `Port/`) | MIT | (c) 2019 WIZnet Co., LTD. | https://github.com/Wiznet/io6Library commit `8ac15e2`, two marked patches (`README_WD.md`, `wd_patches.diff`) |
| mbed OS minimal-printf | `WD_Library/_WD_Core/_wd_print.c` (`mbed_minimal_formatted_string*` functions) | Apache-2.0 | (c) 2017-2020 ARM Limited | https://github.com/ARMmbed/mbed-os `platform/source/minimal-printf/mbed_printf_implementation.c`, adapted (names, configuration, casts) |
| Analog Devices no-OS ad713x driver | `WD_Library/_WD_Component/_Component_ADC/_wd_ad4134.c`, `_wd_ad4134.h` (`ad713x_*`) | BSD-3-Clause | 2020 (c) Analog Devices, Inc. | https://github.com/analogdevicesinc/no-OS `drivers/adc/ad713x`, adapted |
| Analog Devices no-OS ad525x driver | `WD_Library/_WD_Component/_Component_DigiPOT/_wd_ad5235.c`, `_wd_ad5235.h` (`ad525x_*`) | BSD-3-Clause | 2013 (c) Analog Devices, Inc. | https://github.com/analogdevicesinc/no-OS `drivers/potentiometer/ad525x`, adapted |
| STMicroelectronics IS42S32800G SDRAM component | `WD_Library/_WD_Component/_Component_SDRAM/_wd_sdram_is42s.c`, `_wd_sdram_is42s.h` | BSD-3-Clause | (c) 2019 STMicroelectronics | https://github.com/STMicroelectronics/stm32-is42s32800g, adapted |
| STM32CubeMX generated code | `Template/Core/_System/_main.c` (generated skeleton) | BSD-3-Clause | (c) 2024 STMicroelectronics | STM32CubeMX project template |
| LwOW - Lightweight onewire library | `WD_Library/_WD_Component/_Component_Thermometer/_wd_ds18b20.c`, `_wd_ds18b20.h` (`lwow_*`) | MIT | (c) 2024 Tilen MAJERLE | https://github.com/MaJerle/lwow, embedded and adapted |
| CRC-32 table | `WD_Library/_WD_Service/_Protocol_Factory/_wd_factory.c` (disabled block) | permissive notice in the file | (C) 1986 Gary S. Brown | "You may use this program, or code or tables extracted from it, as desired without restriction." |

License texts : `LICENSES/Apache-2.0.txt`, `LICENSES/BSD-3-Clause.txt`, `LICENSES/MIT-third-party.txt`,
`WD_Library/_WD_Third_Party/WIZnet_W6100/LICENSE`.

