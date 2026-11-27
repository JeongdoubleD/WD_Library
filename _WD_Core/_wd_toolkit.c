/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_toolkit.c
 *
 *  Created on: Jan 13, 2025
 *      Author: doubled
 */

#include "wd_core.h"

#if 1 //토글


static const uint16_t crc16_table[256] = {0x0000,
                                          0x1021,
                                          0x2042,
                                          0x3063,
                                          0x4084,
                                          0x50A5,
                                          0x60C6,
                                          0x70E7 //000 ~ 007
                                          ,
                                          0x8108,
                                          0x9129,
                                          0xA14A,
                                          0xB16B,
                                          0xC18C,
                                          0xD1AD,
                                          0xE1CE,
                                          0xF1EF //008 ~ 015
                                          ,
                                          0x1231,
                                          0x0210,
                                          0x3273,
                                          0x2252,
                                          0x52B5,
                                          0x4294,
                                          0x72F7,
                                          0x62D6 //016 ~ 023
                                          ,
                                          0x9339,
                                          0x8318,
                                          0xB37B,
                                          0xA35A,
                                          0xD3BD,
                                          0xC39C,
                                          0xF3FF,
                                          0xE3DE //024 ~ 031
                                          ,
                                          0x2462,
                                          0x3443,
                                          0x0420,
                                          0x1401,
                                          0x64E6,
                                          0x74C7,
                                          0x44A4,
                                          0x5485 //032 ~ 039
                                          ,
                                          0xA56A,
                                          0xB54B,
                                          0x8528,
                                          0x9509,
                                          0xE5EE,
                                          0xF5CF,
                                          0xC5AC,
                                          0xD58D //040 ~ 047
                                          ,
                                          0x3653,
                                          0x2672,
                                          0x1611,
                                          0x0630,
                                          0x76D7,
                                          0x66F6,
                                          0x5695,
                                          0x46B4 //048 ~ 055
                                          ,
                                          0xB75B,
                                          0xA77A,
                                          0x9719,
                                          0x8738,
                                          0xF7DF,
                                          0xE7FE,
                                          0xD79D,
                                          0xC7BC //056 ~ 063
                                          ,
                                          0x48C4,
                                          0x58E5,
                                          0x6886,
                                          0x78A7,
                                          0x0840,
                                          0x1861,
                                          0x2802,
                                          0x3823 //064 ~ 071
                                          ,
                                          0xC9CC,
                                          0xD9ED,
                                          0xE98E,
                                          0xF9AF,
                                          0x8948,
                                          0x9969,
                                          0xA90A,
                                          0xB92B //072 ~ 079
                                          ,
                                          0x5AF5,
                                          0x4AD4,
                                          0x7AB7,
                                          0x6A96,
                                          0x1A71,
                                          0x0A50,
                                          0x3A33,
                                          0x2A12 //080 ~ 087
                                          ,
                                          0xDBFD,
                                          0xCBDC,
                                          0xFBBF,
                                          0xEB9E,
                                          0x9B79,
                                          0x8B58,
                                          0xBB3B,
                                          0xAB1A //088 ~ 095
                                          ,
                                          0x6CA6,
                                          0x7C87,
                                          0x4CE4,
                                          0x5CC5,
                                          0x2C22,
                                          0x3C03,
                                          0x0C60,
                                          0x1C41 //096 ~ 103
                                          ,
                                          0xEDAE,
                                          0xFD8F,
                                          0xCDEC,
                                          0xDDCD,
                                          0xAD2A,
                                          0xBD0B,
                                          0x8D68,
                                          0x9D49 //104 ~ 111
                                          ,
                                          0x7E97,
                                          0x6EB6,
                                          0x5ED5,
                                          0x4EF4,
                                          0x3E13,
                                          0x2E32,
                                          0x1E51,
                                          0x0E70 //112 ~ 119
                                          ,
                                          0xFF9F,
                                          0xEFBE,
                                          0xDFDD,
                                          0xCFFC,
                                          0xBF1B,
                                          0xAF3A,
                                          0x9F59,
                                          0x8F78 //120 ~ 127
                                          ,
                                          0x9188,
                                          0x81A9,
                                          0xB1CA,
                                          0xA1EB,
                                          0xD10C,
                                          0xC12D,
                                          0xF14E,
                                          0xE16F //128 ~ 135
                                          ,
                                          0x1080,
                                          0x00A1,
                                          0x30C2,
                                          0x20E3,
                                          0x5004,
                                          0x4025,
                                          0x7046,
                                          0x6067 //136 ~ 143
                                          ,
                                          0x83B9,
                                          0x9398,
                                          0xA3FB,
                                          0xB3DA,
                                          0xC33D,
                                          0xD31C,
                                          0xE37F,
                                          0xF35E //144 ~ 151
                                          ,
                                          0x02B1,
                                          0x1290,
                                          0x22F3,
                                          0x32D2,
                                          0x4235,
                                          0x5214,
                                          0x6277,
                                          0x7256 //152 ~ 159
                                          ,
                                          0xB5EA,
                                          0xA5CB,
                                          0x95A8,
                                          0x8589,
                                          0xF56E,
                                          0xE54F,
                                          0xD52C,
                                          0xC50D //160 ~ 167
                                          ,
                                          0x34E2,
                                          0x24C3,
                                          0x14A0,
                                          0x0481,
                                          0x7466,
                                          0x6447,
                                          0x5424,
                                          0x4405 //168 ~ 175
                                          ,
                                          0xA7DB,
                                          0xB7FA,
                                          0x8799,
                                          0x97B8,
                                          0xE75F,
                                          0xF77E,
                                          0xC71D,
                                          0xD73C //176 ~ 183
                                          ,
                                          0x26D3,
                                          0x36F2,
                                          0x0691,
                                          0x16B0,
                                          0x6657,
                                          0x7676,
                                          0x4615,
                                          0x5634 //184 ~ 191
                                          ,
                                          0xD94C,
                                          0xC96D,
                                          0xF90E,
                                          0xE92F,
                                          0x99C8,
                                          0x89E9,
                                          0xB98A,
                                          0xA9AB //192 ~ 199
                                          ,
                                          0x5844,
                                          0x4865,
                                          0x7806,
                                          0x6827,
                                          0x18C0,
                                          0x08E1,
                                          0x3882,
                                          0x28A3 //200 ~ 207
                                          ,
                                          0xCB7D,
                                          0xDB5C,
                                          0xEB3F,
                                          0xFB1E,
                                          0x8BF9,
                                          0x9BD8,
                                          0xABBB,
                                          0xBB9A //208 ~ 215
                                          ,
                                          0x4A75,
                                          0x5A54,
                                          0x6A37,
                                          0x7A16,
                                          0x0AF1,
                                          0x1AD0,
                                          0x2AB3,
                                          0x3A92 //216 ~ 223
                                          ,
                                          0xFD2E,
                                          0xED0F,
                                          0xDD6C,
                                          0xCD4D,
                                          0xBDAA,
                                          0xAD8B,
                                          0x9DE8,
                                          0x8DC9 //224 ~ 231
                                          ,
                                          0x7C26,
                                          0x6C07,
                                          0x5C64,
                                          0x4C45,
                                          0x3CA2,
                                          0x2C83,
                                          0x1CE0,
                                          0x0CC1 //232 ~ 239
                                          ,
                                          0xEF1F,
                                          0xFF3E,
                                          0xCF5D,
                                          0xDF7C,
                                          0xAF9B,
                                          0xBFBA,
                                          0x8FD9,
                                          0x9FF8 //240 ~ 247
                                          ,
                                          0x6E17,
                                          0x7E36,
                                          0x4E55,
                                          0x5E74,
                                          0x2E93,
                                          0x3EB2,
                                          0x0ED1,
                                          0x1EF0};

