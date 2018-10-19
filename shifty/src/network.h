#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <stdint.h>

#define NET_PORT						8888
#define NET_CONNECTION_ATTEMPTS_MAX		15
#define NET_CONNECTION_TIMEOUT			120

#define NET_BUF_LEN_MAX					2000
#define NET_BUF_HEADER_LEN				3

typedef enum NetPacketType
{
	NET_PACKET_VERSION
} NetPacketType;

typedef enum NetConnectionStatus
{
	NET_STATUS_DISCONNECTED,
	NET_STATUS_LISTENING,
	NET_STATUS_CONNECTING,
	NET_STATUS_CONNECTED
} NetConnectionStatus;

typedef enum NetConnectionType
{
	NET_CONNECTION_NONE,
	NET_CONNECTION_HOST,
	NET_CONNECTION_CLIENT
} NetConnectionType;

typedef struct NetConnection
{
	NetConnectionType type;
	NetConnectionStatus status;
	unsigned int attempt;
	unsigned int timer;
	
} NetConnection;

typedef struct NetDataBuffer
{
	uint8_t buffer[NET_BUF_LEN_MAX];
	int len;
	uint16_t dataLen;
	int parseData;
} NetDataBuffer;

extern char *hostAddress;
extern NetConnection netConnection;

void networkListen();
void networkConnect();
void networkCloseConnection();
void networkSendPacket(NetPacketType type);
void networkReceivePacket();

#endif /* _NETWORK_H_ */
