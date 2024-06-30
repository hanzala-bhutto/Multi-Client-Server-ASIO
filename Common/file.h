#pragma once

namespace clsrv
{
			namespace file
			{
				std::string inputPath(std::string in_path)
				{
					std::cout << "Enter path of file : ";
					std::getline(std::cin, in_path);
					return in_path;
				}

				bool fileExists(std::string path)
				{
					if (!fs::exists(path))
					{
						std::cerr << "File path does not exist" << std::endl;
						return false;
					}

					std::fstream file;
					file.open(path, std::fstream::in);
					if (file.is_open()) 
					{
						file.close();
						return true;
					}
					std::cerr << "File does not exist" << std::endl;
					return false;
				}

				std::string extractFileName(std::string path)
				{
					return fs::path(path).filename().string();
				}

				std::ifstream openReadFile(std::string path, std::ios_base::openmode mode)
				{
					std::ifstream file;
					file.open(path, mode);
					if (file.fail())
						throw std::fstream::failure("Failed while opening file " + path);
				
					return file;
				}

				std::ofstream openWriteFile(std::string path, std::ios_base::openmode mode)
				{
					std::ofstream file;
					file.open(path, mode);
					if (file.fail())
						throw std::fstream::failure("Failed while opening file " + path);
					return file;
				}

				size_t getFileSize(std::ifstream &file)
				{
					file.seekg(0, file.end);
					size_t fileSize = file.tellg();
					file.seekg(0, file.beg);
					return fileSize;
				}

				std::vector<uint8_t> readFile(std::ifstream &file, size_t fileSize, size_t BUFFER_SIZE)
				{
					size_t bytesToRead = std::min(static_cast<size_t>(fileSize), BUFFER_SIZE);
					std::vector<uint8_t> buffer(bytesToRead);
					file.read(reinterpret_cast<char*>(buffer.data()), bytesToRead);
					return buffer;
				}

				size_t getBytesToRead(size_t fileSize, size_t BUFFER_SIZE)
				{
					return std::min(static_cast<size_t>(fileSize), BUFFER_SIZE);
				}

				void fileCompleted(std::ifstream &file)
				{
					if (file.eof()) {
						std::cout << "File read successfully" << std::endl;
					}
					else if (file.fail()) {
						std::cerr << "Failed while reading file" << std::endl;
					}
				}
		}
}
