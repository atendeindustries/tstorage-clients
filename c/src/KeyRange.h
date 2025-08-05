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

/** @file KeyRange.h
 *
 * Interface of the KeyRange class.
 */

#ifndef KEYRANGE_H
#define KEYRANGE_H

#include <stddef.h>

#include "tstorage-client/client.h"

#include "BufferedOStream.h"
#include "ErrorCode.h"
#include "Key.h"

/**
 * @brief A key range, the range bound by two keys.
 *
 * It is serialized and written to a stream.
 */
typedef struct KeyRange
{
	const TSCLIENT_Key* min;
	const TSCLIENT_Key* max;
} KeyRange;

static inline size_t KeyRange_serializeSize(const KeyRange* this)
{
	return Key_serializeSize(this->min) + Key_serializeSize(this->max);
}

/**
 * @brief Writes the KeyRange onto a stream.
 *
 * @param this The KeyRange being written
 * @param output The stream to write the KeyRange to
 * @return ERR_OK on success.
 * ERR_LIMIT when the KeyRange's size is greater than buffer size.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_SEND on an error in output->write, when flushing data.
 *
 * @public @memberof KeyRange
 */
ErrorCode KeyRange_write(const KeyRange* this, BufferedOStream* output);

#endif /* KEYRANGE_H */
