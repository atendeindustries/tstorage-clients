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

/** @file BufferedOStream.c
 *
 * Implementation of the BufferedOStream class.
 */

#include <assert.h>
#include <stddef.h>

#include "BufferedOStream.h"

#include "DynamicBuffer.h"
#include "ErrorCode.h"

void BufferedOStream_initialize(BufferedOStream* this, DynamicBuffer* buffer, void* stream, BufferedOStream_write_fun writeFun)
{
	this->buffer = buffer;
	this->stream = stream;
	this->write = writeFun;
	BufferedOStream_reset(this);
}

void BufferedOStream_finalize(BufferedOStream* this)
{
}

void BufferedOStream_reset(BufferedOStream* this)
{
	this->reservePos = 0;
}

size_t BufferedOStream_sizeReserved(const BufferedOStream* this)
{
	assert(this->buffer->size >= this->reservePos);
	return this->buffer->size - this->reservePos;
}

ErrorCode BufferedOStream_reserve(BufferedOStream* this, char** buffer, size_t size)
{
	assert(this->buffer->maxSize >= this->buffer->size);
	assert(this->buffer->size >= this->reservePos);

	const size_t freeSize = BufferedOStream_sizeReserved(this);
	if (size > freeSize) {
		if (size > SIZE_MAX - this->reservePos) {
			return ERR_LIMIT;
		}

		const size_t newSize = this->reservePos + size;
		ErrorCode res = DynamicBuffer_resize(this->buffer, newSize);
		if (res != ERR_OK) {
			return res;
		}
	}

	assert(size <= BufferedOStream_sizeReserved(this));

	*buffer = this->buffer->storage + this->reservePos;
	return ERR_OK;
}

ErrorCode BufferedOStream_reserveFlushing(BufferedOStream* this, char** buffer, size_t size)
{
	ErrorCode res = BufferedOStream_reserve(this, buffer, size);
	if (res == ERR_LIMIT) {
		res = BufferedOStream_flush(this);
		if (res != ERR_OK) {
			return res;
		}
		res = BufferedOStream_reserve(this, buffer, size);
	}

	return res;
}

void BufferedOStream_confirm(BufferedOStream* this, size_t size)
{
	assert(size <= BufferedOStream_sizeReserved(this));
	this->reservePos += size;
}

ErrorCode BufferedOStream_flush(BufferedOStream* this)
{
	ErrorCode res = ERR_OK;
	if (this->reservePos > 0) {
		res = this->write(this->stream, this->buffer->storage, this->reservePos);
		this->reservePos = 0;
	}

	return res;
}

size_t BufferedOStream_bufferSize(BufferedOStream* this)
{
	return this->buffer->maxSize;
}
