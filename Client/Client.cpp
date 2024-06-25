
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
	SendFile,
	ReceiveFile
};

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

	void handler(
		const asio::error_code& error, // Result of operation.
		std::size_t bytes_transferred // Number of bytes read.
	)
	{
		std::cout << bytes_transferred << std::endl;
	}

	void grabData(asio::stream_file &file)
	{

		clsrv::net::message<CustomMsgTypes> msg;



		msg.header.id = CustomMsgTypes::SendFile;
		msg.header.size = static_cast<uint32_t>(file.size());

		size_t            bufferSize = 1024;
		std::vector<char> buffer(bufferSize);


		file.async_read_some
		(asio::buffer(buffer, bufferSize),
			[this,&msg,&buffer, &file](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					msg.body.insert(msg.body.end(), buffer.begin(), buffer.end());
					send(msg);

					for(auto c: buffer)
					{
						std::cout << c;
					}

					grabData(file);

				}
				else
				{
					std::cout << "[" << "] Write Header Fail.\n";
				}
			}
		);
	}

	void sendFile()
	{
		auto fileName = "Hero.txt";
		try
		{
			asio::stream_file file(m_context, fileName, asio::stream_file::flags::read_only);
			std::cout << file.size() << std::endl;
			m_context.run();
			grabData(file);
			
		}
		catch (std::exception& e)
		{
			std::cerr << e.what() << std::endl;
		}
	}

};

int main()
{
	CustomClient c;
	c.connect("127.0.0.1", 6000);

	bool key[5] = { false, false, false, false , false};
	bool old_key[5] = { false, false, false, false, false };

	HWND hwndForeground;
	hwndForeground = GetForegroundWindow();

	bool bQuit = false;
	while (!bQuit)
	{
		HWND currentForeground = GetForegroundWindow();

		if (hwndForeground == currentForeground)
		{
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
			key[3] = GetAsyncKeyState('4') & 0x8000;
			key[4] = GetAsyncKeyState('5') & 0x8000;
		}

		if (key[0] && !old_key[0]) c.pingServer();
		if (key[1] && !old_key[1]) c.messageServer();
		if (key[2] && !old_key[2]) c.messageAll();
		if (key[3] && !old_key[3]) c.sendFile();
		if (key[4] && !old_key[4]) bQuit = true;

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