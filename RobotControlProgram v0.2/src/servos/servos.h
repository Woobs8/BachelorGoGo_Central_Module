/****************************************************************
 * This module can control the six analog servo motors.
 * The motor should be connected on the following pins:
 * 
 *		Servo:					I/O Line:		SAM4S Xplained Pro pin:
 *		Propeller 1 inner		PC26				Ext2 - pin 9
 *		Propeller 1 outer		PC27				Ext2 - pin 10
 *		Propeller 2 inner		PC29				Ext3 - pin 3
 *		Propeller 2 outer		PC30				Ext3 - pin 4
 *		Left wheel				PA0					Ext3 - pin 9
 *		Right wheel				PA16				Ext3 - pin 8
 *
 * The servos can be controlled in the following ways:
 *		- Wheel servos and inner propeller servos can rotate a maximum of 
 *		  180 deg. from their starting position
 *
 *		- Outer propeller servos can rotate a maximum of 180 deg. from
 *		  their starting position
 *
 ****************************************************************/

#ifndef SERVOS_H_
#define SERVOS_H_

// PWM frequency in Hz.
#define SERVO_PWM_FREQUENCY 50
#define MOTOR_CTRL_PWM_FREQUENCY 100

// Servo properties
#define SMALL_SERVO_DEG				180
#define SMALL_SERVO_DC_MIN			0.05		// Left - 0 deg.
#define SMALL_SERVO_DC_MAX			0.1			// Right - 180 deg.
#define SMALL_SERVO_DEG_PER_PCT_DC	(SMALL_SERVO_DC_MAX-SMALL_SERVO_DC_MIN)/SMALL_SERVO_DEG
#define LARGE_SERVO_DEG				180	
#define LARGE_SERVO_DC_MIN			0.05		// Left - 0 deg.
#define LARGE_SERVO_DC_MAX			0.1			// Right - 90 deg.
#define LARGE_SERVO_DEG_PER_PCT_DC	(LARGE_SERVO_DC_MAX-LARGE_SERVO_DC_MIN)/LARGE_SERVO_DEG
#define INNER_SERVO_STARTING_DC		0.075
#define OUTER_SERVO_STARTING_DC		0.075

// Demo setup
#define INNER_SERVO_OFFSET			90
#define OUTER_SERVO_OFFSET			90

/** Propeller 1 servos */
void servos_propeller1_start(void);
/* Inner Servo - TIA4/PC26/Ext2-9 */
void servos_propeller1_inner_set_position(uint8_t deg);
/* Outer Servo - TIB4/PC27/Ext2-10 */
void servos_propeller1_outer_set_position(uint8_t deg);

/** Propeller 2 servos */
void servos_propeller2_start(void);
/* Inner Servo - TIOA5/PC29/Ext3-3 */
void servos_propeller2_inner_set_position(uint8_t deg);
/* Outer Servo - TIOB5/PC30/Ext3-4 */
void servos_propeller2_outer_set_position(uint8_t deg);

/** Wheel servos */
/* Left wheel servo - TIOA0/PA0/Ext3-9 */
void servos_left_wheel_start(void);
void servos_left_wheel_set_position(uint8_t deg);
/* Right wheel servo - TIOB1/PA16/Ext3-8 */
void servos_right_wheel_start(void);
void servos_right_wheel_set_position(uint8_t deg);


#endif /* SERVOS_H_ */