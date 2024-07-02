#pragma once
#include "connection.h"

namespace clsrv
{
	namespace net
	{
		template <typename T>
		Connection<T>::Connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, ThSfQueue<OwnedMessage<T>>& qIn)
				: m_asioContext(asioContext), m_socket(std::move(socket)), m_qMessagesIn(qIn)
			{
				m_nOwnerType = parent;
			}

		template <typename T>
		Connection<T>::~Connection(){}

		template <typename T>
		uint32_t Connection<T>::getID() const
		{
			return m_nid;
		}

		template <typename T>
		void Connection<T>::connectToClient(uint32_t uid)
		{
			if (m_nOwnerType == owner::server)
			{
				if (m_socket.is_open())
				{
					m_nid = uid;
					readHeader();
				}
			}
		}

		template <typename T>
		void Connection<T>::connectToServer(const asio::ip::tcp::resolver::results_type& endpoints)
		{
			if (m_nOwnerType == owner::client)
			{
				asio::async_connect(m_socket, endpoints,
					[this](std::error_code ec, asio::ip::tcp::endpoint endpoint)
					{
						if (!ec)
						{
							readHeader();
						}
					});
			}
		}

		template <typename T>
		void Connection<T>::disconnect()
		{
			if (isConnected())
				asio::post(m_asioContext, [this]() { m_socket.close(); });
		}

		template <typename T>
		bool Connection<T>::isConnected() const
		{
			return m_socket.is_open();
		}

		template <typename T>
		void Connection<T>::StartListening(){}

		template <typename T>
		void Connection<T>::send(const Message<T>& msg)
		{
			asio::post(m_asioContext,
				[this, msg]()
				{
					bool bWritingMessage = !m_qMessagesOut.empty();
					m_qMessagesOut.push_back(msg);
					if (!bWritingMessage)
					{
						writeHeader();
					}
				});
		}

		template <typename T>
		void Connection<T>::writeHeader()
		{
			asio::async_write(m_socket, asio::buffer(&m_qMessagesOut.front().header, sizeof(MessageHeader<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_qMessagesOut.front().body.size() > 0)
						{
							writeBody();
						}
						else
						{
							m_qMessagesOut.pop_front();

							if (!m_qMessagesOut.empty())
							{
								writeHeader();
							}
						}
					}
					else
					{
						std::cout << "[" << m_nid << "] Write Header Fail.\n";
						m_socket.close();
					}
				});
		}

		template <typename T>
		void Connection<T>::writeBody()
		{
			asio::async_write(m_socket, asio::buffer(m_qMessagesOut.front().body.data(), m_qMessagesOut.front().body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						m_qMessagesOut.pop_front();

						if (!m_qMessagesOut.empty())
						{
							writeHeader();
						}
					}
					else
					{
						std::cout << "[" << m_nid << "] Write Body Fail.\n";
						m_socket.close();
					}
				});
		}

		template <typename T>
		void Connection<T>::readHeader()
		{
			asio::async_read(m_socket, asio::buffer(&m_msgTemporaryIn.header, sizeof(MessageHeader<T>)),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						if (m_msgTemporaryIn.header.m_nsize > 0)
						{
							m_msgTemporaryIn.body.resize(m_msgTemporaryIn.header.m_nsize);
							readBody();
						}
						else
						{
							addToIncomingMessageQueue();
						}
					}
					else
					{
						std::cout << "[" << m_nid << "] Read Header Fail.\n";
						m_socket.close();
					}
				});
		}

		template <typename T>
		void Connection<T>::readBody()
		{
			asio::async_read(m_socket, asio::buffer(m_msgTemporaryIn.body.data(), m_msgTemporaryIn.body.size()),
				[this](std::error_code ec, std::size_t length)
				{
					if (!ec)
					{
						addToIncomingMessageQueue();
					}
					else
					{
						std::cout << "[" << m_nid << "] Read Body Fail.\n";
						m_socket.close();
					}
				});
		}

		template <typename T>
		void Connection<T>::addToIncomingMessageQueue()
		{
			if (m_nOwnerType == owner::server)
				m_qMessagesIn.push_back({ this->shared_from_this(), m_msgTemporaryIn });
			else
				m_qMessagesIn.push_back({ nullptr, m_msgTemporaryIn });
			readHeader();
		}
	}
}

