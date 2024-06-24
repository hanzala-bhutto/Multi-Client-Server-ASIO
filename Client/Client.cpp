
#include <iostream>
#include <clsrv_net.h>

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageServer,
	MessageAll,
	ServerMessage,
	ServerMessageToClient,
	ServerMessageToAllClients
};

void convertStringToChar()
{

}

class CustomClient : public clsrv::net::ClientInterface<CustomMsgTypes>
{
public:
	void pingServer()
	{
		clsrv::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerPing;
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
		msg << timeNow;
		//std :: cout << msg << std::endl;
		send(msg);
	}

	void messageServer()
	{
		
		clsrv::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::MessageServer;
		std::string hero;
		std::cout << "Enter message to send to server: ";
		std::getline(std::cin, hero);
		msg << hero;
		send(msg);
	}

	void messageAll()
	{
		clsrv::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::MessageAll;
		send(msg);
	}
};

int main()
{
	CustomClient c;
	c.connect("127.0.0.1", 6000);

	bool key[4] = { false, false, false, false };
	bool old_key[4] = { false, false, false, false };

	HWND name;
	name = GetForegroundWindow();

	bool bQuit = false;
	while (!bQuit)
	{
		HWND hwndForeground = GetForegroundWindow();

		if (hwndForeground == name)
		{
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
			key[3] = GetAsyncKeyState('4') & 0x8000;
		}

		if (key[0] && !old_key[0]) c.pingServer();
		if (key[1] && !old_key[1]) c.messageServer();
		if (key[2] && !old_key[2]) c.messageAll();
		if (key[3] && !old_key[3]) bQuit = true;

		for (int i = 0; i < 3; i++) old_key[i] = key[i];

		if (c.isConnected())
		{
			if (!c.incoming().empty())
			{
				auto msg = c.incoming().pop_front().msg;

				switch (msg.header.id)
				{
				case CustomMsgTypes::ServerAccept:
				{
					std::cout << "Server Accepted Connection\n";
				}
				break;


				case CustomMsgTypes::ServerPing:
				{
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					std::cout << "Ping: " << std::chrono::duration<double>(timeNow - timeThen).count() << "\n";
				}
				break;

				case CustomMsgTypes::ServerMessage:
				{
					uint32_t clientID;
					msg >> clientID;
					std::cout << "Hello from [" << clientID << "]\n";
				}
				break;

				case CustomMsgTypes::ServerMessageToClient:
				{
					std::string message;
					msg >> message;
					std::cout << "[SERVER said]: " << message << "\n";
				}
				break;
				}
			}
		}
		else
		{
			std::cout << "Server Down\n";
			bQuit = true;
		}

	}

	return 0;
}