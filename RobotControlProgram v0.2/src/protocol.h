#include "asf.h"

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#define CMD_SPECIFIER_SIZE	6
#define PACKET_SIZE			255

/** Command type specifiers */
#define CMD_CONTROL							"CMD*CT"
#define CMD_STATUS							"CMD*ST"
#define CMD_SETTINGS						"CMD*SE"
#define CMD_ACK								"CMD*OK"
#define CMD_NACK							"CMD*NO"

/** Tag type specifiers */
// Settings tags
#define TAG_SETTINGS_NAME					"Name"
#define TAG_SETTINGS_ASSISTED_DRIVE_MODE	"Assisted"
#define TAG_SETTINGS_POWER_SAVE_MODE		"PowerMode"
#define TAG_SETTINGS_VIDEO_QUALITY			"VideoQuality"

// Status tags
#define TAG_STATUS_NAME						"Name"
#define TAG_STATUS_MAC_ADDR					"Mac"
#define TAG_STATUS_IP_ADDR					"Ip"
#define TAG_STATUS_BATTERY					"Battery"
#define TAG_STATUS_CAMERA					"Camera"
#define TAG_STATUS_STORAGE_SPACE			"Space"
#define TAG_STATUS_STORAGE_REMAINING		"Remaining"
	
// Control tags
#define TAG_CONTROL_STEERING_X				"X"
#define TAG_CONTROL_STEERING_Y				"Y"
#define TAG_CONTROL_STEERING_POWER			"Pwr"
#define TAG_CONTROL_STEERING_ANGLE			"Agl"

// Format tags
#define DATA_TAG_SPACING					":"
#define TAG_SEPARATOR						";"

// Values
#define VALUE_TRUE							"1"
#define VALUE_FALSE							"0"

// Returns
#define PARSER_SUCCESS						(0)
#define PARSER_ERROR						(-1)
#define CONTROL_INPUT_ERROR					(-2)
#define SETTINGS_ERROR						(-3)
	
/* Event handler for network messages */
int8_t network_message_handler(char *msg);

/* Generate a status packet formatted according to the transfer protocol */
int8_t network_generate_status_packet(char* packet,
									char* name, 
									uint8_t battery, 
									int8_t camera, 
									uint16_t space, 
									uint16_t remaining_space);
	
/* Generate a settings packet formatted according to the transfer protocol */								
int8_t generate_settings_packet(char* packet,
								char* name, 
								int8_t power_mode,
								int8_t assisted_drive,
								uint8_t video_quality);

#endif /* PROTOCOL_H_ */