static uint16_t MODBUS_CRC16_table[256] = {0x0000,
                                           0xC0C1,
                                           0xC181,
                                           0x0140,
                                           0xC301,
                                           0x03C0,
                                           0x0280,
                                           0xC241 //000 ~ 007
                                           ,
                                           0xC601,
                                           0x06C0,
                                           0x0780,
                                           0xC741,
                                           0x0500,
                                           0xC5C1,
                                           0xC481,
                                           0x0440 //008 ~ 015
                                           ,
                                           0xCC01,
                                           0x0CC0,
                                           0x0D80,
                                           0xCD41,
                                           0x0F00,
                                           0xCFC1,
                                           0xCE81,
                                           0x0E40 //016 ~ 023
                                           ,
                                           0x0A00,
                                           0xCAC1,
                                           0xCB81,
                                           0x0B40,
                                           0xC901,
                                           0x09C0,
                                           0x0880,
                                           0xC841 //024 ~ 031
                                           ,
                                           0xD801,
                                           0x18C0,
                                           0x1980,
                                           0xD941,
                                           0x1B00,
                                           0xDBC1,
                                           0xDA81,
                                           0x1A40 //032 ~ 039
                                           ,
                                           0x1E00,
                                           0xDEC1,
                                           0xDF81,
                                           0x1F40,
                                           0xDD01,
                                           0x1DC0,
                                           0x1C80,
                                           0xDC41 //040 ~ 047
                                           ,
                                           0x1400,
                                           0xD4C1,
                                           0xD581,
                                           0x1540,
                                           0xD701,
                                           0x17C0,
                                           0x1680,
                                           0xD641 //048 ~ 055
                                           ,
                                           0xD201,
                                           0x12C0,
                                           0x1380,
                                           0xD341,
                                           0x1100,
                                           0xD1C1,
                                           0xD081,
                                           0x1040 //056 ~ 063
                                           ,
                                           0xF001,
                                           0x30C0,
                                           0x3180,
                                           0xF141,
                                           0x3300,
                                           0xF3C1,
                                           0xF281,
                                           0x3240 //064 ~ 071
                                           ,
                                           0x3600,
                                           0xF6C1,
                                           0xF781,
                                           0x3740,
                                           0xF501,
                                           0x35C0,
                                           0x3480,
                                           0xF441 //072 ~ 079
                                           ,
                                           0x3C00,
                                           0xFCC1,
                                           0xFD81,
                                           0x3D40,
                                           0xFF01,
                                           0x3FC0,
                                           0x3E80,
                                           0xFE41 //080 ~ 087
                                           ,
                                           0xFA01,
                                           0x3AC0,
                                           0x3B80,
                                           0xFB41,
                                           0x3900,
                                           0xF9C1,
                                           0xF881,
                                           0x3840 //088 ~ 095
                                           ,
                                           0x2800,
                                           0xE8C1,
                                           0xE981,
                                           0x2940,
                                           0xEB01,
                                           0x2BC0,
                                           0x2A80,
                                           0xEA41 //096 ~ 103
                                           ,
                                           0xEE01,
                                           0x2EC0,
                                           0x2F80,
                                           0xEF41,
                                           0x2D00,
                                           0xEDC1,
                                           0xEC81,
                                           0x2C40 //104 ~ 111
                                           ,
                                           0xE401,
                                           0x24C0,
                                           0x2580,
                                           0xE541,
                                           0x2700,
                                           0xE7C1,
                                           0xE681,
                                           0x2640 //112 ~ 119
                                           ,
                                           0x2200,
                                           0xE2C1,
                                           0xE381,
                                           0x2340,
                                           0xE101,
                                           0x21C0,
                                           0x2080,
                                           0xE041 //120 ~ 127
                                           ,
                                           0xA001,
                                           0x60C0,
                                           0x6180,
                                           0xA141,
                                           0x6300,
                                           0xA3C1,
                                           0xA281,
                                           0x6240 //128 ~ 135
                                           ,
                                           0x6600,
                                           0xA6C1,
                                           0xA781,
                                           0x6740,
                                           0xA501,
                                           0x65C0,
                                           0x6480,
                                           0xA441 //136 ~ 143
                                           ,
                                           0x6C00,
                                           0xACC1,
                                           0xAD81,
                                           0x6D40,
                                           0xAF01,
                                           0x6FC0,
                                           0x6E80,
                                           0xAE41 //144 ~ 151
                                           ,
                                           0xAA01,
                                           0x6AC0,
                                           0x6B80,
                                           0xAB41,
                                           0x6900,
                                           0xA9C1,
                                           0xA881,
                                           0x6840 //152 ~ 159
                                           ,
                                           0x7800,
                                           0xB8C1,
                                           0xB981,
                                           0x7940,
                                           0xBB01,
                                           0x7BC0,
                                           0x7A80,
                                           0xBA41 //160 ~ 167
                                           ,
                                           0xBE01,
                                           0x7EC0,
                                           0x7F80,
                                           0xBF41,
                                           0x7D00,
                                           0xBDC1,
                                           0xBC81,
                                           0x7C40 //168 ~ 175
                                           ,
                                           0xB401,
                                           0x74C0,
                                           0x7580,
                                           0xB541,
                                           0x7700,
                                           0xB7C1,
                                           0xB681,
                                           0x7640 //176 ~ 183
                                           ,
                                           0x7200,
                                           0xB2C1,
                                           0xB381,
                                           0x7340,
                                           0xB101,
                                           0x71C0,
                                           0x7080,
                                           0xB041 //184 ~ 191
                                           ,
                                           0x5000,
                                           0x90C1,
                                           0x9181,
                                           0x5140,
                                           0x9301,
                                           0x53C0,
                                           0x5280,
                                           0x9241 //192 ~ 199
                                           ,
                                           0x9601,
                                           0x56C0,
                                           0x5780,
                                           0x9741,
                                           0x5500,
                                           0x95C1,
                                           0x9481,
                                           0x5440 //200 ~ 207
                                           ,
                                           0x9C01,
                                           0x5CC0,
                                           0x5D80,
                                           0x9D41,
                                           0x5F00,
                                           0x9FC1,
                                           0x9E81,
                                           0x5E40 //208 ~ 215
                                           ,
                                           0x5A00,
                                           0x9AC1,
                                           0x9B81,
                                           0x5B40,
                                           0x9901,
                                           0x59C0,
                                           0x5880,
                                           0x9841 //216 ~ 223
                                           ,
                                           0x8801,
                                           0x48C0,
                                           0x4980,
                                           0x8941,
                                           0x4B00,
                                           0x8BC1,
                                           0x8A81,
                                           0x4A40 //224 ~ 231
                                           ,
                                           0x4E00,
                                           0x8EC1,
                                           0x8F81,
                                           0x4F40,
                                           0x8D01,
                                           0x4DC0,
                                           0x4C80,
                                           0x8C41 //232 ~ 239
                                           ,
                                           0x4400,
                                           0x84C1,
                                           0x8581,
                                           0x4540,
                                           0x8701,
                                           0x47C0,
                                           0x4680,
                                           0x8641 //240 ~ 247
                                           ,
                                           0x8201,
                                           0x42C0,
                                           0x4380,
                                           0x8341,
                                           0x4100,
                                           0x81C1,
                                           0x8081,
                                           0x4040};

