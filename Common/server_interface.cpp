#pragma once
#include "server_interface.h"

namespace clsrv
{
	namespace net
	{
		template<typename T>
		ServerInterface<T>::ServerInterface(uint16_t port): m_asioAcceptor(m_asioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)){}

		template<typename T>
		ServerInterface<T>::~ServerInterface()
		{
			stop();
		}

		template<typename T>
		bool ServerInterface<T>::start()
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
		
		template<typename T>
		void ServerInterface<T>::stop()
		{
			m_asioContext.stop();
			if (m_threadContext.joinable())
				m_threadContext.join();
			std::cout << "[SERVER] Stopped!\n";
		}

		template<typename T>
		void ServerInterface<T>::waitForClientConnection()
		{
			m_asioAcceptor.async_accept
			(
				[this](std::error_code ec, asio::ip::tcp::socket socket)
				{
					if (!ec)
					{
						std::cout << "[SERVER] New Connection: " << socket.remote_endpoint() << "\n";
						std::shared_ptr<Connection<T>> newconn =
							std::make_shared<Connection<T>>(Connection<T>::owner::server,
								m_asioContext, std::move(socket), m_qMessagesIn);
						if (onClientConnect(newconn))
						{
							m_deqConnections.push_back(std::move(newconn));
							m_deqConnections.back()->connectToClient(m_nIDCounter++);
							std::cout << "[" << m_deqConnections.back()->getID() << "] Connection Approved\n";
						}
						else
						{
							std::cout << "[-----] Connection Denied\n";
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

		template<typename T>
		void ServerInterface<T>::messageClient(std::shared_ptr<Connection<T>> client, const Message<T>& msg)
		{
			if (client && client->isConnected())
			{
				client->send(msg);
			}
			else
			{
				onClientDisconnect(client);
				client.reset();
				m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), client), m_deqConnections.end());

			}
		}

		template<typename T>
		void ServerInterface<T>::messageAllClients(const Message<T>& msg, std::shared_ptr<Connection<T>> pIgnoreClient)
		{
			bool bInvalidClientExists = false;
			for (auto& client : m_deqConnections)
			{
				if (client && client->isConnected())
				{
					if (client != pIgnoreClient)
						client->send(msg);
				}
				else
				{
					onClientDisconnect(client);
					client.reset();
					bInvalidClientExists = true;
				}
			}
			if (bInvalidClientExists)
				m_deqConnections.erase(std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
		}

		template<typename T>
		void ServerInterface<T>::update(size_t nMaxMessages)
		{
			size_t nMessageCount = 0;
			while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
			{
				auto msg = m_qMessagesIn.pop_front();
				onMessage(msg.remote, msg.msg);
				nMessageCount++;
			}
		}

		template<typename T>
		bool ServerInterface<T>::onClientConnect(std::shared_ptr<Connection<T>> client)
		{
			return false;
		}

		template<typename T>
		void ServerInterface<T>::onClientDisconnect(std::shared_ptr<Connection<T>> client) {}

		template<typename T>
		void ServerInterface<T>::onMessage(std::shared_ptr<Connection<T>> client, Message<T>& msg) {}
	}
}

