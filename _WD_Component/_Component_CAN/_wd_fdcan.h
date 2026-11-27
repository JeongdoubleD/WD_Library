/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_fdcan.h
 *
 *  Created on: Nov 17, 2025
 *      Author: doubled
 */

#ifndef WD_COMPONENT_WD_FDCAN_H
#define WD_COMPONENT_WD_FDCAN_H

#include "wd_core.h"

#if 1 // 토글

#if (RTE_COMPONENT_CAN)

#define RTE_COMPONENT_CAN_BAUDRATE      500000U //30000000u
#define RTE_COMPONENT_CAN_DRIVER_NUMBER 1
#ifndef RTE_COMPONENT_CAN_DRIVER_REGISTER
#define RTE_COMPONENT_CAN_DRIVER_REGISTER FDCAN1
#endif
#ifndef RTE_COMPONENT_CAN_DRIVER_HANDLE
#define RTE_COMPONENT_CAN_DRIVER_HANDLE (&hfdcan1)
#endif

/* CAN 이벤트 콜백 타입 (Ethernet의 wd_eth_socket_signal_event_t 와 동일 컨셉) */
typedef wd_event_callback_t wd_fdcan_signal_event_t;

/* CAN 드라이버 확장 Capability (필요시 확장, 지금은 예시로 Dummy 필드만) */
typedef struct {
    uint32_t Dummy;
} wd_fdcan_capabilities_t;

/* CAN 드라이버 전용 Info 구조체 (port->Private 에 매핑) */
typedef struct {
    uint32_t RxCount;    /* 수신된 CAN 프레임 수 */
    uint32_t TxCount;    /* 전송된 CAN 프레임 수 */
    uint32_t ErrorCount; /* 에러 카운트 */
    uint32_t LastError;  /* 마지막 에러 코드 (HAL 반환 값 등) */
} wd_fdcan_info_t;

/* 상위(Net 계층)에서 사용하는 공통 Driver 테이블 (Ethernet의 wd_eth_data 와 동일 역할) */
extern wd_network_driver_t wd_fdcan_data;

/*
 * CAN 하위 모듈 드라이버 (Ethernet의 wd_eth_t 과 유사하지만 단순화)
 * - socket, IP는 없고, 순수 프레임 단위 송수신만 담당
 */
typedef struct {
    int32_t (*Initialize)(uint32_t arg);
    int32_t (*Send)(uint32_t id, const uint8_t *data, uint8_t dlc);
    int32_t (*Receive)(uint32_t *id, uint8_t *data, uint8_t *dlc);
    int32_t (*GetRxCount)(void);
    int32_t *State;
} const wd_fdcan_module_t;


/* 하위 CAN 모듈 핸들 */
extern wd_fdcan_module_t wd_fdcan_module;

/* === 테스트용 유틸 함수들 (에코 & 단일 프레임 전송) ======================= */
extern uint8_t wd_fdcan_echo_mode;
extern uint8_t wd_fdcan_g_can_data[64]; /* CAN FD: 나중엔 64바이트까지 쓸 수 있음 */
extern uint8_t wd_fdcan_dlc;
extern int32_t wd_fdcan_len;

/* 에코 모드 설정: enable != 0 이면 수신 프레임을 그대로 송신 */
void wd_fdcan_test_set_echo_mode(uint8_t enable);

/* === 조합(Composite) 패킷(18018000h ~ 18018FFFh) 전송 유틸 ===================== */
/* 조합 패킷 ID/길이 정의 */
#ifndef WD_CAN_COMPOSITE_ID_BASE
#define WD_CAN_COMPOSITE_ID_BASE 0x18018000UL /* 첫 조합 패킷 ID */
#endif
#ifndef WD_CAN_COMPOSITE_ID_MAX
#define WD_CAN_COMPOSITE_ID_MAX 0x18018FFFUL /* 마지막 조합 패킷 ID */
#endif
#ifndef WD_CAN_COMPOSITE_ID_MASK
#define WD_CAN_COMPOSITE_ID_MASK 0xFFFFF000UL /* 상위 ID 공통 부분 */
#endif
#ifndef WD_CAN_COMPOSITE_SEGMENT_MAX
#define WD_CAN_COMPOSITE_SEGMENT_MAX 0x00000FFFUL /* 세그먼트 index 최대값 (4095) */
#endif

#ifndef WD_CAN_COMPOSITE_FRAME_DATA_MAX
#define WD_CAN_COMPOSITE_FRAME_DATA_MAX 64U /* FDCAN 1프레임 최대 payload */
#endif
#ifndef WD_CAN_COMPOSITE_PACKET_MAX
#define WD_CAN_COMPOSITE_PACKET_MAX 1024U /* 논리 패킷 최대 길이(STX포함) */
#endif

/*
 * 조합 패킷 송신기
 *  - STX(0x02)로 시작하는 완성된 논리 패킷(packet, packet_len)을
 *    64B 단위로 나눠 18018000h + n ID로 연속 송신.
 *  - base_id 를 18018000h 이상 18018FFFh 이하로 지정하면, 그 값을 시작 ID 로 사용.
 *    (그 외 값이면 자동으로 WD_CAN_COMPOSITE_ID_BASE 를 사용)
 */
int32_t wd_fdcan_send_composite(uint32_t base_id, const uint8_t *packet, uint16_t packet_len);

#endif // (RTE_COMPONENT_CAN)
#endif // #if 1 // 토글
#endif /* WD_COMPONENT_WD_FDCAN_H */
