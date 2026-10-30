#include <Arduino.h>
#include "HAL.h"
#include "Configs/Config.h"
#include <analogWrite.h>

static HAL::PID_Param_t pos_pid;
static HAL::Grip_Info_t gripper_info;
static double targetPosition = 200;  // 设置目标位置（例如 100 度）
static double currentPosition = 183.0;   // 当前电机位置（通过编码器获取）

TaskHandle_t PIDTaskHandle;

void PIDTask(void *pvParameters) 
{
    const TickType_t xFrequency = pdMS_TO_TICKS(1); // 任务运行周期（1ms = 1000Hz，20kHz = 0.05ms）
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) 
    {
        HAL::calculatePID();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void HAL::Motor_Init()
{
    pinMode(MOTOR_RIGHT_AIN1_PIN, OUTPUT);
    pinMode(MOTOR_RIGHT_AIN2_PIN, OUTPUT);
    pinMode(MOTOR_LEFT_BIN1_PIN, OUTPUT);
    pinMode(MOTOR_LEFT_BIN2_PIN, OUTPUT);

    pos_pid.Kp = 0.5;
    pos_pid.Ki = 0.01;
    pos_pid.Kd = 0.1;
    pos_pid.integral = 0.0;
    pos_pid.previousError = 0.0;

    // // 创建 PID 控制任务
    // xTaskCreatePinnedToCore(
    //     PIDTask,  
    //     "PIDTask", 
    //     1024, 
    //     NULL, 
    //     1,  
    //     &PIDTaskHandle, 
    //     0               
    // );
}

void HAL::Motor_Enable()
{
    // digitalWrite(MOTOR_RIGHT_AIN1_PIN, HIGH);
    // digitalWrite(MOTOR_RIGHT_AIN2_PIN, HIGH);
    // digitalWrite(MOTOR_LEFT_BIN1_PIN, HIGH);
    // digitalWrite(MOTOR_LEFT_BIN2_PIN, HIGH);

    // digitalWrite(MOTOR_RIGHT_AIN1_PIN, LOW);
    // digitalWrite(MOTOR_RIGHT_AIN2_PIN, LOW);
    // digitalWrite(MOTOR_LEFT_BIN1_PIN, LOW);
    // digitalWrite(MOTOR_LEFT_BIN2_PIN, LOW);

    // digitalWrite(MOTOR_RIGHT_AIN1_PIN, HIGH);
    // digitalWrite(MOTOR_RIGHT_AIN2_PIN, LOW);
    digitalWrite(MOTOR_LEFT_BIN1_PIN, HIGH);
    digitalWrite(MOTOR_LEFT_BIN2_PIN, LOW);
}

void HAL::Motor_Disable()
{
    digitalWrite(MOTOR_RIGHT_AIN1_PIN, LOW);
    digitalWrite(MOTOR_RIGHT_AIN2_PIN, LOW);
    digitalWrite(MOTOR_LEFT_BIN1_PIN, LOW);
    digitalWrite(MOTOR_LEFT_BIN2_PIN, LOW);
}

void HAL::setMotorSpeed(int speed) 
{
    if (speed > 0)
    {
        analogWrite(MOTOR_LEFT_BIN1_PIN, 0);  // 设置电机正转
        analogWrite(MOTOR_LEFT_BIN2_PIN, speed);      // 关闭反向转动
    } else 
    {
        analogWrite(MOTOR_LEFT_BIN1_PIN, -speed);      // 关闭正向转动
        analogWrite(MOTOR_LEFT_BIN2_PIN, 0); // 设置电机反向转动
    }
}

void HAL::calculatePID()
{
    //HAL::ADC_Angle_Update();
    HAL::ADC_GetInfo(&gripper_info);
    currentPosition = gripper_info.angle_left;
    double error = targetPosition - currentPosition;
    pos_pid.integral += error;
    double derivative = error - pos_pid.previousError;
    double pidOutput = pos_pid.Kp * error + pos_pid.Ki * pos_pid.integral + pos_pid.Kd * derivative;

    int motorSpeed = constrain(pidOutput, -MAX_SPEED, MAX_SPEED);
    HAL::setMotorSpeed(motorSpeed);
    pos_pid.previousError = error;

    // Serial.print("PidOutput: ");
    // Serial.print(pidOutput);
    // Serial.print("Target: ");
    // Serial.print(targetPosition);
    // Serial.print(" Current: ");
    // Serial.print(currentPosition);
    // Serial.print(" Error: ");
    // Serial.print(error);
    // Serial.print(" Motor Speed: ");
    // Serial.println(motorSpeed);
}
