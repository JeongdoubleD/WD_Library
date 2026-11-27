#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <stdint.h>


#define SOCK_TCP4			 (Sn_MR_TCP)
#define SOCK_TCP6			 (Sn_MR_TCP6)
#define SOCK_TCPD			 (Sn_MR_TCPD)

#define SOCK_UDP4			 (Sn_MR_UDP4)
#define SOCK_UDP6			 (Sn_MR_UDP6)
#define SOCK_UDPD			 (Sn_MR_UDPD)

#define AS_IPV4 2
#define AS_IPV6 23
#define AS_IPDUAL 11
/* WD-PATCH-2 : WD Library port layer (bounded waits, symbols the io6Library expects from the application) */
#include "_wd_w6100_port.h"
#endif
