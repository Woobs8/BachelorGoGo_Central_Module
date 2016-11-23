#include <asf.h>
#include <pwm.h>
#include "conf_board.h"
#include "pwm_generator.h"

/* PWM Modules */
/** Configure and start PWM channel 1 */
void pwm_ch1_start(uint32_t freq, uint32_t period_value, uint32_t duty_cycle_value)
{
	// Configure IO pin
	pio_configure_pin(PWM1_DAC, PIO_TYPE_PIO_PERIPH_B);
	
	// Enable PWM peripheral clock
	pmc_enable_periph_clk(ID_PWM);
	pwm_channel_disable(PWM, PWM_CHANNEL_1);
	
	// Set PWM clock A as PWM_FREQUENCY*PERIOD_VALUE (clock B is not used).
	pwm_clock_t clock_setting = {
		.ul_clka = freq * period_value,
		.ul_clkb = 0,
		.ul_mck = sysclk_get_cpu_hz()
	};
	pwm_init(PWM, &clock_setting);
	
	// Configure PWM waveform
	g_pwm_channel_1.channel = PWM_CHANNEL_1;			// Set PWM channel
	g_pwm_channel_1.alignment = PWM_ALIGN_LEFT;			// Period is left-aligned.
	g_pwm_channel_1.polarity = PWM_HIGH;				// Output waveform starts at a low level.
	g_pwm_channel_1.ul_prescaler = PWM_CMR_CPRE_CLKA;	// Use PWM clock A as source clock.
	g_pwm_channel_1.ul_period = period_value;			// Period value of output waveform.
	g_pwm_channel_1.ul_duty = duty_cycle_value;			// Duty cycle value of output waveform.
	pwm_channel_init(PWM, &g_pwm_channel_1);
	
	// Enable output from PWM channel 1
	pwm_channel_enable(PWM, PWM_CHANNEL_1);	
}

/** Stop PWM channel 1 */
void pwm_ch1_stop(void)
{
	pwm_channel_disable(PWM, PWM_CHANNEL_1);	
}

/** Set duty cycle for PWM channel 1 */
void pwm_ch1_set_duty_cycle(uint32_t duty_cycle_value)
{
	g_pwm_channel_1.channel = PWM_CHANNEL_1;
	pwm_channel_update_duty(PWM, &g_pwm_channel_1, duty_cycle_value);	
}

///////////////////////////////////////////////////////////////////////////

/** Configure and start PWM channel 2 */
void pwm_ch2_start(uint32_t freq, uint32_t period_value, uint32_t duty_cycle_value)
{
	// Configure IO pin
	pio_configure_pin(PWM2_DAC, PIO_TYPE_PIO_PERIPH_B);
	
	// Enable PWM peripheral clock
	pmc_enable_periph_clk(ID_PWM);
	pwm_channel_disable(PWM, PWM_CHANNEL_2);
	
	// Set PWM clock A as PWM_FREQUENCY*PERIOD_VALUE (clock B is not used).
	pwm_clock_t clock_setting = {
		.ul_clka = freq * period_value,
		.ul_clkb = 0,
		.ul_mck = sysclk_get_cpu_hz()
	};
	pwm_init(PWM, &clock_setting);
	
	// Configure PWM waveform
	g_pwm_channel_2.channel = PWM_CHANNEL_2;			// Set PWM channel
	g_pwm_channel_2.alignment = PWM_ALIGN_LEFT;			// Period is left-aligned.
	g_pwm_channel_2.polarity = PWM_HIGH;				// Output waveform starts at a low level.
	g_pwm_channel_2.ul_prescaler = PWM_CMR_CPRE_CLKA;	// Use PWM clock A as source clock.
	g_pwm_channel_2.ul_period = period_value;			// Period value of output waveform.
	g_pwm_channel_2.ul_duty = duty_cycle_value;			// Duty cycle value of output waveform.
	pwm_channel_init(PWM, &g_pwm_channel_2);
	
	// Enable output from PWM channel 1
	pwm_channel_enable(PWM, PWM_CHANNEL_2);
}

/** Stop PWM channel 1 */
void pwm_ch2_stop(void)
{
	pwm_channel_disable(PWM, PWM_CHANNEL_2);
}

/** Set duty cycle for PWM channel 2 */
void pwm_ch2_set_duty_cycle(uint32_t duty_cycle_value)
{
	g_pwm_channel_1.channel = PWM_CHANNEL_2;
	pwm_channel_update_duty(PWM, &g_pwm_channel_2, duty_cycle_value);
}


