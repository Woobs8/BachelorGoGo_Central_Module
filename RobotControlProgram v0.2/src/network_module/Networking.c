#include "Networking.h"
#include "WiFi_P2P.h"
#include "String.h"
#include "protocol.h"
#include "driver/include/m2m_wifi.h"
#include "freertos_tasks.h"

/** Receive buffer definition. */
static uint8_t TCPPortExchangeRxBuffer[TCP_PORT_EXCHANGE_BUFFER_SIZE];
static uint8_t UDPCommandRxBuffer[UDP_COMMAND_BUFFER_SIZE];
static uint8_t TCPSettingsRxBuffer[TCP_SETTINGS_BUFFER_SIZE];

/** Sockets */
SOCKET tcp_port_exchange_socket = -1;		// TCP client socket
SOCKET udp_command_socket = -1;				// UDP server socket
SOCKET tcp_settings_listen_socket = -1;		// TCP server listening socket
SOCKET tcp_settings_data_socket = -1;		// TCP server data transfer socket
SOCKET udp_status_socket = -1;				// UDP client socket

/** Network ports */
uint16_t host_udp_port;



/**
 * Callback to handle socket events
 */
static void socket_event_handler_cb(SOCKET sock, uint8_t u8Msg, void *pvMsg)
{
	/** Establishing connection */
	if(sock == tcp_port_exchange_socket)
	{
		/* Socket connected */
		if(u8Msg == SOCKET_MSG_CONNECT)
		{
			tstrSocketConnectMsg *pstrConnect = (tstrSocketConnectMsg *)pvMsg;
			if (pstrConnect && pstrConnect->s8Error >= 0) {
				printf("tcp_port_exchange_socket: connect success!\r\n");
				// Perform data exchange.
				s_msg_port port_msg;
			
				// Send UDP command port
				sprintf(port_msg.port, "%d", UDP_COMMAND_PORT);
				send(tcp_port_exchange_socket, &port_msg, sizeof(s_msg_port), 0);
				
				// Send TCP settings port
				sprintf(port_msg.port, "%d", TCP_SETTINGS_PORT);
				send(tcp_port_exchange_socket, &port_msg, sizeof(s_msg_port), 0);
			
				// Send TCP settings port
				sprintf(port_msg.port, "%d", HTTP_VIDEO_PORT);
				send(tcp_port_exchange_socket, &port_msg, sizeof(s_msg_port), 0);
			
				// Recv. UDP status port
				recv(sock, TCPPortExchangeRxBuffer, sizeof(TCPPortExchangeRxBuffer), 0);
		
			} else {
				printf("tcp_port_exchange_socket: connect error!\r\n");
				close(tcp_port_exchange_socket);
				tcp_port_exchange_socket = -1;
				m2m_wifi_disconnect();
			}
		}
		/* Message receive */
		else if(u8Msg == SOCKET_MSG_RECV)
		{
			tstrSocketRecvMsg *pstrRecv = (tstrSocketRecvMsg *)pvMsg;
			if (pstrRecv && pstrRecv->s16BufferSize > 0) {
				printf("tcp_port_exchange_socket: recv success!\r\n");
				char *ptr;
				host_udp_port = strtol(TCPPortExchangeRxBuffer,ptr,10);
				printf("Host port resolved to: (%d)\r\n",host_udp_port);
				close(tcp_port_exchange_socket);
				tcp_port_exchange_socket = -1;
				network_connected();
			} else {
				printf("tcp_port_exchange_socket: recv error!\r\n");
				close(tcp_port_exchange_socket);
				tcp_port_exchange_socket = -1;
				m2m_wifi_disconnect();
			}
		}
	}
	/** Listening for control commands */
	else if(sock == udp_command_socket)
	{
		/** Socket bound */
		if(u8Msg == SOCKET_MSG_BIND)
		{
			tstrSocketBindMsg *pstrBind = (tstrSocketBindMsg*)pvMsg;
			if(pstrBind->status == 0)
			{
				// call Recv
				recvfrom(udp_command_socket, UDPCommandRxBuffer, sizeof(UDPCommandRxBuffer), 0);
			}
			else
			{
				printf("udp_command_socket: Bind Failed\n");
			}
		}
		/** Command received */
		else if((u8Msg == SOCKET_MSG_RECV) || (u8Msg == SOCKET_MSG_RECVFROM))
		{
			tstrSocketRecvMsg *pstrRecvMsg = (tstrSocketRecvMsg*)pvMsg;
			if((pstrRecvMsg->pu8Buffer != NULL) && (pstrRecvMsg->s16BufferSize > 0))
			{
				printf("udp_command_socket: recvfrom success!\r\n");
				
				if(pstrRecvMsg->s16BufferSize == UDP_COMMAND_BUFFER_SIZE)
				{
					printf("udp_command_socket: command received: %s\r\n",UDPCommandRxBuffer);
					//xQueueSendToFront(xControl_Msg_Queue_handle, UDPCommandRxBuffer, 0);
					network_message_parser(UDPCommandRxBuffer);
				}
			}
		}
		recvfrom(udp_command_socket, UDPCommandRxBuffer, sizeof(UDPCommandRxBuffer), 0);
	}
	/** Server socket listening for connection request to transfer settings command */
	else if(sock == tcp_settings_listen_socket)
	{
		/** Socket bound */
		if(u8Msg == SOCKET_MSG_BIND)
		{
			tstrSocketBindMsg *pstrBind = (tstrSocketBindMsg*)pvMsg;
			if(pstrBind->status == 0)
			{
				listen(tcp_settings_listen_socket, 0);
			}
			else
			{
				printf("tcp_settings_listen_socket: Bind Failed\n");
				close(tcp_settings_listen_socket);
				tcp_settings_listen_socket = -1;
			}
		}
		/** Socket listening */
		else if(u8Msg == SOCKET_MSG_LISTEN)
		{
			tstrSocketListenMsg *pstrListen = (tstrSocketListenMsg*)pvMsg;
			if (pstrListen && pstrListen->status == 0) {
				printf("tcp_settings_listen_socket: listen success!\r\n");
				accept(tcp_settings_listen_socket, NULL, NULL);
			}
			else
			{
				close(tcp_settings_listen_socket);
				tcp_settings_listen_socket = -1;
				printf("tcp_settings_listen_socket: listen Failed. Restarting...\n");
				network_listen_for_settings();
			}
		}
		/** Accepting incoming connection */
		else if(u8Msg == SOCKET_MSG_ACCEPT)
		{
			// Socket is accepted.
			tstrSocketAcceptMsg *pstrAccept = (tstrSocketAcceptMsg *)pvMsg;
			if(pstrAccept)
			{
				printf("tcp_settings_listen_socket: accept success!\r\n");
				accept(tcp_settings_listen_socket, NULL, NULL);
				tcp_settings_data_socket = pstrAccept->sock;
				recv(tcp_settings_data_socket, TCPSettingsRxBuffer, sizeof(TCPSettingsRxBuffer), 0);
			}
			else
			{
				close(tcp_settings_listen_socket);
				tcp_settings_listen_socket = -1;
				printf("tcp_settings_listen_socket: Accept Failed\n");
				network_listen_for_settings();
			}
		}
	}
	/** Accepted connection for transfer of settings command */
	else if(sock == tcp_settings_data_socket)
	{
		if(u8Msg == SOCKET_MSG_RECV)
		{
			tstrSocketRecvMsg *pstrRecvMsg = (tstrSocketRecvMsg*)pvMsg;
			if((pstrRecvMsg->pu8Buffer != NULL) && (pstrRecvMsg->s16BufferSize > 0))
			{			
				// Process the received message
				printf("tcp_settings_data_socket: settings received: %s\r\n",TCPSettingsRxBuffer);
				
				//TODO: set settings and parse return to determine success or failure
				// for now just return ACK
				s_msg_ack msg;
				strncpy(msg.ack,CMD_ACK,CMD_SPECIFIER_SIZE);
				network_message_parser(TCPSettingsRxBuffer);
				printf("tcp_settings_data_socket: configuration successful. Sending ACK...\r\n");
				send(tcp_settings_data_socket, &msg, sizeof(s_msg_ack), 0);
				
				// Close the accepted socket when finished.
				close(tcp_settings_data_socket);
				tcp_settings_data_socket = -1;
			}
		}
	}
	else if(sock == udp_status_socket)
	{
		if(u8Msg == SOCKET_MSG_SENDTO)
		{
			printf("udp_status_socket: Status sent!\r\n");
			if(udp_status_socket > 0) {
				close(udp_status_socket);
				udp_status_socket = -1;
			}
		}
	}
}

