/*
*/
#include <Arduino.h>
#include "HAL/HAL.h"
#include "Configs/Config.h"

static HAL::Grip_Info_t grip_info;
static unsigned long lastReadTime = 0; // 上次读取时间
static float lastAngle_left = 0; // 上次的角度值
static float lastAngle_right = 0; // 上次的角度值

void HAL::ADC_Init()
{
    pinMode(ANGLE_LEFT_PIN, INPUT);
    pinMode(ANGLE_RIGHT_PIN, INPUT);
    pinMode(CURRENT_INPUT_PIN, INPUT);
    pinMode(CURRENT_PIN, INPUT);
}

void HAL::ADC_Angle_Update() {
    unsigned long currentMillis = millis();

    // 初始化累加器和计数器
    uint32_t adcValue_left_sum = 0;
    uint32_t adcValue_right_sum = 0;
    const int num_samples = 10;  // 采样次数

    // 进行10次采样并累加
    for (int i = 0; i < num_samples; i++) {
        adcValue_left_sum += analogRead(ANGLE_LEFT_PIN);
        adcValue_right_sum += analogRead(ANGLE_RIGHT_PIN);
        delay(5); // 如果需要减少采样间隔，可以调整这里的延迟
    }

    // 计算平均值
    uint32_t adcValue_left_avg = adcValue_left_sum / num_samples;
    uint32_t adcValue_right_avg = adcValue_right_sum / num_samples;

    // 将平均值映射到电压
    int vol_left = map(adcValue_left_avg, 0, 4095, 0, 3300);
    int vol_right = map(adcValue_right_avg, 0, 4095, 0, 3300);

    // 将电压映射为角度
    float angle_left = map(vol_left, 0, 2500, 0, 333.3);
    float angle_right = map(vol_right, 0, 2500, 0, 333.3);

    // 计算时间差
    unsigned long timeDiff = currentMillis - lastReadTime;

    // 计算速度（角度变化除以时间变化）
    float speed_left = (angle_left - lastAngle_left) / (timeDiff / 1000.0); // 转换时间差为秒
    float speed_right = (angle_right - lastAngle_right) / (timeDiff / 1000.0); // 转换时间差为秒

    // 更新上次读取时间和角度
    lastReadTime = currentMillis;
    lastAngle_left = angle_left;
    lastAngle_right = angle_right;

    // 存储当前角度和速度
    grip_info.angle_left = angle_left;
    grip_info.speed_left = speed_left;
    grip_info.angle_right = angle_right;
    grip_info.speed_right = speed_right;

    Serial.printf("angle_left = %.2f°, speed_left = %.2f°/s, angle_right = %.2f°, speed_right = %.2f°/s",
                    grip_info.angle_left, grip_info.speed_left, grip_info.angle_right, grip_info.speed_right);
    Serial.println();
}

void HAL::ADC_Current_Update()
{
    uint32_t adcValue_input_sum = 0;
    uint32_t adcValue_current_sum = 0;
    const int num_samples = 10;  // 采样次数

    // 进行10次采样并累加
    for (int i = 0; i < num_samples; i++) {
        adcValue_input_sum += analogRead(CURRENT_INPUT_PIN);
        adcValue_current_sum += analogRead(CURRENT_PIN);
        delay(5);
    }

    // 计算平均值
    uint32_t adcValue_input_avg = adcValue_input_sum / num_samples;
    uint32_t adcValue_current_avg = adcValue_current_sum / num_samples;

        // 将平均值映射到电压
    int vol_input = map(adcValue_input_avg, 0, 4095, 0, 3300);
    int vol_current = map(adcValue_current_avg, 0, 4095, 0, 3300);

    int current = 2*(vol_input - vol_current);
    if (current < 0)
    {
        current = 0;
    }
    
    grip_info.current = current;

    Serial.printf("current = %d", current);
    Serial.println();

}

void HAL::ADC_GetInfo(Grip_Info_t* info)
{
    info->angle_left = grip_info.angle_left;
    info->angle_right = grip_info.angle_right;
    info->speed_left = grip_info.speed_left;
    info->speed_right = grip_info.speed_right;
    info->current = grip_info.current;
}