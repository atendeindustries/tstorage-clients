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

/** @file BufferedOStream.h
 *
 * Interface of the BufferedOStream class.
 */

#ifndef BUFFEREDOSTREAM_H
#define BUFFEREDOSTREAM_H

#include <stddef.h>

#include "DynamicBuffer.h"
#include "ErrorCode.h"

/**
 * @brief Stream write function for BufferedOStream.
 *
 * A BufferedOStream has .stream, an abstract output stream, and .write, a
 * stream write function. Whenever the BufferedOStream wishes to flush data to
 * the underlying stream, it calls .write passing .stream as the first
 * argument. This function shall write 'size' bytes of the buffer 'buffer' to
 * the stream.
 *
 * @param stream The stream to which the data is written.
 * @param buffer The data to be written.
 * @param size Size of 'buffer' in bytes.
 * @return ERR_OK on success, ERR_SEND on write error.
 */
typedef ErrorCode (*BufferedOStream_write_fun)(void* stream, const char* buffer, size_t size);

/**
 * @brief An output stream with a directly-accessible buffer.
 *
 * It is built on top of an abstract stream object, .stream, and a write
 * function, .write, that is called on that stream to write gathered data.
 *
 * The BufferedOStream has a buffer, .buffer, whose maxSize should be set
 * beforehand. To obtain space in the buffer of a specified size from the
 * BufferedOStream, 'reserve' it by calling BufferedOStream_reserve. This
 * gives you a pointer to a place in .buffer to which you may write data. It
 * is not possible to reserve more data than the buffer's maxSize.
 *
 * The reserved data must be 'confirmed' or it will be overwritten via a next
 * 'reserve' call. Call BufferedOStream_confirm to mark reserved data as
 * confirmed. Then a next 'reserve' call will reserve the next area of the
 * buffer.
 *
 * The confirmed data is written out to the stream when you call
 * 'BufferedOStream_flush'. This also frees all space in the buffer. You can
 * also call BufferedOStream_reserveFlushing to automatically flush data while
 * making a reservation.
 *
 * The BufferedOStream stores the start of the current reservation at
 * 'reservePos'. The buffer's current size grows dynamically if needed, up to
 * its maxSize.
 */
typedef struct BufferedOStream
{
	/* Buffer for storing data to write */
	DynamicBuffer* buffer;
	/* The underlying output stream */
	void* stream;
	/* Write function of the underlying stream */
	BufferedOStream_write_fun write;
	/* Start position of the current reservation */
	size_t reservePos;
} BufferedOStream;

/**
 * @brief Initializes the buffered stream.
 *
 * @param this The buffered stream being initialized.
 * @param buffer The buffer this stream shall operate on.
 * @param stream The underlying stream
 * @param writeFun The underlying stream's write function
 *
 * @public @memberof BufferedOStream
 */
void BufferedOStream_initialize(BufferedOStream* this, DynamicBuffer* buffer, void* stream, BufferedOStream_write_fun writeFun);

/**
 * @brief Finalizes the buffered stream.
 *
 * @param this The buffered stream being finalized.
 *
 * @public @memberof BufferedOStream
 */
void BufferedOStream_finalize(BufferedOStream* this);

/**
 * @brief Resets the buffered stream. Empties the buffer.
 *
 * @param this The buffered stream being reset.
 *
 * @public @memberof BufferedOStream
 */
void BufferedOStream_reset(BufferedOStream* this);

/**
 * @brief Returns size of space actually reserved in the buffer.
 *
 * After obtaining a pointer to the buffer with reserve or reserveFlushing,
 * call this function to obtain the actual size of that buffer. The caller may
 * then write to the area [ *buffer .. (*buffer)+the size ).
 *
 * @param this The stream from which the buffer space is requested
 * @return Size of the reserved space.
 *
 * @public @memberof BufferedOStream
 */
size_t BufferedOStream_sizeReserved(const BufferedOStream* this);

/**
 * @brief Ensures a specific amount of space in the stream's buffer.
 *
 * Does not flush anything through the stream. Compare
 * 'BufferedOStream_reserveFlushing'.
 *
 * The function Invalidates any buffer pointers returned by earlier calls to
 * 'BufferedOStream_reserve[Flushing]'.
 *
 * The function may reserve more space than requested. Call
 * BufferedOStream_sizeReserved to obtain actual size of the reserved buffer.
 *
 * The buffer is not emptied until the user confirms the reserved data by
 * calling 'BufferedOStream_confirm' and writes it to the stream with
 * 'BufferedOStream_flush'. When there is no space left in the buffer, this
 * function will return ERR_LIMIT.
 *
 * @param this The stream in which buffer space is being reserved
 * @param[out] buffer Pointer to the reserved space in the buffer
 * @param size Requested size of the space to reserve. May be 0.
 * @return ERR_OK on success.
 * ERR_LIMIT when 'size' is greater than free space in the buffer.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 *
 * @public @memberof BufferedOStream
 */
ErrorCode BufferedOStream_reserve(BufferedOStream* this, char** buffer, size_t size);

/**
 * @brief Ensures a specific amount of space in the stream's buffer.
 *
 * If there is not enough space in the buffer, the function flushes any
 * confirmed data via the underlying stream to free space, then tries to
 * reserve again. Compare 'BufferedOStream_reserve'.
 *
 * The function may reserve more space than requested. Call
 * BufferedOStream_sizeReserved to obtain actual size of the reserved buffer.
 *
 * The function Invalidates any buffer pointers returned by earlier calls to
 * 'BufferedOStream_reserve[Flushing]'.
 *
 * @param this The stream in which buffer space is being reserved
 * @param[out] buffer Pointer to the reserved space in the buffer
 * @param size Requested size of the space to reserve. May be 0.
 * @return ERR_OK on success.
 * ERR_LIMIT when 'size' is greater than the buffer size.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_SEND on an error in this->write, when flushing data.
 *
 * @public @memberof BufferedOStream
 */
ErrorCode BufferedOStream_reserveFlushing(BufferedOStream* this, char** buffer, size_t size);

/**
 * @brief Confirms a specific amount of data in the stream's buffer to sending.
 *
 * Assuming the space was previously reserved (see
 * 'BufferedOStream_reserve') and filled with data, this function ensures
 * that that space will be later flushed through the stream.
 *
 * @param this The stream in which buffer data is being confirmed
 * @param size Size of the confirmed data. Must be not greater than the space
 * reserved but not yet confirmed.
 *
 * @public @memberof BufferedOStream
 */
void BufferedOStream_confirm(BufferedOStream* this, size_t size);

/**
 * @brief Flushes all confirmed data in the buffer through the stream.
 *
 * Empties the buffer afterwards.
 *
 * @param this The stream being flushed
 * @return ERR_OK on success.
 * ERR_SEND on an error in this->write.
 *
 * @public @memberof BufferedOStream
 */
ErrorCode BufferedOStream_flush(BufferedOStream* this);

/**
 * @brief Returns the stream's buffer's size.
 *
 * @param this the stream being investigated
 * @return the maximum size of the buffer (note that the buffer's currently
 * allocated memory may be less than this value).
 *
 * @public @memberof BufferedOStream
 */
size_t BufferedOStream_bufferSize(BufferedOStream* this);

#endif /* BUFFEREDOSTREAM_H */
