#pragma once
#include "common.h"

namespace clsrv
{
	namespace net
	{
		template <typename T>
		struct MessageHeader
		{
			T m_nid{};
			uint32_t m_nsize = 0;
		};

		template <typename T>
		struct Message
		{
			MessageHeader<T> header{};
			std::vector<uint8_t> body;

			size_t size() const
			{
				return body.size();
			}

			friend std::ostream& operator << (std::ostream& os, const Message<T>& msg)
			{
				os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
				return os;
			}

			template<typename DataType>
			friend Message<T>& operator << (Message<T>& msg, const DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector");
				size_t i = msg.body.size();
				msg.body.resize(msg.body.size() + sizeof(DataType));
				std::memcpy(msg.body.data() + i, &data, sizeof(DataType));
				msg.header.m_nsize = msg.size();
				return msg;
			}

			template<typename DataType>
			friend Message<T>& operator >> (Message<T>& msg, DataType& data)
			{
				static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pulled from vector");
				size_t i = msg.body.size() - sizeof(DataType);
				std::memcpy(&data, msg.body.data() + i, sizeof(DataType));
				msg.body.resize(i);
				msg.header.m_nsize = msg.size();
				return msg;
			}

			friend Message<T>& operator<<(Message<T>& msg, const std::string& str) {
				uint32_t size = static_cast<uint32_t>(str.size());
				for (const char& c : str) {
					msg.body.push_back(static_cast<uint8_t>(c));
				}
				msg.header.m_nsize = msg.size();
				return msg;
			}

			friend Message<T>& operator>>(Message<T>& msg, std::string& str) {
				str.clear();
				for (auto it = msg.body.begin(); it != msg.body.end(); ++it) {
					char c =  static_cast<char>(*it);
					str.push_back(c);
				}
				msg.body.clear();
				msg.header.m_nsize = msg.size();
				return msg;
			}
		};

		template <typename T>
		class Connection;

		template <typename T>
		struct OwnedMessage
		{
			std::shared_ptr<Connection<T>> remote = nullptr;
			Message<T> msg;

			friend std::ostream& operator<<(std::ostream& os, const OwnedMessage<T>& msg)
			{
				os << msg.msg;
				return os;
			}
		};
	}
}