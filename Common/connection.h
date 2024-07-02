#pragma once
#include "thsfqueue.h"
#include "message.h"

namespace clsrv
{
	namespace net
	{
		template <typename T>
		class Connection : public std::enable_shared_from_this<Connection<T>>
		{
		public:
			enum class owner
			{
				server,
				client
			};
		public:
			Connection(owner parent, asio::io_context& asioContext, asio::ip::tcp::socket socket, ThSfQueue<OwnedMessage<T>>& qIn);
			virtual ~Connection();
			uint32_t getID() const;
			void connectToClient(uint32_t uid = 0);
			void connectToServer(const asio::ip::tcp::resolver::results_type& endpoints);
			void disconnect();
			bool isConnected() const;
			void StartListening();
			void send(const Message<T>& msg);
		private:
			void writeHeader();
			void writeBody();
			void readHeader();
			void readBody();
			void addToIncomingMessageQueue();
		protected:
			asio::ip::tcp::socket m_socket;
			asio::io_context& m_asioContext;
			ThSfQueue<Message<T>> m_qMessagesOut;
			ThSfQueue<OwnedMessage<T>>& m_qMessagesIn;
			Message<T> m_msgTemporaryIn;
			owner m_nOwnerType = owner::server;
			uint32_t m_nid = 0;
		};
	}
}