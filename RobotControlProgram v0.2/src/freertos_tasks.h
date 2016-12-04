#ifndef FREERTOS_TASKS_H_
#define FREERTOS_TASKS_H_

#include "driver/include/m2m_wifi.h"
#include "network_module/WiFi_P2P.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "protocol.h"
#include "network_module/Networking.h"
#include "network_module/WiFi_P2P.h"
#include "nand_flash_storage/nand_flash_storage.h"
#include "propeller_motors/propeller_motors.h"
#include "servos/servos.h"
#include <math.h>
#include <string.h>

#define TASK_LED_STACK_SIZE					(1024/sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY				(tskIDLE_PRIORITY)
#define TASK_LED_DELAY						(1000)

#define TASK_WINC_STACK_SIZE				(8192/sizeof(portSTACK_TYPE))
#define TASK_WINC_STACK_PRIORITY			(tskIDLE_PRIORITY)

#define TASK_SEND_STATUS_SIZE				(4096/sizeof(portSTACK_TYPE))
#define TASK_SEND_STATUS_PRIORITY			(tskIDLE_PRIORITY)
#define TASK_SEND_STATUS_DELAY				(20000)

#define TASK_CONTROL_LOOP_STACK_SIZE		(4096/sizeof(portSTACK_TYPE))
#define TASK_CONTROL_LOOP_PRIORITY			(tskIDLE_PRIORITY)
#define TASK_CONTROL_INPUT_VALID_TIME_MS	(300)
#define TASK_CONTROL_NO_INPUT_TIME_MS		(0)
#define TASK_CONTROL_CHECK_SETTINGS_TIME_MS	(10000)

#define CONTROL_MSG_QUEUE_LENGTH			(1)
#define CONTROL_MSG_QUEUE_ITEM_SIZE			(2)

#define SETTINGS_MSG_QUEUE_LENGTH			(1)
#define SETTINGS_MSG_QUEUE_ITEM_SIZE		(3)

#define NAME_QUEUE_LENGTH					(1)
#define NAME_QUEUE_ITEM_SIZE				(31)

xSemaphoreHandle xWINC_Semaphore_handle;
xQueueHandle xControl_Msg_Queue_handle;
xQueueHandle xSettings_Msg_Queue_handle;
xQueueHandle xName_Queue_handle;


/**
 * \brief This task, when activated, make LED blink at a fixed rate
 */
static void task_led(void *pvParameters)
{
	UNUSED(pvParameters);
	for (;;) {
		LED_Toggle(LED0);
		vTaskDelay(TASK_LED_DELAY);
	}
}

static void task_winc(void *pvParameters)
{
	UNUSED(pvParameters);
	
	/* Read persisted settings from non-volatile memory 
	 *
	 * Must be executed after FreeFRTOS scheduler has been started. */
	printf("-I- Restoring settings from non-volatile memory...\r\n");
	uint8_t buf[PAGE_SIZE];
	int16_t ret = nand_flash_storage_read(buf);
	if(ret > 0)
	{
		/* Clear non-data bits */
		memset(buf+ret,0,PAGE_SIZE-ret);
		int8_t ret = network_message_handler(buf);
		if(ret == PARSER_SUCCESS) {
			printf("-I- Settings successfully restored\r\n");
		} else if (ret == PARSER_ERROR) {
			iPower_save_mode = DEFAULT_POWER_SAVE_MODE;
			iAssisted_drive_mode = DEFAULT_ASSISTED_DRIVE_MODE;
			uiVideo_quality = DEFAULT_VIDEO_QUALITY;
			
			ret = apply_default_settings();
			if(ret != PARSER_SUCCESS)
				while(1) {
				}
			printf("-I- No settings found. Default settings applied.\r\n");
		} else {
			iPower_save_mode = DEFAULT_POWER_SAVE_MODE;
			iAssisted_drive_mode = DEFAULT_ASSISTED_DRIVE_MODE;
			uiVideo_quality = DEFAULT_VIDEO_QUALITY;
			
			ret = apply_default_settings();
				if(ret != PARSER_SUCCESS)
					while(1) {
					}
			printf("-E- Error restoring settings. Default settings applied.\r\n");
		}
	} else {
		iPower_save_mode = DEFAULT_POWER_SAVE_MODE;
		iAssisted_drive_mode = DEFAULT_ASSISTED_DRIVE_MODE;
		uiVideo_quality = DEFAULT_VIDEO_QUALITY;
		
		ret = apply_default_settings();
			if(ret != PARSER_SUCCESS)
				while(1) {
				}
		printf("-E- Error retrieving settings. Default settings applied.\r\n");
	}
		
	/* Start Wi-Fi P2P mode */
	wifi_p2p_start();
		
	for(;;) {
		xSemaphoreTake(xWINC_Semaphore_handle, portMAX_DELAY);
		m2m_wifi_handle_events(NULL);
	}
}

/* Periodically send a status message to the remote control app containing
 * information about the current state of the robot. 
 *
 * Battery status and storage space are currently not implemented, so these
 * are simulated.
 */
