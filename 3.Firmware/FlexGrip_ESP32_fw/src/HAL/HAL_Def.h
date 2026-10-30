#ifndef __HAL_DEF_H
#define __HAL_DEF_H

#include <stdint.h>

namespace HAL
{
    /* Motor selection */
    typedef enum
    {
        MOTOR_LEFT = 0,
        MOTOR_RIGHT = 1
    } Motor_t;

    /* The angle，speed and current of motor */
    typedef struct
    {
        float angle_left;
        float angle_right;
        float speed_left;
        float speed_right;
        float current;
    } Grip_Info_t;

}

#endif
