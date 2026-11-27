/* SPDX-License-Identifier: MIT
 * Copyright (c) 2025-2026 DoubleD
 */
/*
 * _Driver_Blcok_Device.c
 *
 *  Created on: Feb 7, 2025
 *      Author: doubled
 */

#include "_wd_blockdev.h"

#if 1 // 토글

#if (RTE_DRIVERS_BLOCK_DEVICE)
uint32_t wd_blockdev_retry_error_e_w = 0;
uint32_t wd_blockdev_retry_error_r = 0;

static int32_t block_device_driver_initialize(wd_blockdev_instance_t *instance,
                                              wd_event_callback_t cb_event) {
    WD_PRINT_INFO(WD_MSG_INFO_LOG, "%s _Block_Device_Driver_Initialize Constructor",
                  instance->Res->Capabilities.Name);

    if (instance->Initialize(cb_event) < 0) {
        return (WD_CODE_ERROR);
    }

    if (instance->Res->IO.WP != RTE_CONFIG_IDLE_PIN_NC) {
        wd_gpio_config(instance->Res->IO.WP, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
    }

    if (instance->PowerControl(WD_POWER_FULL) < 0) {
        return (WD_CODE_ERROR);
    }

    if (instance->Res->IO.WP != RTE_CONFIG_IDLE_PIN_NC) {
        wd_gpio_write(instance->Res->IO.WP, WD_GPIO_ON);
    }

    return (WD_CODE_OK);
}

static int32_t block_device_driver_handle(wd_blockdev_instance_t *instance, uint32_t addr,
                                          const uint8_t *data, uint32_t num, uint32_t arg,
                                          uint32_t job) {
    int32_t ret = WD_CODE_OK;

    uint32_t retry = 0;
    uint32_t offset = 0;
    uint32_t tx_num = 0;
    uint32_t remaining = 0;

    uint32_t erase_remaining = 0;
    uint32_t erase_address = 0;
    uint32_t erase_size = 0;

    uint32_t tx_offset = 0;
    uint32_t tx_cnt_num = 0;

    uint32_t erase_bulk = false;
    uint32_t erase = false;
    uint32_t write = false;

    UNUSED(arg);

    if (instance == NULL) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }


    if ((!WD_RANGE(instance->Res->Capabilities.Addr, addr,
                   instance->Res->Capabilities.Addr + instance->Res->Capabilities.Size)) ||
        (!WD_RANGE(instance->Res->Capabilities.Addr, addr + num,
                   instance->Res->Capabilities.Addr + instance->Res->Capabilities.Size))) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

#if (RTE_RTOS)
    instance->Res->Lock();
#endif

    if (instance->Res->IO.WP != RTE_CONFIG_IDLE_PIN_NC) {
        wd_gpio_config(instance->Res->IO.WP, WD_GPIO_FUNCTION_AUTO | WD_GPIO_OFF);
    }

    if (instance->Res->Capabilities.Direct_Access != 0U) {
        if (instance->Res->Capabilities.Buf == NULL && instance->Res->Capabilities.Buf_Size == 0) {
            if (job == WD_BLOCK_DEVICE_ARGUMENT_WRITE) {
                if (memcmp((const void *)addr, data, num) != 0) {
                    WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "%s     Instance Write(0x%.8X, 0x%.8X, %d)",
                                   instance->Res->Capabilities.Name, addr, data, num);
                    if (instance->Write(addr, data, num) < 0) {
                        ret = WD_CODE_ERROR;
                    }
                    if (memcmp((const void *)addr, data, num) != 0) {
                        WD_LOG_ERROR();
                        ret = WD_CODE_ERROR;
                    }
                }
            } else if (job == WD_BLOCK_DEVICE_ARGUMENT_ERASE) {
                if (wd_memchrn((const void *)addr, data[0], num)) {
                    WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "%s     Instance Erase(0x%.8X, 0x%02X, %d)",
                                   instance->Res->Capabilities.Name, addr, data[0], num);
                    if (instance->Erase(addr, data[0], num) < 0) {
                        ret = WD_CODE_ERROR;
                    }
                    if (wd_memchrn((const void *)addr, data[0], num)) {
                        WD_LOG_ERROR();
                        ret = WD_CODE_ERROR;
                    }
                }
            } else {
                WD_LOG_ERROR();
            }
        } else {
            WD_LOG_ERROR();
            ret = WD_CODE_ERROR;
        }
    } else {
        for (offset = 0; offset < num; offset += tx_num) {
            WD_WDT_REFRESH();

            remaining = (addr + offset) % instance->Res->Capabilities.Program_Size;
            if (instance->Res->Capabilities.Program_Size - remaining >= (num - offset)) {
                tx_num = (num - offset);
            } else {
                tx_num = instance->Res->Capabilities.Program_Size - remaining;
            }

            do {
                WD_WDT_REFRESH();

                if (ret != WD_CODE_OK) {
                    wd_blockdev_driver.Control(instance, WDIF_BLOCK_DEVICE_COMMAND_INSTANCE_RESET,
                                               0);

                    retry++;

                    wd_blockdev_retry_error_e_w++;

                    if (retry > WD_BLOCK_DEVICE_RETRY) {
                        ret = WD_CODE_ERROR_TIMEOUT;
                        break;
                    } else {
                        ret = WD_CODE_OK;
                    }
                }

                erase_remaining = (addr + offset) % instance->Res->Capabilities.Erase_Sector_Size;
                erase_address =
                    (addr + offset) >= erase_remaining ? addr + offset - erase_remaining : 0;
                erase_size =
                    (instance->Res->Capabilities.Erase_Sector_Size - erase_remaining) >=
                            (num - offset)
                        ? num - offset
                        : (instance->Res->Capabilities.Erase_Sector_Size - erase_remaining);

                if (instance->Res->Capabilities.Erase_Sector_Size != 0) {
                    if (job == WD_BLOCK_DEVICE_ARGUMENT_WRITE) {
                        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG,
                                       "%s     Instance Read (0x%.8X, 0x%.8X, %d)",
                                       instance->Res->Capabilities.Name, addr + offset,
                                       instance->Res->Capabilities.Buf, tx_num);
                        if (instance->Read(addr + offset, instance->Res->Capabilities.Buf, tx_num) <
                            0) {
                            ret = WD_CODE_ERROR;
                            continue;
                        }

                        if (memcmp(instance->Res->Capabilities.Buf, (const uint8_t *)data + offset,
                                   tx_num) != 0) {
                            if (wd_memchrn(instance->Res->Capabilities.Buf, 0xFF, tx_num) == 0) {
                                write = false;
                                erase = false;

                                memcpy(instance->Res->Capabilities.Buf, data + offset, tx_num);

                                WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG,
                                               "%s     Instance Write(0x%.8X, 0x%.8X, %d)",
                                               instance->Res->Capabilities.Name, addr + offset,
                                               instance->Res->Capabilities.Buf, tx_num);
                                if (instance->Write(
                                        addr + offset,
                                        (const uint8_t *)instance->Res->Capabilities.Buf,
                                        tx_num) < 0) {
                                    ret = WD_CODE_ERROR;
                                    continue;
                                }
                                if (wd_blockdev_driver.Sync(
                                        instance,
                                        instance->Res->Capabilities.Program_TimeOut / 1000) < 0) {
                                    ret = WD_CODE_ERROR_TIMEOUT;
                                    continue;
                                }
                            } else {
                                WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG,
                                               "%s     Instance Read (0x%.8X, 0x%.8X, %d)",
                                               instance->Res->Capabilities.Name, erase_address,
                                               instance->Res->Capabilities.Buf,
                                               instance->Res->Capabilities.Erase_Sector_Size);
                                if (instance->Read(erase_address, instance->Res->Capabilities.Buf,
                                                   instance->Res->Capabilities.Erase_Sector_Size) <
                                    0) {
                                    ret = WD_CODE_ERROR;
                                    continue;
                                }

                                tx_num = erase_size;

                                memcpy(instance->Res->Capabilities.Buf + erase_remaining,
                                       (const uint8_t *)data + offset, erase_size);
                                write = true;
                                erase = true;
                            }
                        }
                    } else if (job == WD_BLOCK_DEVICE_ARGUMENT_ERASE) {
                        if ((instance->Res->Capabilities.Erase_Block_Addr > 0) &&
                            ((addr + offset) >= instance->Res->Capabilities.Erase_Block_Addr) &&
                            ((addr + offset) % instance->Res->Capabilities.Erase_Block_Size == 0) &&
                            ((num - offset) >= instance->Res->Capabilities.Erase_Block_Size)) {
                            erase_bulk = true;
                            erase = true;
                            write = false;
                            tx_num = instance->Res->Capabilities.Erase_Block_Size;
                        } else {
                            erase_bulk = false;

                            if (instance->Read(erase_address, instance->Res->Capabilities.Buf,
                                               instance->Res->Capabilities.Erase_Sector_Size) < 0) {
                                ret = WD_CODE_ERROR;
                                continue;
                            }
                            tx_num = erase_size;

                            if (wd_memchrn(instance->Res->Capabilities.Buf, data[0], erase_size)) {
                                memset(instance->Res->Capabilities.Buf + erase_remaining, data[0],
                                       erase_size);
                                erase = true;
                                write = true;
                                if (data[0] == 0xFF) {
                                    write = false;
                                }
                            }
                        }
                        if (!erase) {
                            WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG,
                                           "%s     Instance Erase(0x%.8X, 0x%02X, %d) - Pass",
                                           instance->Res->Capabilities.Name, erase_address, 0xFF,
                                           erase_size);
                        }
                    } else {
                        WD_LOG_ERROR();
                    }

                    if (erase) {
                        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG,
                                       "%s     Instance Erase(0x%.8X, 0x%02X, %d)",
                                       instance->Res->Capabilities.Name, erase_address, 0xFF,
                                       erase_bulk ? instance->Res->Capabilities.Erase_Block_Size
                                                  : instance->Res->Capabilities.Erase_Sector_Size);

                        if (instance->Erase(
                                erase_address, 0xFF,
                                erase_bulk ? instance->Res->Capabilities.Erase_Block_Size
                                           : instance->Res->Capabilities.Erase_Sector_Size) < 0) {
                            ret = WD_CODE_ERROR;
                            continue;
                        }
                        if (wd_blockdev_driver.Sync(
                                instance, instance->Res->Capabilities.Erase_Sector_TimeOut / 1000) <
                            0) {
                            ret = WD_CODE_ERROR;
                            continue;
                        }
                    }
                    if (write) {
                        for (tx_offset = 0;
                             tx_offset < instance->Res->Capabilities.Erase_Sector_Size;
                             tx_offset += tx_cnt_num) {
                            WD_WDT_REFRESH();

                            tx_cnt_num = instance->Res->Capabilities.Program_Size;

                            WD_PRINT_DEBUG(
                                WD_MSG_DEBUG_LOG, "%s     Instance Write(0x%.8X, 0x%.8X, %d)",
                                instance->Res->Capabilities.Name, erase_address + tx_offset,
                                (const uint8_t *)instance->Res->Capabilities.Buf + tx_offset,
                                tx_cnt_num);
                            if (instance->Write(erase_address + tx_offset,
                                                (const uint8_t *)instance->Res->Capabilities.Buf +
                                                    tx_offset,
                                                tx_cnt_num) < 0) {
                                ret = WD_CODE_ERROR;
                                continue;
                            }
                            if (wd_blockdev_driver.Sync(
                                    instance, instance->Res->Capabilities.Program_TimeOut / 1000) <
                                0) {
                                ret = WD_CODE_ERROR_TIMEOUT;
                                continue;
                            }
                        }
                    }
                } else {
                    if (job == WD_BLOCK_DEVICE_ARGUMENT_WRITE) {
                        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG,
                                       "%s     Instance Write(0x%.8X, 0x%.8X, %d)",
                                       instance->Res->Capabilities.Name, addr + offset,
                                       (const uint8_t *)data + offset, tx_num);

                        if (instance->Write(addr + offset, (const uint8_t *)data + offset, tx_num) <
                            0) {
                            ret = WD_CODE_ERROR;
                            continue;
                        }
                        if (wd_blockdev_driver.Sync(
                                instance, instance->Res->Capabilities.Program_TimeOut / 1000) < 0) {
                            ret = WD_CODE_ERROR_TIMEOUT;
                            continue;
                        }
                    } else if (job == WD_BLOCK_DEVICE_ARGUMENT_ERASE) {
                        memset(instance->Res->Capabilities.Buf, data[0], tx_num);
                        WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG,
                                       "%s     Instance Write(0x%.8X, 0x%.8X, %d)",
                                       instance->Res->Capabilities.Name, addr + offset,
                                       instance->Res->Capabilities.Buf, tx_num);

                        if (instance->Write(addr + offset, instance->Res->Capabilities.Buf,
                                            tx_num) < 0) {
                            ret = WD_CODE_ERROR;
                            continue;
                        }
                        if (wd_blockdev_driver.Sync(
                                instance, instance->Res->Capabilities.Program_TimeOut / 1000) < 0) {
                            ret = WD_CODE_ERROR_TIMEOUT;
                            continue;
                        }
                    } else {
                        WD_LOG_ERROR();
                    }
                }
            } while (ret != WD_CODE_OK);
        }
    }

    WD_CHECK(ret);

    if (instance->Res->IO.WP != RTE_CONFIG_IDLE_PIN_NC) {
        wd_gpio_config(instance->Res->IO.WP, WD_GPIO_FUNCTION_AUTO | WD_GPIO_ON);
    }

