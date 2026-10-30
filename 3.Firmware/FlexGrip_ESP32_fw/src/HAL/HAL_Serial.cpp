#include "HAL/HAL.h"
#include "HAL/HAL_Def.h"
#include "Configs/Version.h"
void HAL::Serial_Init()
{
    Serial.begin(115200);
    Serial.println(VERSION_FIRMWARE_NAME);
    Serial.println("Version: " VERSION_SOFTWARE);
    Serial.println("Author: " VERSION_AUTHOR_NAME);
}
