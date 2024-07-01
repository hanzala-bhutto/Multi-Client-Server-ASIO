//#include "file.h"
//
//namespace clsrv
//{
//	namespace file
//	{
//		string inputPath(string in_path)
//		{
//			std::cout << "Enter path of file : ";
//			std::getline(cin, in_path);
//			return in_path;
//		}
//
//		bool fileExists(string path)
//		{
//			if (!fs::exists(path))
//			{
//				cerr << "File path does not exist" << endl;
//				return false;
//			}
//
//			fstream file;
//			file.open(path, fstream::in);
//			if (file.is_open())
//			{
//				file.close();
//				return true;
//			}
//			cerr << "File does not exist" << endl;
//			return false;
//		}
//
//		string extractFileName(string path)
//		{
//			return fs::path(path).filename().string();
//		}
//
//		size_t getFileSize(ifstream& file)
//		{
//			file.seekg(0, file.end);
//			size_t fileSize = file.tellg();
//			file.seekg(0, file.beg);
//			return fileSize;
//		}
//
//		size_t getBytesToRead(size_t fileSize, size_t BUFFER_SIZE)
//		{
//			return min(static_cast<size_t>(fileSize), BUFFER_SIZE);
//		}
//
//
//		size_t calculateChunkSize(size_t fileSize)
//		{
//			size_t minChunkSize = 1024;
//			size_t maxChunkSize = 1024 * 1024 * 20; // 20 mb
//			size_t chunkSize = (fileSize / 100);
//			while (chunkSize > maxChunkSize)
//			{
//				chunkSize = chunkSize / 10;
//			}
//			chunkSize = max(minChunkSize, chunkSize);
//			return chunkSize;
//		}
//
//		ifstream openReadFile(string path, ios_base::openmode mode)
//		{
//			ifstream file;
//			file.open(path, mode);
//			if (file.fail())
//				throw std::fstream::failure("Failed while opening file " + path);
//
//			return file;
//		}
//
//		ofstream openWriteFile(std::string path, ios_base::openmode mode)
//		{
//			std::ofstream file;
//			file.open(path, mode);
//			if (file.fail())
//				throw std::fstream::failure("Failed while opening file " + path);
//			return file;
//		}
//
//		vector<uint8_t> readFile(ifstream& file, size_t fileSize, size_t BUFFER_SIZE)
//		{
//			size_t bytesToRead = min(static_cast<size_t>(fileSize), BUFFER_SIZE);
//			std::vector<uint8_t> buffer(bytesToRead);
//			file.read(reinterpret_cast<char*>(buffer.data()), bytesToRead);
//			return buffer;
//		}
//
//		void fileCompleted(ifstream& file)
//		{
//			if (file.eof()) {
//				cout << "File read successfully" << endl;
//			}
//			else if (file.fail()) {
//				cerr << "Failed while reading file" << endl;
//			}
//		}
//	}
//}
