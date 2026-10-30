#include <Arduino.h>

#include <analogWrite.h>

#include "Configs/Config.h"
#include "HAL/HAL.h"

static bool motorEnabled = false;

void HAL::Motor_Init()
{
    pinMode(MOTOR_RIGHT_AIN1_PIN, OUTPUT);
    pinMode(MOTOR_RIGHT_AIN2_PIN, OUTPUT);
    pinMode(MOTOR_LEFT_BIN1_PIN, OUTPUT);
    pinMode(MOTOR_LEFT_BIN2_PIN, OUTPUT);

    HAL::Motor_Disable();
}

void HAL::Motor_Enable()
{
    motorEnabled = true;
}

void HAL::Motor_Disable()
{
    motorEnabled = false;
    HAL::Motor_Stop(MOTOR_LEFT);
    HAL::Motor_Stop(MOTOR_RIGHT);
}

void HAL::Motor_Stop(Motor_t motor)
{
    if (motor == MOTOR_LEFT)
    {
        analogWrite(MOTOR_LEFT_BIN1_PIN, 0);
        analogWrite(MOTOR_LEFT_BIN2_PIN, 0);
    }
    else
    {
        analogWrite(MOTOR_RIGHT_AIN1_PIN, 0);
        analogWrite(MOTOR_RIGHT_AIN2_PIN, 0);
    }
}

void HAL::Motor_SetDuty(Motor_t motor, int duty)
{
    if (!motorEnabled)
    {
        HAL::Motor_Stop(motor);
        return;
    }

    duty = constrain(duty, -MAX_SPEED, MAX_SPEED);

    if (motor == MOTOR_LEFT)
    {
        // Left motor: BIN1=HIGH is the OPEN direction (angle decreases),
        // so the CLAMP direction is BIN2=HIGH.
        if (duty >= 0)
        {
            analogWrite(MOTOR_LEFT_BIN1_PIN, 0);
            analogWrite(MOTOR_LEFT_BIN2_PIN, duty);
        }
        else
        {
            analogWrite(MOTOR_LEFT_BIN1_PIN, -duty);
            analogWrite(MOTOR_LEFT_BIN2_PIN, 0);
        }
    }
    else  // MOTOR_RIGHT
    {
        // Right motor: AIN1=HIGH is the CLAMP direction.
        if (duty >= 0)
        {
            analogWrite(MOTOR_RIGHT_AIN1_PIN, duty);
            analogWrite(MOTOR_RIGHT_AIN2_PIN, 0);
        }
        else
        {
            analogWrite(MOTOR_RIGHT_AIN1_PIN, 0);
            analogWrite(MOTOR_RIGHT_AIN2_PIN, -duty);
        }
    }
}
