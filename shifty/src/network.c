#include "network.h"

#include <stdint.h>
#include "version.h"

#if defined(NETWORKING)
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#if defined(NETWORKING)
struct sockaddr_in server;
struct sockaddr_in client;
#endif

char *hostAddress = "127.0.0.1";			/* Default address. */
static int conFd;							/* File descriptor for current connection. */
static int clientFd;						/* File descriptor for the client's connection. */
static int *outFd;							/* Current file descriptor. */

static NetDataBuffer netBuffer;
NetConnection netConnection;
int connectionSuccess;
int connectionConnecting;
int connectionRetry;

static int serverInit();
static int serverListen();
static int serverAccept();
static void serverClose();

#if !defined(NETWORKING)

void networkListen()
{
}

void networkConnect()
{
}

void networkCloseConnection()
{
}

void networkSendPacket(uint8_t type)
{
}

void networkReceivePacket(void)
{
}

#else

static int serverInit()
{
	conFd = socket(AF_INET, SOCK_STREAM, 0);

	if (conFd == -1)
		return -1;

	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_family = AF_INET;
	server.sin_port = htons(NET_PORT);

	if (bind(conFd, (struct sockaddr *)&server, sizeof(server)) < 0)
		return -1;

	listen(conFd, 3);

	return 0;
}

static int serverListen()
{
	struct pollfd fds[1];
	int optionValue;
	socklen_t optionLen = sizeof(socklen_t);

	fds[0].fd = conFd;
	fds[0].events = POLLIN;

	if (poll(fds, 1, 0) <= 0)
		return 0;

	if ((fds[0].revents & POLLIN) == 0)
		return 0;

	getsockopt(conFd, SOL_SOCKET, SO_ERROR, &optionValue, &optionLen);

	if (optionValue)
	{
		return -1;
	}

	return serverAccept();
}

static int serverAccept()
{
	socklen_t len = sizeof(client);

	clientFd = accept(conFd, (struct sockaddr *)&client, &len);

	if (clientFd < 0)
		return errno == EAGAIN ? 0 : -1;

	fcntl(clientFd, F_SETFL, O_NONBLOCK);
	outFd = &clientFd;

	netConnection.status = NET_STATUS_CONNECTED;

	return 0;
}

static void serverClose()
{
	close(conFd);

	close(clientFd);
	outFd = NULL;
}

static int clientInit()
{
	conFd = socket(AF_INET, SOCK_STREAM, 0);

	if (conFd == -1)
		return -1;

	server.sin_addr.s_addr = inet_addr(hostAddress);
	server.sin_family = AF_INET;
	server.sin_port = htons(NET_PORT);

	fcntl(conFd, F_SETFL, O_NONBLOCK);
	outFd = &conFd;

	if (connect(conFd, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		switch (errno)
		{
			case EINPROGRESS:
			return 0;

			default:
			return -1;
		}
	}

	printf("reached\n");

	return 0;
}

static int clientConnect()
{
	struct pollfd fds[1];
	int optionValue;
	socklen_t optionLen = sizeof(socklen_t);

	fds[0].fd = conFd;
	fds[0].events = POLLOUT;

	if (poll(fds, 1, 0) <= 0)
		return 0;

	if ((fds[0].revents & POLLOUT) == 0)
		return 0;

	getsockopt(conFd, SOL_SOCKET, SO_ERROR, &optionValue, &optionLen);

	switch (optionValue)
	{
		case 0:
			netConnection.status = NET_STATUS_CONNECTED;
		break;

		case ECONNREFUSED:
			netConnection.timer = 0;

			close(conFd);
			clientInit();
		break;

		default:
		return -1;
	}

	return 0;
}

static void clientClose()
{
	close(conFd);
}

void networkListen()
{
	if (netConnection.status == NET_STATUS_DISCONNECTED)
	{
		if (serverInit())
			goto error;

		netConnection.status = NET_STATUS_LISTENING;
	}

	if (serverListen())
		goto error;

	return;

	error:
		networkCloseConnection();
	return;
}

void networkConnect()
{
	if (netConnection.status == NET_STATUS_DISCONNECTED)
	{
		if (clientInit())
			goto error;

		netConnection.status = NET_STATUS_CONNECTING;
	}

	if (clientConnect())
		goto error;

	return;

	error:
		networkCloseConnection();
	return;
}

void networkCloseConnection()
{
	if (netConnection.type == NET_CONNECTION_HOST)
	{
		serverClose();
	}
	else
	{
		clientClose();
	}

	memset(&netConnection, 0, sizeof(NetConnection));
	netConnection.status = NET_STATUS_DISCONNECTED;
}

void networkSendPacket(NetPacketType type)
{
	NetDataBuffer packet;

	if (netConnection.status != NET_STATUS_CONNECTED || !outFd)
		return;

	memset(&packet, 0, sizeof(NetDataBuffer));

	packet.buffer[0] = type;

	switch (type)
	{
		case NET_PACKET_VERSION:
			packet.dataLen = 3;

			packet.buffer[3] = PROGRAM_MAJOR_VERSION & 0xf;
			packet.buffer[4] = PROGRAM_MINOR_VERSION & 0xf;
			packet.buffer[5] = PROGRAM_PATCH_VERSION & 0xf;
		break;
	}

	packet.buffer[1] = (packet.dataLen >> 8);
	packet.buffer[2] = packet.dataLen & 0xff;

	send(*outFd, packet.buffer, NET_BUF_HEADER_LEN + packet.dataLen, MSG_NOSIGNAL);
}

void networkReceivePacket()
{
	int len;

	if (netConnection.status != NET_STATUS_CONNECTED || !outFd)
		return;

	len = recv(*outFd, netBuffer.buffer + netBuffer.len, sizeof(netBuffer.buffer) - netBuffer.len, MSG_NOSIGNAL);

	if (len <= 0)
	{
		if (len && errno == EAGAIN)
			return;

		networkCloseConnection();
		return;
	}

	netBuffer.len += len;

	while (netBuffer.len >= NET_BUF_HEADER_LEN)
	{
		if (netBuffer.len >= NET_BUF_LEN_MAX)
		{
			/* Buffer overflow. */
			return;
		}

		if (!netBuffer.parseData)
		{
			netBuffer.dataLen = (((uint16_t)netBuffer.buffer[1]) << 8) | netBuffer.buffer[2];
			netBuffer.parseData = 1;
		}

		if (netBuffer.len < NET_BUF_HEADER_LEN + netBuffer.dataLen)
			break;

		switch (netBuffer.buffer[0])
		{
			case NET_PACKET_VERSION:
				printf("Opponent's game version: %d.%d.%d\n", netBuffer.buffer[3], netBuffer.buffer[4], netBuffer.buffer[5]);
			break;

			default:
				/* Unknown packet type. */
			break;
		}

		memmove(&netBuffer.buffer, netBuffer.buffer + NET_BUF_HEADER_LEN + netBuffer.dataLen, netBuffer.len - (NET_BUF_HEADER_LEN + netBuffer.dataLen));
		netBuffer.len -= NET_BUF_HEADER_LEN + netBuffer.dataLen;
		netBuffer.parseData = 0;
	}
}

#endif
