#pragma once
#include <cstring>
#include <thread>

#ifdef __WIN32__
#include <WinSock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
struct User {
	#ifdef __WIN32__
	SOCKET socket;
	#else
	int socket;
	#endif
	int id;
};

class Server {
	const char* m_Ip;
	const char* m_Port;
	
	#ifdef __WIND32__
	SOCKET listeningSocket;
	#else
	int listeningSocket;
	#endif
	
	User* clients;
	std::thread *threads;

	int online;
	int clientLimit;
public:
	bool closed;
	std::string serverLog;
public:
	Server(const char* ip, const char* port);
	~Server();

	#ifdef __WIN32__
	void Start(int clientlimit);
	#else
	void *Start(void *arg);
	#endif
	
private:
	bool Init();
	void ClientSession(User client);
	bool SendAll(const char* message);
	void Print();
	void ClearConsole();
	void SetTitle(const char* title);
};

namespace NetworkingUtils {

	inline bool isValidIpv4(const char* ip)
	{
		if (ip == nullptr)
			return false;

		if (sizeof(ip) > 15)
			return false;
		int period = 0;
		while (*ip != '\0')
		{
			if ((*ip < '0') || (*ip > '9')) //check if not numeric
			{
				if (*ip == '.')
					period++;
				else
					return false;

			}
		ip++;
		}
		if(period == 3)
			return true;
		return false;
	}

	inline bool isValidPort(const char* port)
	{

		if (port == nullptr)
			return false;

		while (*port != '\0')
		{
			if ((*port < '0') || (*port > '9')) //check if not numeric
				return false;
			port++;
		}
		return true;
	}
}


