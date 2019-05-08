/* Aufgabenstellung serverSys.c - Win32 Konsole Version - connectionless */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "data.h"
#include "config.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ctype.h>


#define WIN32_LEAN_AND_MEAN

// Link with ws2_32.lib
#pragma comment(lib, "Ws2_32.lib")

/************************************************/
/*** Declaration socket descriptor            ***/
/************************************************/
SOCKET ConnSocket;

/*****************************************************/
/*** Deklaration of socket address "local"         ***/
/*****************************************************/

struct sockaddr *sockaddr_ip6_local = NULL;

/******************************************************/
/*** Deklaration of socket address "remote" static  ***/
/******************************************************/
static struct sockaddr_in6 remoteAddr;   //Broker Address

int joinMCAddress(char *mcGroupAddr)
{
	struct ipv6_mreq mreq;  //multicast address
	struct addrinfo *resultMulticastAddress = NULL, 
				    *ptr = NULL,
					 hints;

	/* Resolve multicast group address to join mc group*/
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_NUMERICHOST;
	if (getaddrinfo(mcGroupAddr, NULL, &hints, &resultMulticastAddress) != 0){
		fprintf(stderr, "getaddrinfo MCAddress failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		exit(-1);
	}

	/****************************************************************************/
	/* Join the multicast topic group to get publish message from broker		*/
	/****************************************************************************/
	memcpy(&mreq.ipv6mr_multiaddr, &((struct sockaddr_in6*)(resultMulticastAddress->ai_addr))->sin6_addr, sizeof(mreq.ipv6mr_multiaddr));

	/* Accept multicast from any interface */
	// scope ID 0 : from any interface, x your Interface. scope (cmd: 'netsh int ipv6 sh addr')
	mreq.ipv6mr_interface = 0;

	/* Join the multicast address (heck with: netsh interface ipv6 show joins x)*/
	if (setsockopt(ConnSocket, IPPROTO_IPV6, IPV6_JOIN_GROUP,(char*)&mreq, sizeof(mreq)) < 0)
	{
		printf("setsockopt Failed with Error: %d\n", WSAGetLastError());
		closesocket(ConnSocket);
		exit(-1);
	}

	freeaddrinfo(resultMulticastAddress);

	return(0);
}

int initClient() 
{   
	//default MC Adress and port DEFAULT_SUBSCRIBER_PORT to get the broker announcements 

	int trueValue = 1, loopback = 0;  //setsockopt
	int val, i = 0;
	int addr_len;
	long rc;

	char *MCAddress = DEFAULT_ANNOUNCEMENT_MC; // announcement mc socket udp
	char *Port = DEFAULT_SUBSCRIBER_PORT;
	struct addrinfo *resultLocalAddress = NULL, 
					*ptr = NULL,
					hints;
	WSADATA wsaData;
	WORD wVersionRequested;

	wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) == SOCKET_ERROR)
	{
		printf("SERVER:   WSAStartup() failed!\n");
		printf("          error code:   %d\n", WSAGetLastError());
		exit(-1);
	}

	/**********************************************************/
	/*** Create a UDP Socket,								***/
	/*** connectionless service, address family INET6       ***/
	/**********************************************************/
	ConnSocket = socket(AF_INET6, SOCK_DGRAM, 0);
	if (ConnSocket == INVALID_SOCKET)
	{
		printf("Error: The Socket couldn't be created, error code: %d\n", WSAGetLastError());
		return 1;
	}
	else
	{
		printf("UDP Socket created!\n");
	}

	/* Initialize socket */
	/* Reusing port for several subscriber listening to same multicast addr and port (testing on local machine only) */
	if (setsockopt(ConnSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&trueValue, sizeof(trueValue)) < 0)
	{
		printf("setsockopt Failed with Error: %d\n", WSAGetLastError());
		closesocket(ConnSocket);
		exit(-1);
	}

	
	/* Resolve local address (anyaddr) to bind*/
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET6;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	hints.ai_flags = AI_PASSIVE; //localhost

	val = getaddrinfo(NULL, Port, &hints, &resultLocalAddress);

	if (val != 0) 
	{
		printf("getaddrinfo localAddress failed with error: %d\n", val);
		WSACleanup();
		exit(-1);
	}

	// Retrieve the address 
	for (ptr = resultLocalAddress; ptr != NULL; ptr = ptr->ai_next) 
	{

		printf("getaddrinfo response %d\n", i++);
		printf("\tFlags: 0x%x\n", ptr->ai_flags);
		printf("\tFamily: ");
		switch (ptr->ai_family) {
		case AF_UNSPEC:
			printf("Unspecified\n");
			break;
		case AF_INET:
			printf("AF_INET (IPv4)\n");
			break;
		case AF_INET6:
			printf("AF_INET6 (IPv6)\n");

			sockaddr_ip6_local = (struct sockaddr *) ptr->ai_addr;
			addr_len = ptr->ai_addrlen;
			break;
		default:
			printf("Other %ld\n", ptr->ai_family);
			break;
		}
	}

	/*********************************************/
	/*** Bind Socket ***/
	/*********************************************/
	
	//...
	rc = bind(ConnSocket, sockaddr_ip6_local, addr_len);
	if (rc == SOCKET_ERROR)
	{
		printf("Error: bind, error code: %d\n", WSAGetLastError());
		return;
	}
	else
	{
		printf("Socket bound to port 50002\n");
	}

	freeaddrinfo(resultLocalAddress);

	/****************************************************************************/
	/* Specify the multicast group to get announcements from broker				*/
	/****************************************************************************/

	if ( joinMCAddress(DEFAULT_ANNOUNCEMENT_MC)!=0)
		printf("Joining MC Group Address for getting Announcements failed!\n");
	
	return(0);
}

void getRequest(struct genericMsg *req) 
{
	
	int recvcc;
	SOCKADDR_IN6 remoteAddr;
	
	int remoteAddrSize = sizeof(struct sockaddr_in6);

	printf("in getRequest\n");

	//receice UDP
	recvcc = recvfrom(ConnSocket, (char *)req, sizeof(struct genericMsg), 0, (SOCKADDR_IN6*)&remoteAddr, &remoteAddrSize);
	if (recvcc == SOCKET_ERROR)
	{
		printf("Error: recvfrom, error code: %d\n", WSAGetLastError());
		
	}
	else
	{
		printf("%d Bytes received!\n", recvcc);
	}
}

int closeClient() {

	/************************/
	/*** Close Socket     ***/
	/************************/
	
	//...
	return(0);
}




