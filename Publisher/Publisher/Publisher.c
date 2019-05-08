#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <ws2ipdef.h>
#include <in6addr.h>
#include <WS2tcpip.h>
#include "data.h"

#pragma comment( lib,"ws2_32.lib")

//Forward Declare
int startWinsock();

int main(int argc,char *argv[])
{	
	SOCKET s;
	struct publishMsg msg;
	struct addrinfo hints;
	struct addrinfo *result;
	char topic[BUFFER_S];
	float value;
	long rc;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//start WSA
	rc = startWinsock();
	if (rc != 0)
	{
		printf("Error: startWinsock, error code: %d\n", rc);
		return 1;
	}
	else
	{
		printf("Winsock started!\n");
	}

	//getaddrinfo
	if (getaddrinfo(argv[1], argv[2], &hints, &result) != 0)
	{
		printf("\ngetaddrinfo throws Error No. %ld v: \n", WSAGetLastError());
		WSACleanup();
		exit(-1);
	}

	//create Socket
	s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (s == INVALID_SOCKET)
	{
		printf("Error: Socket couldn't be created, error code: %d\n", WSAGetLastError());
		return 1;
	}
	else
	{
		printf("Socket created!\n");
	}

	//connect
	rc = connect(s, result->ai_addr, result->ai_addrlen);
	if (rc == SOCKET_ERROR)
	{
		printf("Error: connect failed, error code: %d\n", WSAGetLastError());
		return 1;
	}
	else
	{
		printf("Connected to %s on Port: %s\n",argv[1],argv[2]);
	}

	//send Topics and Values (infinite loop for testing purpose)
	while (1)
	{
		printf("\n\nPlease enter topic!\n");
		scanf("%s", &topic);

		printf("\nPlease enter Value!\n");
		scanf("%f", &value);

		msg.header.ReqType = 'P';
		msg.packetId = 0;
		strcpy(msg.topic, topic);
		msg.qos = 0;
		msg.retainFlag = 0;

		if ((value - (int)value == 0))
		{
			msg.payload.payloadType = 0;
			msg.payload.intValue = (int)value;
		}
		else
		{
			msg.payload.payloadType = 1;
			msg.payload.floatValue = value;
		}

		rc = send(s, &msg, sizeof(msg), 0);
		if (rc == SOCKET_ERROR)
		{
			printf("Error: connect failed, error code: %d\n", WSAGetLastError());
			return 1;
		}
		else
		{
			printf("\n%d Bytes sent!\n", rc);
		}
	}

	return 0;
}

int startWinsock()
{
	WSADATA wsa;
	return WSAStartup(MAKEWORD(2, 2), &wsa);
}