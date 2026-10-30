#include "HAL/HAL.h"

void HAL::Init()
{
    HAL::Serial_Init();
    HAL::ADC_Init();
    HAL::Motor_Init();
    HAL::Motor_Enable();
}

void HAL::Update()
{
    HAL::ADC_Angle_Update();
    HAL::ADC_Current_Update();
    delay(1);
}
