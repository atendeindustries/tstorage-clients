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

/** @file Socket.h
 *
 * Interface of the Socket class.
 */

#ifndef SOCKET_H
#define SOCKET_H

#include <stddef.h>
#include <sys/time.h>

#include "ErrorCode.h"

typedef struct Socket
{
	int sockfd;
} Socket;

/**
 * @brief Initializes a socket.
 *
 * @param this The socket being initialized.
 *
 * @public @memberof Socket
 */
void Socket_initialize(Socket* this);

/**
 * @brief Finalizes a previously initialized Socket.
 *
 * @param this The socket being finalized.
 *
 * @public @memberof Socket
 */
void Socket_finalize(Socket* this);

/**
 * @brief Checks if the socked is opened.
 *
 * @param this The socked being investigated.
 * @return non-0 if opened, 0 if closed
 *
 * @public @memberof Socket
 */
int Socket_isOpen(Socket* this);

/**
 * @brief Opens and connects a socket.
 *
 * @param this The socket being opened. It may not be in the open state.
 * @param[in] host hostname of the connection
 * @param port TCP port number of the connection
 * @return ERR_OK on success.
 * ERR_RECEIVE - getaddrinfo(3) error. Check errno(3).
 * ERR_RESOURCE - socket(2) error. Check errno(3).
 * ERR_SEND - connect(2) error. Check errno(3).
 *
 * @public @memberof Socket
 */
ErrorCode Socket_open(Socket* this, const char* host, int port);

/**
 * @brief Closes a socket.
 *
 * @param this The socket to close. It must be in open state.
 * @return ERR_OK on success.
 * ERR_RESOURCE on close(2) error. Check errno(3).
 *
 * @public @memberof Socket
 */
ErrorCode Socket_close(Socket* this);

/**
 * @brief Transmits data through a socket.
 *
 * @param this The socket to transmit through.
 * @param[out] buffer The data to send.
 * @param size Size of data to send in 'buffer'.
 * @return ERR_OK on success.
 * ERR_SEND on send(2) error. Check errno(3).
 *
 * @public @memberof Socket
 */
ErrorCode Socket_send(Socket* this, const char* buffer, size_t size);

/**
 * @brief Receives data from a socket.
 *
 * @param this The socket to receive from.
 * @param[in] buffer The buffer to store received data.
 * @param size Minimum size of data to receive into 'buffer'. If 0, then the
 * function does nothing and returns 0.
 * @param maxSize Maximum size of data to receive into 'buffer'.
 * @return number of data received. If an error occurs before 'size' bytes of
 * data were received, the function returns a value < size; check errno(3) for
 * a recv(2) error code or to 0 to indicate that the server closed the
 * connection. Otherwise, a value >= 'size' and <= 'maxSize' is returned.
 *
 * @public @memberof Socket
 */
size_t Socket_receive(Socket* this, char* buffer, size_t size, size_t maxSize);

/**
 * @brief Set the socket's receive and send timeouts to a given value.
 *
 * @param this The socket to change.
 * @param[in] timeout The timeout.
 * @return ERR_OK on success.
 * ERR_RESOURCE on setsockopt(2) error. Check errno(3).
 *
 * @public @memberof Socket
 */
ErrorCode Socket_setTimeout(Socket* this, const struct timeval* timeout);

#endif /* SOCKET_H */
