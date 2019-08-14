#pragma once
#include <iostream>

#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

class Client {
	const char* m_Ip;
	const char* m_Port;

	SOCKET server;
public:
	bool closed;
	bool connected;

	std::string serverLog;
	std::string errorMsg;
public:
	Client();
	~Client();

	bool Connect(const char* ip, const char* port);
	void Disconnect();

	bool Send(const char* message);

	void ClearConsole() const;
private:
	bool Init();

	void UpdateScreen();
};

namespace NetworkingUtils {

	static bool isValidIpv4(const char* ip)
	{
		if (ip == nullptr)
			return false;

		if (sizeof(ip) > 15)
			return false;
		int period = 0;
		while (*ip != '\0')
		{
			if ((*ip < '0') || (*ip > '9')) //check if not numeric
				if (*ip == '.')
					period++;
				else
					return false;
			*ip++;
		}
		if (period == 3)
			return true;
		return false;
	}

	static bool isValidPort(const char* port)
	{

		if (port == nullptr)
			return false;

		while (*port != '\0')
		{
			if ((*port < '0') || (*port > '9')) //check if not numeric
				return false;
			*port++;
		}
		return true;
	}
}