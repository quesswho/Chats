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
	#ifdef __WIN32__
	for (int i = 0; i < clientLimit; i++)
	{
		threads[i].detach();
		closesocket(clients[i].socket);
	}
	closesocket(listeningSocket);
	WSACleanup();
	#else
	for (int i = 0; i < clientLimit; i++)
	{
		threads[i].detach();
		shutdown(clients[i].socket, SHUT_RDWR);
	}
	shutdown(listeningSocket, SHUT_RDWR);
	#endif
	closed = true;
}

bool Server::Init()
{
	#ifdef __WIN32__
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
	#else
	listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(listeningSocket < 0)
	{
		std::cout << "Failed to create listening socket!" << std::endl;
		return false;
	}
	struct sockaddr_in addrInfo;
	memset((char*) &addrInfo, 0, sizeof(addrInfo));
	addrInfo.sin_family = AF_INET;
	addrInfo.sin_addr.s_addr = inet_addr(m_Ip);
	addrInfo.sin_port = htons(atoi(m_Port)); // from char* to int with host byte order to network byte order
	if(bind(listeningSocket, (struct sockaddr *) &addrInfo, sizeof(addrInfo)) < 0)
	{
		std::cout << "Error: Failed to bind ip: " << m_Ip << ":" << m_Port << std::endl;
	}
		
	#endif
	closed = false;
	serverLog = "";
	return true;
}
#ifdef __WIN32__
void Server::Start(int clientlimit)
{
	clientLimit = clientlimit;
#else
void *Server::Start(void *arg)
{
	clientLimit = *((int *) arg);
#endif
	
	int i;
	clients = new User[clientLimit];
	threads = new std::thread[clientLimit];

#ifdef __WIN32__
	for (i = 0; i < clientLimit; i++) {
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
#else
	for (i = 0; i < clientLimit; i++) {
		clients[i].id = -1;
		clients[i].socket = -1;
	}
	//Listen on ip and port
	if (listen(listeningSocket, 5) < 0) {
		std::cout << "Listen failed: " << errno << std::endl;
		shutdown(listeningSocket, SHUT_RDWR);
		return 0;
	}
#endif

	while (true)
	{
		if (online == clientLimit) {
#ifdef DEBUG
			std::cout << "Server is full!" << std::endl;
#endif
			serverLog.append("Server is full!");

			continue;
		}


		#ifdef __WIN32__
		SOCKET tempsocket = accept(listeningSocket, NULL, NULL);
		if (tempsocket == INVALID_SOCKET) continue;

		for (i = 0; i < clientlimit; i++)
		{
			if (clients[i].id == -1) {
				clients[i].id = i;
				clients[i].socket = tempsocket;
				break;
			}
		}
		#else
		int tempsocket = accept(listeningSocket, (struct sockaddr *) NULL, NULL); //Check here first
		if(tempsocket < 0) continue;
		
		for (i = 0; i < clientLimit; i++)
		{
			if (clients[i].id == -1) {
				clients[i].id = i;
				clients[i].socket = tempsocket;
				break;
			}
		}
		#endif

		online++;
#ifdef DEBUG
		std::cout << "Client " << i << " has connected!" << std::endl;
#endif
		serverLog.append("Client ").append(std::to_string(i)).append(" has connected!\n");
		SendAll(std::string("Client ").append(std::to_string(i)).append(" has connected!\n").c_str());

		threads[i] = std::thread(&Server::ClientSession, this, clients[i]);
	}
}

void Server::ClientSession(User client)
{
	std::string msg = "";
	char tempmsg[DEFAULT_BUFLEN] = "";

	while (true)
	{
		memset(tempmsg, 0, DEFAULT_BUFLEN);

		if (client.socket != 0 && !(client.socket < 0))
		{
			#ifdef __WIN32__
			int error = recv(client.socket, tempmsg, DEFAULT_BUFLEN, 0);
			if (error != SOCKET_ERROR)
			{
			#else
			int error = recv(client.socket, tempmsg, DEFAULT_BUFLEN, 0);
			if(error != -1)
			{
			#endif
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
				#ifdef __WIN32__
				closesocket(clients[client.id].socket);
				#else
				shutdown(clients[client.id].socket, SHUT_RDWR);
				#endif
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
		#ifdef __WIN32__
		if (clients[i].id != -1 && clients[i].socket != INVALID_SOCKET)
		{
			int error = send(clients[i].socket, message, (int)strlen(message), 0);
			if (error == SOCKET_ERROR) {
				std::cout << "Failed to send to " << WSAGetLastError();
				closesocket(clients[i].socket);
				WSACleanup();
			}
		}
		#else
		if (clients[i].id != -1 && clients[i].socket != -1)
		{
			int error = send(clients[i].socket, message, (int)strlen(message), 0);
			if (error < 0) {
				std::cout << "Failed to send to " << errno;
				shutdown(clients[i].socket, SHUT_RDWR);
			}
		}
		#endif
	}
	Print();
	return true;
}

void Server::Print()
{
	ClearConsole();
	std::cout << "Server: " << m_Ip << ":" << m_Port << std::endl;
	std::cout << serverLog;
//	SetTitle(std::string("Server: ").append(m_Ip).append(":").append(m_Port).c_str());
}

void Server::ClearConsole()
{
	#ifdef __WIN32__
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
	#else
	std::cout << "\033[2J\033[1;1H";
	#endif
}

void SetTitle(const char* title)
{
	#ifdef __WIN32__
	SetConsoleTitle(title);
	#else
	std::cout << "\033]0;" << title << "\007";
	#endif
}