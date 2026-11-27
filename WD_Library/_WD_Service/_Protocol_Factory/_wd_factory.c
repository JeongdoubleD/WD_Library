/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_factory.c
 *
 *  Created on: Jan 13, 2025
 *      Author: doubled
 */


//#if   (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_BOOTLOADER)
//#define WD_CONF_CONSOLE_LEVEL_COMPILE                                             WD_CONSOLE_LEVEL_INFO
//#elif (RTE_CONFIG_SOFTWARE_TYPE == RTE_SOFTWARE_TYPE_APPLICATON)
//#define WD_CONF_CONSOLE_LEVEL_COMPILE                                             WD_CONSOLE_LEVEL_INFO
//#endif

#include "wd.h"


#if 1 // 토글

#if (RTE_COMPONENT_PROTOCOL_FACTORY)

/* keep the console quiet during long NVM operations (only when the CLI is built) */
#if (RTE_COMPONENT_CLI)
#define FACTORY_CONSOLE_LOCK()   wd_cli_lock()
#define FACTORY_CONSOLE_UNLOCK() wd_cli_unlock()
#else
#define FACTORY_CONSOLE_LOCK()   ((void)0)
#define FACTORY_CONSOLE_UNLOCK() ((void)0)
#endif
/***************************************************************************************************************************************************************
 **********  XYZ Modem
 **************************************************************************************************************************************************************/
#define ZPAD   '*'           /* 052 Padding character begins frames */
#define ZDLE   030           /* Ctrl-X Zmodem escape - `ala BISYNC DLE */
#define ZDLEE  (ZDLE ^ 0100) /* Escaped ZDLE as transmitted */
#define ZBIN   'A'           /* Binary frame indicator */
#define ZHEX   'B'           /* HEX frame indicator */
#define ZBIN32 'C'           /* Binary frame with 32 bit FCS */

/* Frame types (see array "frametypes" in zm.c) */
#define ZRQINIT    0  /* Request receive init */
#define ZRINIT     1  /* Receive init */
#define ZSINIT     2  /* Send init sequence (optional) */
#define ZACK       3  /* ACK to above */
#define ZFILE      4  /* File name from sender */
#define ZSKIP      5  /* To sender: skip this file */
#define ZNAK       6  /* Last packet was garbled */
#define ZABORT     7  /* Abort batch transfers */
#define ZFIN       8  /* Finish session */
#define ZRPOS      9  /* Resume data trans at this position */
#define ZDATA      10 /* Data packet(s) follow */
#define ZEOF       11 /* End of file */
#define ZFERR      12 /* Fatal Read or Write error Detected */
#define ZCRC       13 /* Request for file CRC and response */
#define ZCHALLENGE 14 /* Receiver's Challenge */
#define ZCOMPL     15 /* Request is complete */
#define ZCAN       16 /* Other end canned session with CAN*5 */
#define ZFREECNT   17 /* Request for free bytes on filesystem */
#define ZCOMMAND   18 /* Command from sending program */
#define ZSTDERR    19 /* Output to standard error, data follows */

/* ZDLE sequences */
#define ZCRCE 'h' /* CRC next, frame ends, header packet follows */
#define ZCRCG 'i' /* CRC next, frame continues nonstop */
#define ZCRCQ 'j' /* CRC next, frame continues, ZACK expected */
#define ZCRCW 'k' /* CRC next, ZACK expected, end of frame */
#define ZRUB0 'l' /* Translate to rubout 0177 */
#define ZRUB1 'm' /* Translate to rubout 0377 */

/* zdlread return values (internal) */
/* -1 is general error, -2 is timeout */
#define GOTOR   0400
#define GOTCRCE (ZCRCE | GOTOR) /* ZDLE-ZCRCE received */
#define GOTCRCG (ZCRCG | GOTOR) /* ZDLE-ZCRCG received */
#define GOTCRCQ (ZCRCQ | GOTOR) /* ZDLE-ZCRCQ received */
#define GOTCRCW (ZCRCW | GOTOR) /* ZDLE-ZCRCW received */
#define GOTCAN  (GOTOR | 030)   /* CAN*5 seen */

/* Byte positions within header array */
#define ZF0 3 /* First flags byte */
#define ZF1 2
#define ZF2 1
#define ZF3 0
#define ZP0 0 /* Low order 8 bits of position */
#define ZP1 1
#define ZP2 2
#define ZP3 3 /* High order 8 bits of file position */

/* Bit Masks for ZRINIT flags byte ZF0 */
#define CANFDX  0x01 /* Rx can send and receive true FDX */
#define CANOVIO 0x02 /* Rx can receive data during disk I/O */
#define CANBRK  0x04 /* Rx can send a break signal */
#define CANCRY  0x08 /* Receiver can decrypt */
#define CANLZW  0x10 /* Receiver can uncompress */
#define CANFC32 0x20 /* Receiver can use 32 bit Frame Check */
#define ESCCTL  0x40 /* Receiver expects ctl chars to be escaped */
#define ESC8    0x80 /* Receiver expects 8th bit to be escaped */

/* Bit Masks for ZRINIT flags byze ZF1 */
#define ZF1_CANVHDR  0x01 /* Variable headers OK, unused in lrzsz */
#define ZF1_TIMESYNC 0x02 /* nonstandard, Receiver request timesync */

/* Parameters for ZSINIT frame */
#define ZATTNLEN 32 /* Max length of attention string */

/* Bit Masks for ZSINIT flags byte ZF0 */
#define TESCCTL 0100 /* Transmitter expects ctl chars to be escaped */
#define TESC8   0200 /* Transmitter expects 8th bit to be escaped */

/* Parameters for ZFILE frame */
/* Conversion options one of these in ZF0 */
#define ZCBIN   1 /* Binary transfer - inhibit conversion */
#define ZCNL    2 /* Convert NL to local end of line convention */
#define ZCRESUM 3 /* Resume interrupted file transfer */

/* Management include options, one of these ored in ZF1 */
#define ZF1_ZMSKNOLOC 0x80 /* Skip file if not present at rx */

/* Management options, one of these ored in ZF1 */
#define ZF1_ZMMASK 0x1f /* Mask for the choices below */
#define ZF1_ZMNEWL 1    /* Transfer if source newer or longer */
#define ZF1_ZMCRC  2    /* Transfer if different file CRC or length */
#define ZF1_ZMAPND 3    /* Append contents to existing file (if any) */
#define ZF1_ZMCLOB 4    /* Replace existing file */
#define ZF1_ZMNEW  5    /* Transfer if source newer */
/* Number 5 is alive ... */
#define ZF1_ZMDIFF 6 /* Transfer if dates or lengths different */
#define ZF1_ZMPROT 7 /* Protect destination file */
#define ZF1_ZMCHNG 8 /* Change filename if destination exists */

/* Transport options, one of these in ZF2 */
#define ZTLZW   1 /* Lempel-Ziv compression */
#define ZTCRYPT 2 /* Encryption */
#define ZTRLE   3 /* Run Length encoding */

/* Extended options for ZF3, bit encoded */
#define ZXSPARS 64 /* Encoding for sparse file operations */

/* Parameters for ZCOMMAND frame ZF0 (otherwise 0) */
#define ZCACK1 1 /* Acknowledge, then do command */

/*--------------- ASCII ---------------*/
#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ENQ  0x05
#define ACK  0x06
#define LF   0x0a
#define CR   0x0d
#define XON  0x11
#define XOFF 0x13
#define NAK  0x15
#define CAN_ 0x18

