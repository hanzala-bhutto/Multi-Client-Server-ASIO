
#include <iostream>
#include <clsrv_net.h>

enum class CustomMsgTypes : uint32_t
{
	Fire,
	Move
};

class Client : public clsrv::net::ClientInterface<CustomMsgTypes>
{
public:
};

int main()
{
	//Client c;
	//c.connect("", 60000);
	return 0;
}
