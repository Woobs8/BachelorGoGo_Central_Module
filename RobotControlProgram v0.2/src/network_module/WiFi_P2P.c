#include "WiFi_P2P.h"
#include "Networking.h"
#include <string.h>
#include "driver/include/m2m_wifi.h"
#include "driver/source/nmasic.h"
#include <inttypes.h>

uint8_t WIFI_CONNECTION_STATE = 0;

/**
 * \Callback to get the Wi-Fi status update.
 *
 * \param[in] u8MsgType type of Wi-Fi notification. Possible types are:
 *  - [M2M_WIFI_RESP_CON_STATE_CHANGED](@ref M2M_WIFI_RESP_CON_STATE_CHANGED)
 *  - [M2M_WIFI_REQ_DHCP_CONF](@ref M2M_WIFI_REQ_DHCP_CONF)
 * \param[in] pvMsg A pointer to a buffer containing the notification parameters
 * (if any). It should be casted to the correct data type corresponding to the
 * notification type.
 */
void wifi_cb(uint8_t u8MsgType, void *pvMsg)
{
	switch (u8MsgType) {
		case M2M_WIFI_RESP_CON_STATE_CHANGED:
		{
			tstrM2mWifiStateChanged *pstrWifiState = (tstrM2mWifiStateChanged *)pvMsg;
			if (pstrWifiState->u8CurrState == M2M_WIFI_CONNECTED) {
				m2m_wifi_request_dhcp_client();
			} else if (pstrWifiState->u8CurrState == M2M_WIFI_DISCONNECTED) {
				WIFI_CONNECTION_STATE = 0;
				printf("-I- Wi-Fi disconnected\r\n");
				network_disconnected();
			}

			break;
		}

		case M2M_WIFI_REQ_DHCP_CONF:
		{
			WIFI_CONNECTION_STATE = 1;
			uint8_t *pu8IPAddress = (uint8_t *)pvMsg;
			printf("-I- Wi-Fi connected\r\n");
			printf("-I- Wi-Fi IP is %u.%u.%u.%u\r\n",
			pu8IPAddress[0], pu8IPAddress[1], pu8IPAddress[2], pu8IPAddress[3]);
			printf("-I- Wi-Fi Group owner IP is %u.%u.%u.%u\r\n",
			pu8IPAddress[4], pu8IPAddress[5], pu8IPAddress[6], pu8IPAddress[7]);
			
			// Convert IP address from uint8 array to uint32
			peer_address = pu8IPAddress[4]<<24 | pu8IPAddress[5]<<16 | pu8IPAddress[6]<<8 | pu8IPAddress[7];
			delay_ms(100);	// Delay necessary to allow app to open socket
			network_establish_connection(peer_address);
			break;
		}

		default:
		{
			break;
		}
	}
}

/**
 * \Function to initialize the Wi-Fi module. 
 * \ Sets the status callback and device name
 */
void wifi_init(void)
{
		tstrWifiInitParam param;
		int8_t ret;

		/* Initialize Wi-Fi parameters structure. */
		memset((uint8_t *)&param, 0, sizeof(tstrWifiInitParam));

		/* Initialize Wi-Fi driver with data and status callbacks. */
		param.pfAppWifiCb = wifi_cb;
		ret = m2m_wifi_init(&param);
		if (M2M_SUCCESS != ret) {
			printf("WiFi_P2P: m2m_wifi_init call error!(%d)\r\n", ret);
			while (1) {
			}
		}

		/* Set device name to be shown in peer device. */
		wifi_set_device_name(DEFAULT_WLAN_DEVICE_NAME,strlen(DEFAULT_WLAN_DEVICE_NAME)+1);
}

/**
 * \Function to start the Wi-Fi module in P2P mode.
 */
void wifi_p2p_start(void)
{
		int8_t ret;
		
		/* Bring up P2P mode with channel number. */
		ret = m2m_wifi_p2p(M2M_WIFI_CH_6);
		if (M2M_SUCCESS != ret) {
			printf("-E- WiFi_P2P: m2m_wifi_p2p call error!\r\n");
		}

		printf("-I- P2P mode started. %s is awaiting connection.\r\n", (char *)device_name);
}

/*
 * Set the peer device name
*/
int8_t wifi_set_device_name(char* name, uint8_t size)
{
	int8_t ret = 0;
	if(strcmp(name, device_name) != 0)
	{
		volatile char tmp[strlen(SYSTEM_IDENTIFIER)+size];
		memcpy(tmp,SYSTEM_IDENTIFIER,strlen(SYSTEM_IDENTIFIER));
		memcpy(tmp+strlen(SYSTEM_IDENTIFIER),name,size);
						
		/* Set device name to be shown in peer device. */
		ret = m2m_wifi_set_device_name((uint8_t *)tmp, strlen(tmp));
		if (M2M_SUCCESS != ret) {
			printf("-E- WiFi_P2P: m2m_wifi_set_device_name call error!\r\n");
			while (1) {
			}
		} else {
			free(device_name);
			device_name = malloc(size);
			memcpy(device_name,name,size);
		}
	}
	return ret;
}