////
uint16_t wd_toolkit_crc16_ccitt(const uint8_t *data, size_t length) {
    uint16_t crc = 0xFFFF; // 초기값 or 0x0000, MIZ 프로토콜 상 초기값 0xFFFF

    for (size_t i = 0; i < length; ++i) {
        uint8_t index = (uint8_t)((crc >> 8) ^ data[i]);
        crc = (crc << 8) ^ crc16_table[index];
    }

    return crc;
}

int wd_toolkit_crc16_ccitt_verify(const uint8_t *data, size_t length, uint16_t expected_crc) {
    return wd_toolkit_crc16_ccitt(data, length) == expected_crc;
}

void wd_toolkit_crc16_ccitt_append(const uint8_t *data, size_t length, uint8_t *crc_hi,
                                   uint8_t *crc_lo) {
    uint16_t crc = wd_toolkit_crc16_ccitt(data, length);
    *crc_hi = (uint8_t)(crc >> 8);
    *crc_lo = (uint8_t)(crc & 0xFF);
}
////

////
uint16_t wd_toolkit_crc16_modbus(const uint8_t *_buffer, int _start_idx, int _length) {
    uint16_t crc = 0xFFFF;

    for (int ii = _start_idx; ii < _start_idx + _length; ++ii) {
        crc = (uint16_t)(MODBUS_CRC16_table[(crc ^ _buffer[ii]) & 0xFF] ^ (crc >> 8));
    }
    return crc;
}