static void task_send_status(void *pvParameters)
{
	UNUSED(pvParameters);
	int8_t iBattery = 100;
	uint8_t name_buf[NAME_QUEUE_ITEM_SIZE];
	portBASE_TYPE xStatus = pdFAIL;
	uint16_t storage_space = 200;
	uint16_t remaining_space = 100;
	char status_packet[PACKET_SIZE] = {0};
	vTaskDelay(TASK_SEND_STATUS_DELAY);
	for (;;) {
		if(network_is_connected == IS_CONNECTED) {
			// Read device name from queue
			xStatus = xQueuePeek(xName_Queue_handle,name_buf,0);
			if(xStatus == pdPASS) {						// Only send status if device name is read
				uint8_t name_len = name_buf[NAME_SIZE];
				char cName[name_len];
				memcpy(cName,&name_buf[NAME],name_len);
				
				// Generate status packet
				if(generate_status_packet(status_packet, &cName, iBattery, 1, storage_space, remaining_space) == -1 ) {
					printf("-E- Error generating status message\r\n");
				}
				
				network_send_status(status_packet, peer_address);
				
				// Simulate changing battery level
				iBattery -= 10;
				if (iBattery <= 0)
				{
					iBattery = 100;
				}
			}
		}
		vTaskDelay(TASK_SEND_STATUS_DELAY);
	}
}

static void task_control_loop(void *pvParameters)
{
	UNUSED(pvParameters);
	// Queue buffers
	volatile int8_t control_buf[CONTROL_MSG_QUEUE_ITEM_SIZE] = {0, 0};
	volatile int8_t settings_buf[SETTINGS_MSG_QUEUE_ITEM_SIZE];
	portBASE_TYPE xStatus = pdFAIL;
	
	// Timeout handling
	portTickType xInputValidTicks = TASK_CONTROL_INPUT_VALID_TIME_MS/portTICK_RATE_MS;
	portTickType xNoInputTicks = TASK_CONTROL_NO_INPUT_TIME_MS/portTICK_RATE_MS;
	portTickType xSettingsValidTicks = TASK_CONTROL_CHECK_SETTINGS_TIME_MS/portTICK_RATE_MS;
	xTimeOutType xSettingsTime;
	xTimeOutType xInputTIme;
	portTickType xInputTimeToWait = xInputValidTicks;
	portTickType xSettingsTImeToWait = xSettingsValidTicks;
	vTaskSetTimeOutState( &xSettingsTime );
	vTaskSetTimeOutState( &xInputTIme );
	
	// Flags for input validation
	int8_t check_for_settings = 1;
	int8_t read_control_input = 1;
	
	// Motor initialization
	propeller_motor1_start(0);
	servos_propeller1_start();
	for (;;)
	{
		/* Read settings */
		if(check_for_settings > 0) {
			xStatus = xQueuePeek(xSettings_Msg_Queue_handle, settings_buf, 0);
			if(xStatus == pdPASS)
			{
				vTaskSetTimeOutState( &xSettingsTime );
				xSettingsTImeToWait = xSettingsValidTicks;
				check_for_settings = -1;
			}
		}
		
		/* Read Control input */
		if(read_control_input > 0) {
			xStatus = xQueueReceive(xControl_Msg_Queue_handle, control_buf, 0);
			if(xStatus == pdPASS)
			{
				vTaskSetTimeOutState( &xInputTIme );
				xInputTimeToWait = xInputValidTicks;
				read_control_input = -1;
			} else {
				// Could not read control input. Set input to zero.
				control_buf[POW] = 0;
				control_buf[ANG] = 0;
				vTaskSetTimeOutState( &xInputTIme );
				xInputTimeToWait = xNoInputTicks;
				read_control_input = -1;
			}
		}
		
		while(1) {
#ifdef XY_COORDS_INPUT
			// Calculate propeller speed as magnitude of input vector
			uint8_t propeller_speed = sqrt(pow(control_buf[X_COORD],2) + pow(control_buf[Y_COORD],2));
			propeller_motor1_set_speed(propeller_speed);
			
			// Determine rotor positions from X,Y-coordinates
			int16_t inner_rotor_deg = INNER_SERVO_OFFSET + control_buf[X_COORD];
			if(inner_rotor_deg < 0)
				inner_rotor_deg = 0;
				
			int16_t outer_rotor_deg = OUTER_SERVO_OFFSET + control_buf[Y_COORD];
			if(outer_rotor_deg < 0)
				outer_rotor_deg = 0;
			servos_propeller1_inner_set_position((uint8_t) inner_rotor_deg);
			servos_propeller1_outer_set_position((uint8_t) outer_rotor_deg);							
#elif POW_ANG_INPUT
			// Propeller speed = input power
			propeller_motor1_set_speed(control_buf[POW]);
			
			// Determine x,y from hypotenuse and angle
			
#endif
			
			// Check for settings validity
			if( xTaskCheckForTimeOut( &xSettingsTime, &xSettingsTImeToWait ) != pdFALSE ) {
				// Settings have become invalid and should be checked again
				check_for_settings = 1;
				break;
			}
			
			// Check for input validity
			if( xTaskCheckForTimeOut( &xInputTIme, &xInputTimeToWait ) != pdFALSE ) {
				// Input have become invalid and should be read again
				read_control_input = 1;
				break;
			}
		}
	}	
}

#endif /* FREERTOS_TASKS_H_ */