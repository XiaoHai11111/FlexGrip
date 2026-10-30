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

/* Finger Angle */
#define FINGER_OPEN             45
#define FINGER_CLOSE            0

/* Encoder Angle Offset */
#define ANGLE_OFFSET_LEFT       233
#define Angle_OFFSET_RIGHT      62

/* CAN */
#define CANR_PIN                4
#define CAND_PIN                5

#endif
