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

/** @file RecordsSet.c
 *
 * Implementation of the TSCLIENT_Key class.
 */

#include <string.h>

#include "tstorage-client/client.h"

#include "Key.h"

#include "BufferedOStream.h"
#include "ErrorCode.h"

/**
 * @brief Serializes the Key into *dest. dest must be large enough.
 *
 * @param this The Key being serialized
 * @param dest[out] The buffer for deserializartion
 */
static void serialize(const TSCLIENT_Key* this, char* dest)
{
	memcpy(dest, &this->cid, sizeof(this->cid));
	memcpy(dest += sizeof(this->cid), &this->mid, sizeof(this->mid));
	memcpy(dest += sizeof(this->mid), &this->moid, sizeof(this->moid));
	memcpy(dest += sizeof(this->moid), &this->cap, sizeof(this->cap));
	memcpy(dest += sizeof(this->cap), &this->acq, sizeof(this->acq));
}

ErrorCode Key_write(const TSCLIENT_Key* this, BufferedOStream* output)
{
	size_t serSize = Key_serializeSize(this);

	char* buf;
	ErrorCode res = BufferedOStream_reserveFlushing(output, &buf, serSize);
	if (res != ERR_OK) {
		return res;
	}

	serialize(this, buf);

	BufferedOStream_confirm(output, serSize);

	return ERR_OK;
}