#if 0
/*--------------- ZMODEM ---------------*/
#define ZPAD  0x2a /* pad character; begins frames */
#define ZDLE  0x18 /* ctrl-x zmodem escape */
#define ZDLEE 0x58 /* escaped ZDLE */

#define ZBIN    0x41 /* binary frame indicator (wd_toolkit_crc16) */
#define ZHEX    0x42 /* hex frame indicator */
#define ZBIN32  0x43 /* binary frame indicator (wd_toolkit_crc32) */
#define ZBINR32 0x44 /* run length encoded binary frame (wd_toolkit_crc32) */

#define ZVBIN    0x61 /* binary frame indicator (wd_toolkit_crc16) */
#define ZVHEX    0x62 /* hex frame indicator */
#define ZVBIN32  0x63 /* binary frame indicator (wd_toolkit_crc32) */
#define ZVBINR32 0x64 /* run length encoded binary frame (wd_toolkit_crc32) */

#define ZRESC      0x7e /* run length encoding flag / escape character */

/*--------------- FRAME TYPES ---------------*/
#define ZRQINIT    0x00 /* request receive init (s->r) */
#define ZRINIT     0x01 /* receive init (r->s) */
#define ZSINIT     0x02 /* send init sequence (optional) (s->r) */
#define ZACK       0x03 /* ack to ZRQINIT ZRINIT or ZSINIT (s<->r) */
#define ZFILE      0x04 /* file name (s->r) */
#define ZSKIP      0x05 /* skip this file (r->s) */
#define ZNAK       0x06 /* last packet was corrupted (?) */
#define ZABORT     0x07 /* abort batch transfers (?) */
#define ZFIN       0x08 /* finish session (s<->r) */
#define ZRPOS      0x09 /* resume data transmission here (r->s) */
#define ZDATA      0x0a /* data packet(s) follow (s->r) */
#define ZEOF       0x0b /* end of file reached (s->r) */
#define ZFERR      0x0c /* fatal read or write error detected (?) */
#define ZCRC       0x0d /* request for file CRC and response (?) */
#define ZCHALLENGE 0x0e /* security challenge (r->s) */
#define ZCOMPL     0x0f /* request is complete (?) */
#define ZCAN       0x10 /* pseudo frame; other end cancelled session with 5* CAN */
#define ZFREECNT   0x11 /* request free bytes on file system (s->r) */
#define ZCOMMAND   0x12 /* issue command (s->r) */
#define ZSTDERR    0x13 /* output data to stderr (??) */

/*--------------- ZDLE SEQUENCES ---------------*/
#define ZCRCE      0x68 /* CRC next, frame ends, header packet follows */
#define ZCRCG      0x69 /* CRC next, frame continues nonstop */
#define ZCRCQ      0x6a /* CRC next, frame continuous, ZACK expected */
#define ZCRCW      0x6b /* CRC next, ZACK expected, end of frame */
#define ZRUB0      0x6c /* translate to rubout 0x7f */
#define ZRUB1      0x6d /* translate to rubout 0xff */

/*--------------- RECEIVER CAPABILITY FLAGS ---------------*/
#define CANFDX     0x01 /* Rx can send and receive true full duplex */
#define CANOVIO    0x02 /* Rx can receive data during disk I/O */
#define CANBRK     0x04 /* Rx can send a break signal */
#define CANCRY     0x08 /* Receiver can decrypt */
#define CANLZW     0x10 /* Receiver can uncompress */
#define CANFC32    0x20 /* Receiver can use 32 bit Frame Check */
#define ESCCTL     0x40 /* Receiver expects ctl chars to be escaped */
#define ESC8       0x80 /* Receiver expects 8th bit to be escaped */
#endif

static unsigned short crctab[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b,
    0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
    0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738,
    0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96,
    0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
    0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb,
    0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2,
    0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
    0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827,
    0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d,
    0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
    0x2e93, 0x3eb2, 0x0ed1, 0x1ef0};

/*
 * updcrc macro derived from article Copyright (C) 1986 Stephen Satchell.
 *  NOTE: First srgument must be in range 0 to 255.
 *        Second argument is referenced twice.
 *
 * Programmers may incorporate any or all code into their programs,
 * giving proper credit within the source. Publication of the
 * source routines is permitted so long as proper credit is given
 * to Stephen Satchell, Satchell Evaluations and Chuck Forsberg,
 * Omen Technology.
 */

#define updcrc(cp, crc) (uint16_t)(crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp) // CRC-16/XMODEM

#if 0
/*
 * Copyright (C) 1986 Gary S. Brown.  You may use this program, or
 * code or tables extracted from it, as desired without restriction.
 */

/* First, the polynomial itself and its table of feedback terms.  The  */
/* polynomial is                                                       */
/* X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0 */
/* Note that we take it "backwards" and put the highest-order term in  */
/* the lowest-order bit.  The X^32 term is "implied"; the LSB is the   */
/* X^31 term, etc.  The X^0 term (usually shown as "+1") results in    */
/* the MSB being 1.                                                    */

/* Note that the usual hardware shift register implementation, which   */
/* is what we're using (we're merely optimizing it by doing eight-bit  */
/* chunks at a time) shifts bits into the lowest-order term.  In our   */
/* implementation, that means shifting towards the right.  Why do we   */
/* do it this way?  Because the calculated CRC must be transmitted in  */
/* order from highest-order term to lowest-order term.  UARTs transmit */
/* characters in order from LSB to MSB.  By storing the CRC this way,  */
/* we hand it to the UART in the order low-byte to high-byte; the UART */
/* sends each low-bit to hight-bit; and the result is transmission bit */
/* by bit from highest- to lowest-order term without requiring any bit */
/* shuffling on our part.  Reception works similarly.                  */

/* The feedback terms table consists of 256, 32-bit entries.  Notes:   */
/*                                                                     */
/*     The table can be generated at runtime if desired; code to do so */
/*     is shown later.  It might not be obvious, but the feedback      */
/*     terms simply represent the results of eight shift/xor opera-    */
/*     tions for all combinations of data and CRC register values.     */
/*                                                                     */
/*     The values must be right-shifted by eight bits by the "updcrc"  */
/*     logic; the shift must be unsigned (bring in zeroes).  On some   */
/*     hardware you could probably optimize the shift in assembler by  */
/*     using byte-swap instructions.                                   */

