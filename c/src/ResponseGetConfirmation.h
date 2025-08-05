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

/** @file ResponseGetConfirmation.h
 *
 * Interface of the ResponseGetConfirmation class.
 */

#ifndef RESPONSEGETCONFIRMATION_H
#define RESPONSEGETCONFIRMATION_H

#include <stdint.h>

#include "BufferedIStream.h"
#include "ErrorCode.h"
#include "ResponseHeader.h"

/**
 * @brief A TStorage Get Confirmation and GetAcq response.
 *
 * It is read from a stream and deserialized.
 */
typedef struct ResponseGetConfirmation
{
	ResponseHeader base;
	int64_t acq;
} ResponseGetConfirmation;

/**
 * @brief Reads a ResponseGetConfirmation from a stream.
 *
 * @param this The ResponseGetConfirmation being read
 * @param input The stream to read the data from
 * @return ERR_OK on success.
 * ERR_LIMIT when the read data's size is greater than buffer size.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_RECEIVE on an error in input->read.
 * ERR_UNEXPECTED on malformed response.
 *
 * @public @memberof ResponseGetConfirmation
 */
ErrorCode ResponseGetConfirmation_read(ResponseGetConfirmation* this, BufferedIStream* input);

#endif /* RESPONSEGETCONFIRMATION_H */
