#include <iostream>
#include <clsrv_net.h>

class Server : public clsrv::net::ServerInterface<CustomMsgTypes>
{
public:
	Server(uint16_t nPort) : clsrv::net::ServerInterface<CustomMsgTypes>(nPort) {}

protected:
	virtual bool onClientConnect(std::shared_ptr<clsrv::net::Connection<CustomMsgTypes>> client)
	{
		clsrv::net::Message<CustomMsgTypes> msg;
		msg.header.m_nid = CustomMsgTypes::ServerAccept;
		client->send(msg);
		return true;
	}

	virtual void onClientDisconnect(std::shared_ptr<clsrv::net::Connection<CustomMsgTypes>> client)
	{
		cout << "Removing client [" << client->getID() << "]\n";
	}

	virtual void onMessage(std::shared_ptr<clsrv::net::Connection<CustomMsgTypes>> client, clsrv::net::Message<CustomMsgTypes>& msg)
	{
		switch (msg.header.m_nid)
		{
		case CustomMsgTypes::ServerPing:
		{
			std::chrono::system_clock::time_point timeThen;
			msg << timeThen;
			cout << "[" << client->getID() << "]: Server Ping\n";
			client->send(msg);
		}
		break;

		case CustomMsgTypes::MessageServer:
		{
			string hero;
			msg >> hero;
			cout << "[" << client->getID() << "]: ";
			cout << hero << endl;
		}
		break;

		case CustomMsgTypes::MessageAll:
		{
			cout << "[" << client->getID() << "]: Message All\n";
			clsrv::net::Message<CustomMsgTypes> msg;
			msg.header.m_nid = CustomMsgTypes::ServerMessage;
			msg << client->getID();
			messageAllClients(msg, client);
		}
		break;

		case CustomMsgTypes::UploadFileName:
		{
			string fileName;
			msg >> fileName;

			try
			{
				std::string out_path = "FilesDatabase/" + std::to_string(client->getID());
				if (!fs::exists(out_path)) 
				{
					std::filesystem::create_directory(out_path);
				}
				out_path += "/" + fileName;
				m_clientFiles[client->getID()] = fileName;
				clsrv::file::openWriteFile(out_path, std::ios_base::binary | std::ios_base::out);
			}
			catch (std::exception& ex)
			{
				std::cerr << ex.what() << std::endl;
			}
		}
		break;

		case CustomMsgTypes::UploadChunk:
		{
			handleFileUpload(client, msg);
		}
		break;

		case CustomMsgTypes::DownloadChunk:
		{
			handleFileDownloadAsync(client,msg);
		}
		break;
		}
	}
	
	void handleFileUpload(std::shared_ptr<clsrv::net::Connection<CustomMsgTypes>> client,
		clsrv::net::Message<CustomMsgTypes>& msg) 
	{
		std::string out_path = "FilesDatabase/" + std::to_string(client->getID()) + "/" + m_clientFiles[client->getID()];
		if (!fs::exists(out_path))
			cout << "File path does not exist" << endl;

		//std::unique_lock<std::mutex> mapLock(mutexForMap);
		//if (m_clientMutexes.find(out_path) == m_clientMutexes.end())
		//	m_clientMutexes[out_path] = std::make_unique<std::mutex>();

		//std::unique_lock<std::mutex> lock(*m_clientMutexes[out_path], std::defer_lock);
		//while (!lock.try_lock())
		//	std::this_thread::sleep_for(std::chrono::milliseconds(10));
		//lock.unlock();

		//scoped_lock<mutex> lock(*m_clientMutexes[out_path]);
		
		std::scoped_lock lock(m_fileMutex);
		{
			const std::vector<uint8_t>& fileData = std::move(msg.body);
			std::ofstream file(out_path, std::ios::binary | std::ios::app);

			while (!file.is_open())
			{
				file.open(out_path, std::ios_base::binary | std::ios_base::app);
			}

			if (file)
			{
				file.write(reinterpret_cast<const char*>(&fileData[0]), fileData.size()).flush();
				std::cout << "\r[" << client->getID() << "]: " << fileData.size() << " bytes written to " << out_path << std::flush;
				file.close();
			}
			else
			{
				cout << "Error Opening File" << endl;
			}
		}


		//std::scoped_lock lock(m_fileMutex);
		/*try 
		{
			m_targetFile.open(out_path, std::ios_base::binary | std::ios_base::app);
			while (!m_targetFile.is_open()) 
			{
				m_targetFile.open(out_path, std::ios_base::binary | std::ios_base::app);
			}
			std::vector<uint8_t> buffer(msg.size());
			buffer = std::move(msg.body);
			msg.header.m_nsize = msg.size();
			m_targetFile.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
			m_targetFile.close();
			std::cout << "\r[" << client->getID() << "]: " << buffer.size() << " bytes written to " << out_path << std::flush;
		}
		catch (std::exception& ex) 
		{
			std::cerr << ex.what() << std::endl;
		}*/
	}

