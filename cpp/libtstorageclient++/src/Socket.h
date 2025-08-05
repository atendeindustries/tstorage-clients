/*
 * TStorage: Client library (C++)
 *
 * Socket.h
 *   A simple, connection-oriented socket class.
 *
 * Copyright 2025 Atende Industries
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef D_TSTORAGE_SOCKET_PH
#define D_TSTORAGE_SOCKET_PH

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <sys/socket.h>

#include <tstorageclient++/DataTypes.h>

/** @file
 * @brief Defines a simple TCP socket class. */

namespace tstorage {
namespace impl {

/**
 * @brief A standard abstraction of a system-provided TCP socket.
 *
 * This class implements a connection-oriented, blocking socket with a user
 * specified timeout. Since we don't need server functionality here, the
 * `listen()`/`accept()` execution path has been left out.
 *
 * Each `Socket` method which invokes syscalls internally (`recv()` family,
 * `send()`, `skip()`, as well as `connect()`, `close()`, `abort()`,
 * `shutdown()` and `setTimeoutMs()`) sets `errno` on error. To get the `errno`
 * of the last call for more fine-grained error management, use `getErrno()`.
 */
class Socket
{
public:
	/** @brief Shutdown flags.*/
	enum Shut : int {
		READ = SHUT_RD,
		WRITE = SHUT_WR,
		READWRITE = SHUT_RDWR,
	};

private:
	/** @brief Default socket timeout for receive and send.*/
	static constexpr std::int32_t cDefaultTimeoutMs = 20'000;
	/** @brief Static buffer size for data skipping.*/
	static constexpr std::int32_t cSkipBufferSize = 1024;

public:
	/** @brief Default constructor. Sets a default, invalid host. */
	Socket() : mSocketFd(-1), mErrno{}, mPort(0), mTimeoutMs(cDefaultTimeoutMs) {}
	/** @brief A constructor. Sets the address and port of the target server. */
	Socket(std::string addr, std::uint16_t port)
		: mSocketFd(-1)
		, mErrno{}
		, mAddr(std::move(addr))
		, mPort(port)
		, mTimeoutMs(cDefaultTimeoutMs)
	{
	}
	/** @brief A destructor. Closes the connection if open. */
	~Socket() { close(); }

	Socket(const Socket&) = delete;
	Socket(Socket&&) = delete;
	Socket& operator=(const Socket&) = delete;
	Socket& operator=(Socket&&) = delete;

	/**
	 * @brief Allocates a socket FD/handle and establishes a connection to a
	 * host.
	 *
	 * This method may return one of the following error codes.
	 *  - `result_t::BAD_ADDRESS`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNREFUSED`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::SETOPT_ERROR`
	 *  - `result_t::SIGNAL`
	 *  - `result_t::SOCKET_ERROR`
	 *
	 * For their meaning see `tstorage::result_t` in `DataTypes.h`. For
	 * supplementary, platform-dependent error diagnostics, use `getErrno()`.
	 *
	 * @return Status code.
	 */
	result_t connect();
	/**
	 * @brief Gracefully closes the connection and frees the socket FD/handle.
	 *
	 * If the connection is not established at the moment the method is called,
	 * then nothing is done and the method returns with
	 * `result_t::NOT_CONNECTED`. In the event the `close` syscall fails, the
	 * method sets the `mErrno` field and exits with `result_t::CONNERROR`.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNERROR`
	 *  - `result_t::NOT_CONNECTED`
	 *
	 * @return Status code.
	 */
	result_t close();
	/**
	 * @brief Ends the communication in a given direction.
	 *
	 * When given an argument `Shut::READ`, the read end is closed and each
	 * future `recv()` call on the current connection yields no further data.
	 *
	 * When given an argument `Shut::WRITE`, the socket sends a TCP FIN message
	 * indicating that no further data will be sent to the host.
	 *
	 * When given an argument `Shut::READWRITE`, both of the above actions are
	 * performed.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::NOT_CONNECTED`
	 *
	 * @param direction The direction of communication to close.
	 * @return Status code.
	 */
	result_t shutdown(Shut direction);
	/** @brief	Closes the connection forcefully, without regard to the state of
	 * the socket. Sends RST to the server if still connected. */
	void abort();

