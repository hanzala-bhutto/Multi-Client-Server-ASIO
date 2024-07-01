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
			ClientInterface();
			virtual ~ClientInterface();
		public:
			bool connect(const std::string& host, const uint16_t port);
			void disconnect();
			bool isConnected();
			void send(const Message<T>& msg);
			ThSfQueue<OwnedMessage<T>>& incoming();
		protected:
			asio::io_context m_context;
			std::thread m_thrContext;
			std::unique_ptr<Connection<T>> m_connection;
		private:
			ThSfQueue<OwnedMessage<T>> m_qMessagesIn;
		};
	};
}