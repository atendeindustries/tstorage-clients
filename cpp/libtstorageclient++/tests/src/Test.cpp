/*
 * TStorage: Client library tests (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include <cstdint>
#include <iostream>
#include <map>
#include <string>

#include <tstorageclient++/Timestamp.h>

#include "BufferTests.h"
#include "ChannelTests.h"
#include "SerializerTests.h"
#include "SocketTests.h"

using namespace tstorage;
using std::cout;
using std::endl;

const std::map<std::string, int(*)()> testMap = {
	{"test_socket_close", test_socket_close},
	{"test_socket_send", test_socket_send},
	{"test_socket_recv", test_socket_recv},
	{"test_socket_recv_exactly", test_socket_recv_exactly},
	{"test_socket_recv_atleast", test_socket_recv_atleast},
	{"test_socket_skip_exactly", test_socket_skip_exactly},
	{"test_socket_shutdown_send", test_socket_shutdown_send},
	{"test_socket_shutdown_recv", test_socket_shutdown_recv},
	{"test_socket_dialog", test_socket_dialog},
	{"test_socket_send_timeout", test_socket_send_timeout},
	{"test_socket_recv_timeout", test_socket_recv_timeout},

	{"test_buffer_create", test_buffer_create},
	{"test_buffer_heads", test_buffer_heads},
	{"test_buffer_reserve", test_buffer_reserve},

	{"test_serializer_put", test_serializer_put},
	{"test_serializer_get", test_serializer_get},
	{"test_serializer_buffer", test_serializer_buffer},

	{"test_channel_connect", test_channel_connect},
	{"test_channel_getacq", test_channel_getacq},
	{"test_channel_get_empty", test_channel_get_empty},
	{"test_channel_put_empty", test_channel_put_empty},
	{"test_channel_put", test_channel_put},
	{"test_channel_puta", test_channel_puta},
	{"test_channel_put_variable_payload_size", test_channel_put_variable_payload_size},
	{"test_channel_get", test_channel_get},
	{"test_channel_many_records", test_channel_many_records},
	{"test_channel_large_payloads", test_channel_large_payloads},
	{"test_channel_mixed_payloads", test_channel_mixed_payloads},
	{"test_channel_get_with_memory_limit", test_channel_get_with_memory_limit},
	{"test_channel_get_stream", test_channel_get_stream},
	{"test_channel_put_bad_cid", test_channel_put_bad_cid},
	{"test_channel_put_key_out_of_range", test_channel_put_key_out_of_range},
};

namespace globals {

std::string addr("127.0.0.1");
std::uint16_t port = 2090;

} /*namespace globals*/

int main(int argc, char** argv)
{
	if (argc <= 1 || argc > 4) {
		cout << "[ERROR] Expected at least 1 and at most 3 command line arguments"
			<< " (test name, server's IP address and port respectively)" << endl;
		return -1;
	}

	std::string testname{argv[1]};
	if (argc > 2) {
		globals::addr = std::string{argv[2]};
	}
	if (argc > 3) {
		globals::port = std::stoul(argv[3]);
	}
	if (testMap.find(argv[1]) == testMap.end()) {
		cout << "[ERROR] Test " << argv[1] << " not found" << endl;
		return -1;
	}
	cout << "Server address: " << globals::addr << ":" << globals::port << endl;

	cout << "Test start @ " << Timestamp::now() << endl;
	int retVal = testMap.at(testname)();
	cout << "Test ended @ " << Timestamp::now() << endl;
	return retVal;
}
