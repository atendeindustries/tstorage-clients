/*
 * TStorage: Client library tests (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include "SocketTests.h"

#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#include <tstorageclient++/DataTypes.h>

#include "Buffer.h"
#include "Socket.h"

namespace globals {

extern std::string addr;
extern std::uint16_t port;

} /*namespace globals*/

using std::cout;
using std::endl;

/* NOLINTBEGIN(cppcoreguidelines-macro-usage) */
#define CALLANDLOG(func, callId, id) \
	res = (func); \
	if (res != result_t::OK) { \
		if ((std::int32_t)res > 0) { \
			cout << "[WARNINIG] " << (callId) << " result code " << (int)res << ", errno: " << socket.getErrno() \
				 << " (" << strerror(socket.getErrno()) << ")" << endl; \
		} else { \
			cout << "[ERROR] " << (callId) << " result code " << (int)res << ", errno: " << socket.getErrno() \
				 << " (" << strerror(socket.getErrno()) << ")" << endl; \
			return (id); \
		} \
	} \
	cout << (callId) << "..." << endl;
/* NOLINTEND(cppcoreguidelines-macro-usage) */

namespace tstorage {
using namespace impl;

int test_socket_close()
{
	Socket socket(globals::addr, globals::port);
	result_t res = result_t::OK;
	CALLANDLOG(socket.connect(), "Connect", 1)
	CALLANDLOG(socket.close(), "Close", 2)
	CALLANDLOG(socket.connect(), "Connect after close", 3)
	CALLANDLOG(socket.close(), "Final close", 4)
	return 0;
}

int test_socket_send()
{
	Socket socket(globals::addr, globals::port);
	Buffer buf(2048);

	const char* sentence = "Hello world! This sentence is false.";
	const std::size_t sentenceLen = strlen(sentence);
	strlcpy(static_cast<char*>(buf.writeData()), sentence, sentenceLen);

	result_t res{};
	std::size_t amtSent{};
	CALLANDLOG(socket.connect(), "Connect", 1)
	CALLANDLOG(socket.send(buf.readData(), 12, amtSent), "Send", 2)
	CALLANDLOG(socket.close(), "Close", 3)

	return 0;
}

int test_socket_recv()
{
	Socket socket(globals::addr, globals::port);
	Buffer buf(2048);

	result_t res = result_t::OK;
	CALLANDLOG(socket.connect(), "Connect", 1)

	int ctr = 1;
	std::size_t amtRecvd = 0;
	std::size_t totalAmtRecvd = 0;
	while (res == result_t::OK) {
		CALLANDLOG(socket.recv(buf.writeData(), 1000, amtRecvd), std::string("Recv ") + std::to_string(ctr), 2)
		++ctr;
		totalAmtRecvd += amtRecvd;
		buf.writeAdvance(amtRecvd);
	}
	cout << "(This warning signals that the connection has closed gracefully)" << endl;
	CALLANDLOG(socket.close(), "Close", 3)

	std::string msg(static_cast<const char*>(buf.readData()), totalAmtRecvd);

	cout << "\"" << msg << "\"" << endl;
	if (msg != "Hello from python!") {
		cout << "[ERROR] Read incorrect message!" << endl;
		return 4;
	}

	return 0;
}

int test_socket_recv_exactly()
{
	Socket socket(globals::addr, globals::port);
	Buffer buf(2048);

	result_t res = result_t::OK;
	CALLANDLOG(socket.connect(), "Connect", 1)

	std::size_t amtRecvd{};
	CALLANDLOG(socket.recvExactly(buf.writeData(), 12, amtRecvd), "Recv exactly", 2)
	CALLANDLOG(socket.close(), "Close", 3)

	if (amtRecvd != 12) {
		cout << "[ERROR] Received incorrect number of bytes! (" << amtRecvd << ", expected 12)" << endl;
		return 4;
	}

	std::string msg(static_cast<const char*>(buf.readData()), amtRecvd);
	cout << "\"" << msg << "\"" << endl;
	if (msg != "Hello world!") {
		cout << "[ERROR] Read incorrect message!" << endl;
		return 5;
	}

	return 0;
}

int test_socket_recv_atleast()
{
	Socket socket(globals::addr, globals::port);
	Buffer buf(2048);

	result_t res = result_t::OK;
	CALLANDLOG(socket.connect(), "Connect", 1)

	std::size_t amtRecvd{};
	CALLANDLOG(socket.recvAtLeast(buf.writeData(), 2048, 12, amtRecvd), "Recv exactly", 2)
	CALLANDLOG(socket.close(), "Close", 3)

	if (amtRecvd < 12) {
		cout << "[ERROR] Received incorrect number of bytes! (" << amtRecvd << ", expected 12)" << endl;
		return 4;
	}

	std::string msg(static_cast<const char*>(buf.readData()), amtRecvd);
	cout << "\"" << msg << "\"" << endl;
	if (amtRecvd < 12 || (amtRecvd % 5) != 0) {
		cout << "[ERROR] Incorrect message length (" << amtRecvd
			 << ", expected a multiple of 5, at least 12)" << endl;
		return 5;
	}

	return 0;
}

int test_socket_skip_exactly()
{
	Socket socket(globals::addr, globals::port);
	Buffer buf(2048);

	result_t res = result_t::OK;
	CALLANDLOG(socket.connect(), "Connect", 1)

	std::size_t totalRecvd = 0;
	std::size_t amtRecvd = 0;
	std::size_t amtSkipped = 0;
	CALLANDLOG(socket.recvExactly(buf.writeData(), 5, amtRecvd), "Recv exactly", 2)
	CALLANDLOG(socket.skipExactly(9, amtSkipped), "Skip exactly", 3)
	buf.writeAdvance(amtRecvd);
	CALLANDLOG(socket.recvExactly(buf.writeData(), 100, totalRecvd), "Recv exactly", 4)
	CALLANDLOG(socket.close(), "Close", 5)
	totalRecvd += amtRecvd;

	if (totalRecvd != 12) {
		cout << "[ERROR] Received incorrect number of bytes! (" << totalRecvd << ")" << endl;
		return 6;
	}

	if (amtSkipped != 9) {
		cout << "[ERROR] Skipped incorrect number of bytes! (" << amtSkipped << ")" << endl;
		return 7;
	}

	std::string msg(static_cast<const char*>(buf.readData()), totalRecvd);
	cout << "Msg: " << msg << endl;
	if (msg != "Hello world!") {
		cout << "[ERROR] Read incorrect message!" << endl;
		return 8;
	}

	return 0;
}

int test_socket_shutdown_send()
{
	Socket socket(globals::addr, globals::port);
	Buffer buf(2048);

	const char* sentence = "Hello?";
	const std::size_t sentenceLen = strlen(sentence);
	strlcpy(static_cast<char*>(buf.writeData()), sentence, sentenceLen);

	result_t res = result_t::OK;
	CALLANDLOG(socket.connect(), "Connect", 1)

	std::size_t amtSent = 0;
	CALLANDLOG(socket.send(buf.readData(), 6, amtSent), "Send", 2)
	CALLANDLOG(socket.shutdown(Socket::Shut::WRITE), "Shutdown (write)", 3)

	if (amtSent != 6) {
		cout << "[ERROR] Received incorrect number of bytes! ("
			 << amtSent << ")" << endl;
		return 4;
	}

	res = socket.send(buf.readData(), 6, amtSent);
	if (res != result_t::CONNCLOSED) {
		cout << "[ERROR] " << "Send after shutdown result code "
			 << (int)res << ", errno: " << socket.getErrno()
			 << " (" << strerror(socket.getErrno()) << ")" << endl;
		return 5;
	}

	std::size_t amtRecvd = 0;
	CALLANDLOG(socket.recvExactly(buf.writeData(), 2048, amtRecvd), "Recv after write shutdown", 6)
	cout << "Number of bytes received: " << std::to_string(amtRecvd) << endl;
	std::string msg(static_cast<const char*>(buf.readData()));
	if (msg != "OK") {
		cout << "[ERROR] Received incorrect confirmation (" << msg
			 << ", expected \"OK\")" << endl;
		return 7;
	}

	cout << "Closing socket..." << endl;
	socket.close();

	return 0;
}

int test_socket_shutdown_recv()
{
	Socket socket(globals::addr, globals::port);
	Buffer buf(2048);

	result_t res = result_t::OK;
	CALLANDLOG(socket.connect(), "Connect", 1)

	std::size_t amtRecvd = 0;
	CALLANDLOG(socket.recv(buf.writeData(), 10, amtRecvd), "Recv", 2)
	cout << "  Buffer usage: " << amtRecvd << "B" << endl;
	cout << "  Buffer content: " << std::string(static_cast<const char*>(buf.readData()), amtRecvd) << endl;
	CALLANDLOG(socket.shutdown(Socket::Shut::READ), "Shutdown (read)", 3)

	cout << "Sending confirmation..." << endl;

	const char* ok = "OK";
	std::size_t amtSent = 0;
	CALLANDLOG(socket.send(ok, 3, amtSent), "Send after read shutdown", 4)

	CALLANDLOG(socket.recvExactly(buf.writeData(), 10, amtRecvd), "Recv after shutdown", 5)
	if (amtRecvd != 0) {
		cout << "[ERROR] Recv returned actual data, not EOS" << endl;
		cout << "  Buffer usage: " << amtRecvd << "B" << endl;
		cout << "  Buffer content: " << std::string(static_cast<const char*>(buf.readData()), amtRecvd) << endl;
		return 6;
	}

	cout << "Closing socket..." << endl;
	socket.close();

	return 0;
}

int test_socket_dialog()
{
	Socket socket(globals::addr, globals::port);
	Buffer buf{2048};

	result_t res = result_t::OK;
	CALLANDLOG(socket.connect(), "Connect", 1)

	const char* msg1 = "Hello. Are you world?";
	std::size_t amtSent = 0;
	std::size_t amtToSend = 22;
	CALLANDLOG(socket.send(&amtToSend, 4, amtSent), "Send len1", 2)
	CALLANDLOG(socket.send(msg1, amtToSend, amtSent), "Send msg1", 3)

	std::size_t amtRecvd = 0;
	CALLANDLOG(socket.recvExactly(buf.writeData(), 4, amtRecvd), "Recv len1", 4)
	if (amtRecvd < 4) {
		cout << "[ERROR] Early connection shutdown" << endl;
		return 5;
	}
	unsigned int msgLen = *static_cast<const unsigned int*>(buf.readData());
	CALLANDLOG(socket.recvExactly(buf.writeData(), msgLen, amtRecvd), "Recv msg1", 6)
	if (amtRecvd != msgLen) {
		cout << "[ERROR] Early connection shutdown" << endl;
		return 7;
	}
	std::string recvd = std::string(static_cast<const char*>(buf.readData()));
	std::string validmsg = "No, you must have been mistaken. I'm just a dumb server.";
	cout << "\"" << recvd << "\"" << endl;
	if (recvd != validmsg) {
		cout << "[ERROR] bad message (expected '" << validmsg << "')" << endl;
		return 8;
	}

	const char* msg2 = "Oh, sorry then. Goodbye!";
	amtToSend = 25;
	CALLANDLOG(socket.send(&amtToSend, 4, amtSent), "Send len2", 8)
	CALLANDLOG(socket.send(msg2, amtToSend, amtSent), "Send msg2", 9)

	CALLANDLOG(socket.recvExactly(buf.writeData(), 4, amtRecvd), "Recv len2", 10)
	if (amtRecvd < 4) {
		cout << "[ERROR] Early connection shutdown" << endl;
		return 11;
	}
	msgLen = *static_cast<const unsigned int*>(buf.readData());
	CALLANDLOG(socket.recvExactly(buf.writeData(), msgLen, amtRecvd), "Recv msg2", 12)
	if (amtRecvd != msgLen) {
		cout << "[ERROR] Early connection shutdown" << endl;
		return 13;
	}
	recvd = std::string(static_cast<const char*>(buf.readData()));
	validmsg = "Hey, wait up!";
	cout << "\"" << recvd << "\"" << endl;
	if (recvd != validmsg) {
		cout << "[ERROR] bad message (expected '" << validmsg << "')" << endl;
		return 8;
	}

	CALLANDLOG(socket.close(), "Close", 14)
	return 0;
}

int test_socket_send_timeout()
{
	Socket socket(globals::addr, globals::port);
	Buffer buf{2048};

	cout << "Setting timeout: 0.2s" << endl;
	socket.setTimeoutMs(200);

	result_t res = result_t::OK;
	CALLANDLOG(socket.connect(), "Connect", 1)

	std::size_t len = 64 * 1024L;
	std::string data(len, '\0');
	std::size_t amtSent{};
	std::size_t amtRepeats = 0;
	result_t resSend = result_t::OK;
	cout << "Sending lots of data to an unresponsive server..." << endl;
	auto t1 = std::chrono::steady_clock::now();
	while (resSend == result_t::OK && amtRepeats < 1024 * 1024L) {
		t1 = std::chrono::steady_clock::now();
		resSend = socket.send(data.c_str(), len, amtSent);
		++amtRepeats;
	}
	if (resSend != result_t::CONNTIMEOUT) {
		cout << "[ERROR] Send did not time out. Result: "
			 << (int)res << ", errno: " << socket.getErrno()
			 << " (" << strerror(socket.getErrno()) << ")" << endl;
		return 3;
	}
	auto deltat = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t1);
	cout << "Send timed out after " << deltat.count() << "ms "
		 << "(" << deltat.count() / 200 << " zero-window ACKs), "
		 << "pushed at least " << len * (amtRepeats - 1) << "B" << endl;
	if (deltat.count() < 200) {
		cout << "[ERROR] Timeout too early." << endl;
		return 4;
	}
	if (deltat.count() > 600) {
		cout << "[ERROR] Timeout too late. Expected at most 400ms delay "
			 << "(2 extra zero-window ACKs), got "
			 << deltat.count() - 200 << "ms" << endl;
		return 5;
	}
	CALLANDLOG(socket.close(), "Close", 4)
	return 0;
}

