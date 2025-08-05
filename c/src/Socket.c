/*
 * Copyright 2025 Atende Industries sp. z o.o.
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
 *
 */

/** @file Socket.c
 *
 * Implementation of the Socket class.
 */

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "Socket.h"

#include "ErrorCode.h"

void Socket_initialize(Socket* this)
{
	this->sockfd = -1;
}

void Socket_finalize(Socket* this)
{
}

int Socket_isOpen(Socket* this)
{
	return this->sockfd >= 0;
}

ErrorCode Socket_open(Socket* this, const char* host, int port)
{
	struct addrinfo hints = {
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_STREAM,
		.ai_protocol = IPPROTO_TCP};

	struct addrinfo* addrs;

	int addrErrno = getaddrinfo(host, NULL, &hints, &addrs);
	if (addrErrno != 0) {
		errno = addrErrno;
		return ERR_RECEIVE;
	}

	in_port_t portNetOrder = (in_port_t)htons((uint16_t)port);

	ErrorCode res = ERR_RECEIVE;

	for (struct addrinfo* addr = addrs; addr != NULL; addr = addr->ai_next) {
		assert(this->sockfd == -1);

		switch (addr->ai_family) {
			case AF_INET:
				assert(addr->ai_addrlen == sizeof(struct sockaddr_in));
				((struct sockaddr_in*)addr->ai_addr)->sin_port = portNetOrder;
				break;
			case AF_INET6:
				assert(addr->ai_addrlen == sizeof(struct sockaddr_in6));
				((struct sockaddr_in6*)addr->ai_addr)->sin6_port = portNetOrder;
				break;
			default: /* not IPv4/v6, omit */
				continue;
		}

		if ((this->sockfd = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) == -1) {
			res = ERR_RESOURCE;
			continue;
		}

		if (connect(this->sockfd, addr->ai_addr, addr->ai_addrlen) == 0) {
			res = ERR_OK;
			break; /* Success! */
		}

		close(this->sockfd);
		this->sockfd = -1;
		res = ERR_SEND;
	}

	freeaddrinfo(addrs);

	assert((this->sockfd == -1) == (res != ERR_OK));

	return res;
}

ErrorCode Socket_close(Socket* this)
{
	assert(this->sockfd != -1);

	int res = close(this->sockfd) == 0 ? ERR_OK : ERR_RESOURCE;
	this->sockfd = -1;
	return res;
}

ErrorCode Socket_send(Socket* this, const char* buffer, size_t size)
{
	assert(this->sockfd != -1);

	while (size > 0) {
		ssize_t numSent = send(this->sockfd, buffer, size, MSG_NOSIGNAL);
		if (numSent < 0) {
			return ERR_SEND;
		}
		buffer += numSent;
		size -= numSent;
	}

	return ERR_OK;
}

size_t Socket_receive(Socket* this, char* buffer, size_t size, size_t maxSize)
{
	assert(size <= maxSize);
	assert(this->sockfd != -1);

	size_t sumRecv = 0;

	while (sumRecv < size) {
		ssize_t numRecv = recv(this->sockfd, buffer, maxSize, 0);
		if (numRecv <= 0) {
			/* 0 means socket shut down, < 0 means error */
			if (numRecv == 0) {
				errno = 0;
			}
			return sumRecv;
		}

		sumRecv += numRecv;
		buffer += numRecv;
		maxSize -= numRecv;
	}

	return sumRecv;
}

ErrorCode Socket_setTimeout(Socket* this, const struct timeval* timeout)
{
	assert(this->sockfd != -1);

	int res = setsockopt(this->sockfd, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(*timeout));
	if (res != 0) {
		return ERR_RESOURCE;
	}
	res = setsockopt(this->sockfd, SOL_SOCKET, SO_SNDTIMEO, timeout, sizeof(*timeout));
	if (res != 0) {
		return ERR_RESOURCE;
	}

	return ERR_OK;
}
