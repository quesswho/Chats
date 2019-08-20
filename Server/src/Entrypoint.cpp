#include "network/Server.h"
#include <iostream>
#if defined(__WIN32__)
#include <thread>
#else
#include <pthread.h>
#endif
#include <string.h>

const char* LOCALHOST = "127.0.0.1";
const char* DEFAULT_PORT = "79977";

const int CLIENT_LIMIT = 5;

#ifdef __WIN32__
#define ClearConsole() system("cls")
#else
#define ClearConsole() system("clear");
#endif


const char* ip_address;
const char* port;

Server* server;

void ChooseIpAndPort();
void _SetTitle(const char* title);

struct startargs {
	int limit;
};

#ifndef __WIN32__
void *Start(void* args)
{
	server->Start(args);
	return NULL;
}
#endif

int main() {

	ChooseIpAndPort();

	// Initialize server
	server = new Server(ip_address, port);

#ifdef __WIN32__
	std::thread listener(&Server::Start, server, CLIENT_LIMIT); //listen for joining users
#else
	pthread_t listener;
	startargs args;
	args.limit = 5;
	
	pthread_create(&listener, NULL, &Server, (void*) &args);
#endif
	while (!server->closed)
	{
		
	}
#ifdef __WIN32__
	listener.detach();
#else
	pthread_cancel(listener);
	#endif
}

void ChooseIpAndPort()
{
	char* input1 = new char[14]; // 255.255.255.255 = 15 chars
	char* input2 = new char[6];
	char* inputyn = new char[0];
	while (true)
	{
		_SetTitle("Server");
		ClearConsole();
		std::cout << "Please enter desired ip address." << std::endl;
		std::cin >> input1;

		memset(inputyn, 0, 1);
		if (!NetworkingUtils::isValidIpv4(input1))
		{
			while (true)
			{
				ClearConsole();
				std::cout << input1 << " is not a valid ip address! Would you like to to use " << LOCALHOST << "? y/n" << std::endl;
				std::cin >> inputyn;
				if (inputyn[0] == 'y') {
					ip_address = LOCALHOST;
					break;
				}
				else if (inputyn[0] == 'n')
					break;
			}
		}
		if (inputyn[0] == 'n') {
			continue;
		}
		else if (inputyn[0] == 'y')
			break;
		else if (ip_address != LOCALHOST) {
			ip_address = input1;
			break;
		}
	}

	while (true)
	{
		ClearConsole();
		std::cout << "Please enter desired port." << std::endl;
		std::cin >> input2;

		memset(inputyn, 0, 1);
		if (!NetworkingUtils::isValidPort(input2))
		{
			while (true)
			{
				ClearConsole();
				std::cout << input2 << " is not a valid port! Would you like to to use " << DEFAULT_PORT << "? y/n" << std::endl;
				std::cin >> inputyn;
				if (inputyn[0] == 'y') {
					port = DEFAULT_PORT;
					break;
				}
				else if (inputyn[0] == 'n')
					break;
			}
		}
		if (inputyn[0] == 'n') {
			continue;
		}
		else if (inputyn[0] == 'y')
			break;
		else if (port != DEFAULT_PORT) {
			port = input2;
			break;
		}
	}
}

inline void _SetTitle(const char* title)
{
	#ifdef __WIN32__
	SetConsoleTitle(title);
	#else
	std::cout << "\033]0;" << title << "\007";
	#endif
}