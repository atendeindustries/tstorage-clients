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

/** @file ResponseGet.h
 *
 * Interface of the ResponseGet class.
 */

#ifndef RESPONSEGET_H
#define RESPONSEGET_H

#include "tstorage-client/client.h"

#include "BufferedIStream.h"
#include "ErrorCode.h"
#include "ResponseHeader.h"

/**
 * @brief A TStorage Get response.
 *
 * It is read from a stream and deserialized.
 */
typedef struct ResponseGet
{
	ResponseHeader base;
	TSCLIENT_RecordsSet* records;
} ResponseGet;

/**
 * @brief Reads a ResponseGet from a stream.
 *
 * The function sets this->records to NULL or to a new RecordsSet - the user
 * becomes its owner and must destroy it afterwards.
 *
 * @param this The ResponseGet being read.
 * @param input The stream to read the data from
 * @param payloadType payload type to use when creating the resulting RecordSet
 * on this->records
 * @return ERR_OK on success. In case of failure, this->records will be NULL,
 * or will contain records deserialied so far.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_INVALID when deserialization of a record failed.
 * ERR_LIMIT when the read data's size is greater than buffer size.
 * ERR_RECEIVE on an error in input->read.
 * ERR_UNEXPECTED on malformed response.
 *
 * @public @memberof ResponseGet
 */
ErrorCode ResponseGet_read(ResponseGet* this, BufferedIStream* input, const TSCLIENT_PayloadType* payloadType);

#endif /* RESPONSEGET_H */
