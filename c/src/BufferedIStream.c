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

/** @file BufferedIStream.c
 *
 * Implementation of the BufferedIStream class.
 */

#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "BufferedIStream.h"

#include "DynamicBuffer.h"
#include "ErrorCode.h"

void BufferedIStream_initialize(BufferedIStream* this, DynamicBuffer* buffer, void* stream, BufferedIStream_read_fun readFun)
{
	this->buffer = buffer;
	this->stream = stream;
	this->read = readFun;
	BufferedIStream_reset(this);
}

void BufferedIStream_finalize(BufferedIStream* this)
{
}

void BufferedIStream_reset(BufferedIStream* this)
{
	this->readPos = this->reservePos = 0;
}

/**
 * @brief Returns size of free buffer space (i.e. not filled with read data).
 *
 * @param this The stream
 * @return Size of the free space.
 *
 * @public @memberof BufferedOStream
 */
static size_t sizeUnused(const BufferedIStream* this)
{
	assert(this->buffer->size >= this->readPos);
	return this->buffer->size - this->readPos;
}

/**
 * @brief Returns size of free buffer space if the buffer had maximum size.
 *
 * Comapare 'sizeUnused'.
 *
 * @param this The stream
 * @return Size of the free space.
 *
 * @public @memberof BufferedOStream
 */
static size_t maxSizeUnused(const BufferedIStream* this)
{
	assert(this->buffer->maxSize >= this->readPos);
	return this->buffer->maxSize - this->readPos;
}

ErrorCode BufferedIStream_reserve(BufferedIStream* this, char** buffer, size_t size)
{
	assert(this->buffer->maxSize >= this->buffer->size);
	assert(this->buffer->size >= this->readPos);
	assert(this->readPos >= this->reservePos);

	const size_t sizeAhead = this->readPos - this->reservePos;
	if (size > sizeAhead) {
		const size_t sizeToRead = size - sizeAhead;

		size_t freeSize = sizeUnused(this);
		if (sizeToRead > freeSize) {
			/* Enlarge the buffer. */
			if (sizeToRead > maxSizeUnused(this)) {
				/* Reservation too large even if the buffer's size would be maxed out */
				return ERR_LIMIT;
			}

			const size_t newSize = this->readPos + sizeToRead;
			ErrorCode res = DynamicBuffer_resize(this->buffer, newSize);
			if (res != ERR_OK) {
				return res;
			}

			freeSize = sizeUnused(this);
		}

		size_t sizeRead = this->read(this->stream, this->buffer->storage + this->readPos, sizeToRead, freeSize);

		if (sizeRead < sizeToRead) {
			return ERR_RECEIVE;
		}

		this->readPos += sizeRead;
	}

	assert(this->readPos <= this->buffer->size);
	assert(size <= this->readPos - this->reservePos);
	*buffer = this->buffer->storage + this->reservePos;
	this->reservePos += size;

	return ERR_OK;
}

void BufferedIStream_confirm(BufferedIStream* this)
{
	/* Is there data in the buffer that's already read but not reserved yet? */
	size_t remaining = this->readPos - this->reservePos;
	if (remaining > 0) {
		/* Shift the buffer's contents to its beginning. */
		memmove(this->buffer->storage, this->buffer->storage + this->reservePos, remaining);
	}
	this->reservePos = 0;
	this->readPos = remaining;
}
