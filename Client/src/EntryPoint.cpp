#include <iostream>
#include <string>
#include <thread>
#include "network/Client.h"

#define LOCALHOST "127.0.0.1"
#define DEFAULT_PORT "79977"

std::string ip_address;
std::string port;

int main() {
	Client* client = new Client();
	char* input1 = new char[14]; // 255.255.255.255 = 15 chars
	char* input2 = new char[6];
	char* inputyn = new char[0];
	while (true)
	{
		SetConsoleTitle("Client");
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
		else if (ip_address != LOCALHOST) {
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
		else if (port != DEFAULT_PORT) {
			port = input2;
			break;
		}
	}

	std::thread session(&Client::StartSession, client, ip_address.c_str(), port.c_str());
	std::string message;
	while (!client->closed)
	{
		system("cls");
		std::cout << "Client: " << ip_address.c_str() << ":" << port.c_str() << std::endl;
		SetConsoleTitle(std::string("Client: ").append(ip_address).append(":").append(port).c_str());
		std::getline(std::cin, message);
		client->Send(message.c_str());
		std::cout << client->serverLog << std::endl;
	}
}