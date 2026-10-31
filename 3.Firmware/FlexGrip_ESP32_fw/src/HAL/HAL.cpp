#include "HAL/HAL.h"

#include "Ctrl/GripControl.h"

static uint32_t lastLoopMicros = 0;
static uint32_t lastCurrentMillis = 0;
static uint32_t lastStatusMillis = 0;

void HAL::Init()
{
    HAL::Serial_Init();
    HAL::ADC_Init();
    HAL::Motor_Init();
    HAL::CAN_Init();

    // Sample the encoders once so the controller holds the current position
    // instead of slamming the gripper on power-up.
    HAL::ADC_Angle_Update();
    Ctrl::GripControl::Init();

    lastLoopMicros = micros();
    lastCurrentMillis = millis();
    lastStatusMillis = millis();
}

void HAL::Update()
{
    uint32_t nowMicros = micros();
    float dt = (float)(nowMicros - lastLoopMicros) / 1000000.0f;
    lastLoopMicros = nowMicros;
    if (dt <= 0.0f)
        dt = 0.001f;
    if (dt > 0.1f)
        dt = 0.1f;

    HAL::ADC_Angle_Update();

    uint32_t nowMillis = millis();
    if (nowMillis - lastCurrentMillis >= 5)
    {
        lastCurrentMillis = nowMillis;
        HAL::ADC_Current_Update();
    }

    HAL::Serial_Update();
    HAL::CAN_Update();

    Ctrl::GripControl::Update(dt);

    if (nowMillis - lastStatusMillis >= 200)
    {
        lastStatusMillis = nowMillis;
        Ctrl::GripControl::PrintStatus();
    }
}
