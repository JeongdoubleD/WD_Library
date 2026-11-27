/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_fdcan.c
 *
 *  Created on: Nov 17, 2025
 *      Author: doubled
 */
#include "_wd_fdcan.h"

#if 1 // 토글

#if (RTE_COMPONENT_CAN)

/*********************************************************************************************************************
 *  Low-level HAL Wrapper (FDCAN)
 *********************************************************************************************************************/
#define CAN_DRV_NUM      RTE_COMPONENT_CAN_DRIVER_NUMBER
#define CAN_BAUDRATE_NET RTE_COMPONENT_CAN_BAUDRATE
#define CAN_REGISTER     RTE_COMPONENT_CAN_DRIVER_REGISTER
#define CAN_HANDLE       RTE_COMPONENT_CAN_DRIVER_HANDLE

/* 에코 테스트 모드 플래그 */
uint8_t wd_fdcan_echo_mode;
uint8_t wd_fdcan_g_can_data[64]; /* CAN FD: 나중엔 64바이트까지 쓸 수 있음 */
uint8_t wd_fdcan_dlc;
int32_t wd_fdcan_len;

/* 내부 상태 플래그 */
static int32_t Driver_CAN_Module_State = 0;

/* 수신 카운트 (Thread에서 Polling Receive 할 때마다 증가) */
static volatile uint32_t CAN_RxCount = 0;

/* ========================================================================== */
/*  CAN 조합 패킷 수신 컨텍스트                                               */
/*   - ID : 0x18018000 ~ 0x18018FFF                                           */
/*   - 하나의 논리 패킷(최대 1024B)을 여러 프레임으로 쪼개서 송수신           */
/* ========================================================================== */
#define CAN_COMPOSITE_BASE_ID    (WD_CAN_COMPOSITE_ID_BASE)
#define CAN_COMPOSITE_LAST_ID    (WD_CAN_COMPOSITE_ID_MAX)
#define CAN_COMPOSITE_MAX_PACKET (WD_CAN_COMPOSITE_PACKET_MAX)

/* 조합 패킷 수신 상태 (드라이버 내부에서만 사용) */
typedef struct {
    uint8_t Buffer[CAN_COMPOSITE_MAX_PACKET]; /* STX ~ wd_toolkit_crc16 전체 패킷 */
    uint16_t Expected;                        /* 전체 패킷 예상 길이 (0이면 아직 미결정)   	*/
    uint16_t Received;                        /* 현재까지 누적된 바이트 수                 	*/
    uint32_t BaseID;                          /* 기준 ID (0x18018000)                  */
    uint32_t LastID;                          /* 마지막으로 수신한 ID                     	*/
    uint8_t Active;                           /* 1: 조합 진행 중, 0: 유휴                	*/
} CAN_COMPOSITE_RX_CTX;

static CAN_COMPOSITE_RX_CTX CAN_Composite_Rx;

/* 바이트 길이 → DLC 코드 */
static uint32_t CAN_DLC_FromLength(uint8_t len_bytes) {
    if (len_bytes <= 8U) {
        /* 0~8바이트는 DLC 값이 곧 길이와 동일 (0~8) */
        return (uint32_t)len_bytes;
    } else if (len_bytes <= 12U) {
        return FDCAN_DLC_BYTES_12; // 9~12바이트 → 12바이트 DLC
    } else if (len_bytes <= 16U) {
        return FDCAN_DLC_BYTES_16; // 13~16바이트 → 16바이트 DLC
    } else if (len_bytes <= 20U) {
        return FDCAN_DLC_BYTES_20; // 17~20바이트
    } else if (len_bytes <= 24U) {
        return FDCAN_DLC_BYTES_24; // 21~24바이트
    } else if (len_bytes <= 32U) {
        return FDCAN_DLC_BYTES_32; // 25~32바이트
    } else if (len_bytes <= 48U) {
        return FDCAN_DLC_BYTES_48; // 33~48바이트
    } else {
        /* 49~64바이트는 전부 64바이트 DLC로 묶음 (최대 64바이트) */
        return FDCAN_DLC_BYTES_64;
    }
}