/* Timer Counter waveforms */
/** Timer Counter 0 - Channel 0 - TIOA0 */
void tc0_ch0_waveform_start(uint32_t freq, float duty_cycle_a)
{
	uint32_t ra;
	
	if(duty_cycle_a > 1)
		duty_cycle_a = 1;
	else if(duty_cycle_a < 0)
		duty_cycle_a = 0;
	
	// Configure IO pin
	pio_configure_pin(TC0_TIOA_CH0, PIO_TYPE_PIO_PERIPH_B);
		
	pmc_enable_periph_clk(ID_TC0);
	
	tc_init(TC0, TC0_CHANNEL0,
	TC_CMR_TCCLKS_TIMER_CLOCK4		// MCK/128
	| TC_CMR_WAVE					// Waveform mode is enabled
	| TC_CMR_ACPA_SET				// RA Compare Effect: set
	| TC_CMR_ACPC_CLEAR				// TIOA RC Compare Effect: clear
	| TC_CMR_WAVSEL_UP_RC			// UP mode with automatic trigger on RC Compare
	);
	
	// Configure waveform frequency and duty cycle.
	tc0_ch0_rc = ((sysclk_get_peripheral_bus_hz(TC0) / TIMER_CLOCK_4_DIVISOR) / freq);	// RC = sys_clk / pre_scaler / freq
	tc_write_rc(TC0, TC0_CHANNEL0, tc0_ch0_rc);
	ra = ((1-duty_cycle_a) * tc0_ch0_rc);
	tc_write_ra(TC0, TC0_CHANNEL0, ra);
	tc_start(TC0, TC0_CHANNEL0);
}

/** Set duty cycle for TIOA0 waveform */
void tc0_ch0a_set_duty_cycle(float duty_cycle)
{
	if(duty_cycle > 1)
		duty_cycle = 1;
	else if(duty_cycle < 0)
		duty_cycle = 0;
		
	int ra;
	ra = ((1-duty_cycle) * tc0_ch0_rc);
	tc_write_ra(TC0, TC0_CHANNEL0, ra);
}

///////////////////////////////////////////////////////////////////////////

/** Timer Counter 0 - Channel 1 - TIOB1 */
void tc0_ch1_waveform_start(uint32_t freq, float duty_cycle_b)
{
	uint32_t rb;
	
	if(duty_cycle_b > 1)
		duty_cycle_b = 1;
	else if(duty_cycle_b < 0)
		duty_cycle_b = 0;
	
	// Configure IO pin
	pio_configure_pin(TC0_TIOB_CH1, PIO_TYPE_PIO_PERIPH_B);
	
	pmc_enable_periph_clk(ID_TC1);
	
	tc_init(TC0, TC0_CHANNEL1,
	TC_CMR_TCCLKS_TIMER_CLOCK4		// MCK/128
	| TC_CMR_EEVT_XC0				// Allow for TIOB to be an output
	| TC_CMR_WAVE					// Waveform mode is enabled
	| TC_CMR_BCPB_SET				// RB Compare Effect: set
	| TC_CMR_BCPC_CLEAR				// TIOB RC Compare Effect: clear
	| TC_CMR_WAVSEL_UP_RC			// UP mode with automatic trigger on RC Compare
	);

	// Configure waveform frequency and duty cycle.
	tc0_ch1_rc = ((sysclk_get_peripheral_bus_hz(TC0) / TIMER_CLOCK_4_DIVISOR) / freq);	// RC = sys_clk / pre_scaler / freq
	tc_write_rc(TC0, TC0_CHANNEL1, tc0_ch1_rc);
	rb = ((1-duty_cycle_b) * tc0_ch1_rc);
	tc_write_rb(TC0, TC0_CHANNEL1, rb);
	tc_start(TC0, TC0_CHANNEL1);	
}

/** Set duty cycle for TIOB1 waveform */
void tc0_ch1b_set_duty_cycle(float duty_cycle)
{
	if(duty_cycle > 1)
		duty_cycle = 1;
	else if(duty_cycle < 0)
		duty_cycle = 0;
	
	uint32_t rb;
	rb = ((1-duty_cycle) * tc0_ch1_rc);
	tc_write_rb(TC0, TC0_CHANNEL1, rb);
}

///////////////////////////////////////////////////////////////////////////

/** Timer Counter 1 - Channel 1 - TIOA4 & TIOB4 */
void tc1_ch1_waveform_start(uint32_t freq, float duty_cycle_a, float duty_cycle_b)
{
	uint32_t ra, rb;
	
	if(duty_cycle_a > 1)
		duty_cycle_a = 1;
	else if(duty_cycle_a < 0)
		duty_cycle_a = 0;
		
	if(duty_cycle_b > 1)
		duty_cycle_b = 1;
	else if(duty_cycle_b < 0)
		duty_cycle_b = 0;
	
	// Configure IO pins
	pio_configure_pin(TC1_TIOA_CH4, PIO_TYPE_PIO_PERIPH_B);
	pio_configure_pin(TC1_TIOB_CH4, PIO_TYPE_PIO_PERIPH_B);
	
	pmc_enable_periph_clk(ID_TC4);
	
	tc_init(TC1, TC1_CHANNEL1,
	TC_CMR_TCCLKS_TIMER_CLOCK4		// MCK/128
	| TC_CMR_EEVT_XC0				// Allow for TIOB to be an output
	| TC_CMR_WAVE					// Waveform mode is enabled
	| TC_CMR_ACPA_SET				// RA Compare Effect: set
	| TC_CMR_BCPB_SET				// RB Compare Effect: set
	| TC_CMR_ACPC_CLEAR				// TIOA RC Compare Effect: clear
	| TC_CMR_BCPC_CLEAR				// TIOB RC Compare Effect: clear
	| TC_CMR_WAVSEL_UP_RC			// UP mode with automatic trigger on RC Compare
	);
	
	// Configure waveform frequency and duty cycle.
	tc1_ch1_rc = ((sysclk_get_peripheral_bus_hz(TC1) / TIMER_CLOCK_4_DIVISOR) / freq);	// RC = sys_clk / pre_scaler / freq
	tc_write_rc(TC1, TC1_CHANNEL1, tc1_ch1_rc);
	ra = ((1-duty_cycle_a) * tc1_ch1_rc);
	rb = ((1-duty_cycle_b) * tc1_ch1_rc);
	tc_write_ra(TC1, TC1_CHANNEL1, ra);
	tc_write_rb(TC1, TC1_CHANNEL1, rb);
	tc_start(TC1, TC1_CHANNEL1);	
}

