#ifndef SERVER_PORT
	#define SERVER_PORT 9999
#endif
#ifndef ARRAY_BYTES
	#define ARRAY_BYTES 20
#endif
#define PREFIX_SIZE 10
#include <boost/asio.hpp>
#include <iostream>
#include <fstream>
#include <streambuf>
#include <unistd.h>

using namespace std;
using namespace std::chrono;
using namespace boost::asio;
using namespace boost::asio::ip;

#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
# define use_awaitable \
  boost::asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

string gen_random_prefix(size_t len) {
    srand((unsigned)time(NULL) * getpid());
    static const char alphanum[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    
    return tmp_s;
}

string get_current_time() {
	auto now = system_clock::now();
    auto ld = floor<days>(now);
    year_month_day ymd{ld};
    hh_mm_ss hms{now - ld};

    int year{ymd.year()};
    int month = unsigned{ymd.month()};
    int day = unsigned{ymd.day()};
    int hour = hms.hours().count();
    int minute = hms.minutes().count();
    int second = hms.seconds().count();
	int milliseconds = hms.subseconds().count();

	return to_string(year) + to_string(month) + to_string(day) + \
		   to_string(hour) + to_string(minute) + to_string(second) + \
		   to_string(milliseconds);
}

size_t write_data_to_file(string filename, uint8_t *data, size_t start, size_t size) {
	size_t i;
	ofstream file;
	file.open(filename, ios::out | ios::app | ios::binary);
	for (i = start; i < size; i++) {
		file << data[i];
	}
	file.close();
	return (i);
}

awaitable<void> echo(tcp::socket socket) {
	uint8_t data[ARRAY_BYTES];
	size_t bytes_read;
	string random_prefix;
	string filename;
	boost::system::error_code ec;

	size_t counter = 0;
	random_prefix = gen_random_prefix(PREFIX_SIZE);
	filename = random_prefix + "_" + get_current_time();
	cout << filename << endl;

	while (true) { // Until EOF exception
		bytes_read = co_await socket.async_read_some(buffer(data, sizeof(data)), use_awaitable);
		counter += bytes_read;

		if (counter <= ARRAY_BYTES) {
			write_data_to_file(filename, data, 0, bytes_read);
		}
		else {
			size_t j = 0;
			j = write_data_to_file(filename, data, j, ARRAY_BYTES - (counter - bytes_read));
			counter -= ARRAY_BYTES;
			usleep(1000);
			filename = random_prefix + "_" + get_current_time();
			cout << endl << filename << endl;
			write_data_to_file(filename, data, j, bytes_read);
		}
	}
}

awaitable<void> listener() {
	auto executor = co_await this_coro::executor;
	tcp::acceptor acceptor(executor, {tcp::v4(), SERVER_PORT});
	for (;;) {
		tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
		co_spawn(executor, echo(std::move(socket)), detached);
	}
}

int main() {
	try {
		boost::asio::io_context io_context(1);

		boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
		signals.async_wait([&](auto, auto){ io_context.stop(); });

		co_spawn(io_context, listener(), detached);

		io_context.run();
	}
	catch (std::exception &e) {
		std::printf("Exception: %s\n", e.what());
	}
}
