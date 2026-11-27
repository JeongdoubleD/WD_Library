# WIZnet io6Library (W6100) - WD Library copy

| | |
|---|---|
| Upstream | https://github.com/Wiznet/io6Library |
| Version | commit `8ac15e2537593998d49a369d4f9b59ac9f9ab72e` (2025-04-17), socket.c V1.0.1 |
| License | MIT, Copyright (c) 2019 WIZnet. See [LICENSE](LICENSE) and the header of every source file |
| Files used | `Ethernet/` (socket, wizchip_conf, W6100), `Internet/DHCP4`, `Internet/DHCP6`, `Internet/DNS`, `Application/Application.h` |
| Not copied | `Application/loopback`, `Internet/SNTP`, `Doxygen` |

The files are kept as in the upstream repository. The folder layout is the same, so a new upstream
version can be dropped in and compared file by file. `.clang-format` in `_WD_Third_Party` disables
formatting, and the WD coding standard does not apply here (COMPLIANCE.md section 4).

## WD patches

Only two upstream files are changed. Every changed line is marked `WD-PATCH-n`.
The complete diff against upstream is in [wd_patches.diff](wd_patches.diff).

| ID | File | Change | Reason |
|---|---|---|---|
| WD-PATCH-1 | `Ethernet/socket.c` | `WIZCHIP_PORT_WAIT_START()` / `WIZCHIP_PORT_WAIT_CHECK()` added to the five `while(1)` wait loops of `send`, `recv`, `sendto` (2), `recvfrom` | Upstream waits forever when the chip stops responding. The wait now returns `SOCKERR_TIMEOUT` after `WD_CONF_W6100_WAIT_TIMEOUT_US` (default 5 s) |
| WD-PATCH-2 | `Application/Application.h` | `#include "_wd_w6100_port.h"` at the end | `Application.h` is the upstream user header included by `wizchip_conf.h`. It brings the port macros into every library source |

## Port files (WD code, DoubleD)

| File | Content |
|---|---|
| `Port/_wd_w6100_port.h` | port macros used by WD-PATCH-1, `WD_CONF_W6100_WAIT_TIMEOUT_US` (0 = wait forever, upstream behaviour) |
| `Port/_wd_w6100_port.c` | bounded wait using the core microsecond timer (`_PF_Time`), timeout message, `uint8_t IP_TYPE` (declared `extern` by `Internet/DNS/dns.c` and normally defined by the upstream example application) |

The SPI callbacks, chip select and critical section are registered at run time by the Ethernet component
(`_WD_Component/_Component_Ethernet`) with the upstream `reg_wizchip_*_cbfunc()` API, so no other port code is needed.

## Changes compared with the previous WD copy (before v1.1)

The previous copy had changes spread over the upstream files. They were replaced as follows:

| Previous change | Now |
|---|---|
| Timeouts written into `socket.c` with `wd_time_get_us()` | WD-PATCH-1 + port file (same 5 s limit, same message) |
| Include guards renamed `WD_DRIVERS_...` | upstream guards |
| `SOCK_TCP4`... / `AS_IPV4`... added to `wizchip_conf.h` | upstream `Application/Application.h` provides them |
| `uint8_t IP_TYPE` added to `wizchip_conf.c` | port file |
| `uint8_t Padding[2]` in `wiz_NetInfo` | removed. The compiler inserts the same 2 bytes, layout unchanged |
| `WD_PRINT_DEBUG()` SPI frame dump calls in `w6100.c` (compiled to nothing) | removed |
| `printf` replaced by `WD_LOG_INFO` in DHCPv4 / DNS, removed in DHCPv6 | upstream `printf`. Only visible when the application calls the DHCP / DNS clients (the WD components do not) |
| `Internet/*.c` flat folder | upstream `Internet/DHCP4`, `DHCP6`, `DNS` folders |

## Updating to a new upstream version

1. Copy the upstream files listed above over this folder (keep `Port/`, `README_WD.md`, `wd_patches.diff`).
2. Re-apply WD-PATCH-1 and WD-PATCH-2 (`wd_patches.diff`), then regenerate `wd_patches.diff`.
3. Run `_Tools/wd_module_check.sh` (modules `w6100`, `eth`) and the full build check.

## Known upstream warnings (not fixed, third party)

`socket.c` unused `local_port`, `dhcpv4.c` misleading indentation, `dhcpv6.c` unused variables and
`memset(..., sizeof(pointer))` at line 1249 (clears only 4 bytes). DHCPv6 is not used by the WD components.