#if (RTE_RTOS)
    instance->Res->Unlock();
#endif

    return (ret);
}


static int32_t block_device_driver_write(wd_blockdev_instance_t *instance, void *ptr,
                                         const void *data, uint32_t num, uint32_t arg) {
    int32_t ret = WD_CODE_OK;
    uint32_t addr = (uint32_t)ptr;

#if (RTE_RTOS)
    instance->Res->Lock();
#endif

    WD_PRINT_DEBUG(WD_MSG_INFO_LOG, "%s _Block_Device_Driver_Write(0x%.8X, 0x%.8X, %d)",
                   instance->Res->Capabilities.Name, addr, data, num);

    ret =
        block_device_driver_handle(instance, addr, data, num, arg, WD_BLOCK_DEVICE_ARGUMENT_WRITE);

    if (ret == WD_CODE_OK) {
        if (arg & WDIF_BLOCK_DEVICE_ARGUMENT_CRC) {
            uint16_t crc = 0xACC8;
            crc = wd_toolkit_crc16((void *)data, num);
            ret = block_device_driver_handle(instance, addr + num, (void *)&crc, WD_CRC_SIZE, 0,
                                             WD_BLOCK_DEVICE_ARGUMENT_WRITE);
        }
    }

#if (RTE_RTOS)
    instance->Res->Unlock();
#endif

    return (ret);
}


