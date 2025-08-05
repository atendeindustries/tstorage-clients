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

/** @file Key.h
 *
 * Interface of the TSCLIENT_Key class. Supplements the public API.
 */

#ifndef KEY_H
#define KEY_H

#include <stddef.h>

#include "tstorage-client/client.h"

#include "ErrorCode.h"
#include "BufferedOStream.h"

static inline size_t Key_serializeSize(const TSCLIENT_Key* this)
{
	return sizeof(this->cid)
		   + sizeof(this->mid)
		   + sizeof(this->moid)
		   + sizeof(this->cap)
		   + sizeof(this->acq);
}

/**
 * @brief Writes the Key onto a stream.
 *
 * @param this The Key being written
 * @param output The stream to write the key to
 * @return ERR_OK on success.
 * ERR_LIMIT when the key's size is greater than buffer size.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_SEND on an error in output->write, when flushing data.
 *
 * @public @memberof TSCLIENT_Key
 */
ErrorCode Key_write(const TSCLIENT_Key* this, BufferedOStream* output);

#endif /* KEY_H */
