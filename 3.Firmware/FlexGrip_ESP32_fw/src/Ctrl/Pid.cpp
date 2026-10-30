#include "Ctrl/Pid.h"

#include <math.h>

namespace Ctrl
{

PID::PID()
    : kp_(0.0f),
      ki_(0.0f),
      kd_(0.0f),
      outMin_(-255.0f),
      outMax_(255.0f),
      target_(0.0f),
      prevError_(0.0f),
      integral_(0.0f),
      output_(0.0f)
{
}

PID::PID(float kp, float ki, float kd, float outMin, float outMax)
    : kp_(kp),
      ki_(ki),
      kd_(kd),
      outMin_(outMin),
      outMax_(outMax),
      target_(0.0f),
      prevError_(0.0f),
      integral_(0.0f),
      output_(0.0f)
{
}

void PID::SetGains(float kp, float ki, float kd)
{
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
}

void PID::SetOutputLimits(float outMin, float outMax)
{
    if (outMin > outMax)
    {
        float tmp = outMin;
        outMin = outMax;
        outMax = tmp;
    }

    outMin_ = outMin;
    outMax_ = outMax;
}

void PID::SetTarget(float target)
{
    target_ = target;
}

void PID::Reset(float measured)
{
    (void)measured;
    prevError_ = 0.0f;
    integral_ = 0.0f;
    output_ = 0.0f;
}

float PID::GetTarget() const
{
    return target_;
}

float PID::GetError() const
{
    return prevError_;
}

float PID::GetOutput() const
{
    return output_;
}

float PID::Compute(float measured, float dt)
{
    if (dt <= 0.0f)
        dt = 0.001f;

    const float error = target_ - measured;

    // Proportional and derivative (on error) terms.
    const float p = kp_ * error;
    const float d = kd_ * (error - prevError_) / dt;

    // Conditional integration (anti-windup): skip accumulating when the
    // output is saturated and the error would push it further into saturation.
    const bool saturatingUp = (output_ >= outMax_ && error > 0.0f);
    const bool saturatingDown = (output_ <= outMin_ && error < 0.0f);
    if (!saturatingUp && !saturatingDown)
        integral_ += ki_ * error * dt;

    // Hard clamp of the integrator as a safety net.
    const float integralLimit = fabsf(outMax_ - outMin_);
    if (integral_ > integralLimit)
        integral_ = integralLimit;
    else if (integral_ < -integralLimit)
        integral_ = -integralLimit;

    float output = p + integral_ + d;
    if (output > outMax_)
        output = outMax_;
    else if (output < outMin_)
        output = outMin_;

    prevError_ = error;
    output_ = output;

    return output;
}

}  // namespace Ctrl
