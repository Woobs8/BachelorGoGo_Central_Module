#include <asf.h>

/****************************************************************
 * This module can generate eight customizable PWM waveform outputs
 * on the following pins:
 * 
 *		Signal		I/O Line		SAM4S Xplained Pro pin
 *		PWM1H		PC19				Ext2 - pin 7
 *		PWM2H		PC20				Ext3 - pin 7
 *		TIOA0		PA0					Ext3 - pin 9
 *		TIOB1		PA16				Ext3 - pin 8
 *		TIOA4		PC26				Ext2 - pin 9
 *		TIOB4		PC27				Ext2 - pin 10
 *		TIOA5		PC29				Ext3 - pin 3
 *		TIOB5		PC30				Ext3 - pin 4
 *
 * The PWM waveforms are customizable in the following ways:
 *	- PWM1H: independent frequency, independent duty cycle
 *	- PWM2H: independent frequency, independent duty cycle
 *	- TIOA0: < 937kHz, independent duty cycle
 *	- TIOB1: < 937kHz, independent duty cycle
 *	- TIOA4: < 937kHz, independent duty cycle
 *	- TIOB4: Same frequency as TIOA4, independent duty cycle
 *	- TIOA5: < 937kHz, independent duty cycle
 *	- TIOB5: Same frequency as TIOA5, independent duty cycle
 *
 ****************************************************************/

#ifndef PWM_H_
#define PWM_H_

// Configure IO ports for PWM
#define PWM1_DAC IOPORT_CREATE_PIN(PIOC, 19)		// PWM1H
#define PWM2_DAC IOPORT_CREATE_PIN(PIOC, 20)		// PWM2H

// Configure IO ports for Timer Counters
#define TC0_TIOA_CH0 IOPORT_CREATE_PIN(PIOA, 0)		// TIOA0
#define TC0_TIOB_CH1 IOPORT_CREATE_PIN(PIOA, 16)	// TIOB1
#define TC1_TIOA_CH4 IOPORT_CREATE_PIN(PIOC, 26)	// TIOA4
#define TC1_TIOB_CH4 IOPORT_CREATE_PIN(PIOC, 27)	// TIOB4
#define TC1_TIOA_CH5 IOPORT_CREATE_PIN(PIOC, 29)	// TIOA5
#define TC1_TIOB_CH5 IOPORT_CREATE_PIN(PIOC, 30)	// TIOB5

//Timer Counter channels
#define TC0_CHANNEL0 0
#define TC0_CHANNEL1 1
#define TC0_CHANNEL2 2
#define TC1_CHANNEL0 0
#define TC1_CHANNEL1 1
#define TC1_CHANNEL2 2

//Timer Module clock divisors
#define TIMER_CLOCK_1_DIVISOR 2
#define TIMER_CLOCK_2_DIVISOR 8
#define TIMER_CLOCK_3_DIVISOR 32
#define TIMER_CLOCK_4_DIVISOR 128
#define TIMER_CLOCK_5_DIVISOR 1

/** PWM1 */
pwm_channel_t g_pwm_channel_1;
void pwm_ch1_start(uint32_t freq, uint32_t period_value, uint32_t duty_cycle_value);
void pwm_ch1_stop(void);
void pwm_ch1_set_duty_cycle(uint32_t duty_cycle_value);

/** PWM2 */
pwm_channel_t g_pwm_channel_2;
void pwm_ch2_start(uint32_t freq, uint32_t period_value, uint32_t duty_cycle_value);
void pwm_ch2_stop(void);
void pwm_ch2_set_duty_cycle(uint32_t duty_cycle_value);

/** Timer Counter 0 - Channel 0 - TIOA0 */
uint32_t tc0_ch0_rc;											// Used to set duty cycle
void tc0_ch0_waveform_start(uint32_t freq, float duty_cycle_a);
void tc0_ch0a_set_duty_cycle(float duty_cycle);				// TIOA0

/** Timer Counter 0 - Channel 1 - TIOB1 */
uint32_t tc0_ch1_rc;											// Used to set duty cycle
void tc0_ch1_waveform_start(uint32_t freq, float duty_cycle_b);
void tc0_ch1b_set_duty_cycle(float duty_cycle);				// TIOB1

/** Timer Counter 1 - Channel 1 - TIOA4 & TIOB4 */
uint32_t tc1_ch1_rc;											// Used to set duty cycle
void tc1_ch1_waveform_start(uint32_t freq, float duty_cycle_a, float duty_cycle_b);
void tc1_ch1a_set_duty_cycle(float duty_cycle);				// TIOA4
void tc1_ch1b_set_duty_cycle(float duty_cycle);				// TIOB4	

/** Timer Counter 1 - Channel 2 - TIOA5 & TIOB5 */
uint32_t tc1_ch2_rc;											// Used to set duty cycle
void tc1_ch2_waveform_start(uint32_t freq, float duty_cycle_a, float duty_cycle_b);
void tc1_ch2a_set_duty_cycle(float duty_cycle);				// TIOA5
void tc1_ch2b_set_duty_cycle(float duty_cycle);				// TIOB5


#endif /* PWM_H_ */