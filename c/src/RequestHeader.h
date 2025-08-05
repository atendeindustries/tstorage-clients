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

/** @file RequestHeader.h
 *
 * Interface of the RequestHeader class.
 */

#ifndef REQUESTHEADER_H
#define REQUESTHEADER_H

#include <stdint.h>

#include "BufferedOStream.h"
#include "ErrorCode.h"

/**
 * @brief A header of each TStorage request.
 *
 * It is serialized and written to a stream.
 */
typedef struct RequestHeader
{
	int32_t cmd;
	uint64_t size;
} RequestHeader;

/**
 * @brief Writes the RequestHeader onto a stream.
 *
 * @param this The RequestHeader being written
 * @param output The stream to write the records to
 * @return ERR_OK on success.
 * ERR_LIMIT when the header's size is greater than buffer size.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_SEND on an error in output->write, when flushing data.
 *
 * @public @memberof RequestHeader
 */
ErrorCode RequestHeader_write(const RequestHeader* this, BufferedOStream* output);

#endif /* REQUESTHEADER_H */