static long cr3tab[] = { /* CRC polynomial 0xedb88320 */
0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

#define UPDC32(b, c) (cr3tab[((int)c ^ b) & 0xff] ^ ((c >> 8) & 0x00FFFFFF)) // CRC-32/XMODEM
#endif

static const wd_factory_protocol_z_modem_capabilities_t DEFAULT_PROTOCOL_Z_MODEM_CAPABILITIES = {
    .Check_Sum = 0};

#undef LOCAL_PREFIX
#define LOCAL_PREFIX_NAME  wd_drv_protocol_z_modem
#define LOCAL_PREFIX(name) drv_protocol_z_modem_##name

#undef LOCAL_DRV_TYPEDEF
#define LOCAL_DRV_TYPEDEF wd_factory_protocol_z_modem_t
#define LOCAL_DRV_CASTING(name, port) \
    LOCAL_DRV_TYPEDEF *name =         \
        &(((wd_net_protocol_resources_t *)((port)->Protocol))->Factory->Z_Modem)

#undef LOCAL_PROFILE_TYPEDEF
#define LOCAL_PROFILE_TYPEDEF wd_net_protocol_profile_t
#define LOCAL_PROFILE_CASTING(name, port) \
    LOCAL_PROFILE_TYPEDEF *name = ((wd_net_protocol_resources_t *)((port)->Protocol))->Profile

static int32_t LOCAL_PREFIX(Initialize)(wd_network_port_t *port, uint32_t arg) {
    LOCAL_DRV_CASTING(z_modem, port);

    UNUSED(arg);

    memset(z_modem, 0, sizeof(wd_factory_protocol_z_modem_t));

    z_modem->Capabilities = DEFAULT_PROTOCOL_Z_MODEM_CAPABILITIES;

    z_modem->State = WD_PROTOCOL_Z_MODEM_STATE_NONE;

    return (WD_CODE_OK);
}
static int32_t LOCAL_PREFIX(Process)(wd_network_port_t *port, uint32_t arg) {
    UNUSED(port);
    UNUSED(arg);
    return (WD_CODE_OK);
}

static int32_t LOCAL_PREFIX(Make_Frame)(wd_network_port_t *port, uint8_t *ptr, uint32_t num,
                                        wd_message_t *message, uint32_t arg) {
    message->ID = port->ID;
    message->Tyoe = 0;
    message->Priority = 0;
    message->Ptr = ptr;
    message->Size = (uint16_t)num;
    if (osMessageQueuePut(port->Info->Msg_Queue, message, 0, RTE_RTOS_MESSAGEQUEUE_TIMEOUT)) {
        WD_PRINT_ERROR(WD_MSG_ERROR_CRLF, "Z_Modem Tx Queue Error");
        osMessageQueueReset(port->Info->Msg_Queue);
    }

    WD_PRINT_DEBUG(WD_MSG_FRAME_DUMP_LOG, "<Z_Modem> %s", port->Capabilities->Info, WD_MSG_MAGIC,
                   WD_NET_TX_ASCII, message->Ptr, message->Size);
    wd_time_wait_us(1); // DUMP_LOG 시 깨짐 발생_이건 USB 콘솔문제인거 같음.

    UNUSED(arg);

    return (WD_CODE_OK);
}

static int32_t LOCAL_PREFIX(Parser)(wd_network_port_t *port, uint32_t head, uint32_t tail,
                                    uint32_t arg) {
    LOCAL_DRV_CASTING(z_modem, port);
    int32_t frame_size = (uint16_t)((tail >= head) ? tail - head : 0);
    int32_t response = WD_CODE_IDLE;
    uint8_t *ptr = (uint8_t *)head;
    int32_t parsed_size = 0;
    uint8_t request_frame_types = 0xFF;
    uint8_t response_frame_types = 0xFF;
    uint8_t request_header[4];

    UNUSED(frame_size);
    UNUSED(arg);

    /*
   * [ZMODEM Frame]
   * HEX Frame                - <frame header> [CR] LF [XON]
   * Binary Frame             - <frame header> [[<* data subpacket> [...]] <ZCRCW | ZCRCE data subpacket>]
   *
   * [Headers]
   *        M         L
   * FTYPE  F3 F2 F1 F0   (flags frame)
   *
   *        L         M
   * FTYPE  P0 P1 P2 P3   (numeric frame)
   *
   * [Frame Header]
   * <frame encoding> <frame type> <frame info> <checksum>
   *
   * 16 BIT CRC BINARY HEADER - *  ZDLE A TYPE F3/P0 F2/P1 F1/P2 F0/P3 CRC-1 CRC-2
   * 32 BIT CRC BINARY HEADER - *  ZDLE C TYPE F3/P0 F2/P1 F1/P2 F0/P3 CRC-1 CRC-2 CRC-3 CRC-4
   * HEX HEADER               - ** ZDLE B TYPE F3/P0 F2/P1 F1/P2 F0/P3 CRC-1 CRC-2 CR LF XON
   *
   * [Example]
   *  -> rz<CR> - initiate a file transfer
   *  -> <PAD><PAD><ZDLE> 42 30 30 30 30 30 30 30  30 30 30 30 30 30 30 <CR> <LF> <XON> - initialization request packet ( ZRQINIT (<ZDLE>+ASCII B))
   * <-  <PAD><PAD><ZDLE> 42 30 31 30 30 30 30 30 30 32 33 62 65 35 30 <CR><LF> - initialization response packet (ZRINIT)
   *  -> <PAD><ZDLE> 41 04 00 00 00 01 99 <FILENAME><FILE LENGTH> <MODIFICATION DATE><OTHER ATTRIBUTES><XON> - ZFILE packet
   * <-  <PAD><PAD><ZDLE> 42 30 39 30 30 30 30 30 30 30 30 61 38 37 63 <CR><LF> - ZRPOS packet
   *  -> <PAD><ZDLE> 41 0A 00 00 00 00 46 AE <FILE DATA> ... <ZDLE> 68 CB 3A 2A - ZDATA packets + ZCRCE escape sequence
   *  -> <ZDLE> 41 0B 36 00 00 00 E7 8F - end-of-file (ZEOF)
   * <-  <PAD><PAD><ZDLE> 42 30 31 30 30 30 30 30 30 32 33 62 65 35 30 <CR><LF> - ZRINIT packet to indicate that it is ready to receive the next transfer file
   *  -> <PAD><PAD><ZDLE> 42 30 38 30 30 30 30 30 30 30 30 30 32 32 64 <CR><LF> - ZFIN packet
   * <-  <PAD><PAD><ZDLE> 42 30 38 30 30 30 30 30 30 30 30 30 32 32 64 <CR><LF> - ZFIN packet
   *  -> 4F 4F - ASCII O x 2 terminates the session
   *
   *  0x1868(ZDLE + ASCII h)  ZCRCE : CRC 다음, 패킷 끝, 헤더 패킷이 이어집니다.
   *  0x1869(ZDLE + ASCII I)  ZCRCG: CRC 다음, 패킷이 논스톱으로 계속됩니다.
   *  0x186A(ZDLE + ASCII j)  ZCRCQ: CRC 다음, 패킷 계속, ZACK이 예상됩니다.
   *  0x186B(ZDLE + ASCII k)  ZCRCW: CRC 다음, ZACK 예상, 패킷 끝.
   *  0x186C(ZDLE + ASCII l)  ZRUB0: rubout 0177로 번역됩니다.
   *  0x186D(ZDLE + ASCII m)  ZRUB1: 루아웃 0377로 번역됩니다.
   *
   */


    if ((memcmp(ptr, "rz\r", strlen("rz\r")) == 0) // initiate a file transfer
        && (frame_size == strlen("rz\r"))) {
        WD_PRINT_INFO(WD_MSG_INFO_LOG, "[Z-MODEM - Initiate File Transfer]");
        return (frame_size);
    }

    /*--------------- Frame Header ---------------*/
    switch (*ptr++) {
    case ZPAD | 0200:
    case ZPAD:
        break;

    case CAN_: // Cancle
        return (0);

    default:
        return (0);
    }

splat:
    switch (*ptr++) {
    case ZPAD:
        goto splat;

    case ZDLE: // ZMODEM Data-link-escape character (^X, ASCII 24).
        break;

    default:
        return (0);
    }

    switch (*ptr++) {
    case ZBIN: {
        uint32_t i;
        uint16_t crc;

        /*--------------- FRAME TYPES ---------------*/
        crc = updcrc(*ptr, 0);
        request_frame_types = *ptr++;

        /*--------------- Headers ---------------*/
        for (i = 0; i < sizeof(request_header); i++) {
            crc = updcrc(*ptr, crc);
            request_header[i] = *ptr++;
        }

        /*--------------- wd_toolkit_crc16 ---------------*/
        crc = updcrc(*ptr++, crc);
        crc = updcrc(*ptr++, crc);

        if (crc & 0xFFFF) {
            return (0); // CRC Error
        }
    } break;

    case ZBIN32:
        WD_LOG_ERROR("Unsupport ZBIN32");
        return (0);

    case ZHEX: {
        uint32_t i;
        uint8_t c;
        uint16_t crc;

        /*--------------- FRAME TYPES ---------------*/
        c = wd_toolkit_char_to_digit(*ptr++) * 10;
        c += wd_toolkit_char_to_digit(*ptr++);
        request_frame_types = c;
        crc = updcrc(c, 0);

        /*--------------- Headers ---------------*/
        for (i = 0; i < sizeof(request_header); i++) {
            c = wd_toolkit_char_to_digit(*ptr++) * 10;
            c += wd_toolkit_char_to_digit(*ptr++);
            crc = updcrc(c, crc);
            request_header[i] = c;
        }

        /*--------------- wd_toolkit_crc16 ---------------*/
        c = wd_toolkit_char_to_digit(*ptr++) * 10;
        c += wd_toolkit_char_to_digit(*ptr++);
        crc = updcrc(c, crc);

        c = wd_toolkit_char_to_digit(*ptr++) * 10;
        c += wd_toolkit_char_to_digit(*ptr++);
        crc = updcrc(c, crc);

        if (crc & 0xFFFF) {
            return (0); // CRC Error
        }

    flow_control:
        if ((uint32_t)ptr < tail) {
            switch (*ptr++) {
            case LF:
            case LF | 0200:
            case CR:
            case CR | 0200:
            case XON:
                goto flow_control;
            default:
                break;
            }
        }

        request_frame_types = ZRQINIT;
    } break;

    case CAN_: // Cancle
        return (0);

    default:
        return (0);
    }

    switch (request_frame_types) {
    case ZRQINIT: /* 0   Request receive init */
        memcpy(z_modem->Request_Header, request_header, sizeof(z_modem->Request_Header));
        parsed_size = (int32_t)ptr - (int32_t)head;

        response_frame_types = ZRINIT;
        break;

#if (RTE_SERVICE_FIRMWARE)
    case ZFILE: /* 4   File name from sender */
    {
        uint32_t len;
        uint32_t i;
        static uint16_t crc = 0;
        char *pos;

        len = strlen((char *)ptr);

        printf("%d", crc);

        memcpy(wd_fw_fota_info.File_Info.File_Name, ptr,
               (sizeof(wd_fw_fota_info.File_Info.File_Name) <= len)
                   ? sizeof(wd_fw_fota_info.File_Info.File_Name) - 1
                   : len);
        for (i = 0; i < len + 1; i++) {
            crc = updcrc((0377 & ptr[i]), crc);
        }
        ptr += (len + 1);

        if (sscanf((char *)ptr, "%" PRIu32 "", &wd_fw_fota_info.File_Info.File_Length) != 1) {
            return (0);
        }

        pos = strchr((char *)ptr, ' ');
        if ((uint32_t)pos <= (uint32_t)ptr) {
            return (0);
        }

        len = (uint32_t)pos - (uint32_t)ptr;
        for (i = 0; i < len + 1; i++) {
            crc = updcrc((0377 & ptr[i]), crc);
        }
        ptr = (uint8_t *)pos + 1;

        if (sscanf((char *)ptr, "%" PRIu32 "", &wd_fw_fota_info.File_Info.Modification_Date) != 1) {
            return (0);
        }
        pos = strchr((char *)ptr, ' ');
        if ((uint32_t)pos <= (uint32_t)ptr) {
            return (0);
        }
        len = (uint32_t)pos - (uint32_t)ptr;
        for (i = 0; i < len + 1; i++) {
            crc = updcrc((0377 & ptr[i]), crc);
        }
        ptr = (uint8_t *)pos + 1;

        for (i = 0; i < 7; i++) {
            crc = updcrc((0377 & ptr[i]), crc);
        }
        ptr += 7; // skip FILE MODE (100644\0 8진수)

        if (*ptr++ != ZDLE) {
            return (0);
        }

        if (*ptr != ZCRCW) {
            return (0);
        }
        crc = updcrc((0377 & (*ptr++)), crc);

        crc = updcrc((0377 & (*ptr++)), crc);
        crc = updcrc((0377 & (*ptr++)), crc);

        if (crc & 0xFFFF) {
            return (0); // CRC Error
        }
    }

        memcpy(z_modem->Request_Header, request_header, sizeof(z_modem->Request_Header));
        parsed_size = (int32_t)ptr - (int32_t)head;

        response_frame_types = ZRPOS;
        break;
#endif


        //    case ZRINIT     :                                                           /* 1   Receive init */
    case ZSINIT: /* 2   Send init sequence (optional) */
    case ZACK:   /* 3   ACK to above */
    case ZSKIP:  /* 5   To sender: skip this file */
    case ZNAK:   /* 6   Last packet was garbled */
    case ZABORT: /* 7   Abort batch transfers */
    case ZFIN:   /* 8   Finish session */
        //    case ZRPOS      :                                                           /* 9   Resume data trans at this position */
    case ZDATA:      /* 10  Data packet(s) follow */
    case ZEOF:       /* 11  End of file */
    case ZFERR:      /* 12  Fatal Read or Write error Detected */
    case ZCRC:       /* 13  Request for file CRC and response */
    case ZCHALLENGE: /* 14  Receiver's Challenge */
    case ZCOMPL:     /* 15  Request is complete */
    case ZCAN:       /* 16  Other end canned session with CAN*5 */
    case ZFREECNT:   /* 17  Request for free bytes on filesystem */
    case ZCOMMAND:   /* 18  Command from sending program */
    case ZSTDERR:    /* 19  Output to standard error, data follows */
        break;
    default:
        break;
    }

    switch (response_frame_types) {
    case ZRINIT: /* 1   Receive init */
    {
        response = WD_PROTOCOL_Z_MODEM_CMD_SEND_HEX_HEADER;
        memset(z_modem->Response_Header, 0, sizeof(z_modem->Response_Header));
        WD_PRINT_INFO(WD_MSG_INFO_LOG,
                      "[Z-MODEM - Initialization Request Packet] Header %02X%02X%02X%02X",
                      z_modem->Request_Header[0], z_modem->Request_Header[1],
                      z_modem->Request_Header[2], z_modem->Request_Header[3]);

#if (RTE_SERVICE_FIRMWARE) && (RTE_SERVICE_MMU)
        {
            uint32_t debug_time;

            debug_time = wd_time_get_us();
            wd_mmu_access(MAP->NVM->Firmware_Download, wd_core_scratch_buffer(),
                          sizeof(MAP->NVM->Firmware_Download), WD_MMU_COMMAND_INIT_0xFF); // Max 10s
            memset(&wd_fw_fota_info, 0, sizeof(wd_fw_fota_info));
            WD_PRINT_INFO(WD_MSG_INFO_LOG, "Firmware Download Initialization Time : %u [us]",
                          wd_time_calc_us(&debug_time));
        }
#endif
    } break;

    case ZRPOS: /* 9   Resume data trans at this position */
    {
        response = WD_PROTOCOL_Z_MODEM_CMD_SEND_HEX_HEADER;
        memset(z_modem->Response_Header, 0, sizeof(z_modem->Response_Header));

        WD_PRINT_INFO(WD_MSG_INFO_LOG, "[Z-MODEM - Reposition the send offset]");
    } break;
    default:
        break;
    }

    if (response == WD_PROTOCOL_Z_MODEM_CMD_SEND_HEX_HEADER) {
        uint32_t i;
        char hex_digits[] = "0123456789abcdef";
        uint16_t crc;
        uint16_t idx = 0;

        /*--------------- ZMODEM HEX header ---------------*/
        z_modem->APDU[idx++] = ZPAD;
        z_modem->APDU[idx++] = ZPAD;
        z_modem->APDU[idx++] = ZDLE;
        z_modem->APDU[idx++] = ZHEX;

        z_modem->APDU[idx++] = hex_digits[(response_frame_types & 0xF0) >> 4];
        z_modem->APDU[idx++] = hex_digits[response_frame_types & 0x0F];

        crc = updcrc(response_frame_types, 0);

        for (i = 0; i < sizeof(z_modem->Response_Header); i++) {
            z_modem->APDU[idx++] = hex_digits[(z_modem->Response_Header[i] & 0xF0) >> 4];
            z_modem->APDU[idx++] = hex_digits[z_modem->Response_Header[i] & 0x0F];
            crc = updcrc((0377 & z_modem->Response_Header[i]), crc);
        }

        crc = updcrc(0, updcrc(0, crc));

        z_modem->APDU[idx++] = hex_digits[((crc >> 8) & 0xF0) >> 4];
        z_modem->APDU[idx++] = hex_digits[(crc >> 8) & 0x0F];
        z_modem->APDU[idx++] = hex_digits[(crc & 0xF0) >> 4];
        z_modem->APDU[idx++] = hex_digits[crc & 0x0F];

        z_modem->APDU[idx++] = CR;
        z_modem->APDU[idx++] = LF | 0200;

        if (response_frame_types != ZFIN && response_frame_types != ZACK) {
            z_modem->APDU[idx++] = XON;
        }

        z_modem->APDU_Len = idx;
        wd_drv_protocol_z_modem.Make_Frame(port, z_modem->APDU, z_modem->APDU_Len, &z_modem->Tx_Msg,
                                           0);
    } else if (response == WD_CODE_ACK) {
        z_modem->APDU[sizeof(z_modem->APDU) - 1] = 0;
        z_modem->APDU_Len =
            (uint16_t)snprintf((char *)z_modem->APDU, sizeof(z_modem->APDU) - 1, "ST,ACK,ED\r\n");
        wd_drv_protocol_z_modem.Make_Frame(port, z_modem->APDU, z_modem->APDU_Len, &z_modem->Tx_Msg,
                                           0);
    } else if (response == WD_CODE_NACK) {
        z_modem->APDU[sizeof(z_modem->APDU) - 1] = 0;
        z_modem->APDU_Len =
            (uint16_t)snprintf((char *)z_modem->APDU, sizeof(z_modem->APDU) - 1, "ST,NACK,ED\r\n");
        wd_drv_protocol_z_modem.Make_Frame(port, z_modem->APDU, z_modem->APDU_Len, &z_modem->Tx_Msg,
                                           0);
    }

    return (parsed_size);
}

static int32_t LOCAL_PREFIX(Control)(wd_network_port_t *port, uint32_t arg) {
    UNUSED(port);
    UNUSED(arg);
    return (WD_CODE_OK);
}

wd_factory_protocol_driver_z_modem_t LOCAL_PREFIX_NAME = {.Initialize = LOCAL_PREFIX(Initialize),
                                                          .Process = LOCAL_PREFIX(Process),
                                                          .Make_Frame = LOCAL_PREFIX(Make_Frame),
                                                          .Parser = LOCAL_PREFIX(Parser),
                                                          .Control = LOCAL_PREFIX(Control)};


/***************************************************************************************************************************************************************
 **********  Y-Modem Receive (UART Console OTA)
 **************************************************************************************************************************************************************/
#if (RTE_SERVICE_FIRMWARE) && (RTE_SERVICE_MMU)

#define YMODEM_SOH_PAYLOAD_SIZE      128
#define YMODEM_STX_PAYLOAD_SIZE      1024
#define YMODEM_PACKET_OVERHEAD       5 /* SOH/STX(1) + SeqNo(1) + ~SeqNo(1) + wd_toolkit_crc16(2) */
#define YMODEM_RX_TIMEOUT_MS         3000
#define YMODEM_HANDSHAKE_TIMEOUT_MS  60000 /* 핸드셰이크 'C' 반복 전송 최대 대기 (60초) */
#define YMODEM_HANDSHAKE_INTERVAL_MS 1000  /* 'C' 전송 간격 (1초) */
#define YMODEM_MAX_ERRORS            10

/* Y-modem 수신 상태 */
typedef enum {
    YMODEM_OK = 0,
    YMODEM_EOT = 256, /* EOT 수신 (seq 0~255와 겹치지 않는 값) */
    YMODEM_ERR_TIMEOUT = -1,
    YMODEM_ERR_CRC = -2,
    YMODEM_ERR_SEQ = -3,
    YMODEM_ERR_ABORT = -4,
    YMODEM_ERR_OVERFLOW = -5,
    YMODEM_ERR_VERIFY = -6,
} YMODEM_STATUS;

/* 표준 CRC-16/XMODEM 계산 - Y-modem 프로토콜용
 * updcrc 매크로(Z-modem 전용)와 다름: 룩업 인덱스에 byte를 XOR */
static uint16_t ymodem_crc16(const uint8_t *buf, uint32_t len) {
    uint16_t crc = 0;
    uint32_t i;
    for (i = 0; i < len; i++) {
        crc = (uint16_t)((crc << 8) ^ crctab[((crc >> 8) ^ buf[i]) & 0xFF]);
    }
    return crc;
}

/* SDRAM에 펌웨어 이미지를 수신 (NVM write 없이 고속 수신) */
#define YMODEM_SDRAM_BUF_SIZE (512 * 1024)
static uint8_t *ymodem_sdram_buf = (uint8_t *)0xD0000000;

/*=== DMA 순환 버퍼 기반 Y-modem 수신 ===
 * DMA2가 UART8 RX 데이터를 RAM_D2에 자동 기록
 * CPU는 NDTR로 DMA write 위치를 추적, D-Cache 무효화 후 데이터 읽기
 * (DMA2는 D2 도메인만 접근 가능 → RAM_D2 사용) */

#define YMODEM_DMA_BUF_SIZE 4096
static uint8_t ymodem_dma_buf[YMODEM_DMA_BUF_SIZE] __attribute__((section(".RAM_D2"), aligned(32)));
static volatile uint32_t ymodem_dma_rd; /* 소프트웨어 read index */

/* DMA 순환 버퍼에서 사용 가능한 바이트 수 */
static uint32_t ymodem_dma_available(void) {
    uint32_t wr = YMODEM_DMA_BUF_SIZE -
                  __HAL_DMA_GET_COUNTER((RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->hdmarx);
    if (wr >= ymodem_dma_rd) {
        return wr - ymodem_dma_rd;
    } else {
        return YMODEM_DMA_BUF_SIZE - ymodem_dma_rd + wr;
    }
}

/* DMA 순환 버퍼에서 1바이트 읽기 (D-Cache 무효화 포함) */
static int32_t ymodem_getc(uint32_t timeout_ms) {
    uint32_t start = wd_time_get_ms();
    uint8_t c;

    while (wd_time_calc_ms(&start) < timeout_ms) {
        if (ymodem_dma_available() > 0) {
            /* DMA가 쓴 데이터를 CPU가 읽기 전에 해당 캐시 라인 무효화 */
            SCB_InvalidateDCache_by_Addr(
                (uint32_t *)((uint32_t)&ymodem_dma_buf[ymodem_dma_rd] & ~31U), 32);
            c = ymodem_dma_buf[ymodem_dma_rd];
            ymodem_dma_rd = (ymodem_dma_rd + 1) % YMODEM_DMA_BUF_SIZE;
            return (int32_t)c;
        }
    }

    return -1; /* timeout */
}

/* DMA 순환 버퍼에서 len바이트 벌크 읽기 (D-Cache 무효화 포함) */
static int32_t ymodem_receive_bulk(uint8_t *buf, uint32_t len, uint32_t timeout_ms) {
    uint32_t i;
    uint32_t start = wd_time_get_ms();

    for (i = 0; i < len; i++) {
        while (ymodem_dma_available() == 0) {
            if (wd_time_calc_ms(&start) >= timeout_ms) {
                WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM-DBG] bulk timeout: got=%u/%u",
                              (unsigned int)i, (unsigned int)len);
                return -1;
            }
        }
        /* 해당 캐시 라인 무효화 후 읽기 */
        SCB_InvalidateDCache_by_Addr((uint32_t *)((uint32_t)&ymodem_dma_buf[ymodem_dma_rd] & ~31U),
                                     32);
        buf[i] = ymodem_dma_buf[ymodem_dma_rd];
        ymodem_dma_rd = (ymodem_dma_rd + 1) % YMODEM_DMA_BUF_SIZE;
    }

    return (int32_t)len;
}

