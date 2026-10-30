#include "HAL/HAL.h"

// // 定义任务句柄
// TaskHandle_t task1Handle;

// void task1(void *pvParameters) 
// {
//   while (1) 
//   {
//     Serial.printf("Task3:joints_info(%.2f, %.2f, %.2f, %.2f, %.2f, %.2f)\r\n", 
//                 joints_info.joint1, joints_info.joint2, joints_info.joint3, joints_info.joint4, joints_info.joint5, joints_info.joint6);
//     vTaskDelay(pdMS_TO_TICKS(1000));  // 每2秒发送一次
//   }
// }

void setup()
{
    HAL::Init();
    //xTaskCreate(task1, "Task1", 10000, NULL, 1, &task1Handle);
}

void loop()
{
    HAL::Update();
    delay(500);
}