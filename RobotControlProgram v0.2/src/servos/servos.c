#include <asf.h>
#include "servos.h"
#include "PWM_Generator/pwm_generator.h"

/************************** Propeller 1  **************************/

/** Start the servo motors for propeller 1
  * Servo starting positions are extreme left, which corresponds to a duty cycle of 1ms or 5% of the 50Hz PWM.  
 */
void servos_propeller1_start(void)
{
	tc1_ch1_waveform_start(SERVO_PWM_FREQUENCY,INNER_SERVO_STARTING_DC,OUTER_SERVO_STARTING_DC);
}

/** Set servo position of the inner servo motor for propeller 1
  *
  * Servo position is encoded in the PWM duty cycle. The servo have a position from 
  * 0-180 deg, which is encoded as a duty cycle between 1-2ms or 5-10%.
 */
void servos_propeller1_inner_set_position(uint8_t deg)
{
	// Servo max rotation is 180 deg.
	if(deg > 180)
		deg = 180;
		
	float duty_cycle = SMALL_SERVO_DC_MIN + deg*SMALL_SERVO_DEG_PER_PCT_DC;
	tc1_ch1a_set_duty_cycle(duty_cycle);
}

/** Set servo position of the outer servo motor for propeller 1
  *
  * Servo position is encoded in the PWM duty cycle. The servo has a position from 
  * 0-180 deg, which is encoded as a duty cycle between 1-2ms or 5-10%.
 */
void servos_propeller1_outer_set_position(uint8_t deg)
{
	// Servo max rotation is 90 deg.
	if(deg > 180)
		deg = 180;
		
	float duty_cycle = LARGE_SERVO_DC_MIN + deg*LARGE_SERVO_DEG_PER_PCT_DC;
	tc1_ch1b_set_duty_cycle(duty_cycle);
}


/************************** Propeller 2 **************************/

/** Start the servo motors for propeller 2
  * Servo starting positions are extreme left, which corresponds to a duty cycle of 1ms or 5% of the 50Hz PWM.  
 */
void servos_propeller2_start(void)
{
	tc1_ch2_waveform_start(SERVO_PWM_FREQUENCY,0.05,0.05);
}

/** Set servo position of the inner servo motor for propeller 2
  *
  * Servo position is encoded in the PWM duty cycle. The servo have a position from 
  * 0-180 deg, which is encoded as a duty cycle between 1-2ms or 5-10%.
 */
void servos_propeller2_inner_set_position(uint8_t deg)
{
	// Servo max rotation is 180 deg.
	if(deg > 180)
		deg = 180;
		
	float duty_cycle = SMALL_SERVO_DC_MIN + deg*SMALL_SERVO_DEG_PER_PCT_DC;
	tc1_ch2a_set_duty_cycle(duty_cycle);
}

/** Set servo position of the inner servo motor for propeller 2
  *
  * Servo position is encoded in the PWM duty cycle. The servo have a position from 
  * 0-180 deg, which is encoded as a duty cycle between 1-2ms or 5-10%.
 */
void servos_propeller2_outer_set_position(uint8_t deg)
{
	// Servo max rotation is 90 deg.
	if(deg > 180)
		deg = 180;
		
	float duty_cycle = LARGE_SERVO_DC_MIN + deg*LARGE_SERVO_DEG_PER_PCT_DC;
	tc1_ch2b_set_duty_cycle(duty_cycle);
}


/************************** Left wheel **************************/
void servos_left_wheel_start(void)
{
	tc0_ch0_waveform_start(SERVO_PWM_FREQUENCY,0.075);
}
void servos_left_wheel_set_position(uint8_t deg)
{
		// Servo max rotation is 180 deg.
	if(deg > 180)
		deg = 180;
		
	float duty_cycle = SMALL_SERVO_DC_MIN + deg*SMALL_SERVO_DEG_PER_PCT_DC;
	tc0_ch0a_set_duty_cycle(duty_cycle);
}


/************************** Right wheel **************************/
void servos_right_wheel_start(void)
{
	tc0_ch1_waveform_start(SERVO_PWM_FREQUENCY,0.075);
}
void servos_right_wheel_set_position(uint8_t deg)
{
		// Servo max rotation is 180 deg.
	if(deg > 180)
		deg = 180;
		
	float duty_cycle = SMALL_SERVO_DC_MIN + deg*SMALL_SERVO_DEG_PER_PCT_DC;
	tc0_ch1b_set_duty_cycle(duty_cycle);
}