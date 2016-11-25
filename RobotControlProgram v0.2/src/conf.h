#include "protocol.h"
#ifndef CONF_H_
#define CONF_H_

// System values
#define SETTING_ENABLED				(1)
#define SETTING_DISABLED			(-1)

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