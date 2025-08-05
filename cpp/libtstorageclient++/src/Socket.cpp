/*
 * TStorage: Client library (C++)
 *
 * Socket.cpp
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

#include "Socket.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <string>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <tstorageclient++/DataTypes.h>

namespace tstorage {
namespace impl {

result_t Socket::connect()
{
	if (mSocketFd >= 0) {
		return result_t::OK;
	}
	mSocketFd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (mSocketFd == -1) {
		mErrno = errno;
		return result_t::SOCKET_ERROR;
	}

	// clang-format off
	struct addrinfo hints{};
	// clang-format on
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = 0;

	struct addrinfo* ai = nullptr;
	const std::string portStr = std::to_string(mPort);
	if (::getaddrinfo(mAddr.c_str(), portStr.c_str(), &hints, &ai) != 0) {
		::close(mSocketFd);
		mSocketFd = -1;
		return result_t::BAD_ADDRESS;
	}

	const result_t resSet = setTimeoutMs();
	if (resSet != result_t::OK) {
		return result_t::SETOPT_ERROR;
	}

	enum ResultPrio : int {
		OK = 0,
		CONNREFUSED = 1,
		CONNERROR = 2,
		CONNTIMEOUT = 3,
	};
	ResultPrio connResultPrio = ResultPrio::OK;
	struct addrinfo* aiNext = ai;
	while (aiNext != nullptr
		&& ::connect(mSocketFd, aiNext->ai_addr, aiNext->ai_addrlen) == -1) {
		aiNext = aiNext->ai_next;
		mErrno = errno;
		if (mErrno == EINTR) {
			::freeaddrinfo(ai);
			::close(mSocketFd);
			mSocketFd = -1;
			return result_t::SIGNAL;
		}
		if (mErrno == ETIMEDOUT || mErrno == EAGAIN || mErrno == EWOULDBLOCK) {
			connResultPrio = std::max(connResultPrio, ResultPrio::CONNTIMEOUT);
		} else if (mErrno != ECONNREFUSED) {
			connResultPrio = std::max(connResultPrio, ResultPrio::CONNERROR);
		} else {
			connResultPrio = std::max(connResultPrio, ResultPrio::CONNREFUSED);
		}
	}

	if (aiNext == nullptr) {
		::freeaddrinfo(ai);
		::close(mSocketFd);
		mSocketFd = -1;
		switch (connResultPrio) {
			case ResultPrio::CONNREFUSED:
				return result_t::CONNREFUSED;
			case ResultPrio::CONNERROR:
				return result_t::CONNERROR;
			case ResultPrio::CONNTIMEOUT:
				return result_t::CONNTIMEOUT;
			default:
				return result_t::BAD_ADDRESS;
		}
	}
	mErrno = 0;
	::freeaddrinfo(ai);
	return result_t::OK;
}

result_t Socket::close()
{
	if (mSocketFd == -1) {
		return result_t::NOT_CONNECTED;
	}
	(void)shutdown(Shut::READWRITE);
	if (::close(mSocketFd) < 0) {
		mErrno = errno;
		mSocketFd = -1;
		return result_t::CONNERROR;
	}
	mSocketFd = -1;
	return result_t::OK;
}

result_t Socket::shutdown(const Socket::Shut direction)
{
	if (mSocketFd == -1) {
		return result_t::NOT_CONNECTED;
	}
	if (::shutdown(mSocketFd, direction) < 0) {
		mErrno = errno;
		if (mErrno == ENOTCONN) {
			return result_t::CONNCLOSED;
		}
		return result_t::CONNERROR;
	}
	return result_t::OK;
}

void Socket::abort()
{
	if (mSocketFd != -1) {
		::close(mSocketFd);
		mSocketFd = -1;
	}
}

result_t Socket::send(
	const void* const bytes, const std::size_t amountBytes, std::size_t& oAmountSent)
{
	if (mSocketFd == -1) {
		return result_t::NOT_CONNECTED;
	}
	oAmountSent = 0;
	const uint8_t* sendBuffer = static_cast<const uint8_t*>(bytes);
	while (oAmountSent < amountBytes) {
		const ssize_t sent =
			::send(mSocketFd, sendBuffer, amountBytes - oAmountSent, MSG_NOSIGNAL);
		if (sent < 0) {
			mErrno = errno;
			if (mErrno == EAGAIN || mErrno == EWOULDBLOCK) {
				return result_t::CONNTIMEOUT;
			}
			switch (mErrno) {
				case ENOTCONN: /* fallthrough */
				case EPIPE:
					return result_t::CONNCLOSED;
				case ECONNRESET:
					return result_t::CONNRESET;
				case EINTR:
					return result_t::SIGNAL;
				default:
					return result_t::CONNERROR;
			}
		}
		oAmountSent += sent;
		sendBuffer += sent;
	}
	return result_t::OK;
}

