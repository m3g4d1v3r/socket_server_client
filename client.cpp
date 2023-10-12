#ifndef SERVER_PORT
	#define SERVER_PORT 9999
#endif
#ifndef SERVER_IP
	#define SERVER_IP "127.0.0.1"
#endif
#ifndef ARRAY_BYTES
	#define ARRAY_BYTES 20
#endif
#include <boost/asio.hpp>
#include <iostream>
#include <unistd.h>

using namespace std;
using namespace boost::asio;
using namespace boost::asio::ip;

// Read from Stdin and send all the data to socket server
int main() {
	io_service io_service;

	tcp::socket client_socket(io_service);

	client_socket.connect(tcp::endpoint(address::from_string(SERVER_IP), SERVER_PORT));

	uint8_t data[ARRAY_BYTES];
	size_t bytes;
	boost::system::error_code ec;

	do {
		bytes = read(STDIN_FILENO, data, sizeof(data)); // Read, unistd lib function
		write(client_socket, buffer(data, bytes), ec); // Write, boost::asio lib function
	} while (bytes > 0);

	return (0);
}