/* DLC 코드 → 실제 바이트 수 */
static uint8_t CAN_Length_FromDLC(uint32_t dlc) {
    switch (dlc) {
    case FDCAN_DLC_BYTES_0:
        return 0U;
    case FDCAN_DLC_BYTES_1:
        return 1U;
    case FDCAN_DLC_BYTES_2:
        return 2U;
    case FDCAN_DLC_BYTES_3:
        return 3U;
    case FDCAN_DLC_BYTES_4:
        return 4U;
    case FDCAN_DLC_BYTES_5:
        return 5U;
    case FDCAN_DLC_BYTES_6:
        return 6U;
    case FDCAN_DLC_BYTES_7:
        return 7U;
    case FDCAN_DLC_BYTES_8:
        return 8U;
    case FDCAN_DLC_BYTES_12:
        return 12U;
    case FDCAN_DLC_BYTES_16:
        return 16U;
    case FDCAN_DLC_BYTES_20:
        return 20U;
    case FDCAN_DLC_BYTES_24:
        return 24U;
    case FDCAN_DLC_BYTES_32:
        return 32U;
    case FDCAN_DLC_BYTES_48:
        return 48U;
    case FDCAN_DLC_BYTES_64:
        return 64U;
    default:
        return 0U;
    }
}

/*
 * CAN 초기화 (필터 및 Start)
 *  - Baudrate, Nominal/Data Timing 은 CubeMX 에서 500kbps 기준으로 설정해 두고
 *    이 함수에서는 필터와 Notification, Start 정도만 담당하는 형태를 권장.
 *  - 예)  0x1801xxxx (Extended ID) 대역을 허용하도록 필터 설정 가능.
 */