/** Set duty cycle for TIOA4 waveform */
void tc1_ch1a_set_duty_cycle(float duty_cycle)
{
	if(duty_cycle > 1)
		duty_cycle = 1;
	else if(duty_cycle < 0)
		duty_cycle = 0;
	
	uint32_t ra;
	ra = ((1-duty_cycle) * tc1_ch1_rc);
	tc_write_ra(TC1, TC1_CHANNEL1, ra);	
}

/** Set duty cycle for TIOB4 waveform */
void tc1_ch1b_set_duty_cycle(float duty_cycle)
{
	if(duty_cycle > 1)
		duty_cycle = 1;
	else if(duty_cycle < 0)
		duty_cycle = 0;
	
	uint32_t rb;
	rb = ((1-duty_cycle) * tc1_ch1_rc);
	tc_write_rb(TC1, TC1_CHANNEL1, rb);	
}

///////////////////////////////////////////////////////////////////////////

/** Timer Counter 1 - Channel 2 - TIOA5 & TIOB5 */
void tc1_ch2_waveform_start(uint32_t freq, float duty_cycle_a, float duty_cycle_b)
{
		uint32_t ra, rb;
		
	if(duty_cycle_a > 1)
		duty_cycle_a = 1;
	else if(duty_cycle_a < 0)
		duty_cycle_a = 0;
		
	if(duty_cycle_b > 1)
		duty_cycle_b = 1;
	else if(duty_cycle_b < 0)
		duty_cycle_b = 0;
		
		// Configure IO pins
		pio_configure_pin(TC1_TIOA_CH5, PIO_TYPE_PIO_PERIPH_B);
		pio_configure_pin(TC1_TIOB_CH5, PIO_TYPE_PIO_PERIPH_B);
		
		pmc_enable_periph_clk(ID_TC5);
		
		tc_init(TC1, TC1_CHANNEL2,
		TC_CMR_TCCLKS_TIMER_CLOCK4		// MCK/128
		| TC_CMR_EEVT_XC0				// Allow for TIOB to be an output
		| TC_CMR_WAVE					// Waveform mode is enabled
		| TC_CMR_ACPA_SET				// RA Compare Effect: set
		| TC_CMR_BCPB_SET				// RB Compare Effect: set
		| TC_CMR_ACPC_CLEAR				// TIOA RC Compare Effect: clear
		| TC_CMR_BCPC_CLEAR				// TIOB RC Compare Effect: clear
		| TC_CMR_WAVSEL_UP_RC			// UP mode with automatic trigger on RC Compare
		);
		
		// Configure waveform frequency and duty cycle.
		tc1_ch2_rc = ((sysclk_get_peripheral_bus_hz(TC1) / TIMER_CLOCK_4_DIVISOR) / freq);	// RC = sys_clk / pre_scaler / freq
		tc_write_rc(TC1, TC1_CHANNEL2, tc1_ch2_rc);
		ra = ((1-duty_cycle_a) * tc1_ch2_rc);
		rb = ((1-duty_cycle_b) * tc1_ch2_rc);
		tc_write_ra(TC1, TC1_CHANNEL2, ra);
		tc_write_rb(TC1, TC1_CHANNEL2, rb);
		tc_start(TC1, TC1_CHANNEL2);
}

/** Set duty cycle for TIOA5 waveform */
void tc1_ch2a_set_duty_cycle(float duty_cycle)
{
	if(duty_cycle > 1)
		duty_cycle = 1;
	else if(duty_cycle < 0)
		duty_cycle = 0;
	
	uint32_t ra;
	ra = ((1-duty_cycle) * tc1_ch2_rc);
	tc_write_ra(TC1, TC1_CHANNEL2, ra);
}

/** Set duty cycle for TIOB5 waveform */
void tc1_ch2b_set_duty_cycle(float duty_cycle)
{
	if(duty_cycle > 1)
		duty_cycle = 1;
	else if(duty_cycle < 0)
		duty_cycle = 0;
	
	uint32_t rb;
	rb = ((1-duty_cycle) * tc1_ch2_rc);
	tc_write_rb(TC1, TC1_CHANNEL2, rb);
}
