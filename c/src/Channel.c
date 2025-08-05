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

/** @file Channel.c
 *
 * Implementation of the TSClient_Channel class.
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "tstorage-client/client.h"

#include "Channel.h"

#include "BufferedIStream.h"
#include "BufferedOStream.h"
#include "DynamicBuffer.h"
#include "ErrorCode.h"
#include "RecordsSet.h"
#include "RequestGet.h"
#include "RequestPut.h"
#include "ResponseGet.h"
#include "ResponseGetConfirmation.h"
#include "ResponseGetWithCallback.h"
#include "ResponsePut.h"
#include "Socket.h"

/**
 * @brief A wrapper around Socket_receive.
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
 */
static size_t socketRead(void* stream, char* buffer, size_t size, size_t maxSize)
{
	return Socket_receive((Socket*)stream, buffer, size, maxSize);
}

/**
 * @brief A wrapper around Socket_send.
 *
 * @param this The socket to transmit through.
 * @param[out] buffer The data to send.
 * @param size Size of data to send in 'buffer'.
 * @return ERR_OK on success
 * ERR_SEND on send(2) error. Check errno(3).
 */
static ErrorCode socketWrite(void* stream, const char* buffer, size_t size)
{
	return Socket_send((Socket*)stream, buffer, size);
}

/* The default is just enough to GET or PUTASAFE one record with 32MB payload. */
static const size_t DEFAULT_MEMORY_LIMIT = (1 << (10 + 10 + 5)) + 56;

/**
 * @brief Initializes a new Channel.
 *
 * @return ERR_OK, ERR_RESOURCE
 */
static ErrorCode initialize(TSCLIENT_Channel* this, const char* host, int port, const TSCLIENT_PayloadType* payloadType)
{
	if ((this->host = malloc(strlen(host) + 1)) == NULL) {
		return ERR_RESOURCE;
	}
	strcpy(this->host, host);

	this->port = port;
	this->payloadType = *payloadType;

	ErrorCode res = DynamicBuffer_initialize(&this->buffer, DEFAULT_MEMORY_LIMIT, 0);
	if (res != ERR_OK) {
		free(this->host);
	} else {
		Socket_initialize(&this->sock);

		BufferedIStream_initialize(&this->input, &this->buffer, &this->sock, socketRead);
		BufferedOStream_initialize(&this->output, &this->buffer, &this->sock, socketWrite);
	}

	return res;
}

static void finalize(TSCLIENT_Channel* this)
{
	BufferedOStream_finalize(&this->output);
	BufferedIStream_finalize(&this->input);
	Socket_finalize(&this->sock);
	DynamicBuffer_finalize(&this->buffer);
	free(this->host);
}

TSCLIENT_Channel* TSCLIENT_Channel_new(const char* host, int port, const TSCLIENT_PayloadType* payloadType)
{
	TSCLIENT_Channel* this = malloc(sizeof(TSCLIENT_Channel));
	if (this == NULL) {
		return NULL;
	}

	ErrorCode res = initialize(this, host, port, payloadType);
	if (res != ERR_OK) {
		free(this);
		return NULL;
	}

	return this;
}

void TSCLIENT_Channel_destroy(TSCLIENT_Channel* this)
{
	finalize(this);
	free(this);
}

int TSCLIENT_Channel_setTimeout(TSCLIENT_Channel* this, struct timeval* timeout)
{
	if (!Socket_isOpen(&this->sock)) {
		return errno = ENOTSOCK;
	}

	return Socket_setTimeout(&this->sock, timeout) != ERR_OK;
}

void TSCLIENT_Channel_setMemoryLimit(TSCLIENT_Channel* this, size_t size)
{
	DynamicBuffer_setMaxSize(&this->buffer, size);
}

TSCLIENT_ResponseStatus TSCLIENT_Channel_connect(TSCLIENT_Channel* this)
{
	if (Socket_isOpen(&this->sock)) {
		return TSCLIENT_ERR_INVALID;
	}

	BufferedOStream_reset(&this->output);
	BufferedIStream_reset(&this->input);

	return Socket_open(&this->sock, this->host, this->port);
}

TSCLIENT_ResponseStatus TSCLIENT_Channel_close(TSCLIENT_Channel* this)
{
	if (!Socket_isOpen(&this->sock)) {
		return TSCLIENT_ERR_INVALID;
	}

	return Socket_close(&this->sock);
}