int wd_toolkit_crc16_modbus_verify(const uint8_t *data, size_t length, int _start_idx,
                                   uint16_t expected_crc) {
    return wd_toolkit_crc16_modbus(data, _start_idx, (int)length) == expected_crc;
}

void wd_toolkit_crc16_modbus_append(const uint8_t *data, size_t length, int _start_idx,
                                    uint8_t *crc_hi, uint8_t *crc_lo) {
    uint16_t crc = wd_toolkit_crc16_modbus(data, _start_idx, (int)length);
    *crc_hi = (uint8_t)(crc >> 8);
    *crc_lo = (uint8_t)(crc & 0xFF);
}
////

////
uint16_t wd_toolkit_crc16(void *ptr, uint32_t num) {
    uint8_t d;
    uint8_t *p = (uint8_t *)ptr;
    uint16_t crc = 0xFFFF;

    if ((num != 0U) && ((wd_core_is_valid_address((uint32_t)p) == false) ||
                        (wd_core_is_valid_address((uint32_t)p + num - 1U) == false))) {
        WD_LOG_ERROR("CRC16 Range Error");
        WD_BREAK();
        return (crc);
    }

    if (!num) {
        return ((uint16_t)WD_CODE_ERROR);
    }

    do {
        d = (uint8_t)(*p++ ^ (crc & 0xFF));
        d ^= d << 4;
        crc = (uint16_t)((d << 3) ^ (d << 8) ^ (crc >> 8) ^ (d >> 4));
    } while (--num);

    crc ^= 0xffff;
    return (crc);
}