static int32_t block_device_driver_read(wd_blockdev_instance_t *instance, void *ptr, void *data,
                                        uint32_t num, uint32_t arg) {
    (void)arg;
    int32_t ret = WD_CODE_OK;
    uint32_t addr = (uint32_t)ptr;

    uint32_t retry = 0;

    if (instance == NULL) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    if (!WD_RANGE(instance->Res->Capabilities.Addr, addr,
                  instance->Res->Capabilities.Addr + instance->Res->Capabilities.Size)) {
        WD_LOG_ERROR();
        return WD_CODE_ERROR;
    }

    WD_PRINT_DEBUG(WD_MSG_INFO_LOG, "%s _Block_Device_Driver_Read (0x%.8X, 0x%.8X, %d)",
                   instance->Res->Capabilities.Name, addr, data, num);

#if (RTE_RTOS)
    instance->Res->Lock();
#endif

    if (instance->Res->Capabilities.Direct_Access != 0U) // ROM
    {
        if (instance->Res->Capabilities.Buf == NULL && instance->Res->Capabilities.Buf_Size == 0) {
            WD_PRINT_DEBUG(WD_MSG_DEBUG_LOG, "%s     Instance Read (0x%.8X, 0x%.8X, %d)",
                           instance->Res->Capabilities.Name, addr, data, num);
            if (instance->Read(addr, data, num) < 0) {
                ret = WD_CODE_ERROR;
            }
        } else {
            WD_LOG_ERROR();
            ret = WD_CODE_ERROR;
        }
    } else // NVM
    {
        do {
            WD_WDT_REFRESH();

            if (ret != WD_CODE_OK) {
                wd_blockdev_driver.Control(instance, WDIF_BLOCK_DEVICE_COMMAND_INSTANCE_RESET, 0);
                retry++;

                wd_blockdev_retry_error_r++;

                if (retry > WD_BLOCK_DEVICE_RETRY) {
                    ret = WD_CODE_ERROR_TIMEOUT;
                    break;
                } else {
                    ret = WD_CODE_OK;
                }
            }

            if (num) {
                if (instance->Read(addr, data, num) < 0) {
                    ret = WD_CODE_ERROR;
                    continue;
                }
            }

        } while (ret != WD_CODE_OK);
    }

    //  if(arg & WDIF_BLOCK_DEVICE_ARGUMENT_CRC)
    //  {
    //    uint16_t crc;
    //    uint16_t crc_calc = wd_toolkit_crc16((void *)data, num);
    //
    //    if(instance->Read(addr + num, &crc, WD_CRC_SIZE) == WD_CODE_OK)
    //    {
    //      if(crc_calc != crc)
    //      {
    //        WD_LOG_ERROR("[%s] Read CRC Error", instance->Res->Capabilities.Name);
    //        ret = WD_CODE_ERROR_CRC;
    //      }
    //    }
    //    else
    //    {
    //
    //      ret = WD_CODE_ERROR;
    //    }
    //  }

    WD_CHECK(ret);

#if (RTE_RTOS)
    instance->Res->Unlock();
#endif

    return (ret);
}

