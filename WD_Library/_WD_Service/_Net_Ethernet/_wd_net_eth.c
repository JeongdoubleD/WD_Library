/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _wd_net_eth.c
 *
 *  Created on: Jan 7, 2025
 *      Author: doubled
 */

#include "wd.h"

#if (RTE_SERVICE_NET_ETHERNET)

#undef LOCAL_PREFIX_NAME
#define LOCAL_PREFIX_NAME "ETH"

#define SPI_HANDLE RTE_COMPONENT_INTERFACE_ETHERNET_DRIVER_HANDLE

/////////APP////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static uint8_t Net_Ethernet_Queue_stack_mem[(sizeof(wd_message_t) * RTE_SERVICE_NET_QUEUE_SIZE)];
static osStaticMutexDef_t Net_Ethernet_Queue_cb_mem;
static const osMessageQueueAttr_t NET_ETHERNET_QUEUE_ATTR = {
    .name = "Ethernet Message Queue",
    .cb_mem = &Net_Ethernet_Queue_cb_mem,
    .cb_size = sizeof(Net_Ethernet_Queue_cb_mem),
    .mq_mem = Net_Ethernet_Queue_stack_mem,
    .mq_size = sizeof(Net_Ethernet_Queue_stack_mem)};

#if 0
static WD_DRIVER_ETHERNET_RS485_CAPABILITIES NET_ETHERNET_RS485_CAPABILITIES =
{
  .DE = NULL,
  .RE = NULL,
  .Octet_Timeout = 1
};
#endif

static wd_eth_capabilities_t NET_ETHERNET_CAPABILITIES = {
#if 0
  .MAC       = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED },
  .IPv4      = {
                .IP           = { 169, 254,  17,   2 },
                .Subnet_Mask  = { 255, 255, 255,   0 },
                .Gateway      = { 169, 254,  17,   1 },
                .DNS          = {   8,   8,   8,   8 }
               },
#endif
    .IP_Protocol = IPPROTO_TCP,
    .Sock_Descriptor = RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_APP,
    .Extend = NULL};

static const wd_network_capabilities_t NET_PORT_ETHERNET_CAPABILITIES = {
    .Info = LOCAL_PREFIX_NAME,
    .Tx_Wait_Timer = 0,
    .Rx_Inter_Octet_Timeout = RTE_SERVICE_NET_RX_INTER_OCTET_TIMEOUT,
    .Rx_Frame_Timeout = RTE_SERVICE_NET_RX_FRAME_TIMEOUT,
    .Private = (void *)(&NET_ETHERNET_CAPABILITIES)};

static wd_net_eth_t Net_Ethernet = {
    0,
};

static void Net_Ethernet_Event_Callback(uint32_t event) {
    Net_Ethernet.Drv_Info.Event |= event;
}

static const wd_net_protocol_capabilities_t NET_PORT_ETHERNET_PROTOCOL_CAPABILITIES = {
    .ID = WD_SYMBOL_02_NET_ETHERNET,
    .Available_Type =
        WD_PROTOCOL_TYPE_FACTORY | WD_PROTOCOL_TYPE_USER /* | WD_PROTOCOL_TYPE_TEST_ECHO*/,
};

static wd_net_protocol_profile_t Net_Ethernet_Protocol_Profile;
#if (RTE_COMPONENT_PROTOCOL_FACTORY)
static wd_factory_t Net_Ethernet_Protocol_Factory;
#endif

wd_net_protocol_resources_t wd_net_eth_protocol = {
    .Capabilities = &NET_PORT_ETHERNET_PROTOCOL_CAPABILITIES,
    .Profile = &Net_Ethernet_Protocol_Profile,
#if (RTE_COMPONENT_PROTOCOL_FACTORY)
    .Factory = &Net_Ethernet_Protocol_Factory,
#endif
};

wd_network_port_t wd_net_eth_net_port_ethernet = // △ 02_Service #001 wd_net_eth_net_port_ethernet
    {.ID = WD_SYMBOL_02_NET_ETHERNET,
     .Capabilities = &NET_PORT_ETHERNET_CAPABILITIES,
     .Signal_Event = Net_Ethernet_Event_Callback,
     .Tx_Buf = Net_Ethernet.Tx_Buf,
     .Tx_Buf_Size = sizeof(Net_Ethernet.Tx_Buf),
     .Rx_Buf = Net_Ethernet.Rx_Buf,
     .Rx_Buf_Size = sizeof(Net_Ethernet.Rx_Buf),
     .Msg_Queue_Attr = &NET_ETHERNET_QUEUE_ATTR,
     .Msg_Queue_Size = RTE_SERVICE_NET_QUEUE_SIZE,
     .Drv = (void const *)&wd_eth_driver_ethernet_module,
     .Handle = (void *)SPI_HANDLE,
     .Protocol = (void const *)&wd_net_eth_protocol,
     .Parser = wd_net_protocol_parser,
     .Info = (void *)&Net_Ethernet.Drv_Info,
     .Private = (void *)&Net_Ethernet.Drv_Info_Private};

