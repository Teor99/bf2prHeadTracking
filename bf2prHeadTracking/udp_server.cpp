#include "pch.h"

using namespace std;

#pragma comment(lib,"ws2_32.lib") // Winsock Library
#pragma warning(disable:4996) 

#define BUFLEN 512
#define PORT 8888

CameraCoordsPacket cc;
bool isNeedToStopThread = false;
std::string statusInfo;

void stopUdpServer() {
	isNeedToStopThread = true;
}

std::string udpServerGetStatusInfo() {
	return statusInfo;
}

void udpServer()
{

	// initialise winsock
	WSADATA wsa;
	sockaddr_in server, client;

	statusInfo = "Initialising Winsock...";
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		statusInfo = std::format("WSAStartup failed. Error Code: {}", WSAGetLastError());
		isNeedToStopThread = true;
	}

	// create a socket
	SOCKET server_socket;
	if ((server_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
		statusInfo = std::format("Could not create socket: {}", WSAGetLastError());
		isNeedToStopThread = true;
	}
	else
	{
		statusInfo = "Socket created";
	}

	// prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	//server.sin_addr.s_addr = INADDR_ANY;
	inet_pton(AF_INET, "127.0.0.1", &(server.sin_addr));
	server.sin_port = htons(PORT);

	// bind
	if (bind(server_socket, (sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		statusInfo = std::format("Bind failed with error code: {}", WSAGetLastError());
		isNeedToStopThread = true;
	}

	char serverHostLine[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(server.sin_addr), serverHostLine, INET_ADDRSTRLEN);
	statusInfo = std::format("Listening on: {}:{}", serverHostLine, PORT);

	while (!isNeedToStopThread)
	{
		char message[BUFLEN] = {};

		// try to receive some data, this is a blocking call
		int message_len;
		int slen = sizeof(sockaddr_in);
		if (message_len = recvfrom(server_socket, message, BUFLEN, 0, (sockaddr*)&client, &slen) == SOCKET_ERROR)
		{
			statusInfo = std::format("recvfrom() failed with error code: {}", WSAGetLastError());
			break;
		}
		else
		{
			char clientHostLine[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, &(client.sin_addr), clientHostLine, INET_ADDRSTRLEN);
			statusInfo = std::format("Receiving data to {}:{} from {}:{}", serverHostLine, PORT, clientHostLine, client.sin_port);
			double* arr = (double*)message;
			cc.x = arr[0];
			cc.y = arr[1];
			cc.z = arr[2];
			cc.yaw = arr[3];
			cc.pitch = arr[4];
			cc.roll = arr[5];
		}
	}

	statusInfo = "Close socket";
	closesocket(server_socket);
	WSACleanup();
}