
#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE

#include <iostream>
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

using namespace std;

int main()
{
	asio::error_code ec;

	asio::io_context context;

	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("93.184.216.34", ec), 80);

	asio::ip::tcp::socket socket(context);

	socket.connect(endpoint, ec);

	if (!ec)
	{
		std::cout << "Connected" << endl;
	}
	else
	{
		std::cout << "Failed to connect to address:\n" << ec.message() << endl;
	}

	if (socket.is_open())
	{
		string sRequest = "GET /index.html HTTP/1.1\r\n" "Host: example.com\r\n" "Connection: close\r\n\r\n";

		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);

		socket.wait(socket.wait_read);

		size_t bytes = socket.available();

		if (bytes > 0)
		{
			vector<char> vBuffer(bytes);
			socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()));

			for (auto c : vBuffer)
				cout << c;

		}

	}

	return 0;
}