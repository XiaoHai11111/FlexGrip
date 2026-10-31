#ifndef __CONFIG_H
#define __CONFIG_H

/*=========================
   Application configuration
 *=========================*/

#define CONFIG_SYSTEM_SAVE_FILE_PATH          "S:/SystemSave.json"
#define CONFIG_SYSTEM_LANGUAGE_DEFAULT        {'e','n','-','G','B'}
#define CONFIG_SYSTEM_GMT_OFFSET_DEFAULT      8 // GMT+ 8


/*=========================
   Hardware Configuration
 *=========================*/

#define NULL_PIN                    PD0

/* ADC */
#define ANGLE_LEFT_PIN          13
#define ANGLE_RIGHT_PIN         15
#define CURRENT_INPUT_PIN       12
#define CURRENT_PIN             14

/* Motor Input */
#define MOTOR_RIGHT_AIN1_PIN    27
#define MOTOR_RIGHT_AIN2_PIN    26
#define MOTOR_LEFT_BIN1_PIN     33
#define MOTOR_LEFT_BIN2_PIN     25

/* Motor Speed */
#define MAX_SPEED 255
#define MIN_SPEED 0

/* Angle sensor calibration: ADC raw -> mV -> degree */
#define ANGLE_ADC_RAW_MAX           4095.0f
#define ANGLE_ADC_VOLTAGE_MAX       3300.0f
#define ANGLE_ADC_VOLTAGE_SCALE     2500.0f
#define ANGLE_ADC_DEGREE_SCALE      333.3f

/* Gripper kinematics (degree, values read from the angle sensors) */
#define GRIP_LEFT_CLOSED_ANGLE      143.0f
#define GRIP_LEFT_OPEN_ANGLE        98.0f
#define GRIP_RIGHT_CLOSED_ANGLE     149.0f
#define GRIP_RIGHT_OPEN_ANGLE       194.0f

/* Gripper command limits (0 = closed/clamped, 45 = fully open) */
#define GRIP_OPEN_MIN               0.0f
#define GRIP_OPEN_MAX               45.0f

/* Position PID */
#define GRIP_PID_KP                 15.0f
#define GRIP_PID_KD                 1.0f
#define GRIP_PID_OUTPUT_MAX         255.0f
#define GRIP_PID_DEADBAND           1.0f
#define GRIP_ANGLE_FILTER_ALPHA     0.35f
#define GRIP_LEFT_KI                50.0f
#define GRIP_RIGHT_KI               6.0f

/* Motion profile (same speed for both motors -> synchronised motion) */
#define GRIP_MOTION_SPEED_DEG_S     40.0f
#define GRIP_LEFT_FF                4.5f
#define GRIP_RIGHT_FF               2.2f
#define GRIP_VEL_DEADBAND           30.0f

/* Current sense (1 ohm resistor in series with the motor supply) */
#define GRIP_CURRENT_GAIN           1.0f
#define GRIP_CURRENT_MAX_MA         1000.0f

/* Current loop (gripping force control) */
#define GRIP_CURRENT_DEFAULT_MA     0.0f
#define GRIP_CURRENT_KP             0.5f
#define GRIP_CURRENT_KI             2.0f

/* CAN protocol (DummyHand compatible) */
#define GRIP_CAN_NODE_ID            7
#define GRIP_CAN_MODE_SET_CURRENT   0x01
#define GRIP_CAN_MODE_SET_ANGLE     0x02

/* CAN */
#define CANR_PIN                4
#define CAND_PIN                5

/* Encoder Angle Offset (kept for future calibration, currently unused) */
#define ANGLE_OFFSET_LEFT       233
#define ANGLE_OFFSET_RIGHT      62

#endif
