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

/** @file ResponseHeader.c
 *
 * Implementation of the ResponseHeader class.
 */

#include <stddef.h>
#include <string.h>

#include "ResponseHeader.h"

#include "BufferedIStream.h"
#include "ErrorCode.h"

/**
 * @brief Computes size of the response header for deserialization.
 */
static size_t headerSize(const ResponseHeader* this)
{
	return sizeof(this->result) + sizeof(this->size);
}

/**
 * @brief Deserializes the response header.
 */
static void deserialize(ResponseHeader* this, const char* src)
{
	memcpy(&this->result, src, sizeof(this->result));
	memcpy(&this->size, src += sizeof(this->result), sizeof(this->size));
}

ErrorCode ResponseHeader_read(ResponseHeader* this, BufferedIStream* input)
{
	char* buf;
	ErrorCode res = BufferedIStream_reserve(input, &buf, headerSize(this));
	if (res != ERR_OK) {
		return res;
	}

	deserialize(this, buf);

	return BufferedIStream_reserve(input, &this->data, this->size);
}