static int32_t Driver_CAN_Module_Initialize(uint32_t arg) {
    UNUSED(arg);

    if (Driver_CAN_Module_State & WD_STATE_INITIALIZED) {
        return WD_CODE_OK;
    }

    /* FDCAN 필터/BitTiming 추가 설정 */
    {
        FDCAN_FilterTypeDef sFilter;
        /* 1) 조합 패킷 ID 범위(0x18018000 ~ 0x18018FFF)만 RxFIFO0로 수신하는 확장 ID Range 필터 */
        memset(&sFilter, 0, sizeof(sFilter));
        sFilter.IdType = FDCAN_EXTENDED_ID;             /* 29-bit ID */
        sFilter.FilterIndex = 0;                        /* 필터 인덱스 0 사용 */
        sFilter.FilterType = FDCAN_FILTER_RANGE;        /* ID1 ~ ID2 범위 */
        sFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0; /* 매칭 시 Rx FIFO0에 저장 */
        sFilter.FilterID1 = CAN_COMPOSITE_BASE_ID;      /* 0x18018000 */
        sFilter.FilterID2 = CAN_COMPOSITE_LAST_ID;      /* 0x18018FFF */

        if (HAL_FDCAN_ConfigFilter(CAN_HANDLE, &sFilter) != HAL_OK) {
            WD_LOG_ERROR("CAN: ConfigFilter failed");
            return WD_CODE_ERROR;
        }

        /* 예시: Global Filter - 모든 메시지는 수신 허용 (프로토콜 레이어에서 ID 필터링) */
        if (HAL_FDCAN_ConfigGlobalFilter(CAN_HANDLE, FDCAN_REJECT, FDCAN_REJECT,
                                         FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE) != HAL_OK) {
            WD_LOG_ERROR("CAN: ConfigGlobalFilter failed");
            return WD_CODE_ERROR;
        }

        /* 프로젝트 정책에 맞게 29bit ID 필터 추가 (예: 0x18010000 ~ 0x1801FFFF) 위치 */
        /* HAL_FDCAN_ConfigFilter(...); */
    }

    if (HAL_FDCAN_Start(CAN_HANDLE) != HAL_OK) {
        WD_LOG_ERROR("CAN: Start failed");
        return WD_CODE_ERROR;
    }

    /* Rx FIFO0 새 메시지 인터럽트 활성화
	 *  - 지금 구조는 Thread에서 Polling(GET RxFifoFillLevel, GetRxMessage) 하므로
	 *    꼭 필요하지는 않지만, HAL 내부 상태 유지를 위해 켜두어도 무방.
	 */
    if (HAL_FDCAN_ActivateNotification(CAN_HANDLE, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
        WD_LOG_ERROR("CAN: ActivateNotification failed");
        return WD_CODE_ERROR;
    }

    /* 변수 초기화	*/
    {
        wd_fdcan_echo_mode = false; // 에코 모드 끄기
    }

    Driver_CAN_Module_State |= WD_STATE_INITIALIZED;

    WD_LOG_INFO("CAN: Initialized");

    return WD_CODE_OK;
}

/*
 * CAN 프레임 송신 (FDCAN 29-bit Extended ID, Data frame)
 *  - 조합(Composite) 패킷에서 사용하는 ID (0x1801xxxx)는 29bit Extended ID 이므로
 *    항상 FDCAN_EXTENDED_ID 를 사용.
 *  - 상위 프로토콜에서 1개 논리 패킷(최대 1024B)을 여러 프레임으로 쪼개어
 *    이 함수를 반복 호출하게 됨.
 */
static int32_t Driver_CAN_Module_Send(uint32_t id, const uint8_t *data, uint8_t len_bytes) {
    FDCAN_TxHeaderTypeDef txHeader = {0};
    uint32_t dlc_code;
    uint8_t dlc_bytes;
    uint8_t tx_buf[64];

    if (!(Driver_CAN_Module_State & WD_STATE_INITIALIZED)) {
        WD_LOG_ERROR("CAN: Send before Initialize");
        return WD_CODE_ERROR;
    }

    if ((data == NULL) && (len_bytes > 0U)) {
        WD_LOG_ERROR("CAN: Send data NULL");
        return WD_CODE_ERROR;
    }

    if (len_bytes > 64U) {
        WD_LOG_ERROR("CAN: Send len too large [%u]", (unsigned int)len_bytes);
        return WD_CODE_ERROR;
    }

    /* DLC 코드 및 실제 전송 바이트 수 계산 */
    dlc_code = CAN_DLC_FromLength(len_bytes);
    dlc_bytes = CAN_Length_FromDLC(dlc_code);

    if (dlc_bytes == 0U && len_bytes > 0U) {
        WD_LOG_ERROR("CAN: Invalid DLC calc [len=%u]", (unsigned int)len_bytes);
        return WD_CODE_ERROR;
    }

    /* 송신 버퍼 구성: 유효 데이터 + 패딩 0 */
    if ((data != NULL) && (len_bytes > 0U)) {
        memcpy(tx_buf, data, len_bytes);
    }
    if (dlc_bytes > len_bytes) {
        memset(&tx_buf[len_bytes], 0x00, (size_t)(dlc_bytes - len_bytes));
    }
    //	for(int i= 0; i < len_bytes; i++)
    //	{
    //		printf("%02x ",data[i]);
    //	}
    //	printf("\r\n");

    txHeader.Identifier = id;
    txHeader.IdType = FDCAN_EXTENDED_ID; /* 29-bit Extended ID 사용 */
    txHeader.TxFrameType = FDCAN_DATA_FRAME;
    txHeader.DataLength = dlc_code;
    txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    txHeader.BitRateSwitch = FDCAN_BRS_ON; //FDCAN_BRS_OFF;
    txHeader.FDFormat = FDCAN_FD_CAN;      /* FD 사용시 변경 가능 */
    txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    txHeader.MessageMarker = 0;

    if (HAL_FDCAN_AddMessageToTxFifoQ(CAN_HANDLE, &txHeader, tx_buf) != HAL_OK) {
        WD_LOG_ERROR("CAN: AddMessageToTxFifoQ failed");
        return WD_CODE_ERROR;
    }

    return WD_CODE_OK;
}

/*
 * CAN 프레임 수신 (Rx FIFO0에서 하나 읽기) – Polling용
 *  - Thread_CAN → wd_fdcan_data.Process() → Rx_Ops() → 여기로 들어옴.
 *  - 1회 호출당 최대 1개의 프레임만 읽고, 없으면 0 리턴.
 */
static int32_t Driver_CAN_Module_Receive(uint32_t *id, uint8_t *data, uint8_t *dlc) {
    FDCAN_RxHeaderTypeDef rxHeader = {0};

    if (!(Driver_CAN_Module_State & WD_STATE_INITIALIZED)) {
        return WD_CODE_ERROR;
    }

    if (HAL_FDCAN_GetRxFifoFillLevel(CAN_HANDLE, FDCAN_RX_FIFO0) == 0) {
        return 0; /* 수신 없음 */
    }

    if (HAL_FDCAN_GetRxMessage(CAN_HANDLE, FDCAN_RX_FIFO0, &rxHeader, data) != HAL_OK) {
        WD_LOG_ERROR("CAN: GetRxMessage failed");
        return WD_CODE_ERROR;
    }

    /* ID 복사 */
    if (id != NULL) {
        *id = rxHeader.Identifier;
    }
    uint8_t dlc_code = (uint8_t)rxHeader.DataLength;
    uint8_t len_bytes = CAN_Length_FromDLC(dlc_code);

    if (dlc != NULL) {
        *dlc = dlc_code;
    }

    CAN_RxCount++;

    return (int32_t)len_bytes;
}

/* 수신 카운트 반환 (옵션) */
static int32_t Driver_CAN_Module_GetRxCount(void) {
    return (int32_t)CAN_RxCount;
}

/*********************************************************************************************************************
 *  wd_fdcan_module_t instance
 *********************************************************************************************************************/

wd_fdcan_module_t wd_fdcan_module = {
    .Initialize = Driver_CAN_Module_Initialize,
    .Send = Driver_CAN_Module_Send,
    .Receive = Driver_CAN_Module_Receive,
    .GetRxCount = Driver_CAN_Module_GetRxCount,
    .State = &Driver_CAN_Module_State,
};

/*********************************************************************************************************************
 *  테스트용 외부 API (에코 & 단일 송신)
 *********************************************************************************************************************/

void wd_fdcan_test_set_echo_mode(uint8_t enable) {
    wd_fdcan_echo_mode = (enable ? 1U : 0U);
}

/*********************************************************************************************************************
 *  조합(Composite) 패킷 유틸 (18018000h ~ 18018FFFh)
 *********************************************************************************************************************/

/* 내부 상태만 리셋 (버퍼 데이터는 건드리지 않음) */
static void CAN_Composite_ClearState(void) {
    CAN_Composite_Rx.Expected = 0U;
    CAN_Composite_Rx.Received = 0U;
    CAN_Composite_Rx.BaseID = 0U;
    CAN_Composite_Rx.LastID = 0U;
    CAN_Composite_Rx.Active = 0U;
}

/* 완전 초기화 (버퍼까지 모두 0) - 오류 시 사용 */
static void CAN_Composite_Reset_Internal(void) {
    memset(&CAN_Composite_Rx, 0, sizeof(CAN_Composite_Rx));
}

/* 조합 패킷 ID인지 검사 */
static uint8_t CAN_Composite_IsCompositeID(uint32_t id) {
    return ((id >= CAN_COMPOSITE_BASE_ID) && (id <= CAN_COMPOSITE_LAST_ID)) ? 1U : 0U;
}

/*
 * 조합 패킷 수신기 (드라이버 내부용)
 *  - 입력:  id, data, len (단일 CAN 프레임)
 *  - 출력:
 *      out_len > 0 && return != NULL : 하나의 논리 패킷 완성됨
 *      out_len == 0                  : 아직 미완성 (계속 프레임을 받아야 함)
 */
static uint8_t *CAN_Composite_OnFrame(uint32_t id, const uint8_t *data, uint8_t len,
                                      uint16_t *out_len) {
    CAN_COMPOSITE_RX_CTX *ctx = &CAN_Composite_Rx;

    if (out_len != NULL) {
        *out_len = 0U;
    }

    /* 조합 패킷 ID 영역이 아니면 현재 조합 상태를 초기화하고 종료 */
    if (!CAN_Composite_IsCompositeID(id)) {
        CAN_Composite_Reset_Internal();
        return NULL;
    }

    /* 새 조합 패킷 시작 조건:
	 *  - 아직 활성 상태가 아니거나
	 *  - Base ID(0x18018000) 프레임이 들어온 경우
	 *  - ID가 비정상적으로 점프한 경우(간단히 LastID+1만 허용) → 새로 시작
	 */
    if (!ctx->Active || id == CAN_COMPOSITE_BASE_ID) {
        CAN_Composite_Reset_Internal();
        ctx->Active = 1U;
        ctx->BaseID = CAN_COMPOSITE_BASE_ID;
        ctx->LastID = id;
    } else {
        ctx->LastID = id;
    }

    /* Expected 길이를 기준으로 이번 프레임에서 실제로 사용할 유효 길이 결정
	*  - 아직 Expected를 모를 때(ctx->Expected == 0U)는 len 전체 사용
	*  - Expected가 설정된 이후에는 (Expected - Received)까지만 사용
	*    → 마지막 프레임이 패딩(0x00) 포함 64바이트여도,
	*       남은 바이트 수만큼만 조합 버퍼에 append
	*/
    if (ctx->Expected > 0U) {
        if (ctx->Received >= ctx->Expected) {
            /* 이미 한 패킷을 다 받았다면, 남는 프레임은 무시 */
            return NULL;
        }

        uint16_t remaining = (uint16_t)(ctx->Expected - ctx->Received);
        if (len > remaining) {
            len = (uint8_t)remaining;
        }
    }

    /* 버퍼 오버런 방지 (패딩을 잘라낸 len 기준으로 검사) */
    if (((uint32_t)ctx->Received + (uint32_t)len) > CAN_COMPOSITE_MAX_PACKET) {
        CAN_Composite_Reset_Internal();
        return NULL;
    }

    /* 데이터 이어붙이기 (len은 이미 남은 길이에 맞게 잘려 있음) */
    if ((data != NULL) && (len > 0U)) {
        memcpy(&ctx->Buffer[ctx->Received], data, len);
        ctx->Received = (uint16_t)(ctx->Received + (uint16_t)len);
    }


    /* 헤더의 Data Length 를 보고 전체 패킷 길이 계산
	*  - 형식: STX(1) + Len(2) + Type(1) + Body(n) + CRC(2)
	*  - Len = Body(Data) 길이 (최대 1000B)
	*  - 아직 Expected가 0이고, 최소 3바이트(STX + Len2)를 받은 시점에 한 번만 계산
	*/
    if ((ctx->Expected == 0U) && (ctx->Received >= 3U)) {
        uint16_t body_len = (uint16_t)((uint16_t)ctx->Buffer[1] | ((uint16_t)ctx->Buffer[2] << 8));
        uint16_t total_len = (uint16_t)(body_len + 1U); /* STX + Len2 + Type + Body + CRC2 */

        if ((body_len < 5U) || (total_len == 0U) || (total_len > CAN_COMPOSITE_MAX_PACKET)) {
            CAN_Composite_Reset_Internal();
            return NULL;
        }

        ctx->Expected = total_len;
    }

    /* 아직 전체 길이를 모른다면 계속 수신 */
    if (ctx->Expected == 0U) {
        return NULL;
    }

    /* 한 논리 패킷 완성 */
    if (ctx->Received >= ctx->Expected) {
        uint16_t packet_len = ctx->Expected;

        /* 상태만 초기화 (버퍼는 그대로 둔다) */
        CAN_Composite_ClearState();

        if (out_len != NULL) {
            *out_len = packet_len;
        }
        /* 버퍼의 앞쪽 packet_len 바이트만 유효 데이터 */
        return ctx->Buffer;
    }
    return NULL;
}

/*
 * 조합 패킷 송신
 *  - STX로 시작하는 완성 패킷(packet, packet_len)을 64B 단위로 나눠
 *  - 실제 데이터가 64보다 작으면 나머지는 0x00으로 패딩
 *    18018000h + n ID 로 보내준다.
 */
int32_t wd_fdcan_send_composite(uint32_t base_id, const uint8_t *packet, uint16_t packet_len) {
    if ((packet == NULL) || (packet_len == 0U) || (packet_len > WD_CAN_COMPOSITE_PACKET_MAX)) {
        return WD_CODE_ERROR;
    }

    /* base_id 가 범위 밖이면 기본값 사용 */
    if ((base_id < WD_CAN_COMPOSITE_ID_BASE) || (base_id > WD_CAN_COMPOSITE_ID_MAX)) {
        base_id = WD_CAN_COMPOSITE_ID_BASE;
    }

    uint16_t offset = 0U;
    uint32_t seg_index = 0U;

    while (offset < packet_len) {
        uint16_t remain = (uint16_t)(packet_len - offset);
        uint8_t chunk_len = (remain > WD_CAN_COMPOSITE_FRAME_DATA_MAX)
                                ? WD_CAN_COMPOSITE_FRAME_DATA_MAX
                                : (uint8_t)remain;

        uint32_t id = base_id + seg_index;

        /* 유효 데이터 길이(chunk_len)를 그대로 넘기면,
		 * Driver_CAN_Module_Send 내부에서 DLC 계산 + 패딩 처리.
		 */
        if (wd_fdcan_module.Send(id, &packet[offset], chunk_len) != WD_CODE_OK) {
            return WD_CODE_ERROR;
        }

        offset = (uint16_t)(offset + (uint16_t)chunk_len);
        seg_index++;

        if (seg_index > WD_CAN_COMPOSITE_SEGMENT_MAX) {
            /* 너무 많은 세그먼트 사용 → 비정상 */
            return WD_CODE_ERROR;
        }
    }

    return WD_CODE_OK;
}

/*********************************************************************************************************************
 *  wd_network_driver_t 계층
 *********************************************************************************************************************/
#undef LOCAL_PREFIX
#define LOCAL_PREFIX_NAME  wd_fdcan_data
#define LOCAL_PREFIX(name) driver_can_##name

#undef LOCAL_DRV_TYPEDEF
#define LOCAL_DRV_TYPEDEF             wd_fdcan_module_t
#define LOCAL_DRV_CASTING(name, port) LOCAL_DRV_TYPEDEF *name = (LOCAL_DRV_TYPEDEF *)((port)->Drv)

#undef LOCAL_CAPABILITIES_TYPEDEF
#define LOCAL_CAPABILITIES_TYPEDEF wd_fdcan_capabilities_t
#define LOCAL_CAPABILITIES_CASTING(name, port) \
    LOCAL_CAPABILITIES_TYPEDEF *name = (LOCAL_CAPABILITIES_TYPEDEF *)((port)->Capabilities->Private)

#undef LOCAL_INFO_TYPEDEF
#define LOCAL_INFO_TYPEDEF wd_fdcan_info_t
#define LOCAL_INFO_CASTING(name, port) \
    LOCAL_INFO_TYPEDEF *name = (LOCAL_INFO_TYPEDEF *)((port)->Private)

/* Register: Port 초기 설정 + CAN 모듈 Initialize 호출 */
static int32_t LOCAL_PREFIX(Register)(wd_network_port_t *port) {
    LOCAL_DRV_CASTING(drv, port);
    LOCAL_INFO_CASTING(info, port);

    UNUSED(drv);

    memset(info, 0, sizeof(*info));

    /* 공통 Xfer 버퍼 연결 */
    port->Info->Xfer.Tx_Buf = port->Tx_Buf;
    port->Info->Xfer.Tx_Buf_Size = port->Tx_Buf_Size;
    port->Info->Xfer.Rx_Buf = port->Rx_Buf;
    port->Info->Xfer.Rx_Buf_Size = port->Rx_Buf_Size;
    port->Info->Wait_Timer = 0;
    port->Info->Time_Stamp = 0;
    port->Info->Tx_Timeout = 0;
    port->Info->Rx_Timeout = 0;

    port->Info->Msg_Queue =
        osMessageQueueNew(port->Msg_Queue_Size, sizeof(wd_message_t), port->Msg_Queue_Attr);
    osMessageQueueReset(port->Info->Msg_Queue);

    /* CAN 모듈 초기화 */
    if (!(Driver_CAN_Module_State & WD_STATE_INITIALIZED)) {
        if (wd_fdcan_module.Initialize(WDIF_ARG_NONE) != WD_CODE_OK) {
            WD_LOG_ERROR("CAN: Driver Initialize failed");
            return WD_CODE_ERROR;
        }
    }

    port->Info->Status.Initialized = true;

    WD_LOG_INFO("CAN: Net Port Registered");

    return WD_CODE_OK;
}

/* Unregister: 스켈레톤 – 필요한 경우 DeInit 추가 */
static int32_t LOCAL_PREFIX(Unregister)(wd_network_port_t *port) {
    port->Info->Status.Initialized = false;
    return WD_CODE_OK;
}

/*
 * Tx_Ops: 메시지 큐에서 CAN 프레임 전송
 *  - Ethernet과 동일하게 MESSAGE 큐를 사용.
 *  - 규약:
 *      msg.Ptr  : CAN 데이터 버퍼 (최대 64바이트)
 *      msg.ID   : CAN Identifier (Extended 29bit)
 *      msg.Size : 유효 데이터 길이 (0 < Size <= 64)
 */
static void LOCAL_PREFIX(Tx_Ops)(wd_network_port_t *port) {
    LOCAL_DRV_CASTING(drv, port);
    LOCAL_INFO_CASTING(info, port);

    if (port->Info->Status.Tx_Busy) {
        return;
    }

    wd_message_t msg;
    osStatus_t status = osMessageQueueGet(port->Info->Msg_Queue, &msg, NULL, 0);

    if (status != osOK) {
        return;
    }

    if (msg.Size == 0U || msg.Size > 64U) {
        WD_LOG_ERROR("CAN: Tx message size invalid [%lu]", (unsigned long)msg.Size);
        return;
    }

    if (drv->Send(msg.ID, (const uint8_t *)msg.Ptr, (uint8_t)msg.Size) != WD_CODE_OK) {
        info->ErrorCount++;
        WD_LOG_ERROR("CAN: Tx failed");
        return;
    }

    port->Info->Status.Tx_Busy = true;
    /* Tx_Timeout 시작 시각은 Process()에서 arg(time)를 통해 설정 */
    info->TxCount++;
}

/* Tx_Complete: 송신 완료 후 상태 리셋 (지금 구조에선 Polling 기반이므로 필요시 상위에서 직접 호출) */
static void LOCAL_PREFIX(Tx_Complete)(wd_network_port_t *port) {
    port->Info->Status.Tx_Busy = false;
    port->Info->Event |= WDIF_EVENT_SEND_COMPLETE;
    port->Info->Tx_Timeout = 0U;
}

/*
 * Rx_Ops: Polling 기반 수신 처리
 *  - wd_fdcan_module.Receive() 로 1프레임 읽음
 *  - 조합 패킷 ID(0x18018000~0x18018FFF)면 CAN_Composite_OnFrame()으로 누적
 *  - 한 논리 패킷이 완성되면 port->Info->Xfer.Rx_Buf 에 넣고 Rx_Busy 세트
 *  - Echo 모드일 때는 수신한 프레임 그대로 즉시 송신
 */
static int32_t LOCAL_PREFIX(Rx_Ops)(wd_network_port_t *port) {
    LOCAL_DRV_CASTING(drv, port);
    LOCAL_INFO_CASTING(info, port);

    uint32_t id = 0;
    uint8_t dlc = 0;
    int32_t len;
    uint8_t frame_buf[64];
    uint16_t assembled_len = 0;
    uint8_t *assembled_ptr = NULL;

    if (port->Info->Xfer.Rx_Buf_Size < 6U) {
        return WD_CODE_ERROR;
    }

    /* 단일 CAN 프레임 수신 */
    len = drv->Receive(&id, frame_buf, &dlc);
    if (len <= 0) {
        return WD_CODE_OK; /* 수신 없음 */
    }

    info->RxCount++;

    /* === 에코 모드: 들어온 프레임 그대로 다시 송신 ====================== */
    if (wd_fdcan_echo_mode) {
        if (drv->Send(id, frame_buf, (uint8_t)len) != WD_CODE_OK) {
            info->ErrorCount++;
            WD_LOG_ERROR("CAN: Echo send failed");
        }
    }

    /* === 조합 패킷 ID이면 조합 버퍼에 누적 ====================== */
    if (CAN_Composite_IsCompositeID(id)) {
        assembled_ptr = CAN_Composite_OnFrame(id, frame_buf, (uint8_t)len, &assembled_len);

        /* 아직 한 패킷이 완성되지 않은 상태 */
        if ((assembled_ptr == NULL) || (assembled_len == 0U)) {
            return WD_CODE_OK;
        }

        /* 완성된 논리 패킷 길이 검사 */
        if (assembled_len > port->Info->Xfer.Rx_Buf_Size) {
            info->ErrorCount++;
            WD_LOG_ERROR("CAN: Composite packet too large [%u]", assembled_len);
            return WD_CODE_ERROR;
        }

        /* 완성된 논리 패킷을 Net 레이어 Rx 버퍼로 복사 */
        memcpy(port->Info->Xfer.Rx_Buf, assembled_ptr, assembled_len);
        port->Info->Xfer.Rx_Cnt = (uint32_t)assembled_len;
        port->Info->Xfer.Rx_Offset = 0;
        port->Info->Status.Rx_Busy = true;

        return WD_CODE_OK;
    }

    /* === 일반 ID 프레임은 그대로 한 패킷으로 취급 =========================== */
    if ((uint32_t)len > port->Info->Xfer.Rx_Buf_Size) {
        info->ErrorCount++;
        WD_LOG_ERROR("CAN: Rx frame too large [%ld]", (long)len);
        return WD_CODE_ERROR;
    }

    memcpy(port->Info->Xfer.Rx_Buf, frame_buf, (uint32_t)len);
    port->Info->Xfer.Rx_Cnt = (uint16_t)len;
    port->Info->Xfer.Rx_Offset = 0;
    port->Info->Status.Rx_Busy = true;

    return WD_CODE_OK;
}

/* Rx_Complete: 수신 버퍼를 port->Parser 로 넘기는 스켈레톤 */
static int32_t LOCAL_PREFIX(Rx_Complete)(wd_network_port_t *port) {
    uint32_t parsed_idx = 0;

    if (!port->Info->Status.Rx_Busy) {
        return WD_CODE_OK;
    }

    if (port->Parser != NULL) /* parser of the owning service */
    {
        parsed_idx = (uint32_t)port->Parser(port, port->Info->Xfer.Rx_Buf,
                                            port->Info->Xfer.Rx_Buf + port->Info->Xfer.Rx_Cnt, 0);
    }

    /* 단순 스켈레톤: 전체를 한번에 처리했다고 가정 */
    port->Info->Xfer.Rx_Cnt = 0;
    port->Info->Xfer.Rx_Offset = 0;
    port->Info->Status.Rx_Busy = false;

    (void)parsed_idx;

    return WD_CODE_OK;
}

/* Rx_Flush: 남은 데이터 버리기 (CAN에서는 단순히 카운터 초기화) */
static void LOCAL_PREFIX(Rx_Flush)(wd_network_port_t *port, uint16_t parsed_data_idx) {
    UNUSED(parsed_data_idx);

    port->Info->Xfer.Rx_Cnt = 0;
    port->Info->Xfer.Rx_Offset = 0;
}

/*
 * Timeout: Tx 타임아웃 처리
 *  - time: Thread_CAN 에서 넘긴 현재 tick (예: Thread_CAN_Timer)
 *  - Tx_Busy 가 세트된 상태에서 Tx_Wait_Timer 를 초과하면 강제로 해제
 */
static int32_t LOCAL_PREFIX(Timeout)(wd_network_port_t *port, uint32_t time) {
    LOCAL_INFO_CASTING(info, port);

    if (port->Info->Status.Tx_Busy) {
        uint32_t start = port->Info->Tx_Timeout;
        uint32_t limit = port->Capabilities->Tx_Wait_Timer;

        if (start == 0U) {
            /* Tx 시작 시각 기록 */
            port->Info->Tx_Timeout = time;
        } else {
            uint32_t elapsed = time - start;
            if (elapsed > limit) {
                port->Info->Status.Tx_Busy = false;
                port->Info->Tx_Timeout = 0U;
                info->ErrorCount++;
                WD_LOG_ERROR("CAN: Tx timeout (>%lu ms)", (unsigned long)limit);
            }
        }
    }

    return WD_CODE_OK;
}

/* Error: 에러 발생시 호출 (스켈레톤) */
static void LOCAL_PREFIX(Error)(wd_network_port_t *port) {
    LOCAL_INFO_CASTING(info, port);

    info->ErrorCount++;
}

/* Check: 링크/상태 확인 (CAN에서는 항상 OK로 가정, 필요시 확장) */
static int32_t LOCAL_PREFIX(Check)(wd_network_port_t *port) {
    UNUSED(port);
    return WD_CODE_OK;
}

/*
 * Process: 주기적으로 호출되는 처리 루틴
 *  - FreeRTOS Thread_CAN 에서
 *      wd_fdcan_data.Process(&wd_net_can_net_port_can, (uint32_t)Thread_CAN_Timer);
 *    형태로 호출됨.
 *  - Tx_Ops 처리 후, Rx_Ops 호출
 *  - Rx_Ops 에서 Rx_Busy 가 세트되면 바로 Rx_Complete 를 호출하여 wd_net_protocol_parser 로 전달
 */
static void LOCAL_PREFIX(Process)(wd_network_port_t *port, uint32_t arg) {
    UNUSED(arg);

    /* 송신 처리 */
    LOCAL_PREFIX(Tx_Ops)(port);

    /* 수신 처리 */
    if (LOCAL_PREFIX(Rx_Ops)(port) == WD_CODE_OK) {
        /* 조합 패킷/단일 프레임으로 인해 Rx_Busy 가 세트되었으면 Parser 호출 */
        if (port->Info->Status.Rx_Busy && (port->Info->Xfer.Rx_Cnt > 0U)) {
            (void)LOCAL_PREFIX(Rx_Complete)(port);
        }
    }
}

/* Get_Priv: driver private pointer 반환 */
static void *LOCAL_PREFIX(Get_Priv)(wd_network_port_t *port) {
    return port->Private;
}

/* wd_network_driver_t 인스턴스 (wd_fdcan_data) */
wd_network_driver_t LOCAL_PREFIX_NAME = {
    .Register = LOCAL_PREFIX(Register),
    .Unregister = LOCAL_PREFIX(Unregister),
    .Tx_Ops = LOCAL_PREFIX(Tx_Ops),
    .Tx_Complete = LOCAL_PREFIX(Tx_Complete),
    .Rx_Ops = LOCAL_PREFIX(Rx_Ops),
    .Rx_Complete = LOCAL_PREFIX(Rx_Complete),
    .Rx_Flush = LOCAL_PREFIX(Rx_Flush),
    .Timeout = LOCAL_PREFIX(Timeout),
    .Error = LOCAL_PREFIX(Error),
    .Check = LOCAL_PREFIX(Check),
    .Process = LOCAL_PREFIX(Process),
    .Get_Priv = LOCAL_PREFIX(Get_Priv),
};

#endif // (RTE_COMPONENT_CAN)

#endif // #if 1 // 토글
