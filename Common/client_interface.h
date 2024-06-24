#pragma once
#include "common.h"
#include "message.h"
#include "thsfqueue.h"
#include "connection.h"

namespace clsrv
{
	namespace net
	{
		template <typename T>
		class ClientInterface
		{
		public:
			ClientInterface()
			{}

			virtual ~ClientInterface()
			{
				disconnect();
			}

		public:
			bool connect(const std::string& host, const uint16_t port)
			{
				try
				{
					asio::ip::tcp::resolver resolver(m_context);
					asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));
					m_connection = std::make_unique<connection<T>>(connection<T>::owner::client, m_context, asio::ip::tcp::socket(m_context), m_qMessagesIn);
					m_connection->connectToServer(endpoints);
					thrContext = std::thread([this]() { m_context.run(); });
				}
				catch (std::exception& e)
				{
					std::cerr << "Client Exception: " << e.what() << "\n";
					return false;
				}
				return true;
			}

			void disconnect()
			{			
				if (isConnected())
				{
					m_connection->disconnect();
				}				
				m_context.stop();
				if (thrContext.joinable())
					thrContext.join();
				m_connection.release();
			}
			bool isConnected()
			{
				if (m_connection)
					return m_connection->isConnected();
				else
					return false;
			}

		public:
			void send(const message<T>& msg)
			{
				if (isConnected())
					m_connection->send(msg);
			}
			ThSfQueue<owned_message<T>>& incoming()
			{
				return m_qMessagesIn;
			}

		protected:
			asio::io_context m_context;
			std::thread thrContext;
			std::unique_ptr<connection<T>> m_connection;

		private:
			ThSfQueue<owned_message<T>> m_qMessagesIn;
		};
	};
}