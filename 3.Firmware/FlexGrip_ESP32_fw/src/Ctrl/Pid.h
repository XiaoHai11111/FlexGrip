#ifndef __PID_H
#define __PID_H

namespace Ctrl
{

// Classic positional PID controller (derivative on error) with
// conditional-integration anti-windup. Gains and output limits are exposed so
// the class can be reused by any control loop in this project.
class PID
{
public:
    PID();
    PID(float kp, float ki, float kd, float outMin, float outMax);

    void SetGains(float kp, float ki, float kd);
    void SetOutputLimits(float outMin, float outMax);
    void SetTarget(float target);
    void Reset(float measured = 0.0f);

    float GetTarget() const;
    float GetError() const;   // error of the last Compute() call
    float GetOutput() const;  // output of the last Compute() call

    // @param measured current process value
    // @param dt       elapsed time in seconds since the previous call
    // @return         control output, clamped to the configured limits
    float Compute(float measured, float dt);

private:
    float kp_;
    float ki_;
    float kd_;
    float outMin_;
    float outMax_;

    float target_;
    float prevError_;
    float integral_;
    float output_;
};

}  // namespace Ctrl

#endif  // __PID_H
