
#include <iostream>
#include <clsrv_net.h>

class CustomClient : public clsrv::net::ClientInterface<CustomMsgTypes>
{
public:
	void pingServer()
	{
		clsrv::net::Message<CustomMsgTypes> msg;
		msg.header.m_nid = CustomMsgTypes::ServerPing;
		chrono::system_clock::time_point timeNow = chrono::system_clock::now();
		msg << timeNow;
		send(msg);
	}

	void messageServer()
	{
		clsrv::net::Message<CustomMsgTypes> msg;
		msg.header.m_nid = CustomMsgTypes::MessageServer;
		string message;
		cout << "Enter message to send to server: ";
		getline(cin, message);
		msg << message;
		send(msg);
	}

	void publish()
	{
		clsrv::net::Message<CustomMsgTypes> msg;
		msg.header.m_nid = CustomMsgTypes::Publish;
		send(msg);
	}

	void sendEndFileMsg()
	{
		clsrv::net::Message<CustomMsgTypes> fileCloseMsg;
		fileCloseMsg.header.m_nid = CustomMsgTypes::FileEnd;
		fileCloseMsg << "File Successfully Sent";
		send(fileCloseMsg);
	}

	void sendFileInfo(std::string fileName)
	{
		clsrv::net::Message<CustomMsgTypes> fileUploadMsg;
		fileUploadMsg.header.m_nid = CustomMsgTypes::UploadFileInfo;
		fileUploadMsg << fileName;
		send(fileUploadMsg);
	}

	void uploadFile()
	{
		string in_path;
		in_path = clsrv::file::inputPath(in_path);
		if (!clsrv::file::fileExists(in_path)) return;
		string fileName = clsrv::file::extractFileName(in_path);
		cout << "File name: " << fileName << endl;
		ifstream sourceFile = clsrv::file::openReadFile(in_path, ios_base::binary | ios_base::ate);
		size_t fileSize = clsrv::file::getFileSize(sourceFile);
		cout << "File size: " << fileSize << " bytes" << endl;
		sendFileInfo(fileName);
		const size_t BUFFER_SIZE = clsrv::file::calculateChunkSize(fileSize);
		cout << "Sending File in Chunks of " << clsrv::file::getBytesToRead(fileSize, BUFFER_SIZE) << " bytes" << endl;
		while (!sourceFile.eof() && fileSize > 0) {
			vector<uint8_t> buffer = clsrv::file::readFile(sourceFile, fileSize, BUFFER_SIZE);
			auto bytesToRead = clsrv::file::getBytesToRead(fileSize, BUFFER_SIZE);
			clsrv::net::Message<CustomMsgTypes> fileUploadMsg;
			fileUploadMsg.header.m_nid = CustomMsgTypes::UploadChunk;
			fileUploadMsg.header.m_nsize = bytesToRead;
			fileUploadMsg.body.assign(buffer.begin(), buffer.begin() + bytesToRead);
			send(fileUploadMsg);
			fileSize -= bytesToRead;
			std::cout << "\rBytes remaining: " << fileSize << flush;
		}
		cout << "\rBytes remaining: " << fileSize << "          " << endl;
		clsrv::file::fileCompleted(sourceFile);
		sendEndFileMsg();
		sourceFile.close();
	}

	void downloadFile()
	{
		clsrv::net::Message<CustomMsgTypes> FileDownloadMsg;
		FileDownloadMsg.header.m_nid = CustomMsgTypes::DownloadChunk;
		string hero;
		cout << "Enter File Name to Download From Server: ";
		getline(std::cin, hero);
		FileDownloadMsg << hero;
		send(FileDownloadMsg);
		m_fileName = hero;
	}

	void handleFileDownload(clsrv::net::Message<CustomMsgTypes>& msg)
	{
		ofstream file(m_downloadPath, ios::binary | ios::app);
		while (!file.is_open())
			file.open(m_downloadPath, ios::binary | ios::app);
		const std::vector<uint8_t>& fileData = std::move(msg.body);
		if (file)
		{
			file.write(reinterpret_cast<const char*>(&fileData[0]), fileData.size()).flush();
			cout << "\r[Server]: " << fileData.size() << " bytes written to " << m_downloadPath << "          " << flush;
			file.close();
		}
	}

	void setDownloadPath(std::string path)
	{
		m_downloadPath = path;
	}

	std::string getFileName()
	{
		return m_fileName;
	}

	private:
		string m_fileName;
		string m_downloadPath;
};

int main()
{
	CustomClient client;
	client.connect("127.0.0.1", 6000);

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

		if (key[0] && !old_key[0]) client.pingServer();
		if (key[1] && !old_key[1]) client.messageServer();
		if (key[2] && !old_key[2]) client.publish();
		if (key[3] && !old_key[3]) client.uploadFile();
		if (key[4] && !old_key[4]) client.downloadFile();
		if (key[5] && !old_key[5]) bQuit = true;

		for (int i = 0; i < 6; i++) old_key[i] = key[i];

		if (client.isConnected())
		{
			if (!client.incoming().empty())
			{
				auto msg = client.incoming().pop_front().msg;

				switch (msg.header.m_nid)
				{
				case CustomMsgTypes::ServerAccept:
				{
					cout << "Server Accepted Connection\n";
				}
				break;


				case CustomMsgTypes::ServerPing:
				{
					chrono::system_clock::time_point timeNow = chrono::system_clock::now();
					chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					cout << "Ping: " << chrono::duration<double>(timeNow - timeThen).count() << "\n";
				}
				break;

				case CustomMsgTypes::ServerMessage:
				{
					uint32_t clientID;
					msg >> clientID;
					cout << "Hello from [" << clientID << "]\n";
				}
				break;

				case CustomMsgTypes::ServerMessageToClient:
				{
					string message;
					msg >> message;
					cout << "[SERVER said]: " << message << "\n";
				}
				break;

				case CustomMsgTypes::DownloadFilePath:
				{
					string downloadPath;
					msg >> downloadPath;
					downloadPath = "ClientFiles/" + downloadPath;
					try
					{
						if (!fs::exists(downloadPath)) {
							fs::create_directories(downloadPath);
						}
						downloadPath += "/" + client.getFileName();
						clsrv::file::openWriteFile(downloadPath, ios_base::binary | ios_base::out);
						client.setDownloadPath(downloadPath);
					}
					catch (std::exception& ex)
					{
						cerr << ex.what() << std::endl;
					}
				}
				break;

				case CustomMsgTypes::DownloadChunk:
				{
					client.handleFileDownload(msg);
				}
				break;

				case CustomMsgTypes::FileEnd:
				{
					cout << endl << endl;
					cout << clsrv::file::extractFileName(client.getFileName()) << " Successfully Received from Server" << endl;
				}
				break;

				}
			}
		}
		else
		{
			cout << "Server Down\n";
			bQuit = true;
		}

	}
	return 0;
}