#if (RTE_COMPONENT_INTERFACE_ETHERNET_DEBUG == 1)
static osStaticMutexDef_t Net_Ethernet_Debug_Queue_cb_mem;
static uint8_t
    Net_Ethernet_Debug_Queue_stack_mem[(sizeof(wd_message_t) * RTE_SERVICE_NET_QUEUE_SIZE)];
static const osMessageQueueAttr_t NET_ETHERNET_DEBUG_QUEUE_ATTR = {
    .name = "Ethernet Debug Message Queue",
    .cb_mem = &Net_Ethernet_Debug_Queue_cb_mem,
    .cb_size = sizeof(Net_Ethernet_Debug_Queue_cb_mem),
    .mq_mem = Net_Ethernet_Debug_Queue_stack_mem,
    .mq_size = sizeof(Net_Ethernet_Debug_Queue_stack_mem)};


static wd_eth_capabilities_t NET_ETHERNET_DEBUG_CAPABILITIES = {
#if 0
  .MAC       = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED },
  .IPv4      = {
                .IP           = { 169, 254,  17,   2 },
                .Subnet_Mask  = { 255, 255, 255,   0 },
                .Gateway      = { 169, 254,  17,   1 },
                .DNS          = {   8,   8,   8,   8 }
               },
#endif
    .IP_Protocol = IPPROTO_TCP,
    .Sock_Descriptor = RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_SERVER_DEBUG,
    .Extend = NULL};

static const wd_network_capabilities_t NET_PORT_ETHERNET_DEBUG_CAPABILITIES = {
    .Info = LOCAL_PREFIX_NAME,
    .Tx_Wait_Timer = 0,
    .Rx_Inter_Octet_Timeout = RTE_SERVICE_NET_RX_INTER_OCTET_TIMEOUT,
    .Rx_Frame_Timeout = RTE_SERVICE_NET_RX_FRAME_TIMEOUT,
    .Private = (void *)(&NET_ETHERNET_DEBUG_CAPABILITIES)};


static wd_net_eth_t Net_Ethernet_Debug = {
    0,
};

static void Net_Ethernet_Debug_Event_Callback(uint32_t event) {
    Net_Ethernet_Debug.Drv_Info.Event |= event;
}

static const wd_net_protocol_capabilities_t NET_PORT_ETHERNET_DEBUG_PROTOCOL_CAPABILITIES = {
    .ID = WD_SYMBOL_02_NET_ETHERNET,
    .Available_Type =
        WD_PROTOCOL_TYPE_FACTORY | WD_PROTOCOL_TYPE_USER /* | WD_PROTOCOL_TYPE_TEST_ECHO*/,
};

static wd_net_protocol_profile_t Net_Ethernet_Debug_Protocol_Profile;
#if (RTE_COMPONENT_PROTOCOL_FACTORY)
static wd_factory_t Net_Ethernet_Debug_Protocol_Factory;
#endif

static wd_net_protocol_resources_t Net_Ethernet_Debug_Protocol = {
    .Capabilities = &NET_PORT_ETHERNET_DEBUG_PROTOCOL_CAPABILITIES,
    .Profile = &Net_Ethernet_Debug_Protocol_Profile,
#if (RTE_COMPONENT_PROTOCOL_FACTORY)
    .Factory = &Net_Ethernet_Debug_Protocol_Factory,
#endif
};


wd_network_port_t
    wd_net_eth_net_port_ethernet_debug = // △ 02_Service #001 wd_net_eth_net_port_ethernet_debug
    {.ID = WD_SYMBOL_02_NET_ETHERNET,
     .Capabilities = &NET_PORT_ETHERNET_DEBUG_CAPABILITIES,
     .Signal_Event = Net_Ethernet_Debug_Event_Callback,
     .Tx_Buf = Net_Ethernet_Debug.Tx_Buf,
     .Tx_Buf_Size = sizeof(Net_Ethernet_Debug.Tx_Buf),
     .Rx_Buf = Net_Ethernet_Debug.Rx_Buf,
     .Rx_Buf_Size = sizeof(Net_Ethernet_Debug.Rx_Buf),
     .Msg_Queue_Attr = &NET_ETHERNET_DEBUG_QUEUE_ATTR,
     .Msg_Queue_Size = RTE_SERVICE_NET_QUEUE_SIZE,
     .Drv = (void const *)&wd_eth_driver_ethernet_module,
     .Handle = (void *)SPI_HANDLE,
     .Protocol = (void const *)&Net_Ethernet_Debug_Protocol,
     .Parser = wd_net_protocol_parser,
     .Info = (void *)&Net_Ethernet_Debug.Drv_Info,
     .Private = (void *)&Net_Ethernet_Debug.Drv_Info_Private};
