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
};

class Server : public clsrv::net::ServerInterface<CustomMsgTypes>
{
public:
	Server(uint16_t nPort) : clsrv::net::ServerInterface<CustomMsgTypes>(nPort) {}

protected:
	virtual bool onClientConnect(std::shared_ptr<clsrv::net::connection<CustomMsgTypes>> client)
	{
		clsrv::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->send(msg);
		return true;
	}

	virtual void onClientDisconnect(std::shared_ptr<clsrv::net::connection<CustomMsgTypes>> client)
	{
		std::cout << "Removing client [" << client->getID() << "]\n";
	}

	virtual void onMessage(std::shared_ptr<clsrv::net::connection<CustomMsgTypes>> client, clsrv::net::message<CustomMsgTypes>& msg)
	{
		switch (msg.header.id)
		{
		case CustomMsgTypes::ServerPing:
		{
			std::chrono::system_clock::time_point timeThen;
			msg << timeThen;
			std::cout << "[" << client->getID() << "]: Server Ping\n";
			// Simply bounce message back to client
			client->send(msg);
		}
		break;

		case CustomMsgTypes::MessageServer:
		{
			std::string hero;
			msg >> hero;

			std::cout << "[" << client->getID() << "]: ";
			std::cout << hero << std :: endl;
		}
		break;

		case CustomMsgTypes::MessageAll:
		{
			std::cout << "[" << client->getID() << "]: Message All\n";
			clsrv::net::message<CustomMsgTypes> msg;
			msg.header.id = CustomMsgTypes::ServerMessage;
			msg << client->getID();
			messageAllClients(msg, client);
		}
		break;
		}
	}
public:
	void messageToClient()
	{
		clsrv::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerMessageToClient;

		int id;
		std::string messg;
		std::cout << "Enter id of client: ";
		std::cin >> id;

		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		std::cout << "Enter message for client: ";
		std::getline(std::cin, messg);

		msg << messg;

		for (auto& client : m_deqConnections)
		{
			if (client && client->isConnected())
			{
				if (client->getID() == static_cast<uint32_t>(id))
				{
					messageClient(client, msg);
					break;
				}
			}
		}
	}

	void messageToAllClients()
	{
		clsrv::net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerMessageToClient;

		std::string messg;

		std::cout << "Enter message for all clients: ";
		std::getline(std::cin, messg);

		msg << messg;

		messageAllClients(msg);
	}

};

int main()
{

	Server s(6000);
	s.start();

	//while (1)
	//{
	//	s.update();
	//}

	bool key[3] = { false, false, false };
	bool old_key[3] = { false, false, false };

	HWND name;
	name = GetForegroundWindow();

	bool bQuit = false;
	while (!bQuit)
	{
		HWND hwndForeground = GetForegroundWindow();
		if (hwndForeground == name) {
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
		}
		if (key[0] && !old_key[0]) s.messageToClient();
		if (key[1] && !old_key[1]) s.messageToAllClients();
		if (key[2] && !old_key[2]) bQuit = true;

		for (int i = 0; i < 3; i++) old_key[i] = key[i];

		s.update();
	}

	return 0;
}
