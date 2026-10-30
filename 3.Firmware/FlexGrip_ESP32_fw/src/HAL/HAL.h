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

    /* ADC_Angle*/
    void ADC_Init();
    void ADC_Angle_Update();
    void ADC_Current_Update();
    void ADC_GetInfo(Grip_Info_t* info);

    /*Motor*/
    void Motor_Init();
    void Motor_Enable();
    void Motor_Disable();
    void calculatePID();
    void setMotorSpeed(int speed);
    // void adjustPIDParameters(String input);
}

#endif
