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

/** @file BufferedIStream.h
 *
 * Interface of the BufferedIStream class.
 */

#ifndef BUFFEREDISTREAM_H
#define BUFFEREDISTREAM_H

#include <stddef.h>

#include "DynamicBuffer.h"
#include "ErrorCode.h"

/**
 * @brief Stream read function for BufferedIStream.
 *
 * A BufferedIStream has .stream, an abstract input stream, and .write, a
 * stream read function. Whenever the BufferedIStream wishes to read data from
 * the underlying stream, it calls .read passing .stream as the first
 * argument. This function shall read at least 'size' bytes and at most
 * 'maxSize' bytes from the stream into the buffer 'buffer'.
 *
 * @param stream The stream from which the data is read.
 * @param buffer The buffer to which data should be read.
 * @param size Minimum size of data to read, in bytes.
 * @param maxSize Size of 'buffer' in bytes.
 * @return Number of data actually read. It must be <= maxSize. To indicate an
 * error, return a value < 'size'.
 */
typedef size_t (*BufferedIStream_read_fun)(void* stream, char* buffer, size_t size, size_t maxSize);

/**
 * @brief An input stream with a directly-accessible buffer.
 *
 * It is built on top of an abstract stream object, .stream, and a read
 * function, .read, that is called on that stream to obtain data.
 *
 * The BufferedIStream has a buffer, .buffer, whose maxSize should be set
 * beforehand. To obtain a specific amount of data from the BufferedIStream,
 * 'reserve' it by calling BufferedIStream_reserve. This gives you a pointer
 * to a place in .buffer at which the data is available. It is not possible to
 * reserve more data than the buffer's maxSize.
 *
 * The reserved data must be 'confirmed' or it will fill the buffer up. Call
 * BufferedIStream_confirm to mark all data reserved so far as confirmed.
 *
 * The BufferedIStream may read more data from the stream than the user
 * reserves, to minimize the number of calls to .read; the number of bytes
 * reserved is stored at .reservePos, while the number of bytes read is stored
 * at .readPos. The buffer's current size grows dynamically if
 * needed, up to its maxSize.
 */
typedef struct BufferedIStream
{
	/* Buffer for storing read data */
	DynamicBuffer* buffer;
	/* The underlying input stream */
	void* stream;
	/* Read function of the underlying stream */
	BufferedIStream_read_fun read;
	/* Number of bytes already read */
	size_t readPos;
	/* Number of bytes already reserved */
	size_t reservePos;
} BufferedIStream;

/**
 * @brief Initializes the buffered stream.
 *
 * @param this The buffered stream being initialized.
 * @param buffer The buffer this stream shall operate on.
 * @param stream The underlying stream
 * @param readFun The underlying stream's read function
 *
 * @public @memberof BufferedIStream
 */
void BufferedIStream_initialize(BufferedIStream* this, DynamicBuffer* buffer, void* stream, BufferedIStream_read_fun readFun);

/**
 * @brief Finalizes the buffered stream.
 *
 * @param this The buffered stream being finalized.
 *
 * @public @memberof BufferedIStream
 */
void BufferedIStream_finalize(BufferedIStream* this);

/**
 * @brief Resets the buffered stream. Empties the buffer.
 *
 * @param this The buffered stream being reset.
 *
 * @public @memberof BufferedIStream
 */
void BufferedIStream_reset(BufferedIStream* this);

/**
 * @brief Ensures at least 'size' bytes of data in the stream buffer.
 *
 * On success, the function sets '*buffer' to point to received data. Then the
 * user may access [ *buffer .. *buffer + size ).
 *
 * The function invalidates any buffer pointers returned by previous calls to
 * 'BufferedIStream_reserve'.
 *
 * The buffer is not emptied until the user confirms the reserved data by
 * calling 'BufferedIStream_confirm'. When there is no space left in the
 * buffer, this function will return ERR_LIMIT.
 *
 * @param this The buffered stream in which data is read
 * @param[out] buffer Pointer to the reserved data in the stream's buffer
 * @param size Requested size of the data to reserve. May be 0.
 * @return ERR_OK on success.
 * ERR_LIMIT when 'size' is greater than free (previously unconfirmed) space
 * to the end of buffer.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_RECEIVE on an error in this->read.
 *
 * @public @memberof BufferedIStream
 */
ErrorCode BufferedIStream_reserve(BufferedIStream* this, char** buffer, size_t size);

/**
 * @brief Confirms all reserved data in the buffer so far.
 *
 * Confirming frees space in the buffer, for next calls to
 * BufferedIStream_reserve.
 *
 * @param this the stream in which data is confirmed
 *
 * @public @memberof BufferedIStream
 */
void BufferedIStream_confirm(BufferedIStream* this);

#endif /* BUFFEREDISTREAM_H */
