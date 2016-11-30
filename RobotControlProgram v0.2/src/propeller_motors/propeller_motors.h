#include "pwm_generator/pwm_generator.h"

#ifndef PROPELLER_MOTORS_H_
#define PROPELLER_MOTORS_H_

#define MOTOR_CTRL_PWM_FREQUENCY_HZ		(100)
#define MOTOR_SPEED_RESOLUTION			(100)
#define MOTOR_ON_MIN_DUTY_CYCLE			(20)
#define MOTOR_ON_MAX_DUTY_CYCLE			(80)
#define MOTOR_CTRL_PERIOD_REG			(3000)
#define DUTY_CYCLE_REG_PER_SPEED_PCT	(MOTOR_CTRL_PERIOD_REG*(MOTOR_ON_MAX_DUTY_CYCLE-MOTOR_ON_MIN_DUTY_CYCLE)/100)/MOTOR_SPEED_RESOLUTION
#define DUTY_CYCLE_REG_MIN				(MOTOR_CTRL_PERIOD_REG*MOTOR_ON_MIN_DUTY_CYCLE/100)
#define MOTOR_OFF_DUTY_CYCLE			10
#define MOTOR_OFF_DUTY_CYCLE_REG		(MOTOR_CTRL_PERIOD_REG*MOTOR_OFF_DUTY_CYCLE)/100

/* Propeller 1 motor - PWM1H/PC19/Ext2-7 */
void propeller_motor1_start(uint8_t speed);
void propeller_motor1_stop(void);
void propeller_motor1_set_speed(uint8_t speed);

/* Propeller 2 motor - PWM2H/PC20/Ext3-7 */
void propeller_motor2_start(uint8_t speed);
void propeller_motor2_stop(void);
void propeller_motor2_set_duty_cycle(uint8_t speed);

#endif /* PROPELLER_MOTORS_H_ */