result_t Socket::recv(
	void* const buffer, const std::size_t amountBytes, std::size_t& oAmountRecvd)
{
	if (mSocketFd == -1) {
		return result_t::NOT_CONNECTED;
	}
	oAmountRecvd = 0;
	const ssize_t recvd = ::recv(mSocketFd, buffer, amountBytes, 0);
	if (recvd < 0) {
		mErrno = errno;
		if (mErrno == EAGAIN || mErrno == EWOULDBLOCK) {
			return result_t::CONNTIMEOUT;
		}
		switch (mErrno) {
			case EINTR:
				return result_t::SIGNAL;
			default:
				return result_t::CONNERROR;
		}
	}
	if (recvd == 0) {
		return result_t::CONNCLOSED;
	}
	oAmountRecvd = recvd;
	return result_t::OK;
}

result_t Socket::recvExactly(
	void* const buffer, const std::size_t amountBytes, std::size_t& oAmountRecvd)
{
	return recvAtLeast(buffer, amountBytes, amountBytes, oAmountRecvd);
}

result_t Socket::recvAtLeast(void* const buffer,
	const std::size_t bufferSize,
	const std::size_t amountBytes,
	std::size_t& oAmountRecvd)
{
	if (mSocketFd == -1) {
		return result_t::NOT_CONNECTED;
	}

	oAmountRecvd = 0;
	std::size_t lastRecvd = 0;
	uint8_t* recvBuffer = static_cast<uint8_t*>(buffer);
	while (oAmountRecvd < amountBytes) {
		const result_t resRecv =
			this->recv(recvBuffer, bufferSize - oAmountRecvd, lastRecvd);
		if (resRecv == result_t::CONNCLOSED) {
			return result_t::OK;
		}
		if (resRecv != result_t::OK) {
			return resRecv;
		}
		oAmountRecvd += lastRecvd;
		recvBuffer += lastRecvd;
	}
	return result_t::OK;
}

result_t Socket::skipExactly(const std::size_t amountBytes, std::size_t& oAmountSkipped)
{
	if (mSocketFd == -1) {
		return result_t::NOT_CONNECTED;
	}

	std::array<uint8_t, cSkipBufferSize> buffer; /* NOLINT(cppcoreguidelines-pro-type-member-init) */
	std::size_t lastRecvd = 0;
	oAmountSkipped = 0;

	while (oAmountSkipped < amountBytes) {
		const size_t nextAmtBytes = std::min(buffer.size(), amountBytes - oAmountSkipped);
		const result_t resRecv =
			this->recvExactly(buffer.data(), nextAmtBytes, lastRecvd);
		if (resRecv != result_t::OK) {
			return resRecv;
		}
		if (lastRecvd < nextAmtBytes) {
			return result_t::OK;
		}
		oAmountSkipped += lastRecvd;
	}
	return result_t::OK;
}

result_t Socket::setTimeoutMs(const std::uint32_t timeoutMs)
{
	mTimeoutMs = timeoutMs;
	return setTimeoutMs();
}

result_t Socket::setTimeoutMs()
{
	if (mSocketFd == -1) {
		return result_t::OK;
	}
	// clang-format off
	struct timeval timeout{};
	// clang-format on
	const int secondMs = 1000;
	const int usecsInMs = 1000;
	timeout.tv_sec = mTimeoutMs / secondMs;
	timeout.tv_usec = static_cast<std::int64_t>(mTimeoutMs % secondMs) * usecsInMs;
	if (setsockopt(mSocketFd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0
		|| setsockopt(mSocketFd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout))
			< 0) {
		mErrno = errno;
		return result_t::SETOPT_ERROR;
	}
	return result_t::OK;
}

} /*namespace impl*/
} /*namespace tstorage*/
