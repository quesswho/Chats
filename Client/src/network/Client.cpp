#include "Client.h"

#define DEFAULT_BUFLEN 512

Client::Client()
	: closed(true), server(INVALID_SOCKET), connected(false)
{
	if (Init()) {
		std::cout << "Sucessfully initialized client!" << std::endl;
		closed = false;
		return;
	} 
	system("PAUSE");
}

Client::~Client()
{
	closesocket(server);
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

bool Client::Connect(const char* ip, const char* port)
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
	getaddrinfo(m_Ip, m_Port, &hints, &addrInfo);

	int error;
	std::cout << "Connecting..." << std::endl;
	for (ptr = addrInfo; ptr != NULL; ptr = ptr->ai_next) {

		// Init socket
		server = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (server == INVALID_SOCKET) {
			std::cout << "Failed to create socket: " << WSAGetLastError() << std::endl;
			WSACleanup();
			return false;
		}

		// Connect to server
		error = connect(server, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (error == SOCKET_ERROR) {
			closesocket(server);
			server = INVALID_SOCKET;
			continue; //try again
		}
		break;
	}

	freeaddrinfo(addrInfo);

	if (server == INVALID_SOCKET) {
		std::cout << "Failed to connect to server " << ip << ":" << port << std::endl;
		errorMsg = std::string("Failed to connect to server: ").append(ip).append(":").append(port).append("\n");
		//WSACleanup();
		return false;
	}
	connected = true;

	std::string msg = "";
	char tempmsg[DEFAULT_BUFLEN] = "";
	while (true)
	{
		memset(tempmsg, 0, DEFAULT_BUFLEN);

		if (server != 0)
		{
			int error = recv(server, tempmsg, DEFAULT_BUFLEN, 0);
			if (error != SOCKET_ERROR)
			{
					serverLog.append(tempmsg);
					UpdateScreen();
			}
			else
			{
				msg = "Server has stopped!";
				errorMsg.append(msg).append("\n");
				serverLog = "";
				connected = false;

				break;
			}
		}
	}
	return false;
}

void Client::Disconnect()
{
	int error = shutdown(server, SD_SEND);
	if (error == SOCKET_ERROR) {
		std::cout << "Failed to disconnect" << WSAGetLastError() << std::endl;
		closesocket(server);
		WSACleanup();
	}
	connected = false;
}

bool Client::Send(const char* message)
{
	int error = send(server, message, (int)strlen(message), 0);
	if (error == SOCKET_ERROR) {
		std::cout << "Failed to send: " << WSAGetLastError();
		closesocket(server);
		WSACleanup();
		return false;
	}
	//serverLog.append(message);
	//UpdateScreen();
	return true;
}

void Client::UpdateScreen()
{
	if (connected)
	{
		ClearConsole();
		std::cout << "Client: " << m_Ip << ":" << m_Port << std::endl;
		SetConsoleTitle(std::string("Client: ").append(m_Ip).append(":").append(m_Port).c_str());
		std::cout << serverLog.c_str() << std::endl;
		Sleep(100);
	}
}

void Client::ClearConsole() const
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