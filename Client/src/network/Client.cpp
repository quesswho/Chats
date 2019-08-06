#include "Client.h"

#define DEFAULT_BUFLEN 512

Client::Client()
	: closed(true), client(INVALID_SOCKET)
{
	if (Init()) {
		std::cout << "Sucessfully initialized client!" << std::endl;
		closed = false;
	}

}

Client::~Client()
{
	closesocket(client);
	WSACleanup();
	closed = true;
}

bool Client::Init()
{
	WSADATA wsaData;
	//init Winsock
	int error = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (error != 0) {
		std::cout << "Error:" << error << "Failed to initialize Winsock " << std::endl;
		return false;
	}
	return true;
	
}

bool Client::StartSession(const char* ip, const char* port) 
{
	m_Ip = ip;
	m_Port = port;
	struct addrinfo *addrInfo = NULL, *ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Setting addrinfo
	int error = getaddrinfo(m_Ip, m_Port, &hints, &addrInfo);
	if (error != 0) {
		std::cout << "Error: " << error << " Failed to connect to ip: " << m_Ip << ":" << m_Port << std::endl;
		WSACleanup();
		return false;
	}

	for (ptr = addrInfo; ptr != NULL; ptr = ptr->ai_next) {

		// Init socket
		client = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (client == INVALID_SOCKET) {
			std::cout << "Failed to create socket: " << WSAGetLastError() << std::endl;
			WSACleanup();
			return false;
		}

		// Connect to server
		error = connect(client, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (error == SOCKET_ERROR) {
			closesocket(client);
			client = INVALID_SOCKET;
			continue; //try again
		}
		break;
	}

	freeaddrinfo(addrInfo);

	if (client == INVALID_SOCKET) {
		std::cout << "Failed to connect to server " << ip << ":" << port << std::endl;
		WSACleanup();
		return false;
	}
}

void Client::Disconnect()
{
	int error = shutdown(client, SD_SEND);
	if (error == SOCKET_ERROR) {
		std::cout << "Failed to disconnect" << WSAGetLastError() << std::endl;
		closesocket(client);
		WSACleanup();
	}
}

bool Client::Send(const char* message)
{
	int error = send(client, message, (int)strlen(message), 0);
	if (error == SOCKET_ERROR) {
		std::cout << "Failed to send: " << WSAGetLastError();
		closesocket(client);
		WSACleanup();
		return 1;
	}
	serverLog.append(message);
}