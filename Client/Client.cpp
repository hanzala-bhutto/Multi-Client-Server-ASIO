
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

	void sendFileName(std::string fileName)
	{
		clsrv::net::message<CustomMsgTypes> fileUploadMsg;
		fileUploadMsg.header.id = CustomMsgTypes::UploadFileName;
		fileUploadMsg << fileName;
		send(fileUploadMsg);
	}

	void uploadFile()
	{
		std::string in_path;
		in_path = clsrv::file::inputPath(in_path);
		if (!clsrv::file::fileExists(in_path)) return;
		std::string fileName = clsrv::file::extractFileName(in_path);
		std::cout << "File name: " << fileName << std::endl;
		std::ifstream sourceFile = clsrv::file::openReadFile(in_path, std::ios_base::binary | std::ios_base::ate);
		size_t fileSize = clsrv::file::getFileSize(sourceFile);
		std::cout << "File size: " << fileSize << " bytes" << std::endl;
		sendFileName(fileName);
		const size_t BUFFER_SIZE = 1024 * 1024 * 10;
		bool isFirstChunk = true;
		while (!sourceFile.eof() && fileSize > 0) {
			std::vector<uint8_t> buffer = clsrv::file::readFile(sourceFile, fileSize, BUFFER_SIZE);
			clsrv::net::message<CustomMsgTypes> fileUploadMsg;
			fileUploadMsg.header.id = CustomMsgTypes::UploadChunk;
			fileUploadMsg.header.size = clsrv::file::getBytesToRead(fileSize,BUFFER_SIZE);
			fileUploadMsg.body.assign(buffer.begin(), buffer.begin() + clsrv::file::getBytesToRead(fileSize, BUFFER_SIZE));
			send(fileUploadMsg);
			fileSize -= clsrv::file::getBytesToRead(fileSize, BUFFER_SIZE);
			isFirstChunk = false;
			std::cout << "Bytes remaining: " << fileSize << std::endl;
		}
		clsrv::file::fileCompleted(sourceFile);
		sourceFile.close();
	}

	void openFile()
	{
		//std::string in_path = "C:/Users/HBhutto/source/repos/Client_Server/Multi-Client-Server-ASIO/x64/Debug/Hero.txt";
		//std::string in_path = "C:/Users/HBhutto/source/repos/Client_Server/Versioning/Client-Server-App.zip";
		std::string in_path = "C:/Users/HBhutto/source/repos/Client_Server/Versioning/Colttaine.zip";

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

		uploadFile(sourceFile, fileName);
	}

	void uploadFile(std::ifstream& sourceFile, std::string fileName)
	{

		if (!sourceFile.is_open()) {
			std::cerr << "File is not open!" << std::endl;
			return;
		}

		sourceFile.seekg(0, sourceFile.end);
		auto fileSize = sourceFile.tellg();
		sourceFile.seekg(0, sourceFile.beg);

		std::cout << "File size: " << fileSize << " bytes" << std::endl;

		const size_t BUFFER_SIZE = 1024 * 1024 * 10;

		bool isFirstChunk = true;
		while (!sourceFile.eof() && fileSize > 0) {
			size_t bytesToRead = std::min(static_cast<size_t>(fileSize), BUFFER_SIZE);
			std::vector<uint8_t> buffer(bytesToRead);
			sourceFile.read(reinterpret_cast<char*>(buffer.data()), bytesToRead);
			clsrv::net::message<CustomMsgTypes> fileUploadMsg;
			fileUploadMsg.header.id = CustomMsgTypes::UploadChunk;
			fileUploadMsg.header.size = bytesToRead;
			fileUploadMsg.body.assign(buffer.begin(), buffer.begin() + bytesToRead);
			send(fileUploadMsg);
			fileSize -= bytesToRead;
			isFirstChunk = false;
			std::cout << "Bytes remaining: " << fileSize << std::endl;
		}

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
		clsrv::net::message<CustomMsgTypes> FileDownloadMsg;
		FileDownloadMsg.header.id = CustomMsgTypes::DownloadChunk;
		std::string hero;
		std::cout << "Enter File Name to Download From Server: ";
		std::getline(std::cin, hero);
		FileDownloadMsg << hero;
		send(FileDownloadMsg);
		m_fileName = hero;
	}

	void handleDownloadFile(clsrv::net::message<CustomMsgTypes>& msg)
	{
		try
		{
			std::ofstream downloadFile(m_downloadPath, std::ios_base::binary | std::ios_base::app);
			while (!downloadFile.is_open()) {
				downloadFile.open(m_downloadPath, std::ios_base::binary | std::ios_base::app);
			}
			std::vector<uint8_t> buffer(msg.size());
			buffer = std::move(msg.body);
			msg.header.size = msg.size();
			downloadFile.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
			downloadFile.close();
			std::cout << "[Server]: ";
			std::cout << buffer.size() << " bytes written to " << m_downloadPath << std::endl;
		}
		catch (std::exception& ex) 
		{
			std::cerr << ex.what() << std::endl;
		}
	}

	void setDownloadPath(std::string path)
	{
		m_downloadPath = path;
		std::cout << m_downloadPath << std::endl;
	}

	std::string getFileName()
	{
		return m_fileName;
	}

	private:
		std::string m_fileName;
		std::string m_downloadPath;
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
		if (key[3] && !old_key[3]) c.uploadFile();
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

				case CustomMsgTypes::DownloadFilePath:
				{
					std::string downloadPath;
					msg >> downloadPath;
					try
					{
						if (!std::filesystem::exists(downloadPath)) {
							std::filesystem::create_directory(downloadPath);
						}
						downloadPath += "/" + c.getFileName();
						clsrv::file::openWriteFile(downloadPath, std::ios_base::binary | std::ios_base::out);
						c.setDownloadPath(downloadPath);
					}
					catch (std::exception& ex)
					{
						std::cerr << ex.what() << std::endl;
					}
				}
				break;

				case CustomMsgTypes::DownloadChunk:
				{
					c.handleDownloadFile(msg);
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