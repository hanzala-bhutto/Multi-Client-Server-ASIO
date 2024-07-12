#pragma once
#include "client_interface.h"

namespace clsrv
{
	namespace net 
	{
		template <typename T>
		ClientInterface<T>::ClientInterface(){}

		template <typename T>
		ClientInterface<T>::~ClientInterface()
		{
			disconnect();
		}

		template <typename T>
		bool ClientInterface<T>::connect(const std::string& host, const uint16_t port)
		{
			try
			{
				asio::ip::tcp::resolver resolver(m_context);
				asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, std::to_string(port));
				m_connection = std::make_unique<Connection<T>>(Connection<T>::owner::client, m_context, asio::ip::tcp::socket(m_context), m_qMessagesIn);
				m_connection->connectToServer(endpoints);
				m_thrContext = std::thread([this]() { m_context.run(); });
			}
			catch (std::exception& e)
			{
				std::cerr << "Client Exception: " << e.what() << "\n";
				return false;
			}
			return true;
		}

		template <typename T>
		void ClientInterface<T>::disconnect()
		{
			if (isConnected())
			{
				m_connection->disconnect();
			}
			m_context.stop();
			if (m_thrContext.joinable())
				m_thrContext.join();
			m_connection.release();
		}

		template <typename T>
		bool ClientInterface<T>::isConnected()
		{
			if (m_connection)
				return m_connection->isConnected();
			else
				return false;
		}

		template <typename T>
		void ClientInterface<T>::send(const Message<T>& msg)
		{
			if (isConnected())
				m_connection->send(msg);
		}

		template <typename T>
		ThSfQueue<OwnedMessage<T>>& ClientInterface<T>::incoming()
		{
			return m_qMessagesIn;
		}
	}
}

