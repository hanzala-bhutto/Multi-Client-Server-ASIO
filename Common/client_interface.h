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

			ClientInterface() : m_scoket(m_context)
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
					m_connection = std::make_unique<connection<T>>();
					asio::ip::tcp::resolver resolver(m_context);
					m_endpoints = resolver.resolve(host, std::to_string(port));
					m_connection->connectToServer(m_endpoints);
					thrContext = std::thread([this]() {m_context.run();})
				}
				catch (std::exception& e)
				{
					std::cerr << "Client Exception: " << e.what() << "\n";
					return false;
				}
				return false;
			}

			void disconnect()
			{
				if (isConnceted())
					m_connection->diconnect();

				m_context.stop();
				if (thrContext.joinable)
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

			ThSfQueue<owned_message<T>>& incoming()
			{
				return m_qMessagesIn;
			}

		protected:
			asio::io_context m_context;
			std::thread thrContext;
			asio::ip::tcp::socket m_socket;
			std::unique_ptr<connection<T>> m_connection;
		
		private:
			ThSfQueue<owned_message<T>> m_qMessagesIn;
		};
	}
}