uint16_t wd_toolkit_crc16_extension(void *ptr, uint32_t num, uint16_t *crc) {
    uint8_t *p = (uint8_t *)ptr;
    uint8_t d;

    if ((num == 0) || (crc == NULL)) {
        return 0;
    }

    if ((num != 0U) && ((wd_core_is_valid_address((uint32_t)p) == false) ||
                        (wd_core_is_valid_address((uint32_t)p + num - 1U) == false))) {
        WD_LOG_ERROR("CRC16 Range Error");
        WD_BREAK();
        return (*crc);
    }

    do {
        d = (uint8_t)(*p++ ^ ((*crc) & 0xFF));
        d ^= d << 4;
        (*crc) = (uint16_t)((d << 3) ^ (d << 8) ^ ((*crc) >> 8) ^ (d >> 4));
    } while (--num);

    return (*crc);
}

uint32_t wd_toolkit_crc32(void *ptr, uint32_t num) {
    unsigned char *p = (unsigned char *)ptr;
    int j;
    unsigned int crc, mask;

    crc = 0xFFFFFFFF;

    if ((num != 0U) && ((wd_core_is_valid_address((uint32_t)p) == false) ||
                        (wd_core_is_valid_address((uint32_t)p + num - 1U) == false))) {
        WD_LOG_ERROR("CRC32 Range Error");
        return (crc);
    }

    if (!num) {
        return ((uint32_t)WD_CODE_ERROR);
    }

    do {
        crc = crc ^ (*p++);
        for (j = 7; j >= 0; j--) { // Do eight times.
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    } while (--num);


    return ~crc;
}

uint32_t wd_toolkit_crc32_extension(void *ptr, uint32_t num, uint32_t *crc) {
    unsigned char *p = (unsigned char *)ptr;
    int j;
    unsigned int mask;

    if ((num != 0U) && ((wd_core_is_valid_address((uint32_t)p) == false) ||
                        (wd_core_is_valid_address((uint32_t)p + num - 1U) == false))) {
        WD_LOG_ERROR("CRC32 Range Error");
        return (*crc);
    }

    if (!num) {
        return ((uint32_t)WD_CODE_ERROR);
    }

    do {
        (*crc) = (*crc) ^ (*p++);
        for (j = 7; j >= 0; j--) { // Do eight times.
            mask = -((*crc) & 1);
            (*crc) = ((*crc) >> 1) ^ (0xEDB88320 & mask);
        }
    } while (--num);

    return (*crc);
}

void wd_toolkit_change_endian_data(void *ptr, uint32_t num) {
    uint8_t data[32];

    if (num < sizeof(data)) {
        uint32_t i;

        memcpy(data, (uint8_t *)ptr, num);
        for (i = 0; i < num; i++) {
            *(((uint8_t *)(ptr)) + i) = data[num - i - 1];
        }
    }
}

uint32_t wd_memchr(const void *ptr, uint8_t c, uint32_t num) {
    uint32_t find = 0;
    uint32_t i;

    for (i = 0; i < num; i++) {
        if (*(((const uint8_t *)ptr) + i) == c) {
            find = true;
            break;
        }
    }

    if (find) {
        return ((uint32_t)ptr + i);
    } else {
        return (0);
    }
}

uint32_t wd_memchrn(const void *ptr, uint8_t c, uint32_t num) {
    uint32_t find = 0;
    uint32_t i;

    for (i = 0; i < num; i++) {
        if (*(((const uint8_t *)ptr) + i) != c) {
            find = true;
            break;
        }
    }

    if (find) {
        return ((uint32_t)ptr + i);
    } else {
        return (0);
    }
}

#if defined(RTE_METROLOGY_TYPE_RN8302B) && (RTE_METROLOGY_TYPE == RTE_METROLOGY_TYPE_RN8302B)
int32_t wd_memchr_idx_16(const void *ptr, uint16_t c, uint32_t num) {
    int32_t i;

    for (i = 0; i < (int32_t)num; i++) {
        if (*(((const uint16_t *)ptr) + i) == c) {
            return (i);
        }
    }

    return (WD_CODE_ERROR);
}
#endif

int32_t wd_memchr_idx(const void *ptr, uint8_t c, uint32_t num) {
    int32_t i;

    for (i = 0; i < (int32_t)num; i++) {
        if (*(((const uint8_t *)ptr) + i) == c) {
            return (i);
        }
    }

    return (WD_CODE_ERROR);
}

uint8_t wd_toolkit_char_to_digit(uint8_t string) {
    switch (string) {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        return (string - '0');

    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
        return (string - 'A' + 10);

    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
        return (string - 'a' + 10);

    default:
        return (0xff);
    }
}

uint32_t wd_toolkit_ascii_to_hex(void *hex, void *ascii, uint32_t num_ascii) {
    uint32_t i, idx = 0;
    uint8_t upper = 0xff;
    uint8_t number = 0xff;

    for (i = 0; i < num_ascii; i++) {
        switch (((uint8_t *)(ascii))[i]) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            number = ((uint8_t *)(ascii))[i] - '0';
            break;

        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
            number = ((uint8_t *)(ascii))[i] - 'A' + 10;
            break;

        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
            number = ((uint8_t *)(ascii))[i] - 'a' + 10;
            break;

        default:
            number = 0xff;
            break;
        }

        if (number != 0xff) {
            if (upper != 0xff) {
                ((uint8_t *)(hex))[idx++] = (uint8_t)((upper << 4) | number);
                upper = 0xff;
            } else {
                upper = number;
            }
        }
    }

    return (idx);
}

