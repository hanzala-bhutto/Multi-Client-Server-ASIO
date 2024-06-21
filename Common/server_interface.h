#pragma once

#include "common.h"
#include "thsfqueue.h"
#include "message.h"
#include "connection.h"

namespace clsrv
{
	namespace net
	{
		template<typename T>
		class ServerInterface
		{
		public:
			ServerInterface(uint16_t port) : m_asioAcceptor(m_asioAcceptor, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
			{

			}

			virtual ~ServerInterface()
			{
				stop();
			}

			bool start()
			{
				try
				{
					waitForClientConnection();
					m_threadContext = std::thread([this]() {m_asioContext.run();});
				}
				catch (std::exception& e)
				{
					std::cerr << "[SERVER] Exception: " << e.what() << "\n";
					return false;
				}
				std::cout << "[SERVER] Started!\n";
				return true;
			}
			void stop()
			{
				m_asioContext.stop();

				if (m_threadContext.joinable())
					m_threadContext.join();

				std::cout << "[SERVER] Stopped!\n";
			}
			void waitForClientConnection()
			{
				m_asioAcceptor.async_accept
				(
					[this](std::error_code ec, asio::ip::tcp::socket socket)
					{
						if (!ec)
						{
							std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";
							std::shared_ptr<connection<T>> newconn = std::make_shared<connection<T>>(connection<T>::owner::server, m_asioContext, std::move(socket), m_qMessagesIn);
							
							if (onClientConnect(newconn))
							{
								m_deqConnections.push_back(std::move(newconn));
								m_deqConnections.back()->connectToClient(nIDCounter++);
								std::cout << "[" << m_deqConnections.back()->GetId() << "] Connection Approved\n";
							}

						}
						else
						{
							std::cout << "[SERVER] New Connection Error: " << ec.message() << "\n";
						}
						waitForClientConnection();
					}
				);
			}
			
			void messageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
			{
				if (client && client->isConnected())
				{
					client->send(msg);
				}
				else
				{
					onClientDisconnect(client);
					client.reset();
					m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), mdeq_Connections.end());

				}
			}

			void update(size_t nMaxMessages = -1)
			{
				size_t nMessageCount = 0;

				while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
				{
					auto msg = m_qMessageIn.pop_front();

					onMessage(msg.remote, msg.msg);

					nMessageCount++;
				}
			}

		protected:
			virtual bool onClientConnect(std::shared_ptr<connection<T>> client)
			{
				return false;
			}

			virtual void onClientDisconnect(std::shared_ptr<connection<T>> client)
			{

			}

			virtual void onMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
			{

			}

		protected:
			ThSfQueue<owned_message<T>> m_qMessagesIn;

			std::deque<std::shared_ptr<connection<T>>> m_deqConnections;

			asio::io_context m_asioContext;
			std::thread m_threadContext;

			asio::ip::tcp::acceptor m_asioAcceptor;

			uint32_t nIDCounter = 1000;
		};
	}
}