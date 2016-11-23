/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * This is a bare minimum user application template.
 *
 * For documentation of the board, go \ref group_common_boards "here" for a link
 * to the board-specific documentation.
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to board_init()
 * -# Basic usage of on-board LED and button
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include "conf_project.h"
#include "freertos_hooks.h"
#include "freertos_tasks.h"
#include "semphr.h"
#include "driver/include/m2m_wifi.h"
#include "network_module/WiFi_P2P.h"

#define STRING_EOL    "\r\n"
#define STRING_HEADER "-- Robo GOGO Robot Control MCU Program --"STRING_EOL \
"-- "BOARD_NAME " --"STRING_EOL	\
"-- Compiled: "__DATE__ " "__TIME__ " --"STRING_EOL



static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate =		CONF_UART_BAUDRATE,
		.charlength =	CONF_UART_CHAR_LENGTH,
		.paritytype =	CONF_UART_PARITY,
		.stopbits =		CONF_UART_STOP_BITS,
	};

	/* Configure UART console. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}

int main (void)
{
	sysclk_init();
	board_init();

	/* Insert application code here, after the board has been initialized. */

	
	configure_console();
	printf(STRING_HEADER);
	
	
#ifdef FREE_RTOS_LED_BLINKER_TASK
	/* Create LED task */
	if(xTaskCreate(task_led, "LED Task", TASK_LED_STACK_SIZE, NULL, TASK_LED_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create LED Task"STRING_EOL);
	} else {
		printf("Created LED Task"STRING_EOL);
	}
#endif

#ifdef FREE_RTOS_WINC_TASK
	/* Initialize the BSP. */
	nm_bsp_init();

	/* Initialize Wi-Fi parameters structure. */
	wifi_init();
	
	/* Start Wi-Fi P2P mode */
	wifi_p2p_start();
	
	vSemaphoreCreateBinary(xWINC_Semaphore_handle);
	xControl_Msg_Queue_handle = xQueueCreate(CONTROL_MSG_QUEUE_LENGTH, 2);
	
	/* Create WINC task */
	if (xTaskCreate(task_winc, "WINC Task", TASK_WINC_STACK_SIZE, NULL, TASK_LED_STACK_PRIORITY, NULL) != pdPASS )
	{
		printf("Failed to create WINC Task"STRING_EOL);
	} 
	else
	{
		printf("Created WINC Task"STRING_EOL);
	}
#endif

#ifdef FREE_RTOS_CONTROL_LOOP_TASK
	if (xTaskCreate(task_control_loop, "Control Loop Task", TASK_CONTROL_LOOP_STACK_SIZE, NULL, TASK_CONTROL_LOOP_PRIORITY, NULL) != pdPASS )
	{
		printf("Failed to create Control Loop Task"STRING_EOL);
	} 
	else
	{
		printf("Created Control Loop Task"STRING_EOL);
	}
#endif

#ifdef FREE_RTOS_SENDER_TASK
	if (xTaskCreate(task_sender, "Sender Task", TASK_CONTROL_LOOP_STACK_SIZE, NULL, TASK_CONTROL_LOOP_PRIORITY, NULL) != pdPASS)
	{
		printf("Failed to create Sender Task"STRING_EOL);
	} 
	else
	{
		printf("Created Sender Task"STRING_EOL);
	}
#endif

#ifdef FREE_RTOS_SEND_STATUS_TASK
	if (xTaskCreate(task_send_status, "Send Status Task", TASK_SEND_STATUS_SIZE, NULL, TASK_SEND_STATUS_PRIORITY, NULL) != pdPASS )
	{
		printf("Failed to create Send Status Task"STRING_EOL);
	}
	else
	{
		printf("Created Send Status Task"STRING_EOL);
	}
#endif

#ifdef FREE_RTOS_SCHEDULER
	vTaskStartScheduler();	
#endif

	while (1) {
		
		#ifdef ONLY_NETWORK_MODULE
			/* Handle pending events from network controller */
			m2m_wifi_handle_events(NULL);
		#endif
	}
}
