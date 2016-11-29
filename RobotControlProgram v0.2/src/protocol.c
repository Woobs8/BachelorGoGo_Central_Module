#include "protocol.h"
#include "conf.h"
#include "String.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "freertos_tasks.h"
#include "nand_flash_storage/nand_flash_storage.h"

/** Event handler for network messages */
int8_t network_message_handler(char *msg)
{
	int8_t error = PARSER_ERROR;
	char cmd[PACKET_SIZE];
	memcpy( cmd, msg, PACKET_SIZE );
	
	/** Control message received */
	if(strstr(cmd, CMD_CONTROL) != NULL)
	{
		char *token;
		
		/* get the first token */
		token = strtok(cmd, TAG_SEPARATOR);
		
		int8_t coords[2];
		int8_t power_ang[2];
		portBASE_TYPE xStatus;
		uint8_t uValidCoords = 0;
		uint8_t uValidPowerAng = 0;
		
		/* walk through other tokens */
		while(token != NULL)
		{
			// Find tag-value separator and extract value
			char* value = strchr(token,DATA_TAG_SPACING[0])+1;
			if(value)
			{				
				/** X coordinate */
				if(strstr(token, TAG_CONTROL_STEERING_X) != NULL)
				{
					float x_coord = strtof(value,NULL);
					printf("X: %s (%d)\r\n",value,(int)x_coord);
					int8_t iX_coord = (int8_t)x_coord;
					coords[X_COORD] = iX_coord;
					uValidCoords++;
				}
				/** Y coordinate */
				else if(strstr(token, TAG_CONTROL_STEERING_Y) != NULL)
				{
					float y_coord = strtof(value,NULL);
					printf("Y: %s (%d)\r\n",value,(int)y_coord);
					int8_t iY_coord = (int8_t)y_coord;
					coords[Y_COORD] = iY_coord;
					uValidCoords++;
				}
				/** Power */
				else if(strstr(token, TAG_CONTROL_STEERING_POWER) != NULL)
				{
					float power = strtof(value,NULL);
					printf("Pwr: %s (%d)\r\n",value,(int)power);
					int8_t iPow = (int8_t)power;
					power_ang[POW] = iPow;
					uValidPowerAng++;
				}
				/** Angle */
				else if(strstr(token, TAG_CONTROL_STEERING_ANGLE) != NULL)
				{
					float angle = strtof(value,NULL);
					printf("Ang: %s (%d)\r\n",value,(int)angle);
					int8_t iAng = (int8_t)angle;
					power_ang[ANG] = iAng;
					uValidPowerAng++;
				}
				
				/* Write to queue */
				if (uValidCoords == 2) {
					xStatus = xQueueSendToBack(xControl_Msg_Queue_handle, coords, 0);
					if ((xStatus == pdPASS))
					{
						printf("-I- Coordinates written to queue\r\n");
						error = PARSER_SUCCESS;
					}
					else if((xStatus == errQUEUE_FULL))
					{
						printf("-E- Queue is full\r\n");
					}
				}  else if (uValidPowerAng == 2) {
					xStatus = xQueueSendToBack(xControl_Msg_Queue_handle, coords, 0);
					if ((xStatus == pdPASS))
					{
						printf("-I- Power/Ang. written to queue\r\n");
						error = PARSER_SUCCESS;
					}
					else if((xStatus == errQUEUE_FULL))
					{
						printf("-E- Queue is full\r\n");
					}
				} else {
					error = CONTROL_INPUT_ERROR;
				}
			}	
			// Read next token
			token = strtok(NULL, TAG_SEPARATOR);
		}
	}
	/** Settings message received*/
	else if(strstr(cmd, CMD_SETTINGS) != NULL)
	{
		char *token;
		char* name = NULL;
		int8_t assisted_drive_mode = -1;
		int8_t power_save_mode = -1;
		int8_t video_quality = -1;
			
		/* get the first token */
		token = strtok(cmd, TAG_SEPARATOR);
		
		/* walk through other tokens */
		while(token != NULL)
		{
			// Find tag-value separator and extract value
			char* value = strchr(token,DATA_TAG_SPACING[0])+1;
			if(value)
			{
				/** Robot name setting */
				if(strstr(token, TAG_SETTINGS_NAME) != NULL)
				{	
					name = value;
				}
				/** Assisted Driving Mode setting */
				else if(strstr(token, TAG_SETTINGS_ASSISTED_DRIVE_MODE) != NULL)
				{
					char* ptr;
					assisted_drive_mode = strtol(value,ptr,10);
				}
				/** Power Save Mode setting */
				else if(strstr(token, TAG_SETTINGS_POWER_SAVE_MODE) != NULL)
				{
					char* ptr;
					power_save_mode = strtol(value,ptr,10);
				}
				/** Video Quality setting*/
				else if(strstr(token, TAG_SETTINGS_VIDEO_QUALITY) != NULL)
				{
					char* ptr;
					video_quality = strtol(value,ptr,10);
				}
			}
			
			// Read next token
			token = strtok(NULL, TAG_SEPARATOR);
		}
		// Only configure robot if all settings have been parsed
		if(name && assisted_drive_mode >= 0 && power_save_mode >= 0 && video_quality >= 0)
		{	
			/* Configure device name */
			uint8_t size = strlen(name);	
			int8_t ret = wifi_set_device_name(name, size+1);	// include NUL-terminator
			if(ret) {
				printf("-E- Could not set device name\r\n");
				error = SETTINGS_ERROR;
			} else {
				printf("-I- Name: %s\r\n",name);
				error = PARSER_SUCCESS;
			}
			
			if(error != SETTINGS_ERROR) {
				/* Configure Assisted Drive Mode */
				printf("-I- Assisted Drive Mode: %d\r\n",assisted_drive_mode);				
			}
			
			if(error != SETTINGS_ERROR) {
				/* Configure Power Save Mode */
				printf("-I- Power Save Mode: %d\r\n",power_save_mode);
			}
			
			if(error != SETTINGS_ERROR) {
				/* Configure Video Quality */
				printf("-I- Video Quality: %d\r\n",video_quality);
			}
			
			if(error != SETTINGS_ERROR) {
				int8_t settings_buf[SETTINGS_MSG_QUEUE_ITEM_SIZE];
				uint8 name_buf[NAME_QUEUE_ITEM_SIZE];
				
				/* Store settings in non-volatile memory */
				nand_flash_storage_write(msg,PACKET_SIZE);
				
				/* Save current settings to RAM */
				if(power_save_mode > 0) {
					iPower_save_mode = SETTING_ENABLED;
					settings_buf[POWER_SAVE_MODE] = SETTING_ENABLED;
				} else {
					iPower_save_mode = SETTING_DISABLED;
					settings_buf[POWER_SAVE_MODE] = SETTING_DISABLED;
				}
				
				if(assisted_drive_mode > 0) {
					iAssisted_drive_mode = SETTING_ENABLED;
					settings_buf[ASSISTED_DRIVE_MODE] = SETTING_ENABLED;
				} else {
					iAssisted_drive_mode = SETTING_DISABLED;
					settings_buf[ASSISTED_DRIVE_MODE] = SETTING_DISABLED;

				}

				uiVideo_quality = video_quality;
				settings_buf[VIDEO_QUALITY] = video_quality;
				
				/* Write name to queue */
				portBASE_TYPE xStatus;
				xStatus = xQueueReset(xName_Queue_handle);						// clear queue
				if(xStatus == pdPASS || xStatus == errQUEUE_EMPTY) {			// Queue successfully cleared
					name_buf[NAME_SIZE] = strlen(name)+1;
					memcpy(name_buf+1,name,strlen(name)+1);
					xStatus = xQueueSendToBack(xName_Queue_handle, name_buf, 0);
					if ((xStatus == pdPASS)) {
						printf("-I- Name written to queue\r\n");
						error = PARSER_SUCCESS;
					} else if((xStatus == errQUEUE_FULL)) {
						printf("-E- Name queue is full\r\n");
						error = PARSER_ERROR;
					}
				} else {
					printf("-E- Queue could not be cleared\r\n");
					error = PARSER_ERROR;
				}
						
				/* Write settings to queue */
				xStatus = xQueueReset(xSettings_Msg_Queue_handle);				// clear queue
				if(xStatus == pdPASS) {											// Queue successfully cleared
					xStatus = xQueueSendToBack(xSettings_Msg_Queue_handle, settings_buf, 0);
					if ((xStatus == pdPASS)) {
						printf("-I- Settings written to queue\r\n");
						error = PARSER_SUCCESS;
					} else if((xStatus == errQUEUE_FULL)) {
						printf("-E- Settings queue is full\r\n");
						error = PARSER_ERROR;
					}
				} else {
					printf("-E- Queue could not be cleared\r\n");
					error = PARSER_ERROR;
				}
			}
		} else {
			error = SETTINGS_ERROR;
		}
	}
	return error;
}

