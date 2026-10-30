#ifndef __HAL_DEF_H
#define __HAL_DEF_H

#include <stdint.h>

namespace HAL
{
    /* The angle，speed and current of motor */
    typedef struct
    {
        float angle_left;
        float angle_right;
        float speed_left;
        float speed_right;
        float current;
    } Grip_Info_t;

    typedef struct
    {
        double Kp;
        double Ki;
        double Kd;
        double previousError;
        double integral;
    } PID_Param_t;

}

#endif