int test_socket_recv_timeout()
{
	Socket socket(globals::addr, globals::port);
	Buffer buf{2048};

	cout << "Setting timeout: 0.2s" << endl;
	socket.setTimeoutMs(200);

	result_t res = result_t::OK;
	CALLANDLOG(socket.connect(), "Connect", 1)

	std::int32_t value{};
	std::size_t amtRecvd{};
	auto t1 = std::chrono::steady_clock::now();
	cout << "Receiving data from a lagging server..." << endl;
	result_t resRecv = socket.recv(&value, sizeof(std::int32_t), amtRecvd);
	if (resRecv != result_t::CONNTIMEOUT) {
		cout << "[ERROR] Recv did not time out. Result: "
			 << (int)res << ", errno: " << socket.getErrno()
			 << " (" << strerror(socket.getErrno()) << ")" << endl;
		return 3;
	}
	auto deltat = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t1);
	cout << "Recv timed out after " << deltat.count() << "ms" << endl;
	if (deltat.count() < 200) {
		cout << "[ERROR] Timeout too early." << endl;
		return 4;
	}
	if (deltat.count() > 300) {
		cout << "[ERROR] Timeout too late (expected at most 100ms delay, got "
			 << deltat.count() - 200 << "ms)" << endl;
		return 5;
	}
	CALLANDLOG(socket.close(), "Close", 4)
	return 0;
}

} /*namespace tstorage*/
