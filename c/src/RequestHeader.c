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

/** @file RequestHeader.c
 *
 * Implementation of the RequestHeader class.
 */

#include <stddef.h>
#include <string.h>

#include "RequestHeader.h"

#include "BufferedOStream.h"
#include "ErrorCode.h"

/**
 * @brief Returns size needed to serialize the RequestHeader.
 *
 * @param this The RequestHeader being serialized
 * @return 'this' size when serialized
 */
static inline size_t serializeSize(const RequestHeader* this)
{
	return sizeof(this->cmd) + sizeof(this->size);
}

/**
 * @brief Serializes the RequestHeader into *dest.
 *
 * Does not check bounds! Call 'serializeSize' before and ensure
 * 'dest' is large enough.
 *
 * @param this The RequestHeader being serialized
 * @param dest[out] The buffer for deserializartion
 */
static void serialize(const RequestHeader* this, char* dest)
{
	memcpy(dest, &this->cmd, sizeof(this->cmd));
	memcpy(dest += sizeof(this->cmd), &this->size, sizeof(this->size));
}

ErrorCode RequestHeader_write(const RequestHeader* this, BufferedOStream* output)
{
	size_t serSize = serializeSize(this);

	char* buf;
	ErrorCode res = BufferedOStream_reserveFlushing(output, &buf, serSize);
	if (res != ERR_OK) {
		return res;
	}

	serialize(this, buf);

	BufferedOStream_confirm(output, serSize);

	return ERR_OK;
}
