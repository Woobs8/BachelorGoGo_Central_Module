#include "asf.h"
#include "socket/include/socket.h"
#include "protocol.h"

#ifndef NETWORKING_H_
#define NETWORKING_H_

#define GO_PORT								(9999)
#define TCP_SETTINGS_PORT					(4999)
#define UDP_COMMAND_PORT					(4998)
#define HTTP_VIDEO_PORT						(4997)
#define TCP_PORT_EXCHANGE_BUFFER_SIZE		4
#define UDP_COMMAND_BUFFER_SIZE				PACKET_SIZE
#define TCP_SETTINGS_BUFFER_SIZE			PACKET_SIZE
#define TCP_ACK_BUFFER_SIZE					CMD_SPECIFIER_SIZE
#define UDP_STATUS_BUFFER_SIZE				PACKET_SIZE

int8_t network_is_connected;
//int8_t network_test;

/** Message format definitions. */
typedef struct s_msg_port {
	char port[TCP_PORT_EXCHANGE_BUFFER_SIZE];
} s_msg_port;

typedef struct s_msg_ack {
	char ack[TCP_ACK_BUFFER_SIZE];
} s_msg_ack;

typedef struct s_msg_status {
	char status[UDP_STATUS_BUFFER_SIZE];
} s_msg_status;

void network_establish_connection(uint32_t peer_address);
void network_listen_for_commands(void);
void network_listen_for_settings(void);
void network_send_status(char* msg, uint32_t peer_address);
void network_connected(void);
void network_disconnected(void);



#endif /* NETWORKING_H_ */