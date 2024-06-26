#pragma once
#include "common.h"

namespace clsrv
{
	namespace net
	{
		template <typename T>
		struct message_header
		{
			T id{};
			uint32_t size = 0;
		};

		template <typename T>
		struct message
		{
			message_header<T> header{};
			std::vector<uint8_t> body;

			size_t size() const
			{
				return body.size();
			}

			friend std::ostream& operator << (std::ostream& os, const message<T>& msg)
			{
				os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
				return os;
			}

			template<typename DataType>
			friend message<T>& operator << (message<T>& msg, const DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");
				size_t i = msg.body.size();
				msg.body.resize(msg.body.size() + sizeof(DataType));
				std::memcpy(msg.body.data() + i, &data, sizeof(DataType));
				msg.header.size = msg.size();
				return msg;
			}

			template<typename DataType>
			friend message<T>& operator >> (message<T>& msg, DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pulled from vector");
				size_t i = msg.body.size() - sizeof(DataType);
				std::memcpy(&data, msg.body.data() + i, sizeof(DataType));
				msg.body.resize(i);
				msg.header.size = msg.size();
				return msg;
			}

			friend message<T>& operator<<(message<T>& msg, const std::string& str) {
				// Serialize the size of the string
				uint32_t size = static_cast<uint32_t>(str.size());

				// Serialize the characters of the string
				for (const char& c : str) {
					msg.body.push_back(static_cast<uint8_t>(c));
				}

				msg.header.size = msg.size();
				return msg;
			}

			// Deserialization operator for std::string
			friend message<T>& operator>>(message<T>& msg, std::string& str) {

				str.clear();

				for (auto it = msg.body.begin(); it != msg.body.end(); ++it) {
					char c =  static_cast<char>(*it);
					str.push_back(c);
				}

				msg.body.clear();
				msg.header.size = msg.size();

				return msg;
			}

			template<typename DataType>
			friend message<T>& operator<<(message<T>& msg, const std::vector<DataType>& vec) {

				std::cout << vec.size() << std::endl;

				size_t i = msg.body.size();
				msg.body.resize(msg.body.size() + vec.size());
				std::memcpy(msg.body.data() + i, &vec, vec.size());
				msg.header.size = msg.size();
				return msg;
			}

			// Overloaded operator >> for std::vector
			template<typename DataType>
			friend message<T>& operator>>(message<T>& msg, std::vector<DataType>& vec) {
				vec.clear();
				vec.resize(msg.body.size());
				size_t i = msg.body.size() - sizeof(DataType);
				vec.assign(msg.body.begin(), msg.body.end());
				msg.body.resize(i);
				msg.header.size = msg.size();

				return msg;
			}


		};

		template <typename T>
		class connection;

		template <typename T>
		struct owned_message
		{
			std::shared_ptr<connection<T>> remote = nullptr;
			message<T> msg;

			friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg)
			{
				os << msg.msg;
				return os;
			}
		};
	}
}