void network_establish_connection(uint32_t address)
{
	int8_t ret;
	struct sockaddr_in addr;
	
	/* Initialize socket address structure. */
	addr.sin_family = AF_INET;
	addr.sin_port = _htons(GO_PORT);
	addr.sin_addr.s_addr = _htonl(address);
	
	/* Initialize socket module */
	socketInit();
	registerSocketCallback(socket_event_handler_cb, NULL);
	
	/* Open client socket. */
	if (tcp_port_exchange_socket < 0) {
		if ((tcp_port_exchange_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			printf("Networking: failed to create TCP client socket error!\r\n");
			m2m_wifi_disconnect();
		}

		/* Connect server */
		ret = connect(tcp_port_exchange_socket, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

		if (ret < 0) {
			printf("Networking: failed to connect TCP client socket error!\r\n");
			close(tcp_port_exchange_socket);
			tcp_port_exchange_socket = -1;
			m2m_wifi_disconnect();
		}
	}	
}

void network_listen_for_commands(void)
{
	struct sockaddr_in strAddr;
	// Initialize the socket layer.
	socketInit();
	
	// Register socket application callbacks.
	registerSocketCallback(socket_event_handler_cb, NULL);
	
	// Open socket
	udp_command_socket = socket(AF_INET,SOCK_DGRAM,0);
	
	// Bind socket and listen from any address
	strAddr.sin_family = AF_INET;
	strAddr.sin_port = _htons(UDP_COMMAND_PORT);
	strAddr.sin_addr.s_addr = 0; //INADDR_ANY
	bind(udp_command_socket, (struct sockaddr*)&strAddr, sizeof(struct sockaddr_in));
}

void network_listen_for_settings(void)
{
	struct sockaddr_in strAddr;
	// Initialize the socket layer.
	socketInit();
	// Register socket application callbacks.
	registerSocketCallback(socket_event_handler_cb, NULL);
	// Create the server listen socket.
	tcp_settings_listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if(tcp_settings_listen_socket >= 0)
	{
		strAddr.sin_family = AF_INET;
		strAddr.sin_port = _htons(TCP_SETTINGS_PORT);
		strAddr.sin_addr.s_addr = 0; //INADDR_ANY
		
		bind(tcp_settings_listen_socket, (struct sockaddr*)&strAddr, sizeof(struct sockaddr_in));
	}
}

void network_send_status(char* msg, uint32_t address)
{
	struct sockaddr_in strAddr;
	// Initialize the socket layer.
	socketInit();
	// Register socket application callbacks.
	registerSocketCallback(socket_event_handler_cb, NULL);
	udp_status_socket = socket(AF_INET,SOCK_DGRAM,0);
	if(udp_status_socket >= 0)
	{
	strAddr.sin_family = AF_INET;
	strAddr.sin_port = _htons(host_udp_port);
	strAddr.sin_addr.s_addr = _htonl(address);
		
	// Fill in data
	s_msg_status status_msg;
	memcpy(status_msg.status,msg,PACKET_SIZE);

	// Format and send status message
	sendto(udp_status_socket, &status_msg, sizeof(s_msg_status), 0, (struct sockaddr*)&strAddr,
	sizeof(struct sockaddr_in));
	}
}

void network_connected(void)
{
	network_is_connected = 1;
	network_listen_for_commands();
	network_listen_for_settings();
}

void network_disconnected(void)
{
	network_is_connected = -1;
	if(tcp_port_exchange_socket > 0)
	{
		close(tcp_port_exchange_socket);
		tcp_port_exchange_socket = -1;
	}
	if(udp_command_socket > 0)
	{
		close(udp_command_socket);
		udp_command_socket = -1;		
	}
	if(tcp_settings_listen_socket > 0)
	{
		close(tcp_settings_listen_socket);
		tcp_settings_listen_socket = -1;
	}
	if(tcp_settings_data_socket > 0)
	{
		close(tcp_settings_data_socket);
		tcp_settings_data_socket = -1;
	}
}
