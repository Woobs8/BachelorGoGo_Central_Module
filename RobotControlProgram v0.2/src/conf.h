#include "protocol.h"
#ifndef CONF_H_
#define CONF_H_

#define DEBUG						(0)

// System values
#define SETTING_ENABLED				(1)
#define SETTING_DISABLED			(-1)

// Control method
#define XY_COORDS_INPUT
//#define POW_ANG_INPUT

// Control Queue index
#define X_COORD						(0)
#define Y_COORD						(1)
#define POW							(0)
#define ANG							(1)

// Settings Queue index
#define POWER_SAVE_MODE				(0)
#define ASSISTED_DRIVE_MODE			(1)
#define VIDEO_QUALITY				(2)

// Name Queue index
#define NAME_SIZE					(0)
#define NAME						(1)

// Default system settings
#define DEFAULT_DEVICE_NAME			"RoboGoGo"
#define DEFAULT_POWER_SAVE_MODE		SETTING_DISABLED
#define DEFAULT_ASSISTED_DRIVE_MODE	SETTING_DISABLED
#define DEFAULT_VIDEO_QUALITY		(1)

// Current system settings
int8_t iPower_save_mode;
int8_t iAssisted_drive_mode;
uint8_t uiVideo_quality;

#endif /* CONF_H_ */