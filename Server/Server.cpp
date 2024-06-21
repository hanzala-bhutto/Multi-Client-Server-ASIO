#include <iostream>
#include <clsrv_net.h>

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage
};

class Server : public clsrv::net::ServerInterface<CustomMsgTypes>
{
public:
	Server(uint16_t nPort) : clsrv::net::ServerInterface<CustomMsgTypes>(nPort) {}

protected:
	virtual bool onClientConnect(std::shared_ptr<clsrv::net::connection<CustomMsgTypes>> client)
	{
		return true;
	}

	virtual void onClientDisconnect(std::shared_ptr<clsrv::net::connection<CustomMsgTypes>> client){}

	virtual void onMessage(std::shared_ptr<clsrv::net::connection<CustomMsgTypes>> client, clsrv::net::message<CustomMsgTypes> msg){}

};

int main()
{
	Server server(6000);
	server.start();

	while (1)
	{
		server.update();
	}
	return 0;
}