uint32_t wd_toolkit_hex_to_ascii(void *ascii, void *hex, uint32_t num_hex) {
    int i;
    uint16_t idx = 0;

    for (i = 0; i < (int)num_hex; i++) {
        idx = (uint16_t)(idx + snprintf(((char *)(ascii)) + idx, 3U, "%02X",
                                        (unsigned int)((const uint8_t *)(hex))[i]));
    }
    return idx;
}

int32_t wd_toolkit_memcmp_hex_ascii(const void *hex, void *ascii, size_t len_ascii) {
    int i;
    uint16_t idx = 0;
    uint8_t hex_data;

    for (i = 0; i < (int)len_ascii; i += 2) {
        if (wd_toolkit_ascii_to_hex(&hex_data, (uint8_t *)(ascii) + i, 2) != 1) {
            WD_LOG_ERROR();
        }
        if (((const uint8_t *)(hex))[idx++] != hex_data) {
            return idx;
        }
    }

    return 0;
}

uint32_t wd_toolkit_calc_circular_buffer_idx(uint32_t idx, uint32_t offset, uint32_t idx_size,
                                             uint8_t direction) {
    if (direction == WD_DIR_LEFT) // decrease idx, counterclockwise direction
    {
        idx = ((idx_size - (idx % idx_size)) + offset) % idx_size;
    } else // increase idx, clock direction
    {
        //    idx = ((idx_size + (idx % idx_size)) - offset) % idx_size;
        //    idx = ((idx % idx_size) + offset) % idx_size;
        WD_LOG_ERROR();
    }

    return (idx);
}

void wd_toolkit_real_type_validation(void *real) {
    if ((__UNALIGNED_UINT32_READ(real) == WD_NAN) // IEEE 754 arithmetic
        || (__UNALIGNED_UINT32_READ(real) == WD_NAN_MAX) ||
        (__UNALIGNED_UINT32_READ(real) == WD_NAN_INF_P) ||
        (__UNALIGNED_UINT32_READ(real) == WD_NAN_INF_N) ||
        (__UNALIGNED_UINT32_READ(real) == WD_NAN_NAN_S) ||
        (__UNALIGNED_UINT32_READ(real) == WD_NAN_NAN_Q)) {
        WD_UNALIGNED_FLOAT_WRITE(real, 0.0);
    }
}