/* Y-modem 바이트 송신 - UART8 레지스터 직접 (HAL Lock 없이) */
static void ymodem_putc(uint8_t c) {
    USART_TypeDef *uart = UART8;
    while (!(uart->ISR & USART_ISR_TXE_TXFNF))
        ; /* TXE 대기 */
    uart->TDR = c;
}

/* Y-modem RX 플러시 - DMA read index를 write 위치로 이동 */
static void ymodem_flush_rx(void) {
    ymodem_dma_rd = YMODEM_DMA_BUF_SIZE -
                    __HAL_DMA_GET_COUNTER((RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->hdmarx);
}

static void ymodem_send_cancel(void) {
    uint32_t i;
    for (i = 0; i < 5; i++) {
        ymodem_putc(CAN_);
    }
    for (i = 0; i < 5; i++) {
        ymodem_putc(0x08); /* BS */
    }
}

/* 패킷 1개 수신 (SOH/STX 이후 데이터+CRC)
 * 리턴: payload 크기 (128 또는 1024), 0=EOT, 음수=에러 */
static int32_t ymodem_receive_packet(uint8_t *buf, uint32_t *payload_size) {
    int32_t ch;
    uint32_t pkt_size;
    uint32_t remain;
    uint16_t crc_calc;
    uint16_t crc_recv;
    /* SOH/STX 이후 전체를 벌크 수신할 임시 버퍼: seq(1)+~seq(1)+payload(max1024)+CRC(2)=1028 */
    static uint8_t raw[2 + YMODEM_STX_PAYLOAD_SIZE + 2] __attribute__((section(".RAM_D2")));

    ch = ymodem_getc(YMODEM_RX_TIMEOUT_MS);
    if (ch < 0) {
        WD_PRINT_INFO(WD_MSG_INFO_LOG,
                      "[YMODEM-DBG] pkt getc timeout: HAL=%u ring=%u ISR=0x%08X\r\n",
                      (unsigned int)(RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->RxState,
                      (unsigned int)wd_print_hal_uart8_size(),
                      (unsigned int)(RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->Instance->ISR);
        return YMODEM_ERR_TIMEOUT;
    }

    switch (ch) {
    case SOH:
        pkt_size = YMODEM_SOH_PAYLOAD_SIZE;
        break;
    case STX:
        pkt_size = YMODEM_STX_PAYLOAD_SIZE;
        break;
    case EOT:
        WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM-DBG] EOT received\r\n");
        return YMODEM_EOT; /* 전송 완료 (seq 0과 구분) */
    case CAN_:
        ch = ymodem_getc(YMODEM_RX_TIMEOUT_MS);
        if (ch == CAN_) {
            return YMODEM_ERR_ABORT;
        }
        return YMODEM_ERR_ABORT;
    default:
        WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM-DBG] unexpected byte=0x%02X\r\n",
                      (unsigned int)(ch & 0xFF));
        return YMODEM_ERR_TIMEOUT;
    }

    /* SOH/STX 직후 → 나머지 전체를 한번에 벌크 수신 (중간에 로그/함수호출 없음) */
    remain = 2 + pkt_size + 2; /* seq + ~seq + payload + wd_toolkit_crc16 */
    if (ymodem_receive_bulk(raw, remain, YMODEM_RX_TIMEOUT_MS) < 0) {
        return YMODEM_ERR_TIMEOUT;
    }

    /* 시퀀스 번호 검증 */
    if ((raw[0] ^ raw[1]) != 0xFF) {
        return YMODEM_ERR_SEQ;
    }

    /* CRC-16 검증 */
    crc_recv = (uint16_t)(raw[2 + pkt_size] << 8) | raw[2 + pkt_size + 1];
    crc_calc = ymodem_crc16(&raw[2], pkt_size);
    if (crc_calc != crc_recv) {
        return YMODEM_ERR_CRC;
    }

    /* 페이로드 복사 */
    memcpy(buf, &raw[2], pkt_size);
    *payload_size = pkt_size;

    return (int32_t)raw[0]; /* seq 번호 리턴 */
}

