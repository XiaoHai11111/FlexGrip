#include "Ctrl/GripControl.h"

#include <Arduino.h>

#include <math.h>

#include "Configs/Config.h"
#include "HAL/HAL.h"

namespace Ctrl
{
namespace GripControl
{

namespace
{
    struct MotorControl
    {
        float kp = GRIP_PID_KP;
        float ki = GRIP_LEFT_KI;
        float kd = GRIP_PID_KD;
        float ff = 0.0f;  // velocity feedforward, PWM per deg/s
        float start = 0.0f;
        float final = 0.0f;
        float setpoint = 0.0f;
        float filtAngle = 0.0f;
        float prevFiltAngle = 0.0f;
        float integral = 0.0f;
    };

    MotorControl left;
    MotorControl right;

    bool moving = false;
    bool enabled = false;
    uint32_t moveStartMs = 0;
    uint32_t moveEndMs = 0;
    float motionSpeedDegS = GRIP_MOTION_SPEED_DEG_S;
    float lastOutLeft = 0.0f;
    float lastOutRight = 0.0f;

    float currentLimitMa = GRIP_CURRENT_DEFAULT_MA;
    float currentKp = GRIP_CURRENT_KP;
    float currentKi = GRIP_CURRENT_KI;
    float currentIntegral = 0.0f;

    // Duty convention: duty > 0 means "close/clamp" for that motor.
    // Left closes by increasing the angle, right closes by decreasing it.
    static const float kLeftClampSign = +1.0f;
    static const float kRightClampSign = -1.0f;

    float clampOutput(float v)
    {
        if (v > GRIP_PID_OUTPUT_MAX)
            return GRIP_PID_OUTPUT_MAX;
        if (v < -GRIP_PID_OUTPUT_MAX)
            return -GRIP_PID_OUTPUT_MAX;
        return v;
    }