#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif
#if defined(__ICCARM__)
#pragma diag_suppress = Pa093
#endif

uint32_t wd_toolkit_isqrt64(uint64_t n) {
    return (sqrtl((long double)n));
}

#if (defined(__ICCARM__))
/**
  @brief         Floating-point complex magnitude.
  @param[in]     pSrc        points to input vector
  @param[out]    pDst        points to output vector
  @param[in]     numSamples  number of samples in each vector
  @return        none
 */
void arm_cmplx_mag_f64(const float64_t *pSrc, float64_t *pDst, uint32_t numSamples) {
    uint32_t blkCnt;      /* loop counter */
    float64_t real, imag; /* Temporary variables to hold input values */

    /* Initialize blkCnt with number of samples */
    blkCnt = numSamples;

    while (blkCnt > 0U) {
        /* C[0] = sqrt(A[0] * A[0] + A[1] * A[1]) */

        real = *pSrc++;
        imag = *pSrc++;

        /* store result in destination buffer. */
        *pDst++ = sqrt((real * real) + (imag * imag));

        /* Decrement loop counter */
        blkCnt--;
    }
}

/**
  @brief         Maximum value of a floating-point vector.
  @param[in]     pSrc       points to the input vector
  @param[in]     blockSize  number of samples in input vector
  @param[out]    pResult    maximum value returned here
  @param[out]    pIndex     index of maximum value returned here
  @return        none
 */
void arm_max_f64(const float64_t *pSrc, uint32_t blockSize, float64_t *pResult, uint32_t *pIndex) {
    float64_t maxVal, out;     /* Temporary variables to store the output value. */
    uint32_t blkCnt, outIndex; /* Loop counter */

    /* Initialise index value to zero. */
    outIndex = 0U;

    /* Load first input value that act as reference value for comparision */
    out = *pSrc++;

    /* Initialize blkCnt with number of samples */
    blkCnt = (blockSize - 1U);

    while (blkCnt > 0U) {
        /* Initialize maxVal to the next consecutive values one by one */
        maxVal = *pSrc++;

        /* compare for the maximum value */
        if (out < maxVal) {
            /* Update the maximum value and it's index */
            out = maxVal;
            outIndex = blockSize - blkCnt;
        }

        /* Decrement loop counter */
        blkCnt--;
    }

    /* Store the maximum value and it's index into destination pointers */
    *pResult = out;
    *pIndex = outIndex;
}
#endif

#if defined(__ARMCC_VERSION) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#if defined(__ICCARM__)
#pragma diag_default = Pa093
#endif

int wd_gcd(int a, int b) {
    if (a == 0 && b == 0) {
        return 1;
    }

    if (a < 0) {
        a = -a;
    }
    if (b < 0) {
        b = -b;
    }

    while (b != 0) {
        int temp = b;
        b = a % b;
        a = temp;
    }

    return a;
}

int wd_lcm(int a, int b) {
    return (a / wd_gcd(a, b)) * b;
}

void wd_memset_rand(void *ptr, size_t size) {
    uint8_t *p = (uint8_t *)ptr;
    for (size_t i = 0; i < size; i++) {
        p[i] = (uint8_t)(rand() % 256); // 0~255 범위의 랜덤 값으로 채움
    }
}

// 불일치 개수를 반환하는 wd_memcmp_print() 구현
int wd_memcmp_print(const void *buf1, const void *buf2, size_t size) {
    const uint8_t *ptr1 = (const uint8_t *)buf1;
    const uint8_t *ptr2 = (const uint8_t *)buf2;
    int mismatch_count = 0;

    // 메모리 비교
    int result = memcmp(ptr1, ptr2, size);

    if (result == 0) {
        printf("buffers are the same \r\n");
        return 0; // 차이 없음
    } else {
        printf("Memory Comparison: Buffer Different!\r\n");
        printf("Address_1: %p\n   Address_2: %p\n", buf1, buf2);

        // 차이점 출력 및 개수 카운트
        for (size_t i = 0; i < size; i++) {
            if (ptr1[i] != ptr2[i]) {
                printf("[Index %zu]: 0x%02X (buf1) != 0x%02X (buf2)\r\n", i, ptr1[i], ptr2[i]);
                mismatch_count++;
            }
        }
    }
    return mismatch_count; // ❗ 불일치한 바이트 개수를 반환
}

#endif //	#if 1 //토글