/** Generate a status packet formatted according to the transfer protocol
  * Parameters:
  *		- packet:			char array of size PACKET_SIZE
  *		- name:				max. 30 characters
  *		- battery:			0-100% (>100 is interpreted as 100)
  *		- camera:			-1 (no) or 1 (yes)
  *		- space:			storage space in MB
  *		- remaining_space:	remaining storage space in MB. Must be set if 'space' parameter is set. Must be (< space).
  *
  * A parameter is ignored if null, unless otherwise specified.
  *
  * Return:
  *		- success:			Number of data bytes
  *		- failure:			-1
 */
int8_t generate_status_packet(char* packet,
									char* name,
									uint8_t battery,
									int8_t camera,
									uint16_t space,
									uint16_t remaining_space)
{
	int8_t ret = 0;
	uint8_t index = 0;
	
	/** Multiple use tag sizes */
	uint8_t data_spacing_size = strlen(DATA_TAG_SPACING);
	uint8_t tag_separator_size = strlen(TAG_SEPARATOR);
		
	/** Initialize packet to zero */
	memset(packet,0,PACKET_SIZE);
		
	/** Cmd specifier */
	memcpy(packet,CMD_STATUS,CMD_SPECIFIER_SIZE);
	index = index + CMD_SPECIFIER_SIZE;
	packet[CMD_SPECIFIER_SIZE] = TAG_SEPARATOR[0];
	index = index + tag_separator_size;			
		
	/** Name */
	if(name != NULL)
	{
		if(sizeof(name) <= 30)
		{
			uint8_t tag_size = strlen(TAG_STATUS_NAME);
			// Data tag specifier
			memcpy(packet+index,TAG_STATUS_NAME,tag_size);
			index = index + tag_size;
			packet[index] = DATA_TAG_SPACING[0];
			index = index + data_spacing_size;
				
			// Data
			uint8_t data_size = strlen(name);
			memcpy(packet+index,name,data_size);
			index = index + data_size;
				
			// Tag separator
			packet[index] = TAG_SEPARATOR[0];
			index = index + tag_separator_size;
		}
		else
		{
			ret = -1;
		}
	}
		
	/** Battery */
	if(battery != NULL)
	{
		uint8_t tag_size = strlen(TAG_STATUS_BATTERY);
		// Data tag specifier
		memcpy(packet+index,TAG_STATUS_BATTERY,tag_size);
		index = index + tag_size;
		packet[index] = DATA_TAG_SPACING[0];
		index = index + data_spacing_size;	
			
		// Battery > 100 -> Battery = 100
		uint8_t digits;
		if(battery >= 100)
		{
			battery = 100;
			digits = 3;
		}
		else
		{
			digits = 2;
		}
		// Data
		char* tmp[digits];
		sprintf(tmp, "%d", battery);
		memcpy(packet+index,tmp,digits);
		index = index + digits;
			
		// Tag separator
		packet[index] = TAG_SEPARATOR[0];
		index = index + tag_separator_size;
	}
		
	/** Camera */
	if(camera != NULL)
	{
		uint8_t tag_size = strlen(TAG_STATUS_CAMERA);
		// Data tag specifier
		memcpy(packet+index,TAG_STATUS_CAMERA,tag_size);
		index = index + tag_size;
		packet[index] = DATA_TAG_SPACING[0];
		index = index + data_spacing_size;	
			
		// Data
		if(camera < 0)
		{
			packet[index] = VALUE_FALSE[0];
			index = index + sizeof(VALUE_FALSE[0]);
		}
		else
		{
			packet[index] = VALUE_TRUE[0];
			index = index + sizeof(VALUE_TRUE[0]);
		}
			
		// Tag separator
		packet[index] = TAG_SEPARATOR[0];
		index = index + tag_separator_size;
	}
		
	/** Storage space */
	if(space != NULL)
	{
		// Remaining space must also be set
		if(remaining_space!= NULL)
		{
			uint8_t tag_size = strlen(TAG_STATUS_STORAGE_SPACE);
			// Data tag specifier
			memcpy(packet+index,TAG_STATUS_STORAGE_SPACE,tag_size);
			index = index + tag_size;
			packet[index] = DATA_TAG_SPACING[0];
			index = index + data_spacing_size;		
						
			// Find number of characters needed to represent the number			
			uint8_t digits;
			// 0-9 = 1 digit
			if(remaining_space < 10)
			{
				digits = 1;
			}
			// 10-99 = 2 digits
			else if(remaining_space < 100)
			{
				digits = 2;
			}
			// 100-999 = 3 digits
			else if(remaining_space < 1000)
			{
				digits = 3;
			}
			// 1000-9999 = 4 digits
			else if(remaining_space < 10000)
			{
				digits = 4;
			}
			// 10000-65536 = 5 digits
			else
			{
				digits = 5;
			}
				
			// Data
			char* tmp[digits];
			sprintf(tmp, "%d", space);
			memcpy(packet+index,tmp,digits);
			index = index + digits;
				
			// Tag separator
			packet[index] = TAG_SEPARATOR[0];
			index = index + tag_separator_size;	
		}
		else
		{
			ret = -1;
		}
	}
					
	/** Remaining storage space */
	if(remaining_space != NULL)
	{
		// Storage space must also be set and must be larger than remaining space
		if(space!=NULL && (remaining_space < space) )
		{
			uint8_t tag_size = strlen(TAG_STATUS_STORAGE_REMAINING);
			// Data tag specifier
			memcpy(packet+index,TAG_STATUS_STORAGE_REMAINING,tag_size);
			index = index + tag_size;
			packet[index] = DATA_TAG_SPACING[0];
			index = index + data_spacing_size;
			
			// Find number of characters needed to represent the number			
			uint8_t digits;
			// 0-9 = 1 digit
			if(remaining_space < 10)
			{
				digits = 1;
			}
			// 10-99 = 2 digits
			else if(remaining_space < 100)
			{
				digits = 2;
			}
			// 100-999 = 3 digits
			else if(remaining_space < 1000)
			{
				digits = 3;
			}
			// 1000-9999 = 4 digits
			else if(remaining_space < 10000)
			{
				digits = 4;
			}
			// 10000-65536 = 5 digits
			else
			{
				digits = 5;
			}
				
			// Data
			char* tmp[digits];
			sprintf(tmp, "%d", remaining_space);
			memcpy(packet+index,tmp,digits);
				
			// Tag separator
			index = index + digits;
			packet[index] = TAG_SEPARATOR[0];
			index = index + tag_separator_size;
		}
		else
		{
			ret = -1;
		}
	}	
	
	if(ret < 0)
		return ret;
	else
		return index;
}