	/**
	 * @brief Sends exactly `amountBytes` of data from a read-only memory block
	 * `bytes`.
	 *
	 * When an unexpected event arises (i.e. when the return value is not
	 * `result_t::OK`) the amount of bytes actually sent might be lower than
	 * `amountBytes`. This amount is stored in `oAmountSent` variable on method
	 * exit.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNRESET`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @param[in] bytes The address of a read-only memory block with data to send.
	 * @param[in] amountBytes The amount of bytes to send from the `bytes` buffer.
	 * @param[out] oAmountSent The amount of bytes actually sent.
	 * @return Status code.
	 */
	result_t send(const void* bytes, std::size_t amountBytes, std::size_t& oAmountSent);
	/**
	 * @brief Receives up to `amountBytes` of data and writes it to a writable
	 * memory block `buffer` by issuing a single syscall.
	 *
	 * The write is performed as soon as data becomes available in the TCP
	 * buffer. This means that the amount of bytes received (stored in
	 * `oAmountRecvd` on method exit) might be lower than the requested
	 * `amountBytes` during normal operation. It is, however always positive
	 * while the connection is open. If the sending side closes its write-end by
	 * sending FIN packet, further `recv()` attempts will set `oAmountRecvd` to
	 * 0.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @param[in] buffer The address of a writable memory block to store the
	 * received data into.
	 * @param[in] amountBytes The maximal amount of bytes to receive into the
	 * `buffer`.
	 * @param[out] oAmountRecvd The amount of bytes actually received.
	 * @return Status code.
	 */
	result_t recv(void* buffer, std::size_t amountBytes, std::size_t& oAmountRecvd);
	/**
	 * @brief Receives exactly `amountBytes` of data and writes it to a writable
	 * memory block `buffer`.
	 *
	 * The method signals that the connection has closed by returning
	 * `result_t::OK` with `oAmountRecvd < amountBytes`.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @param[in] buffer The address of a writable memory block to store the
	 * received data into.
	 * @param[in] amountBytes The amount of bytes to receive into the `buffer`.
	 * @param[out] oAmountRecvd The amount of bytes actually received.
	 * @return Status code.
	 */
	result_t recvExactly(
		void* buffer, std::size_t amountBytes, std::size_t& oAmountRecvd);
	/**
	 * @brief Receives at least `amountBytes` of data and writes it to a writable
	 * memory block `buffer`.
	 *
	 * The method works by repeatedly calling `recv()` until the requested amount
	 * of bytes is fetched from the other end. The size of the response is
	 * limited only by the remaining size of the buffer, not `amountBytes`, hence
	 * we can accumulate more bytes in the buffer than strictly necessary. Under
	 * normal conditions, `oAmountRecvd ≥ amountBytes`. If the method returns
	 * otherwise (with status code `result_t::OK`) then it is a sign that the
	 * connection has closed gracefully and further receive attempts will fetch 0
	 * bytes.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @param[in] buffer The address of a writable memory block to store
	 * received data into.
	 * @param[in] bufferSize The size of the memory block and the upper limit of
	 * amount of data fetched.
	 * @param[in] amountBytes The amount of bytes to receive into the `buffer`.
	 * @param[out] oAmountRecvd The amount of bytes actually received.
	 * @return Status code.
	 */
	result_t recvAtLeast(void* buffer,
		std::size_t bufferSize,
		std::size_t amountBytes,
		std::size_t& oAmountRecvd);
	/**
	 * @brief Receives and discards exactly `amountBytes` of data.
	 *
	 * A status code `result_t::OK` with `oAmountSkipped < amountBytes` signals
	 * that the connection has closed gracefully.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @param[in] amountBytes The amount of bytes to discard.
	 * @param[out] oAmountSkipped The amount of bytes actually skipped.
	 * @return Status code.
	 */
	result_t skipExactly(std::size_t amountBytes, std::size_t& oAmountSkipped);

	/**
	 * @brief Sets the host address/port pair for the next `connect()` attempt.
	 * @param addr Host's address.
	 * @param port The port to connect to.
	 */
	void setHost(const std::string& addr, std::uint16_t port)
	{
		mAddr = addr;
		mPort = port;
	}
	/**
	 * @brief Sets the socket's send and receive timeouts to
	 * `Socket::mTimeoutMs`.
	 * @return `result_t::OK` on success, `result_t::SETOPT_ERROR` otherwise.
	 */
	result_t setTimeoutMs();
	/**
	 * @brief Sets `Socket::mTimeoutMs` to `timeoutMs` and adjusts the socket's
	 * send and receive timeouts accordingly if connected.
	 * @param timeoutMs New timeout.
	 * @return `result_t::OK` on success, `result_t::SETOPT_ERROR` otherwise.
	 */
	result_t setTimeoutMs(std::uint32_t timeoutMs);

	/** @brief Returns the last `errno`. */
	int getErrno() const { return mErrno; }
	/** @brief Returns `true` if the connection has been established but not
	 * explicitly closed, `false` otherwise. */
	bool connectionEstablished() const { return mSocketFd != -1; }

private:
	/** @brief Socket FD/handle. */
	int mSocketFd;
	/** @brief The last `errno`. */
	int mErrno;
	/** @brief The host's address. */
	std::string mAddr;
	/** @brief The host's port. */
	std::uint16_t mPort;
	/** @brief Socket send and receive timeout in milliseconds. */
	std::uint32_t mTimeoutMs;
};

} /*namespace impl*/
} /*namespace tstorage*/

#endif