/**
 * @brief  Y-modem 프로토콜로 펌웨어 이미지를 UART Console에서 수신하여 NVM에 저장
 * @retval YMODEM_OK: 성공, 음수: 에러 코드
 */
int32_t wd_factory_receive_ymodem_firmware(void) {
    static uint8_t pkt_buf[YMODEM_STX_PAYLOAD_SIZE]; /* static: 스택 절약 */
    uint32_t payload_size = 0;
    int32_t ret;
    uint32_t file_size = 0;
    uint32_t total_written = 0;
    uint8_t expected_seq = 0;
    uint32_t errors = 0;
    uint32_t session_done = 0;
    uint32_t file_done = 0;
    uint32_t debug_time;

    WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM] Firmware OTA Update via UART8 (Y-Modem)");
    WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM] UART8 (TX=PJ8, RX=PJ9), CRC-16, 921600 baud");

    /*--- UART8을 921600 baud + DMA 순환 수신 모드로 전환 ---*/
    HAL_UART_AbortReceive(RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE); /* HAL IT 수신 중지 */
    (RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->Instance->CR1 &= ~USART_CR1_UE;
    (RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->Instance->BRR =
        130; /* 921600 baud (120MHz/130≈923077) */
    (RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->Instance->CR1 |= USART_CR1_UE;
    /* 에러 플래그 클리어 */
    (RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->Instance->ICR =
        USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_NECF | USART_ICR_PECF;

    /* DMA 순환 수신 시작 (RAM_D2 버퍼 + D-Cache 무효화) */
    ymodem_dma_rd = 0;
    memset(ymodem_dma_buf, 0, YMODEM_DMA_BUF_SIZE);
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)ymodem_dma_buf, YMODEM_DMA_BUF_SIZE);
    HAL_UART_Receive_DMA(RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE, ymodem_dma_buf,
                         YMODEM_DMA_BUF_SIZE);
    /* DMA IRQ 비활성화 — NDTR 폴링만 사용하므로 콜백 불필요
   * (콜백이 HAL_UART_Receive_IT를 호출하면 DMA 모드와 충돌) */
    NVIC_DisableIRQ(DMA2_Stream7_IRQn);
    WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM-DBG] UART8 DMA circular mode, BRR=%u (921600 baud)",
                  (unsigned int)(RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->Instance->BRR);

    memset(&wd_fw_fota_info, 0, sizeof(wd_fw_fota_info));
    wd_fw_fota_info.Image_Transfer_Status = WDIF_FIRMWARE_FOTA_IMAGE_TRANSFER_INITIATED;

    WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM] Ready. Start Y-modem send now...");

    while (!session_done) {
        file_done = 0;
        expected_seq = 0;
        total_written = 0;
        file_size = 0;
        errors = 0;

        /* 'C' 반복 전송하여 CRC-16 모드 Y-modem 핸드셰이크 (최대 60초 대기)
     * TeraTerm 등에서 Y-modem Send 메뉴 선택할 시간을 확보 */
        {
            uint32_t handshake_start = wd_time_get_ms();
            int32_t ch;
            uint32_t handshake_c_count = 0;
            uint32_t handshake_rx_count = 0;
            uint32_t can_count = 0;

            while (1) {
                if (wd_time_calc_ms(&handshake_start) > YMODEM_HANDSHAKE_TIMEOUT_MS) {
                    WD_PRINT_INFO(
                        WD_MSG_INFO_LOG, "[YMODEM-DBG] handshake timeout: sent_C=%u rx=%u",
                        (unsigned int)handshake_c_count, (unsigned int)handshake_rx_count);
                    ret = YMODEM_ERR_TIMEOUT;
                    goto ymodem_cleanup;
                }

                ymodem_flush_rx();
                ymodem_putc('C');
                handshake_c_count++;

                /* 1초간 SOH/STX 첫 바이트 대기 */
                ch = ymodem_getc(YMODEM_HANDSHAKE_INTERVAL_MS);
                if (ch < 0) {
                    continue; /* 타임아웃 → 'C' 재전송 */
                }

                handshake_rx_count++;

                if (ch == SOH || ch == STX) {
                    /* SOH/STX 수신 즉시 나머지 전체 패킷을 벌크 수신 (로그 출력 금지!)
           * seq(1) + ~seq(1) + payload(128/1024) + CRC(2) = 132 or 1028 bytes */
                    uint32_t pkt_size =
                        (ch == SOH) ? YMODEM_SOH_PAYLOAD_SIZE : YMODEM_STX_PAYLOAD_SIZE;
                    uint32_t remain = 2 + pkt_size + 2; /* seq + ~seq + data + crc16 */
                    static uint8_t raw[2 + YMODEM_STX_PAYLOAD_SIZE + 2]
                        __attribute__((section(".RAM_D2")));
                    uint16_t crc_calc, crc_recv;

                    if (ymodem_receive_bulk(raw, remain, YMODEM_RX_TIMEOUT_MS) < 0) {
                        WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM-DBG] pkt0 bulk timeout");
                        continue;
                    }

                    /* 벌크 수신 완료 후 검증 (이제 로그 출력 안전) */
                    if ((raw[0] ^ raw[1]) != 0xFF) {
                        WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM-DBG] seq mismatch: %02X/%02X",
                                      raw[0], raw[1]);
                        continue;
                    }

                    crc_recv = (uint16_t)(raw[2 + pkt_size] << 8) | raw[2 + pkt_size + 1];
                    crc_calc = ymodem_crc16(&raw[2], pkt_size);
                    if (crc_calc != crc_recv) {
                        WD_PRINT_INFO(WD_MSG_INFO_LOG,
                                      "[YMODEM-DBG] crc mismatch: calc=%04X recv=%04X", crc_calc,
                                      crc_recv);
                        continue;
                    }

                    /* Packet #0 수신 성공 → 페이로드를 pkt_buf에 복사 */
                    memcpy(pkt_buf, &raw[2], pkt_size);
                    payload_size = pkt_size;
                    WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM-DBG] Packet#0 OK (size=%u)",
                                  (unsigned int)pkt_size);
                    break;
                } else if (ch == CAN_) {
                    can_count++;
                    if (can_count >= 2) /* Y-modem 표준: CAN 2연속 = abort */
                    {
                        ret = YMODEM_ERR_ABORT;
                        goto ymodem_cleanup;
                    }
                } else {
                    can_count = 0; /* CAN이 아닌 다른 바이트 → CAN 카운터 리셋 */
                }
                /* 그 외(타임아웃 포함): 'C' 재전송 반복 */
            }
        }

        /* Packet #0 (파일 정보) 직접 처리 */
        {
            if (pkt_buf[0] == '\0') {
                /* 빈 파일명 = 세션 종료 */
                ymodem_putc(ACK);
                session_done = 1;
                continue;
            }

            /* 파일명 파싱 */
            memset(wd_fw_fota_info.File_Info.File_Name, 0,
                   sizeof(wd_fw_fota_info.File_Info.File_Name));
            {
                uint32_t name_len = strlen((char *)pkt_buf);
                if (name_len >= sizeof(wd_fw_fota_info.File_Info.File_Name)) {
                    name_len = sizeof(wd_fw_fota_info.File_Info.File_Name) - 1;
                }
                memcpy(wd_fw_fota_info.File_Info.File_Name, pkt_buf, name_len);
            }

            /* 파일 크기 파싱 */
            {
                char *size_str = (char *)pkt_buf + strlen((char *)pkt_buf) + 1;
                if (sscanf(size_str, "%" PRIu32, &file_size) != 1) {
                    file_size = 0;
                }
                wd_fw_fota_info.File_Info.File_Length = file_size;
            }

            /* 경계 검사 */
            if (file_size > MEMORY_OBJECT_APPLICATION_DOWN_SIZE) {
                ymodem_send_cancel();
                ret = YMODEM_ERR_OVERFLOW;
                goto ymodem_cleanup;
            }

            expected_seq = 1; /* 다음 패킷은 seq #1 */
            ymodem_putc(ACK);
            ymodem_putc('C'); /* 데이터 패킷 요청 */
        }

        /* 데이터 패킷 수신 루프 (seq #1 ~ ) */
        WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM-DBG] data loop start: HAL_state=%u ring_len=%u",
                      (unsigned int)(RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->RxState,
                      (unsigned int)wd_print_hal_uart8_size());

        while (!file_done) {
            ret = ymodem_receive_packet(pkt_buf, &payload_size);

            if (ret == YMODEM_EOT) /* EOT */
            {
                /* Y-modem batch: 첫 EOT → NAK, 두번째 EOT → ACK */
                ymodem_putc(NAK);
                {
                    int32_t eot2 = ymodem_getc(YMODEM_RX_TIMEOUT_MS);
                    if (eot2 == EOT) {
                        ymodem_putc(ACK);
                    }
                }

                if (total_written > 0) {
                    /* 파일 전송 완료 → 'C' 전송 후 세션 종료 Packet#0을 여기서 직접 수신
           * (핸드셰이크 루프로 돌아가면 flush가 응답을 버림) */
                    ymodem_putc('C');

                    /* 세션 종료 Packet#0 수신 (SOH + 빈 파일명) */
                    ret = ymodem_receive_packet(pkt_buf, &payload_size);
                    if (ret >= 0) {
                        ymodem_putc(ACK);
                        session_done = 1;
                    }
                    file_done = 1;
                } else {
                    session_done = 1;
                    file_done = 1;
                }
                continue;
            }

            if (ret < 0) /* 에러 */
            {
                errors++;
                if (errors > YMODEM_MAX_ERRORS) {
                    ymodem_send_cancel();
                    ret = YMODEM_ERR_TIMEOUT;
                    goto ymodem_cleanup;
                }
                ymodem_flush_rx();
                ymodem_putc(NAK);
                continue;
            }

            /* 시퀀스 번호 검증 */
            if ((uint8_t)ret != expected_seq) {
                /* 이전 패킷 재전송인 경우 ACK만 보냄 */
                if ((uint8_t)ret == (uint8_t)(expected_seq - 1)) {
                    ymodem_putc(ACK);
                    continue;
                }
                errors++;
                if (errors > YMODEM_MAX_ERRORS) {
                    ymodem_send_cancel();
                    ret = YMODEM_ERR_SEQ;
                    goto ymodem_cleanup;
                }
                ymodem_putc(NAK);
                continue;
            }

            errors = 0;

            /* 데이터 패킷 → SDRAM에 저장 (NVM write 없이 고속 수신) */
            {
                uint32_t write_size = payload_size;

                /* 마지막 패킷: file_size에 맞게 기록 크기 조정 */
                if ((file_size > 0) && ((total_written + write_size) > file_size)) {
                    write_size = file_size - total_written;
                }

                /* 경계 검사 */
                if ((total_written + write_size) > YMODEM_SDRAM_BUF_SIZE) {
                    ymodem_send_cancel();
                    ret = YMODEM_ERR_OVERFLOW;
                    goto ymodem_cleanup;
                }

                memcpy(ymodem_sdram_buf + total_written, pkt_buf, write_size);
                total_written += write_size;
                wd_fw_fota_info.Set_Offset = total_written;

                /* 진행률 표시 (20% 단위) */
                if (file_size > 0) {
                    uint32_t prev_pct = ((total_written - write_size) * 100) / file_size;
                    uint32_t curr_pct = (total_written * 100) / file_size;
                    if ((curr_pct / 20) != (prev_pct / 20)) {
                        WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM] Progress: %u / %u bytes (%u%%)",
                                      (unsigned int)total_written, (unsigned int)file_size,
                                      (unsigned int)curr_pct);
                    }
                }
            }

            expected_seq++;
            ymodem_putc(ACK);
        }
    }

    WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM] Transfer complete: %u bytes received",
                  (unsigned int)total_written);

    if (total_written == 0) {
        ret = YMODEM_ERR_TIMEOUT;
        goto ymodem_cleanup;
    }

    /*--- SDRAM → NVM 일괄 기록 ---*/
    WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM] Erasing NVM download area...");
    debug_time = wd_time_get_ms();
    FACTORY_CONSOLE_LOCK();
    wd_mmu_access(&MAP->NVM->Firmware_Download, wd_core_scratch_buffer(),
                  sizeof(MAP->NVM->Firmware_Download), WD_MMU_COMMAND_INIT_0xFF);
    FACTORY_CONSOLE_UNLOCK();
    WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM] NVM erase done (%u ms)", wd_time_calc_ms(&debug_time));

    WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM] Writing %u bytes to NVM...",
                  (unsigned int)total_written);
    debug_time = wd_time_get_ms();
    FACTORY_CONSOLE_LOCK();
    wd_mmu_access(MAP->NVM->Firmware_Download, ymodem_sdram_buf, total_written,
                  WD_MMU_COMMAND_WRITE);
    FACTORY_CONSOLE_UNLOCK();
    WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM] NVM write done (%u ms)", wd_time_calc_ms(&debug_time));

    /*--- 펌웨어 이미지 검증 ---*/
    WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM] Verifying firmware image...");
    {
        uint32_t image_size = 0;
        int32_t check_ret;

        check_ret =
            wd_fw_check_image(&wd_fw_fota_info.Firmware_Header,
                              (uint32_t)(uintptr_t)MAP->NVM->Firmware_Download, &image_size);

        if (image_size < (1024 * 32)) {
            check_ret = WD_CODE_ERROR;
        }

        if (check_ret != WD_CODE_OK) {
            wd_fw_fota_info.Image_Transfer_Status = WDIF_FIRMWARE_FOTA_IMAGE_VERIFICATION_FAILED;
            ret = YMODEM_ERR_VERIFY;
            goto ymodem_cleanup;
        }
    }

    WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM] Firmware verification OK");
    wd_fw_print_image(&wd_fw_fota_info.Firmware_Header, "YMODEM Download");

    wd_fw_fota_info.Image_Transfer_Status = WDIF_FIRMWARE_FOTA_IMAGE_VERIFICATION_SUCCESSFUL;
    wd_fw_fota_info.File_Type = wd_fw_fota_info.Firmware_Header.File_Type;
    wd_fw_fota_info.File_Location = wd_fw_fota_info.Firmware_Header.File_Location;
    wd_fw_fota_info.File_Length = wd_fw_fota_info.Firmware_Header.File_Length;
    wd_fw_fota_info.Update_Date = wd_fw_fota_info.Firmware_Header.Update_Date;
    wd_fw_fota_info.File_CheckSum = wd_fw_fota_info.Firmware_Header.File_CheckSum;

    ret = YMODEM_OK;

