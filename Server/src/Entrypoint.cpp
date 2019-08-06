#include <iostream>
#include <thread>
#include "network/Server.h"

#define LOCALHOST "127.0.0.1"
#define DEFAULT_PORT "79977"

#define CLIENT_LIMIT

const char* ip_address;
const char* port;

int main() {
	SetConsoleTitle("Server");

	Server* server;

	char* input1 = new char[14]; // 255.255.255.255 = 15 chars
	char* input2 = new char[6];
	char* inputyn = new char[0];
	while (true)
	{
		system("cls"); //clear console
		std::cout << "Please enter desired ip address." << std::endl;
		std::cin >> input1;

		memset(inputyn, 0, 1);
		if (!NetworkingUtils::isValidIpv4(input1))
		{
			while (true)
			{
				system("cls"); //clear console
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
		else if(ip_address != LOCALHOST) {
			ip_address = input1;
			break;
		}
	}

	while (true)
	{
		system("cls"); //clear console
		std::cout << "Please enter desired port." << std::endl;
		std::cin >> input2;

		memset(inputyn, 0, 1);
		if (!NetworkingUtils::isValidPort(input2))
		{
			while (true)
			{
				system("cls"); //clear console
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
		else if(port != DEFAULT_PORT) {
			port = input2;
			break;
		}
	}

	// Initialize server
	server = new Server(ip_address, port);
	std::thread listener(&Server::Start, server, 5);

	while (!server->closed)
	{
		system("cls");
		std::cout << "Server: " << ip_address << ":" << port << std::endl;
		std::cout << server->serverLog;
		SetConsoleTitle(std::string("Server: ").append(ip_address).append(":").append(port).c_str());
		Sleep(1000);
	}
}