#pragma once
#include "thsfqueue.h"
#include "message.h"

namespace clsrv
{
	namespace net
	{
		template <typename T>
		class connection : public std::enable_shared_from_this<connection<T>>
		{
		public:
			connection(){}

			virtual ~connection(){}

			bool connectToServer();
			bool disconnect();
			bool isConnected() const;

			bool send(const message<T>& msg);

		protected:
			asio::ip::tcp::socket m_socket;
			asio::io_context& m_asioContext;

			ThSfQueue<message<T>>& m_qMessagesOut;

			ThSfQueue<owned_message>& m_qMessagesOut;
		};
	}
}