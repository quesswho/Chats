#include "Server.h"
#include <iostream>

#define DEFAULT_BUFLEN 512

Server::Server(const char* ip, const char* port)
	: m_Ip(ip), m_Port(port), closed(true)
{
	if (Init()) {
		std::cout << "Sucessfully initialized server!" << std::endl;
		closed = false;
	}
}

Server::~Server()
{
	closesocket(listeningSocket);
	WSACleanup();
	closed = true;
}

bool Server::Init()
{
	WSADATA wsaData;
	//init Winsock
	int error = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (error != 0) {
		std::cout << "Error:" << error << "Failed to initialize Winsock " << std::endl;
		return false;
	}

	struct addrinfo *addrInfo = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Setting addrinfo
	error = getaddrinfo(m_Ip, m_Port, &hints, &addrInfo);
	if (error != 0) {
		std::cout << "Error: " << error << " Failed to use ip: " << m_Ip << ":" << m_Port << std::endl;
		WSACleanup();
		return false;
	}

	//Create listening socket
	listeningSocket = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
	if (listeningSocket == INVALID_SOCKET) {
		std::cout << "Failed to create listening socket: " << WSAGetLastError() << std::endl;
		freeaddrinfo(addrInfo);
		WSACleanup();
		return false;
	}

	//Bind socket to ip and port
	error = bind(listeningSocket, addrInfo->ai_addr, (int)addrInfo->ai_addrlen);
	if (error == SOCKET_ERROR) {
		std::cout << "Could not bind port: " << WSAGetLastError() << std::endl;
		freeaddrinfo(addrInfo);
		closesocket(listeningSocket);
		WSACleanup();
		return false;
	}
	freeaddrinfo(addrInfo);
	closed = false;
	serverLog = "";
}

void Server::Start(int clientlimit)
{
	//Listen on ip and port
	if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "Listen failed: " << WSAGetLastError() << std::endl;
		closesocket(listeningSocket);
		WSACleanup();
		return;
	}

	while (true)
	{
		client = accept(listeningSocket, NULL, NULL);
		if (client == INVALID_SOCKET) continue; // no client was trying to connect
		std::cout << "Client has connected!" << std::endl;
		serverLog.append("Client has connected\n");
		Beep(400, 500);
		clientThread = std::thread(&Server::ClientSession, this, client);
	}
}

void Server::ClientSession(SOCKET user)
{
	std::string msg = "";
	char tempmsg[DEFAULT_BUFLEN] = "";

	while (true)
	{
		memset(tempmsg, 0, DEFAULT_BUFLEN);

		if (user != 0)
		{
			int error = recv(user, tempmsg, DEFAULT_BUFLEN, 0);
			if (error != SOCKET_ERROR)
			{
				if (strcmp("", tempmsg)) {
					msg = "Client: ";
					msg.append(tempmsg).append("\n");
				}

				std::cout << msg.c_str() << std::endl;
				serverLog.append(msg);
			}
			else
			{
				msg = "Client Disconnected";

				std::cout << msg << std::endl;
				serverLog.append(msg).append("\n");

				closesocket(user);

				break;
			}
		}
	}

	clientThread.detach();
	return;
}