TSCLIENT_ResponseStatus TSCLIENT_Channel_get(TSCLIENT_Channel* restrict this,
	const TSCLIENT_Key* restrict keyMin,
	const TSCLIENT_Key* restrict keyMax,
	int64_t* acq,
	TSCLIENT_RecordsSet** restrict data)
{
	*data = NULL; /* no records retrieved so far */

	RequestGet request;
	RequestGet_initializeGet(&request, keyMin, keyMax);

	ErrorCode res = RequestGet_write(&request, &this->output);
	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	res = BufferedOStream_flush(&this->output);
	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	ResponseGet response;
	res = ResponseGet_read(&response, &this->input, &this->payloadType);
	/* Always pass records deserialized so far to user, even if an error occured. */
	*data = response.records;

	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	if ((TSCLIENT_ResponseStatus)response.base.result != TSCLIENT_RES_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)response.base.result;
	}

	ResponseGetConfirmation confResponse;
	res = ResponseGetConfirmation_read(&confResponse, &this->input);
	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	if ((TSCLIENT_ResponseStatus)confResponse.base.result != TSCLIENT_RES_OK) {
		TSCLIENT_Channel_close(this);
	} else {
		*acq = confResponse.acq;
	}

	BufferedIStream_reset(&this->input);

	return (TSCLIENT_ResponseStatus)confResponse.base.result;
}

/**
 * @return TSCLIENT_RES_OK on success.
 * TSCLIENT_RES_SEND on send(2). Check errno(3).
 * TSCLIENT_RES_RECEIVE on recv(3). Check errno(3). errno = 0 means the server
 * closed the connection when more data was expected.
 * TSCLIENT_ERR_MEMORYLIMIT when memoryLimit exceeded
 * TSCLIENT_ERR_RESOURCE on realloc(3) error. Check errno(3).
 * TSCLIENT_ERR_UNEXPECTED when received malformed response
 */
static TSCLIENT_ResponseStatus commonPut(TSCLIENT_Channel* restrict this, const RequestPut* request, RecordsSet_write_fun recordsSetSerializeFun)
{
	ErrorCode res = RequestPut_write(request, &this->output, recordsSetSerializeFun);
	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	res = BufferedOStream_flush(&this->output);
	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	ResponsePut response;
	res = ResponsePut_read(&response, &this->input);
	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	if ((TSCLIENT_ResponseStatus)response.base.result != TSCLIENT_RES_OK) {
		TSCLIENT_Channel_close(this);
	}

	BufferedIStream_reset(&this->input);

	return (TSCLIENT_ResponseStatus)response.base.result;
}

TSCLIENT_ResponseStatus TSCLIENT_Channel_put(TSCLIENT_Channel* restrict this, const TSCLIENT_RecordsSet* restrict data)
{
	RequestPut request;
	RequestPut_initializePutSafe(&request, data);

	return commonPut(this, &request, RecordsSet_writePutSafe);
}

TSCLIENT_ResponseStatus TSCLIENT_Channel_puta(TSCLIENT_Channel* restrict this, const TSCLIENT_RecordsSet* restrict data)
{
	RequestPut request;
	RequestPut_initializePutASafe(&request, data);

	return commonPut(this, &request, RecordsSet_writePutASafe);
}

TSCLIENT_ResponseStatus TSCLIENT_Channel_getAcq(TSCLIENT_Channel* restrict this,
	const TSCLIENT_Key* restrict keyMin,
	const TSCLIENT_Key* restrict keyMax,
	int64_t* acq)
{
	RequestGet request;
	RequestGet_initializeGetAcq(&request, keyMin, keyMax);

	ErrorCode res = RequestGet_write(&request, &this->output);
	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	res = BufferedOStream_flush(&this->output);
	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	ResponseGetConfirmation response;
	res = ResponseGetConfirmation_read(&response, &this->input);
	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	if ((TSCLIENT_ResponseStatus)response.base.result != TSCLIENT_RES_OK) {
		TSCLIENT_Channel_close(this);
	} else {
		*acq = response.acq;
	}

	BufferedIStream_reset(&this->input);

	return (TSCLIENT_ResponseStatus)response.base.result;
}

TSCLIENT_ResponseStatus TSCLIENT_Channel_getStream(TSCLIENT_Channel* restrict this,
	const TSCLIENT_Key* restrict keyMin,
	const TSCLIENT_Key* restrict keyMax,
	TSCLIENT_GetCallback callback,
	void* userData,
	int64_t* acq)
{
	RequestGet request;
	RequestGet_initializeGet(&request, keyMin, keyMax);

	ErrorCode res = RequestGet_write(&request, &this->output);
	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	res = BufferedOStream_flush(&this->output);
	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	ResponseGetWithCallback response;
	res = ResponseGetWithCallback_read(&response, &this->input, &this->payloadType, callback, userData);

	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	if ((TSCLIENT_ResponseStatus)response.base.result != TSCLIENT_RES_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)response.base.result;
	}

	ResponseGetConfirmation confResponse;
	res = ResponseGetConfirmation_read(&confResponse, &this->input);
	if (res != ERR_OK) {
		TSCLIENT_Channel_close(this);
		return (TSCLIENT_ResponseStatus)res;
	}

	if ((TSCLIENT_ResponseStatus)confResponse.base.result != TSCLIENT_RES_OK) {
		TSCLIENT_Channel_close(this);
	} else {
		*acq = confResponse.acq;
	}

	BufferedIStream_reset(&this->input);

	return (TSCLIENT_ResponseStatus)confResponse.base.result;
}