    void beginMove(uint32_t nowMs)
    {
        HAL::Grip_Info_t info;
        HAL::ADC_GetInfo(&info);

        left.start = info.angle_left;
        right.start = info.angle_right;

        float distLeft = fabsf(left.final - left.start);
        float distRight = fabsf(right.final - right.start);
        float dist = distLeft > distRight ? distLeft : distRight;

        if (dist < 1.0f || motionSpeedDegS <= 0.0f)
        {
            moving = false;
            left.setpoint = left.final;
            right.setpoint = right.final;
            return;
        }

        moveStartMs = nowMs;
        moveEndMs = nowMs + (uint32_t)(dist / motionSpeedDegS * 1000.0f);
        moving = true;

        left.integral = 0.0f;
        right.integral = 0.0f;
        left.prevFiltAngle = left.filtAngle;
        right.prevFiltAngle = right.filtAngle;
    }
}

void Init()
{
    left.ki = GRIP_LEFT_KI;
    right.ki = GRIP_RIGHT_KI;
    left.ff = GRIP_LEFT_FF;
    right.ff = GRIP_RIGHT_FF;
    motionSpeedDegS = GRIP_MOTION_SPEED_DEG_S;

    HAL::Grip_Info_t info;
    HAL::ADC_GetInfo(&info);

    left.final = left.setpoint = info.angle_left;
    right.final = right.setpoint = info.angle_right;
    left.filtAngle = left.prevFiltAngle = info.angle_left;
    right.filtAngle = right.prevFiltAngle = info.angle_right;

    SetEnabled(true);
}

void Update(float dt)
{
    if (!enabled)
        return;

    if (dt <= 0.0f)
        dt = 0.001f;
    if (dt > 0.1f)
        dt = 0.1f;

    uint32_t nowMs = millis();

    if (moving)
    {
        float t = (moveEndMs > moveStartMs)
                      ? (float)(nowMs - moveStartMs) / (float)(moveEndMs - moveStartMs)
                      : 1.0f;
        if (t >= 1.0f)
        {
            t = 1.0f;
            moving = false;
        }

        left.setpoint = left.start + (left.final - left.start) * t;
        right.setpoint = right.start + (right.final - right.start) * t;
    }

    HAL::Grip_Info_t info;
    HAL::ADC_GetInfo(&info);

    left.filtAngle += GRIP_ANGLE_FILTER_ALPHA * (info.angle_left - left.filtAngle);
    right.filtAngle += GRIP_ANGLE_FILTER_ALPHA * (info.angle_right - right.filtAngle);

    const float velMeasLeft = (left.filtAngle - left.prevFiltAngle) / dt;
    const float velMeasRight = (right.filtAngle - right.prevFiltAngle) / dt;
    left.prevFiltAngle = left.filtAngle;
    right.prevFiltAngle = right.filtAngle;

    // Ignore tiny measured velocities so the damping term does not react to
    // encoder noise while the motor is actually stationary.
    const float velDampLeft = fabsf(velMeasLeft) > GRIP_VEL_DEADBAND ? velMeasLeft : 0.0f;
    const float velDampRight = fabsf(velMeasRight) > GRIP_VEL_DEADBAND ? velMeasRight : 0.0f;

    float velRefLeft = 0.0f;
    float velRefRight = 0.0f;
    if (moving && moveEndMs > moveStartMs)
    {
        const float durSec = (float)(moveEndMs - moveStartMs) / 1000.0f;
        velRefLeft = (left.final - left.start) / durSec;
        velRefRight = (right.final - right.start) / durSec;
    }

    // Left motor
    const float errLeft = left.setpoint - left.filtAngle;
    float outLeft = 0.0f;
    if (!moving && fabsf(errLeft) <= GRIP_PID_DEADBAND)
    {
        left.integral = 0.0f;
    }
    else
    {
        float iLeft = 0.0f;
        if (!moving)
        {
            left.integral += left.ki * errLeft * dt;
            left.integral = clampOutput(left.integral);
            iLeft = left.integral;
        }
        outLeft = clampOutput(left.kp * errLeft +
                              left.kd * (velRefLeft - velDampLeft) +
                              left.ff * velRefLeft + iLeft);
    }

    // Right motor
    const float errRight = right.setpoint - right.filtAngle;
    float outRight = 0.0f;
    if (!moving && fabsf(errRight) <= GRIP_PID_DEADBAND)
    {
        right.integral = 0.0f;
    }
    else
    {
        float iRight = 0.0f;
        if (!moving)
        {
            right.integral += right.ki * errRight * dt;
            right.integral = clampOutput(right.integral);
            iRight = right.integral;
        }
        outRight = clampOutput(right.kp * errRight +
                               right.kd * (velRefRight - velDampRight) +
                               right.ff * velRefRight + iRight);
    }

    lastOutLeft = outLeft;
    lastOutRight = outRight;

    int dutyLeft = (int)(kLeftClampSign * outLeft);
    int dutyRight = (int)(kRightClampSign * outRight);

    // Current loop / gripping force limit. Both motors share one sense
    // resistor, so the limit acts on the combined duty of the two motors.
    if (currentLimitMa > 0.0f)
    {
        const float curErr = currentLimitMa - info.current_ma;
        currentIntegral += currentKi * curErr * dt;
        currentIntegral = constrain(currentIntegral, 0.0f, 2.0f * GRIP_PID_OUTPUT_MAX);

        float dutyLimit = currentKp * curErr + currentIntegral;
        dutyLimit = constrain(dutyLimit, 0.0f, 2.0f * GRIP_PID_OUTPUT_MAX);

        const int totalDuty = abs(dutyLeft) + abs(dutyRight);
        if (totalDuty > 0 && totalDuty > (int)dutyLimit)
        {
            const float scale = dutyLimit / (float)totalDuty;
            dutyLeft = (int)(dutyLeft * scale);
            dutyRight = (int)(dutyRight * scale);
        }
    }
    else
    {
        currentIntegral = 0.0f;
    }

    HAL::Motor_SetDuty(HAL::MOTOR_LEFT, dutyLeft);
    HAL::Motor_SetDuty(HAL::MOTOR_RIGHT, dutyRight);
}

void SetOpenAmount(float amount)
{
    if (amount < GRIP_OPEN_MIN)
        amount = GRIP_OPEN_MIN;
    if (amount > GRIP_OPEN_MAX)
        amount = GRIP_OPEN_MAX;

    const float ratio = amount / GRIP_OPEN_MAX;
    left.final = GRIP_LEFT_CLOSED_ANGLE +
                 (GRIP_LEFT_OPEN_ANGLE - GRIP_LEFT_CLOSED_ANGLE) * ratio;
    right.final = GRIP_RIGHT_CLOSED_ANGLE +
                  (GRIP_RIGHT_OPEN_ANGLE - GRIP_RIGHT_CLOSED_ANGLE) * ratio;

    beginMove(millis());
}

void Open()
{
    SetOpenAmount(GRIP_OPEN_MAX);
}

void SetGains(float kp, float ki, float kd)
{
    left.kp = right.kp = kp;
    left.ki = right.ki = ki;
    left.kd = right.kd = kd;
}

void SetIntegralGains(float leftKi, float rightKi)
{
    left.ki = constrain(leftKi, 0.0f, 200.0f);
    right.ki = constrain(rightKi, 0.0f, 200.0f);
}

void SetMotionSpeed(float degPerSec)
{
    if (degPerSec <= 0.0f)
        degPerSec = 1.0f;
    if (degPerSec > 100.0f)
        degPerSec = 100.0f;
    motionSpeedDegS = degPerSec;
}

void SetFeedforward(float leftFf, float rightFf)
{
    left.ff = constrain(leftFf, 0.0f, 10.0f);
    right.ff = constrain(rightFf, 0.0f, 10.0f);
}

void SetCurrentLimit(float milliAmps)
{
    if (milliAmps < 0.0f)
        milliAmps = 0.0f;
    if (milliAmps > GRIP_CURRENT_MAX_MA)
        milliAmps = GRIP_CURRENT_MAX_MA;

    currentLimitMa = milliAmps;
    if (currentLimitMa <= 0.0f)
        currentIntegral = 0.0f;
}

void SetCurrentGains(float kp, float ki)
{
    currentKp = constrain(kp, 0.0f, 20.0f);
    currentKi = constrain(ki, 0.0f, 100.0f);
}

void SetEnabled(bool flag)
{
    if (flag == enabled)
        return;

    enabled = flag;

    if (enabled)
    {
        HAL::Motor_Enable();
    }
    else
    {
        moving = false;
        HAL::Motor_Disable();

        HAL::Grip_Info_t info;
        HAL::ADC_GetInfo(&info);
        left.final = left.setpoint = info.angle_left;
        right.final = right.setpoint = info.angle_right;
        left.filtAngle = left.prevFiltAngle = info.angle_left;
        right.filtAngle = right.prevFiltAngle = info.angle_right;
        left.integral = 0.0f;
        right.integral = 0.0f;
    }
}

bool IsEnabled()
{
    return enabled;
}

void PrintStatus()
{
    HAL::Grip_Info_t info;
    HAL::ADC_GetInfo(&info);

    Serial.printf("[Grip] en=%d L:%.1f/%.1f R:%.1f/%.1f out=%.0f/%.0f\r\n",
                  enabled ? 1 : 0,
                  left.setpoint, info.angle_left,
                  right.setpoint, info.angle_right,
                  lastOutLeft, lastOutRight);
    Serial.printf("[Grip] cur=%.0fmA lim=%.0fmA\r\n", info.current_ma, currentLimitMa);
}

}  // namespace GripControl
}  // namespace Ctrl