/** Generate a settings packet formatted according to the transfer protocol
  * Parameters:
  *		- packet:			char array of size PACKET_SIZE
  *		- name:				Device Name: max. 30 characters
  *		- power_mode:		Power Save Mode: 1 (activated) or -1 (deactivated)
  *		- assisted_drive:	Assisted Drive Mode: 1 (activated) or -1 (deactivated)
  *		- video_quality:	1-255 (currently only 1-4 supported)
  *
  * Return:
  *		- success:			Number of data bytes
  *		- failure:			-1
 */
int8_t generate_settings_packet(char* packet,
								char* name,
								int8_t power_mode,
								int8_t assisted_drive,
								uint8_t video_quality)
{
	int8_t ret = 0;
	uint8_t index = 0;
	
	/** Multiple use tag sizes */
	uint8_t data_spacing_size = strlen(DATA_TAG_SPACING);
	uint8_t tag_separator_size = strlen(TAG_SEPARATOR);	
	
	/** Initialize packet to zero */
	memset(packet,0,PACKET_SIZE);
	
	/** Cmd specifier */
	memcpy(packet,CMD_SETTINGS,CMD_SPECIFIER_SIZE);
	index = index + CMD_SPECIFIER_SIZE;
	packet[CMD_SPECIFIER_SIZE] = TAG_SEPARATOR[0];
	index = index + tag_separator_size;
	
	/** Name */
	if(name != NULL)
	{
		if(strlen(name) <= 30)
		{
			uint8_t tag_size = strlen(TAG_SETTINGS_NAME);	//ignore NUL-terminator
			// Data tag specifier
			memcpy(packet+index,TAG_SETTINGS_NAME,tag_size);
			index = index + tag_size;
			packet[index] = DATA_TAG_SPACING[0];
			index = index + data_spacing_size;
			
			// Data
			uint8_t data_size = strlen(name);				
			memcpy(packet+index,name,data_size);
			index = index + data_size;
			
			// Tag separator
			packet[index] = TAG_SEPARATOR[0];
			index = index + tag_separator_size;
		}
		else
		{
			ret = -1;
		}
	}
	
	/** Power Save Mode */
	if(power_mode != NULL)
	{
		uint8_t tag_size = strlen(TAG_SETTINGS_POWER_SAVE_MODE);	//ignore NUL-terminator
		// Data tag specifier
		memcpy(packet+index,TAG_SETTINGS_POWER_SAVE_MODE,tag_size);
		index = index + tag_size;
		packet[index] = DATA_TAG_SPACING[0];
		index = index + data_spacing_size;
		
		// Data
		if(power_mode < 0 )			// Activated
			packet[index] = VALUE_FALSE[0];
		else						// Deactivated
			packet[index] = VALUE_TRUE[0];

		index = index + 1;
		
		// Tag separator
		packet[index] = TAG_SEPARATOR[0];
		index = index + tag_separator_size;
	}
	
	/** Assisted Drive Mode */
	if(assisted_drive != NULL)
	{
		uint8_t tag_size = strlen(TAG_SETTINGS_ASSISTED_DRIVE_MODE);
		// Data tag specifier
		memcpy(packet+index,TAG_SETTINGS_ASSISTED_DRIVE_MODE,tag_size);
		index = index + tag_size;
		packet[index] = DATA_TAG_SPACING[0];
		index = index + data_spacing_size;
		
		// Data
		if(assisted_drive < 0)
			packet[index] = VALUE_FALSE[0];
		else
			packet[index] = VALUE_TRUE[0];
		
		index = index + 1;
		
		// Tag separator
		packet[index] = TAG_SEPARATOR[0];
		index = index + tag_separator_size;
	}
	
	/** Video Quality */
	if(video_quality != NULL)
	{
		uint8_t tag_size = strlen(TAG_SETTINGS_VIDEO_QUALITY);
		// Data tag specifier
		memcpy(packet+index,TAG_SETTINGS_VIDEO_QUALITY,tag_size);
		index = index + tag_size;
		packet[index] = DATA_TAG_SPACING[0];
		index = index + data_spacing_size;
			
		// Find number of characters needed to represent the number
		uint8_t digits;
		// 0-9 = 1 digit
		if(video_quality < 10)
		{
			digits = 1;
		}
		// 10-99 = 2 digits
		else if(video_quality < 100)
		{
			digits = 2;
		}
		// 100-255 = 3 digits
		else
		{
			digits = 3;
		}
			
		// Data
		char* tmp[digits];
		sprintf(tmp, "%d", video_quality);
		memcpy(packet+index,tmp,digits);
		index = index + digits;
			
		// Tag separator
		packet[index] = TAG_SEPARATOR[0];
		index = index + tag_separator_size;
	}
	
	if(ret < 0)
		return ret;
	else
		return index;
}

int8_t apply_default_settings(void) 
{
	int8_t settings_buf[SETTINGS_MSG_QUEUE_ITEM_SIZE];
	portBASE_TYPE xStatus;
	int8_t error = PARSER_ERROR;

	settings_buf[POWER_SAVE_MODE] = DEFAULT_POWER_SAVE_MODE;
	settings_buf[ASSISTED_DRIVE_MODE] = DEFAULT_ASSISTED_DRIVE_MODE;
	settings_buf[VIDEO_QUALITY] = DEFAULT_VIDEO_QUALITY;
	
	/* Write settings to queue */
	xStatus = xQueueReset(xSettings_Msg_Queue_handle);		// clear queue
	if(xStatus == pdPASS) {									// Queue successfully cleared
		xStatus = xQueueSendToBack(xSettings_Msg_Queue_handle, settings_buf, 0);
		if ((xStatus == pdPASS)) {
			printf("-I- Settings written to queue\r\n");
			error = PARSER_SUCCESS;
		} else if((xStatus == errQUEUE_FULL)) {
			printf("-E- Settings queue is full\r\n");
			error = PARSER_ERROR;
		}
	} else {
		printf("-E- Queue could not be cleared\r\n");
		error = PARSER_ERROR;
	}
	
	return error;
}