
#include <iostream>
#include <clsrv_net.h>

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

	void openFile()
	{
		//std::string in_path = "C:/Users/HBhutto/source/repos/Client_Server/Multi-Client-Server-ASIO/x64/Debug/Hero.txt";
		std::string in_path = "C:/Users/HBhutto/source/repos/Client_Server/Versioning/Client-Server-App.zip";
	/*	std::cout << "Enter path of file : " << std::endl;
		std::getline(std::cin, in_path);*/

		if (!fs::exists(in_path)) {
			throw "File path does not exist.";
		}

		// Extract the file name
		std::string fileName = fs::path(in_path).filename().string();
		std::cout << "File name: " << fileName << std::endl;

		std::ifstream sourceFile;
		sourceFile.open(in_path, std::ios_base::binary | std::ios_base::ate);
		if (sourceFile.fail())
			throw std::fstream::failure("Failed while opening file " + in_path);

		//std::cout << "Hero" << std::endl;

		uploadFile(sourceFile,fileName);
	}

	void uploadFile(std::ifstream &sourceFile, std::string fileName)
	{

		if (!sourceFile.is_open()) {
			std::cerr << "File is not open!" << std::endl;
			return;
		}

		sourceFile.seekg(0, sourceFile.end);
		auto fileSize = sourceFile.tellg();
		sourceFile.seekg(0, sourceFile.beg);

		std::cout << "File size: " << fileSize << " bytes" << std::endl;

		const size_t BUFFER_SIZE = 1024 * 1024;
		//std::vector<uint8_t> buffer(BUFFER_SIZE);
		std::vector<uint8_t> buffer(BUFFER_SIZE);

		sourceFile.read(reinterpret_cast<char*>(buffer.data()), BUFFER_SIZE);

		clsrv::net::message<CustomMsgTypes> fileUploadMsg;
		fileUploadMsg.header.id = CustomMsgTypes::UploadFile;
		fileUploadMsg.header.size = buffer.size();
		fileUploadMsg.body = std::move(buffer);
		send(fileUploadMsg);
		fileUploadMsg.body.clear();

		while (!sourceFile.eof()) 
		{
			std::cout << "Remaining bytes: " << fileSize - sourceFile.tellg() << std::endl;
			bool checkForData = fileSize - sourceFile.tellg() < BUFFER_SIZE;
			std::cout << checkForData << std::endl;
			std::vector<uint8_t> buffer(BUFFER_SIZE);
			sourceFile.read(reinterpret_cast<char*>(buffer.data()), BUFFER_SIZE);
			clsrv::net::message<CustomMsgTypes> fileUploadMsg;
			fileUploadMsg.header.id = CustomMsgTypes::UploadMore;
			fileUploadMsg.header.size = checkForData ? fileSize - sourceFile.tellg() : buffer.size();
			//fileUploadMsg.header.size = buffer.size();
			fileUploadMsg.body = std::move(buffer);
			send(fileUploadMsg);
			fileUploadMsg.body.clear();
		}

		std::cout << sourceFile.tellg() << std::endl;

		if (sourceFile.eof()) {
			std::cout << "File read successfully" << std::endl;
		}
		else if (sourceFile.fail()) {
			std::cerr << "Failed while reading file" << std::endl;
		}

		sourceFile.close();

	}

	void downloadFile()
	{

	}

};

int main()
{
	CustomClient c;
	c.connect("127.0.0.1", 6000);

	bool key[6] = { false, false, false, false , false, false };
	bool old_key[6] = { false, false, false, false, false, false };

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
			key[5] = GetAsyncKeyState('6') & 0x8000;
		}

		if (key[0] && !old_key[0]) c.pingServer();
		if (key[1] && !old_key[1]) c.messageServer();
		if (key[2] && !old_key[2]) c.messageAll();
		if (key[3] && !old_key[3]) c.openFile();
		if (key[4] && !old_key[4]) c.downloadFile();
		if (key[5] && !old_key[5]) bQuit = true;

		for (int i = 0; i < 6; i++) old_key[i] = key[i];

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