static int32_t block_device_driver_erase(wd_blockdev_instance_t *instance, void *ptr,
                                         const uint8_t data, uint32_t num, uint32_t arg) {
    int32_t ret = WD_CODE_OK;
    uint32_t addr = (uint32_t)ptr;

    WD_PRINT_DEBUG(WD_MSG_INFO_LOG, "%s _Block_Device_Driver_Erase(0x%.8X, 0x%02X, %d)",
                   instance->Res->Capabilities.Name, addr, data, num);

    ret =
        block_device_driver_handle(instance, addr, &data, num, arg, WD_BLOCK_DEVICE_ARGUMENT_ERASE);

    if (ret == WD_CODE_OK) {
        if (arg & WDIF_BLOCK_DEVICE_ARGUMENT_CRC) {
            uint8_t d;
            uint8_t p = data;
            uint16_t crc = 0xFFFF;
            uint32_t count = num;

            do {
                d = (uint8_t)(p ^ (crc & 0xFF));
                d ^= d << 4;
                crc = (uint16_t)((d << 3) ^ (d << 8) ^ (crc >> 8) ^ (d >> 4));
            } while (--count);

            crc ^= 0xffff;

            ret = block_device_driver_handle(instance, addr + num, (void *)&crc, WD_CRC_SIZE, 0,
                                             WD_BLOCK_DEVICE_ARGUMENT_WRITE);
        }
    }
    return (ret);
}

