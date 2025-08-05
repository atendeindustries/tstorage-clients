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

/** @file RequestPut.h
 *
 * Interface of the RequestPut class.
 */

#ifndef REQUESTPUT_H
#define REQUESTPUT_H

#include "tstorage-client/client.h"

#include "BufferedOStream.h"
#include "ErrorCode.h"
#include "RecordsSet.h"
#include "RequestHeader.h"

/**
 * @brief A TStorage Put and PutA request.
 *
 * It is serialized and written to a stream.
 */
typedef struct RequestPut
{
	RequestHeader base;
	const TSCLIENT_RecordsSet* records;
} RequestPut;

void RequestPut_initializePutSafe(RequestPut* this, const TSCLIENT_RecordsSet* records);

void RequestPut_initializePutASafe(RequestPut* this, const TSCLIENT_RecordsSet* records);

/**
 * @brief Writes the RequestPut onto a stream.
 *
 * @param this The RequestPut being written
 * @param output The stream to write the records to
 * @param recordsSetSerializeFun function that serializes this->records
 * @return ERR_OK on success.
 * ERR_LIMIT when the request's size is greater than buffer size.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_SEND on an error in output->write, when flushing data.
 *
 * @public @memberof RequestPut
 */
ErrorCode RequestPut_write(const RequestPut* this, BufferedOStream* output, RecordsSet_write_fun recordsSetSerializeFun);

#endif /* REQUESTPUT_H */
