#include "propeller_motors.h"
#include "pwm_generator/pwm_generator.h"

/* Propeller 1 motor - PWM1H/PC19/Ext2-7 */
void propeller_motor1_start(uint8_t speed)
{
	if(speed > 100)
		speed = 100;
	
	if(speed > 0) {		// Motor should spin
		uint32_t duty_cycle_reg = DUTY_CYCLE_REG_MIN + speed*DUTY_CYCLE_REG_PER_SPEED_PCT;
		pwm_ch1_start(MOTOR_CTRL_PWM_FREQUENCY_HZ,MOTOR_CTRL_PERIOD_REG,duty_cycle_reg);
	} else {			// Motor should stop
		pwm_ch1_start(MOTOR_CTRL_PWM_FREQUENCY_HZ,MOTOR_CTRL_PERIOD_REG,MOTOR_OFF_DUTY_CYCLE_REG);
	}
}

void propeller_motor1_stop()
{
	pwm_ch1_set_duty_cycle(MOTOR_OFF_DUTY_CYCLE_REG);
}

void propeller_motor1_set_speed(uint8_t speed)
{
	if(speed > 100)
		speed = 100;
	
	if(speed > 0) {		// Motor should spin
		uint32_t duty_cycle_reg = DUTY_CYCLE_REG_MIN + speed*DUTY_CYCLE_REG_PER_SPEED_PCT;
		pwm_ch1_set_duty_cycle(duty_cycle_reg);
	} else {			// Motor should stop
		pwm_ch1_set_duty_cycle(MOTOR_OFF_DUTY_CYCLE_REG);
	}
}



/* Propeller 2 motor - PWM2H/PC20/Ext3-7 */
void propeller_motor2_start(uint8_t speed)
{
	if(speed > 100)
	speed = 100;
	
	if(speed > 0) {		// Motor should spin
		uint32_t duty_cycle_reg = DUTY_CYCLE_REG_MIN + speed*DUTY_CYCLE_REG_PER_SPEED_PCT;
		pwm_ch2_start(MOTOR_CTRL_PWM_FREQUENCY_HZ,MOTOR_CTRL_PERIOD_REG,duty_cycle_reg);
		} else {			// Motor should stop
		pwm_ch2_start(MOTOR_CTRL_PWM_FREQUENCY_HZ,MOTOR_CTRL_PERIOD_REG,MOTOR_OFF_DUTY_CYCLE_REG);
	}
}

void propeller_motor2_stop()
{
	pwm_ch2_set_duty_cycle(MOTOR_OFF_DUTY_CYCLE_REG);
}

void propeller_motor2_set_speed(uint8_t speed)
{
	if(speed > 100)
	speed = 100;
	
	if(speed > 0) {		// Motor should spin
		uint32_t duty_cycle_reg = DUTY_CYCLE_REG_MIN + speed*DUTY_CYCLE_REG_PER_SPEED_PCT;
		pwm_ch2_set_duty_cycle(duty_cycle_reg);
		} else {			// Motor should stop
		pwm_ch2_set_duty_cycle(MOTOR_OFF_DUTY_CYCLE_REG);
	}
}