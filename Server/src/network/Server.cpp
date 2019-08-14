#include "Server.h"
#include <iostream>

#define DEFAULT_BUFLEN 512

Server::Server(const char* ip, const char* port)
	: m_Ip(ip), m_Port(port), closed(true)
{
	if (Init()) {
		std::cout << "Sucessfully initialized server!" << std::endl;
		Print();
	}
}

Server::~Server()
{
	for (int i = 0; i < clientLimit; i++)
	{
		threads[i].detach();
		closesocket(clients[i].socket);
	}
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
	return true;
}

void Server::Start(int clientlimit)
{
	clientLimit = clientlimit;
	int i;
	clients = new User[clientlimit];
	threads = new std::thread[clientlimit];

	for (i = 0; i < clientlimit; i++) {
		clients[i].id = -1;
		clients[i].socket = INVALID_SOCKET;
	}
	//Listen on ip and port
	if (listen(listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
		std::cout << "Listen failed: " << WSAGetLastError() << std::endl;
		closesocket(listeningSocket);
		WSACleanup();
		return;
	}

	while (true)
	{
		if (online == clientlimit) {
#ifdef DEBUG
			std::cout << "Server is full!" << std::endl;
#endif
			serverLog.append("Server is full!");

			continue;
		}


		
		SOCKET tempsocket = accept(listeningSocket, NULL, NULL);
		if (tempsocket == INVALID_SOCKET) continue;

		for (i = 0; i < clientlimit; i++)
		{
			//if (clients[i].id == -1 && clients[i].socket == INVALID_SOCKET) {
			if (clients[i].id == -1) {
				clients[i].id = i;
				clients[i].socket = tempsocket;
				break;
			}
		}

		online++;
#ifdef DEBUG
		std::cout << "Client " << i << " has connected!" << std::endl;
#endif
		serverLog.append("Client ").append(std::to_string(i)).append(" has connected!\n");
		SendAll(std::string("Client ").append(std::to_string(i)).append(" has connected!\n").c_str());

		threads[i] = std::thread(&Server::ClientSession, this, clients[i]);
		Beep(400, 500);
	}
}

void Server::ClientSession(User client)
{
	std::string msg = "";
	char tempmsg[DEFAULT_BUFLEN] = "";

	while (true)
	{
		memset(tempmsg, 0, DEFAULT_BUFLEN);

		if (client.socket != 0)
		{
			int error = recv(client.socket, tempmsg, DEFAULT_BUFLEN, 0);
			if (error != SOCKET_ERROR)
			{
				if (strcmp("", tempmsg)) {
					msg = std::string("Client ").append(std::to_string(client.id)).append(": ").append(tempmsg).append("\n");
#ifdef DEBUG
					std::cout << msg << std::endl;
#endif
					serverLog.append(msg);
					SendAll(msg.c_str());
				}

			}
			else
			{
				msg = std::string("Client ").append(std::to_string(client.id)).append(" has disconnected");
				online--;
				std::cout << msg << std::endl;
				serverLog.append(msg).append("\n");
				clients[client.id].id = -1;
				closesocket(clients[client.id].socket);
				SendAll(msg.c_str());


				break;
			}
		}
	}

	threads[client.id].detach();
	return;
}

bool Server::SendAll(const char* message)
{
	for (int i = 0; i < clientLimit; i++)
	{
		if (clients[i].id != -1 && clients[i].socket != INVALID_SOCKET)
		{
			int error = send(clients[i].socket, message, (int)strlen(message), 0);
			if (error == SOCKET_ERROR) {
				std::cout << "Failed to send to " << WSAGetLastError();
				closesocket(clients[i].socket);
				WSACleanup();
			}
		}
	}
	Print();
	return true;
}

void Server::Print()
{
	ClearConsole();
	std::cout << "Server: " << m_Ip << ":" << m_Port << std::endl;
	std::cout << serverLog;
	SetConsoleTitle(std::string("Server: ").append(m_Ip).append(":").append(m_Port).c_str());
	Sleep(1000);
}

void Server::ClearConsole()
{
	HANDLE                     hStdOut;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD                      count;
	DWORD                      cellCount;
	COORD                      homeCoords = { 0, 0 };

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut == INVALID_HANDLE_VALUE) return;

	/* Get the number of cells in the current buffer */
	if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;
	cellCount = csbi.dwSize.X *csbi.dwSize.Y;

	/* Fill the entire buffer with spaces */
	if (!FillConsoleOutputCharacter(
		hStdOut,
		(TCHAR) ' ',
		cellCount,
		homeCoords,
		&count
	)) return;

	/* Fill the entire buffer with the current colors and attributes */
	if (!FillConsoleOutputAttribute(
		hStdOut,
		csbi.wAttributes,
		cellCount,
		homeCoords,
		&count
	)) return;

	/* Move the cursor home */
	SetConsoleCursorPosition(hStdOut, homeCoords);
}