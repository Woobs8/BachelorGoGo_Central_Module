#include "asf.h"

#ifndef NETWORK_MODULE_H_
#define NETWORK_MODULE_H_

/** P2P device name. */
#define SYSTEM_IDENTIFIER			"BachelorGoGo*"
#define DEFAULT_WLAN_DEVICE_NAME	"RoboGoGo"
char *device_name;

/** Wi-Fi connection state */
extern uint8_t WIFI_CONNECTION_STATE;

/** Networking */
uint32_t peer_address;

static void wifi_cb(uint8_t u8MsgType, void *pvMsg);
void wifi_init(void);
void wifi_p2p_start(void);
int8_t wifi_set_device_name(char* name, uint8_t size);



#endif /* NETWORK_MODULE_H_ */