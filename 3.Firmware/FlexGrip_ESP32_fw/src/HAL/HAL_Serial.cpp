#include <Arduino.h>

#include <cstdlib>
#include <cstdio>

#include "Configs/Version.h"
#include "Ctrl/GripControl.h"
#include "HAL/HAL.h"

namespace
{
    bool parseFloat(const String& text, float& value)
    {
        if (text.length() == 0)
            return false;

        const char* begin = text.c_str();
        char* end = nullptr;
        value = strtof(begin, &end);
        return end != begin;
    }
}

void HAL::Serial_Init()
{
    Serial.begin(115200);
    Serial.println(VERSION_FIRMWARE_NAME);
    Serial.println("Version: " VERSION_SOFTWARE);
    Serial.println("Author: " VERSION_AUTHOR_NAME);
    Serial.println("Commands: angle <0~45>, open, close, enable, disable, status, pid <kp> <ki> <kd>, ki <left> <right>, speed <deg/s>, ff <left> <right>");
}

void HAL::Serial_Update()
{
    while (Serial.available())
    {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd.length() == 0)
            continue;

        if (cmd.equalsIgnoreCase("open"))
        {
            Ctrl::GripControl::Open();
            Serial.println("ok open");
        }
        else if (cmd.equalsIgnoreCase("close"))
        {
            Ctrl::GripControl::SetOpenAmount(0.0f);
            Serial.println("ok close");
        }
        else if (cmd.equalsIgnoreCase("enable"))
        {
            Ctrl::GripControl::SetEnabled(true);
            Serial.println("ok enable");
        }
        else if (cmd.equalsIgnoreCase("disable"))
        {
            Ctrl::GripControl::SetEnabled(false);
            Serial.println("ok disable");
        }
        else if (cmd.equalsIgnoreCase("status"))
        {
            Ctrl::GripControl::PrintStatus();
        }
        else if (cmd.startsWith("pid"))
        {
            float kp = 0.0f;
            float ki = 0.0f;
            float kd = 0.0f;
            if (sscanf(cmd.c_str(), "pid %f %f %f", &kp, &ki, &kd) == 3)
            {
                Ctrl::GripControl::SetGains(kp, ki, kd);
                Serial.printf("ok pid %.3f %.3f %.3f\r\n", kp, ki, kd);
            }
            else
            {
                Serial.println("err usage: pid <kp> <ki> <kd>");
            }
        }
        else if (cmd.startsWith("speed"))
        {
            float speed = 0.0f;
            if (sscanf(cmd.c_str(), "speed %f", &speed) == 1)
            {
                Ctrl::GripControl::SetMotionSpeed(speed);
                Serial.printf("ok speed %.1f\r\n", speed);
            }
            else
            {
                Serial.println("err usage: speed <deg/s>");
            }
        }
        else if (cmd.startsWith("ki"))
        {
            float left = 0.0f;
            float right = 0.0f;
            if (sscanf(cmd.c_str(), "ki %f %f", &left, &right) == 2)
            {
                Ctrl::GripControl::SetIntegralGains(left, right);
                Serial.printf("ok ki %.1f %.1f\r\n", left, right);
            }
            else
            {
                Serial.println("err usage: ki <left> <right>");
            }
        }
        else if (cmd.startsWith("ff"))
        {
            float left = 0.0f;
            float right = 0.0f;
            if (sscanf(cmd.c_str(), "ff %f %f", &left, &right) == 2)
            {
                Ctrl::GripControl::SetFeedforward(left, right);
                Serial.printf("ok ff %.2f %.2f\r\n", left, right);
            }
            else
            {
                Serial.println("err usage: ff <left> <right>");
            }
        }
        else if (cmd.startsWith("angle"))
        {
            float depth = 0.0f;
            if (parseFloat(cmd.substring(5), depth))
            {
                Ctrl::GripControl::SetOpenAmount(depth);
                Serial.printf("ok angle %.2f\r\n", depth);
            }
            else
            {
                Serial.println("err usage: angle <0~45>");
            }
        }
        else
        {
            float depth = 0.0f;
            if (parseFloat(cmd, depth))
            {
                Ctrl::GripControl::SetOpenAmount(depth);
                Serial.printf("ok angle %.2f\r\n", depth);
            }
            else
            {
                Serial.println("err unknown command");
            }
        }
    }
}