	bool sendErrorMsg(std::shared_ptr<clsrv::net::Connection<CustomMsgTypes>> client,std::string file_path)
	{
		if (!std::filesystem::exists(file_path))
		{
			clsrv::net::Message<CustomMsgTypes> msg;
			msg.header.m_nid = CustomMsgTypes::ServerMessageToClient;
			std::string message = "The File You Requested Does not Exist";
			msg << message;
			messageClient(client, msg);
			return true;
		}
		return false;
	}

	void sendDownloadPath(std::shared_ptr<clsrv::net::Connection<CustomMsgTypes>> client, std::string fileName)
	{
		std::string downloadPath = std::to_string(client->getID());
		clsrv::net::Message<CustomMsgTypes> fileUploadMsg;
		fileUploadMsg.header.m_nid = CustomMsgTypes::DownloadFilePath;
		fileUploadMsg << downloadPath;
		messageClient(client,fileUploadMsg);
	}

	void handleFileDownloadAsync(std::shared_ptr<clsrv::net::Connection<CustomMsgTypes>> client,
		clsrv::net::Message<CustomMsgTypes> msg)
		{
		std::string fileName;
		msg >> fileName;
		std::string filePath = "FilesDatabase/" + std::to_string(client->getID()) + "/" + fileName;
		std::cout << filePath << std::endl;
		if (sendErrorMsg(client,filePath)) return;
		sendDownloadPath(client, fileName);
		std::thread downloadThread([this, client, filePath]() {
			try
			{
				std::ifstream sourceFile = clsrv::file::openReadFile(filePath, std::ios_base::binary | std::ios_base::ate);
				size_t fileSize = clsrv::file::getFileSize(sourceFile);
				std::cout << "File name: " << clsrv::file::extractFileName(filePath) << std::endl;
 				std::cout << "File size: " << fileSize << " bytes" << std::endl;
				const size_t BUFFER_SIZE = clsrv::file::calculateChunkSize(fileSize);
				cout << "Sending File in Chunks of " << BUFFER_SIZE << " bytes" << endl;
				while (!sourceFile.eof() && fileSize > 0) {
					std::vector<uint8_t> buffer = clsrv::file::readFile(sourceFile, fileSize, BUFFER_SIZE);
					auto bytesToRead = clsrv::file::getBytesToRead(fileSize, BUFFER_SIZE);
					clsrv::net::Message<CustomMsgTypes> fileUploadMsg;
					fileUploadMsg.header.m_nid = CustomMsgTypes::DownloadChunk;
					fileUploadMsg.header.m_nsize = bytesToRead;
					fileUploadMsg.body.assign(buffer.begin(), buffer.begin() + bytesToRead);
					messageClient(client, fileUploadMsg);
					fileSize -= bytesToRead;
					std::cout << "\rBytes remaining: " << fileSize << std::flush;
				}
				cout << endl;
				clsrv::file::fileCompleted(sourceFile);
				sourceFile.close();
			}
			catch (std::exception& ex)
			{
				std::cerr << ex.what() << std::endl;
			}
		});
		downloadThread.detach();
	}

	public:
		void messageToClient()
		{
			clsrv::net::Message<CustomMsgTypes> msg;
			msg.header.m_nid = CustomMsgTypes::ServerMessageToClient;
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
			clsrv::net::Message<CustomMsgTypes> msg;
			msg.header.m_nid = CustomMsgTypes::ServerMessageToClient;
			std::string messg;
			std::cout << "Enter message for all clients: ";
			std::getline(std::cin, messg);
			msg << messg;
			messageAllClients(msg);
		}
	private:
		std::mutex m_fileMutex;
		std::unordered_map<int, std::string> m_clientFiles;
		//std::unordered_map<string, unique_ptr<std::mutex>> m_clientMutexes;
		//std::mutex mutexForMap;
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
