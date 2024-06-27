#include <iostream>
#include <clsrv_net.h>

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

		case CustomMsgTypes::UploadFile:
		{
			handleFileUpload(client, msg, true);
		}
		break;

		case CustomMsgTypes::UploadMore:
		{
			handleFileUpload(client, msg, false);
		}
		break;

		//case CustomMsgTypes::UploadLast:
		//{
		//	handleFileUpload(client, msg);
		//}
		//break;
		}


	}
	
	void handleFileUpload(std::shared_ptr<clsrv::net::connection<CustomMsgTypes>> client,
		clsrv::net::message<CustomMsgTypes>& msg, bool firstChunk) {
		std::scoped_lock lock(m_fileMutex);
		try {
			std::string out_path = "FilesDatabase/" + std::to_string(client->getID());

			if (!std::filesystem::exists(out_path)) {
				std::filesystem::create_directory(out_path);
			}

			//std::string file_path = out_path + "/Hero.txt";
			std::string file_path = out_path + "/Client-Server-App.zip";

			std::ios_base::openmode file_mode = std::ios_base::binary;


		// Determine file open mode based on file existence
			if (firstChunk)
			{
				file_mode |= std::ios_base::out;
			}
			else
			{
				file_mode |= std::ios_base::app;
			}

			m_targetFile.open(file_path, file_mode);
			while (!m_targetFile.is_open()) {
				m_targetFile.open(file_path, file_mode);
			}

			std::vector<uint8_t> buffer(msg.size());
			buffer = std::move(msg.body);
			msg.header.size = msg.size();

			m_targetFile.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
			m_targetFile.close();

			std::cout << "[" << client->getID() << "]: ";
			std::cout << buffer.size() << " bytes written to " << file_path << std::endl;
		}
		catch (std::exception& ex) {
			std::cerr << ex.what() << std::endl;
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

	private:
		std::mutex m_fileMutex;
		std::ofstream m_targetFile;

};

int main()
{

	Server s(6000);
	s.start();

	bool key[3] = { false, false, false };
	bool old_key[3] = { false, false, false };

	HWND hwndForeground;
	hwndForeground = GetForegroundWindow();

	bool bQuit = false;
	while (!bQuit)
	{
		HWND currentForeground = GetForegroundWindow();
		if (hwndForeground == currentForeground) {
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