ymodem_cleanup:
    /*--- DMA 중지 + UART8 원래 baud rate(2Mbaud) + HAL IT 모드 복원 ---*/
    HAL_UART_DMAStop(RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE);
    NVIC_EnableIRQ(DMA2_Stream7_IRQn); /* DMA IRQ 복원 */
    HAL_UART_AbortReceive(RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE);
    (RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->Instance->CR1 &= ~USART_CR1_UE;
    (RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->Instance->BRR = 60; /* 2Mbaud 복원 (120MHz/60) */
    (RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->Instance->ICR =
        USART_ICR_ORECF | USART_ICR_FECF | USART_ICR_NECF | USART_ICR_PECF;
    (RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->Instance->CR1 |= USART_CR1_UE;
    wd_print_hal_clear_uart8();
    (RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE)->RxState = HAL_UART_STATE_READY;
    HAL_UART_Receive_IT(RTE_SERVICE_PROTOCOL_FACTORY_UART_HANDLE,
                        (uint8_t *)&wd_print_hal_uart8_rx_ring->temp,
                        sizeof(wd_print_hal_uart8_rx_ring->temp));

    if (ret != YMODEM_OK) {
        WD_PRINT_INFO(WD_MSG_INFO_LOG, "[YMODEM] Failed (error: %d)", (int)ret);
    }

    return ret;
}

#endif /* (RTE_SERVICE_FIRMWARE) && (RTE_SERVICE_MMU) */


#endif // (RTE_COMPONENT_PROTOCOL_FACTORY)

#endif // #if 1 // 토글
