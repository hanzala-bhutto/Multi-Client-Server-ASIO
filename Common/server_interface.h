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
			ServerInterface(uint16_t port);
			virtual ~ServerInterface();
			bool start();
			void stop();
			void waitForClientConnection();
			void messageClient(std::shared_ptr<Connection<T>> client, const Message<T>& msg);
			void messageAllClients(const Message<T>& msg, std::shared_ptr<Connection<T>> pIgnoreClient = nullptr);
			void update(size_t nMaxMessages = -1);
		protected:
			virtual bool onClientConnect(std::shared_ptr<Connection<T>> client);
			virtual void onClientDisconnect(std::shared_ptr<Connection<T>> client);
			virtual void onMessage(std::shared_ptr<Connection<T>> client, Message<T>& msg);
		protected:
			ThSfQueue<OwnedMessage<T>> m_qMessagesIn;
			std::deque<std::shared_ptr<Connection<T>>> m_deqConnections;
			asio::io_context m_asioContext;
			std::thread m_threadContext;
			asio::ip::tcp::acceptor m_asioAcceptor;
			uint32_t m_nIDCounter = 1000;
		};
	}
}