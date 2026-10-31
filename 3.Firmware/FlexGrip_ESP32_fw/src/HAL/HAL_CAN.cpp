#include <Arduino.h>

#include <string.h>

#include <CAN_config.h>
#include <ESP32CAN.h>

#include "Configs/Config.h"
#include "Ctrl/GripControl.h"
#include "HAL/HAL.h"

// The ESP32-CAN library expects the application to provide this instance.
CAN_device_t CAN_cfg;

namespace
{
    const uint32_t kSetCurrentId =
        (uint32_t)(GRIP_CAN_NODE_ID << 7) | GRIP_CAN_MODE_SET_CURRENT;
    const uint32_t kSetAngleId =
        (uint32_t)(GRIP_CAN_NODE_ID << 7) | GRIP_CAN_MODE_SET_ANGLE;
}

void HAL::CAN_Init()
{
    CAN_cfg.speed = CAN_SPEED_1000KBPS;
    CAN_cfg.tx_pin_id = (gpio_num_t)CAND_PIN;   // 5
    CAN_cfg.rx_pin_id = (gpio_num_t)CANR_PIN;   // 4
    CAN_cfg.rx_queue = xQueueCreate(16, sizeof(CAN_frame_t));
    CAN_cfg.tx_queue = xQueueCreate(16, sizeof(CAN_frame_t));

    ESP32Can.CANInit();
}

void HAL::CAN_Update()
{
    CAN_frame_t rxFrame;
    while (xQueueReceive(CAN_cfg.rx_queue, &rxFrame, 0) == pdTRUE)
    {
        if (rxFrame.FIR.B.FF != CAN_frame_std)
            continue;
        if (rxFrame.FIR.B.RTR == CAN_RTR)
            continue;

        if (rxFrame.MsgID == kSetAngleId && rxFrame.FIR.B.DLC >= 4)
        {
            float amount = 0.0f;
            memcpy(&amount, rxFrame.data.u8, sizeof(amount));
            Ctrl::GripControl::SetOpenAmount(amount);
        }
        else if (rxFrame.MsgID == kSetCurrentId && rxFrame.FIR.B.DLC >= 4)
        {
            float amps = 0.0f;
            memcpy(&amps, rxFrame.data.u8, sizeof(amps));
            if (amps > 0.0f)
            {
                Ctrl::GripControl::SetEnabled(true);
                Ctrl::GripControl::SetCurrentLimit(amps * 1000.0f);
            }
            else
            {
                Ctrl::GripControl::SetEnabled(false);
            }
        }
    }
}
