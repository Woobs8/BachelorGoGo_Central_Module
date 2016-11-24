/*
 * freertos_tasks.h
 *
 * Created: 17-11-2016 11:03:45
 *  Author: MadsNKjaersgaard
 */ 


#ifndef FREERTOS_TASKS_H_
#define FREERTOS_TASKS_H_

#include "driver/include/m2m_wifi.h"
#include "network_module/WiFi_P2P.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"
#include "protocol.h"
#include "network_module/Networking.h"
#include "network_module/WiFi_P2P.h"

#define TASK_LED_STACK_SIZE					(1024/sizeof(portSTACK_TYPE))
#define TASK_LED_STACK_PRIORITY				(tskIDLE_PRIORITY)
#define TASK_LED_DELAY						(1000)

#define TASK_WINC_STACK_SIZE				(4096/sizeof(portSTACK_TYPE))
#define TASK_WINC_STACK_PRIORITY			(tskIDLE_PRIORITY)

#define TASK_SEND_STATUS_SIZE				(1024/sizeof(portSTACK_TYPE))
#define TASK_SEND_STATUS_PRIORITY			(tskIDLE_PRIORITY)
#define TASK_SEND_STATUS_DELAY				(20000)

#define TASK_CONTROL_LOOP_STACK_SIZE		(1024/sizeof(portSTACK_TYPE))
#define TASK_CONTROL_LOOP_PRIORITY			(tskIDLE_PRIORITY)

#define CONTROL_MSG_QUEUE_LENGTH			(10)
#define CONTROL_MSG_QUEUE_ITEM_SIZE			(255)

xSemaphoreHandle xWINC_Semaphore_handle;
xQueueHandle xControl_Msg_Queue_handle;

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
	for(;;) {
		xSemaphoreTake(xWINC_Semaphore_handle, portMAX_DELAY);
		m2m_wifi_handle_events(NULL);
	}
}

static void task_send_status(void *pvParameters)
{
	UNUSED(pvParameters);
	int8_t iBattery = 0;
	char cName = "Test";
	uint16_t storage_space = 200;
	uint16_t remaining_space = 100;
	char status_packet[PACKET_SIZE] = {0};
	vTaskDelay(10000);
	for (;;) {
		if(network_is_connected == IS_CONNECTED) {
			if(network_generate_status_packet(status_packet, &cName, iBattery, 1, storage_space, remaining_space) == -1 ) {
				printf("-E- Error generating status message\r\n");
			}
			network_send_status(status_packet, peer_address);
			iBattery += 10;
			if (iBattery >= 100)
			{
				iBattery = 0;
			}
		}
		vTaskDelay(TASK_SEND_STATUS_DELAY);
	}
}

static void task_control_loop(void *pvParameters)
{
	UNUSED(pvParameters);
	//int8_t uiReceivedVal = 0;
	int8_t buffer[2] = {0, 0};
	portBASE_TYPE xStatus = pdFAIL;
	volatile uiSuccesess = 0;
	volatile uiFailures = 0;
	for (;;)
	{
		//xStatus = xQueueReceive(xControl_Msg_Queue_handle, &uiReceivedVal, 1);
		xStatus = xQueueReceive(xControl_Msg_Queue_handle, buffer, 1);
		if(xStatus == pdPASS)
		{
			uiSuccesess++;
			//printf("Receive task: %d \r\n", uiReceivedVal);
		}
		else
		{
			//printf("Receive task: Did not receive anything \r\n");
			uiFailures++;
		}
		//vTaskDelay(1000);
	}	
}

static void task_sender(void *pvParameters)
{
	UNUSED(pvParameters);
	int8_t uiValueToSend = 112;
	int8_t buffer[2] = {112, 113};
	portBASE_TYPE xStatus;
	
	for (;;)
	{
		xStatus = xQueueSendToBack(xControl_Msg_Queue_handle, buffer, 0);
		
		if((xStatus == errQUEUE_FULL))
		{
			//printf("Sender Task: Could not print to Queue \r\n");
		}
		else if((xStatus == pdPASS))
		{
			//printf("Sender Task: printed to the queue\r\n");
		}
		//uiValueToSend++;
		vTaskDelay(2000);
	}
}

#endif /* FREERTOS_TASKS_H_ */