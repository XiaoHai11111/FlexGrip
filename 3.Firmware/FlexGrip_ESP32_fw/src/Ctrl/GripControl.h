#ifndef __GRIP_CONTROL_H
#define __GRIP_CONTROL_H

namespace Ctrl
{
namespace GripControl
{

// Initialise the position controllers and hold the current position on boot.
void Init();

// Run one control step (called from HAL::Update at the loop rate).
void Update(float dt);

// Set the gripper opening amount in degrees.
//   0  -> closed / clamped (left 143 deg, right 149 deg)
//   45 -> fully open       (left 98 deg,  right 194 deg)
void SetOpenAmount(float amount);

// Fully open the gripper (same as SetOpenAmount(45)).
void Open();

// Set position PID gains for both motors (used for live tuning).
void SetGains(float kp, float ki, float kd);

// Set the hold integral gain per motor (breaks static friction on the last bit).
void SetIntegralGains(float left, float right);

// Set the common motion speed (deg/s) used to synchronise both motors.
void SetMotionSpeed(float degPerSec);

// Set per-motor velocity feedforward (PWM per deg/s).
void SetFeedforward(float left, float right);

void SetEnabled(bool enabled);
bool IsEnabled();

// Print a one-line status report over the serial monitor.
void PrintStatus();

}  // namespace GripControl
}  // namespace Ctrl

#endif  // __GRIP_CONTROL_H