#endif

void wd_net_eth_open(void) {
    wd_eth_data.Register(&wd_net_eth_net_port_ethernet);
    wd_net_eth_net_port_ethernet.Info->Status.Power = true;
    wd_eth_data.Check(&wd_net_eth_net_port_ethernet);
    wd_net_eth_net_port_ethernet.Info->Status.Configure = true;

    wd_net_eth_protocol.Profile->Current_Type = wd_net_eth_protocol.Capabilities->Available_Type;

#if (RTE_COMPONENT_PROTOCOL_FACTORY)
#endif
    if (wd_net_eth_protocol.Profile->Current_Type & WD_PROTOCOL_TYPE_USER) {
        wd_net_initialize_user_protocol(&wd_net_eth_net_port_ethernet);
    }


#if (RTE_COMPONENT_INTERFACE_ETHERNET_DEBUG == 1)
    wd_eth_data.Register(&wd_net_eth_net_port_ethernet_debug);
    wd_net_eth_net_port_ethernet_debug.Info->Status.Power = true;
    wd_eth_data.Check(&wd_net_eth_net_port_ethernet_debug);
    wd_net_eth_net_port_ethernet_debug.Info->Status.Configure = true;

    Net_Ethernet_Debug_Protocol.Profile->Current_Type =
        Net_Ethernet_Debug_Protocol.Capabilities->Available_Type;

#if (RTE_COMPONENT_PROTOCOL_FACTORY)
#endif
    if (Net_Ethernet_Debug_Protocol.Profile->Current_Type & WD_PROTOCOL_TYPE_USER) {
        wd_net_initialize_user_protocol(&wd_net_eth_net_port_ethernet_debug);
    }

#endif
}

void wd_net_eth_close(void) {
}

void wd_net_eth_report(void) {
}


#if 0 //(RTE_COMPONENT_DRIVER_ETHERNET_DEBUG == 2) || (RTE_COMPONENT_INTERFACE_ETHERNET_TELNET)
static uint8_t Driver_Ethernet_Socket_Buf[2048];
#endif

#if 0 //(RTE_COMPONENT_INTERFACE_ETHERNET_TELNET)
/***************************************************************************************************************************************************************
 **********  Telnet
 **************************************************************************************************************************************************************/
void wd_eth_write_telnet (const void *data, uint32_t num, uint32_t arg)
{
  int32_t ret;
  datasize_t sentsize=0;
  uint8_t *buf = (uint8_t *)((uint32_t) data);
  uint32_t __time_start = wd_time_get_us();

  UNUSED(arg);

  if(wd_eth_socket[RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_CONSOLE].Status != SOCK_ESTABLISHED)
  {
    WD_LOG_ERROR("Socket is not ESTABLISHED");
    return;
  }

  while((datasize_t)num != sentsize)
  {
    if(wd_time_calc_us(&__time_start) > 1000000)
    {
      WD_LOG_ERROR("Socket Send TimeOut");
      return;
    }

    ret = send(RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_CONSOLE, buf + sentsize, (datasize_t)num - sentsize);
    if(ret < 0)
    {
      close(RTE_COMPONENT_INTERFACE_ETHERNET_SOCKET_NUMBER_TCP_CONSOLE);
      WD_LOG_ERROR("Socket Error [%s]", wd_eth_socket_error_msg_box(ret));
      return;
    }
    sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
  }
}

int32_t wd_eth_telnet_ops (uint8_t sock_num)
{
  if(wd_eth_tcp_ops(sock_num) != WD_CODE_OK)
  {
    return (WD_CODE_ERROR);
  }

  if(wd_eth_socket[sock_num].Status == SOCK_ESTABLISHED)
  {
    uint16_t rx_cnt;

    rx_cnt = (uint16_t)wd_eth_get_rx_count(sock_num);

    if(rx_cnt > 0)
    {
      if(recv(sock_num, Driver_Ethernet_Socket_Buf, (datasize_t)rx_cnt) <= 0)
      {
        WD_LOG_ERROR();
      }
      else
      {
#if (RTE_PLATFORM_PRINT_TELNET)
        wd_cbuf_push(&wd_eth_telnet_rx_c_buf, Driver_Ethernet_Socket_Buf, rx_cnt);
#endif
      }
    }

    {
      datasize_t tx_free_size;

      getsockopt(sock_num,SO_SENDBUF,&tx_free_size);

#if (RTE_PLATFORM_PRINT_TELNET)
      if((tx_free_size >= 2048) && wd_eth_telnet_tx_c_buf.Size)
      {
        if(    (wd_time_calc_us(&wd_eth_telnet_tx_time_stamp) > 50000)
            || (wd_eth_telnet_tx_c_buf.Size > 1900)
          )
        {
          wd_eth_telnet_flush_telnet(NULL);
        }
      }
#endif
    }

  }

  return (WD_CODE_OK);
}
#endif

#endif
