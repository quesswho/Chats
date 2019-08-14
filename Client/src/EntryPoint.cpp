#include <iostream>
#include <string>
#include <thread>
#include "network/Client.h"

#define LOCALHOST "127.0.0.1"
#define DEFAULT_PORT "79977"

std::string ip_address;
std::string port;

Client* client;

void ChooseIpAndPort();
void Session();

int main() {

	client = new Client();
	
	ChooseIpAndPort();
	std::thread session(Session);
	client->ClearConsole();

	while (!client->closed)
	{
		while (client->connected && !client->closed)
		{
			std::string message;
			std::getline(std::cin, message);
			if (client->connected)
			{
				if (!strcmp(message.c_str(), ""))
					continue;
				client->Send(message.c_str());
			}
		}
	}

	client->~Client();
	session.detach();
	//input.detach();
	return 0;
}

void Session()
{
	while(!client->closed)
		if (!client->Connect(ip_address.c_str(), port.c_str())) //listening loop
		{
			ChooseIpAndPort();
		}
}

void ChooseIpAndPort()
{
	char* input1 = new char[14]; // 255.255.255.255 = 15 chars
	char* input2 = new char[6];
	char* inputyn = new char[0];
	while (true)
	{
		SetConsoleTitle("Client");
		client->ClearConsole();
		std::cout << client->errorMsg;
		std::cout << "Please enter desired ip address." << std::endl;
		std::cin >> input1;

		memset(inputyn, 0, 1);
		if (!NetworkingUtils::isValidIpv4(input1))
		{
			while (true)
			{
				client->ClearConsole();
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
		client->ClearConsole();
		std::cout << "Please enter desired port." << std::endl;
		std::cin >> input2;

		memset(inputyn, 0, 1);
		if (!NetworkingUtils::isValidPort(input2))
		{
			while (true)
			{
				client->ClearConsole();
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