static int32_t block_device_driver_sync(wd_blockdev_instance_t *instance, uint32_t timeout_ms) {
    int32_t ret = WD_CODE_OK;

    WD_WHILE_RET_MS((instance->Get_Status() < 0), timeout_ms, ret);

    return (ret);
}

static fsStatus block_device_driver_device_ctrl(fsDevCtrlCode code, void *p) {
    fsStatus ret = fsOK;

    UNUSED(p);

    switch ((uint32_t)code) {
    case fsDevCtrlCodeCheckMedia:
        break;
    default:
        break;
    }
    return (ret);
}

static int32_t block_device_driver_control(wd_blockdev_instance_t *instance, uint32_t control,
                                           uint32_t arg) {
    int32_t ret = WD_CODE_OK;

    UNUSED(arg);

    switch (control & WDIF_BLOCK_DEVICE_COMMAND_MSK) {
    case WDIF_BLOCK_DEVICE_COMMAND_INSTANCE_RESET: {
        WD_PRINT_INFO(WD_MSG_INFO_LOG, "%s _Block_Device_Driver_Control::INSTANCE_RESET(0x%.8X)",
                      instance->Res->Capabilities.Name, instance);

        ret = instance->PowerControl(WD_POWER_OFF);
        if (ret < 0) {
            WD_LOG_ERROR();
        }

        ret = instance->PowerControl(WD_POWER_FULL);
        if (ret < 0) {
            WD_LOG_ERROR();
        }
    } break;

    case WDIF_BLOCK_DEVICE_COMMAND_VALIDATION:
        if (instance->Validation != NULL) {
            instance->Validation();
        }
        break;
    default:
        break;
    }

    return (ret);
}

static int32_t block_device_driver_object_write(wd_blockdev_object_multi_t *object, uint32_t arg) {
    int32_t ret = WD_CODE_OK;

    UNUSED(object);
    UNUSED(arg);

    return (ret);
}

static int32_t block_device_driver_object_read(wd_blockdev_object_multi_t *object, uint32_t arg) {
    int32_t ret = WD_CODE_OK;

    UNUSED(object);
    UNUSED(arg);

    return (ret);
}


wd_block_device_t wd_blockdev_driver = {
    .Write = block_device_driver_write,
    .Initialize = block_device_driver_initialize,
    .Read = block_device_driver_read,
    .Erase = block_device_driver_erase,
    .Sync = block_device_driver_sync,
    .DeviceCtrl = block_device_driver_device_ctrl,
    .Control = block_device_driver_control,

    .Object_Write = block_device_driver_object_write,
    .Object_Read = block_device_driver_object_read,
};

wd_block_device_t *const wd_drv_block_device = &wd_blockdev_driver;

#endif // #if (RTE_DRIVERS_BLOCK_DEVICE)
#endif // #if 1 // 토글
