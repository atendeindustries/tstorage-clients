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

/** @file ResponseHeader.h
 *
 * Interface of the ResponseHeader class.
 */

#ifndef RESPONSEHEADER_H
#define RESPONSEHEADER_H

#include <stdint.h>

#include "BufferedIStream.h"
#include "ErrorCode.h"

/**
 * @brief A header of each TStorage response.
 *
 * It is read from a stream and deserialized.
 */
typedef struct ResponseHeader
{
	int32_t result;
	uint64_t size;
	char* data;
} ResponseHeader;

/**
 * @brief Reads a ResponseHeader from a stream.
 *
 * this->data will point to a proper buffer only on success.
 *
 * @param this The ResponseHeader being read
 * @param input The stream to receive the data from
 * @return ERR_OK on success.
 * ERR_LIMIT when the received data's size is greater than buffer size.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_RECEIVE on an error in input->read.
 *
 * @public @memberof ResponseHeader
 */
ErrorCode ResponseHeader_read(ResponseHeader* this, BufferedIStream* input);

#endif /* RESPONSEHEADER_H */
