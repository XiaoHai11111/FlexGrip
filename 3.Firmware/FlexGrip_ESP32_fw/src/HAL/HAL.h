#ifndef __HAL_H
#define __HAL_H

#include <stdint.h>
#include <Arduino.h>
#include "HAL_Def.h"
#include "FreeRTOS.h"

namespace HAL
{
    void Init();
    void Update();

    /* Serial */
    void Serial_Init();
    void Serial_Update();

    /* ADC_Angle*/
    void ADC_Init();
    void ADC_Angle_Update();
    void ADC_Current_Update();
    void ADC_GetInfo(Grip_Info_t* info);

    /*Motor*/
    void Motor_Init();
    void Motor_Enable();
    void Motor_Disable();
    void Motor_Stop(Motor_t motor);
    void Motor_SetDuty(Motor_t motor, int duty);   // duty in [-255,255], positive = clamp

    /* CAN */
    void CAN_Init();
    void CAN_Update();
}

#endif
