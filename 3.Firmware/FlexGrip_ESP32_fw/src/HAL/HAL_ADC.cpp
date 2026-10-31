#include <Arduino.h>

#include "Configs/Config.h"
#include "HAL/HAL.h"

static HAL::Grip_Info_t grip_info;
static uint32_t lastAngleMicros = 0;
static float lastAngleLeft = 0.0f;
static float lastAngleRight = 0.0f;

namespace
{
    float adcToAngle(float raw)
    {
        // ADC raw (0..4095) -> mV (0..3300) -> degree (0..333.3)
        float mv = raw * (ANGLE_ADC_VOLTAGE_MAX / ANGLE_ADC_RAW_MAX);
        return mv * (ANGLE_ADC_DEGREE_SCALE / ANGLE_ADC_VOLTAGE_SCALE);
    }
}

void HAL::ADC_Init()
{
    pinMode(ANGLE_LEFT_PIN, INPUT);
    pinMode(ANGLE_RIGHT_PIN, INPUT);
    pinMode(CURRENT_INPUT_PIN, INPUT);
    pinMode(CURRENT_PIN, INPUT);
}

void HAL::ADC_Angle_Update()
{
    const int numSamples = 10;
    uint32_t leftSum = 0;
    uint32_t rightSum = 0;

    for (int i = 0; i < numSamples; i++)
    {
        leftSum += analogRead(ANGLE_LEFT_PIN);
        rightSum += analogRead(ANGLE_RIGHT_PIN);
    }

    float angleLeft = adcToAngle((float)leftSum / numSamples);
    float angleRight = adcToAngle((float)rightSum / numSamples);

    uint32_t now = micros();
    float dtSec = (float)(now - lastAngleMicros) / 1000000.0f;
    if (dtSec <= 0.0f)
        dtSec = 0.001f;

    grip_info.angle_left = angleLeft;
    grip_info.angle_right = angleRight;
    grip_info.speed_left = (angleLeft - lastAngleLeft) / dtSec;
    grip_info.speed_right = (angleRight - lastAngleRight) / dtSec;

    lastAngleMicros = now;
    lastAngleLeft = angleLeft;
    lastAngleRight = angleRight;
}

void HAL::ADC_Current_Update()
{
    const int numSamples = 10;
    uint32_t inputSum = 0;
    uint32_t currentSum = 0;

    for (int i = 0; i < numSamples; i++)
    {
        inputSum += analogRead(CURRENT_INPUT_PIN);
        currentSum += analogRead(CURRENT_PIN);
    }

    float volInput = ((float)inputSum / numSamples) *
                     (ANGLE_ADC_VOLTAGE_MAX / ANGLE_ADC_RAW_MAX);
    float volCurrent = ((float)currentSum / numSamples) *
                       (ANGLE_ADC_VOLTAGE_MAX / ANGLE_ADC_RAW_MAX);

    // Motor supply current flows through a 1 ohm sense resistor between 5 V
    // and the DRV8833 VM pin.
    //   GPIO12 (CURRENT_INPUT_PIN) = 2.5 V reference (5 V / 2 divider).
    //   GPIO14 (CURRENT_PIN)        = VM / 2 (divider from the sense node).
    //   I(mA) = (5 V - VM) / 1 ohm * 1000 = 2 * (Vref - Vsen/2) in mV.
    float currentMa = GRIP_CURRENT_GAIN * 2.0f * (volInput - volCurrent);
    if (currentMa < 0.0f)
        currentMa = 0.0f;

    grip_info.current_ma = currentMa;
}

void HAL::ADC_GetInfo(Grip_Info_t* info)
{
    info->angle_left = grip_info.angle_left;
    info->angle_right = grip_info.angle_right;
    info->speed_left = grip_info.speed_left;
    info->speed_right = grip_info.speed_right;
    info->current_ma = grip_info.current_ma;
}
