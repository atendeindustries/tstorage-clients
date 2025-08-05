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

/** @file ResponseGetWithCallback.h
 *
 * Interface of the ResponseGetWithCallback class.
 */

#ifndef RESPONSEGETWITHCALLBACK_H
#define RESPONSEGETWITHCALLBACK_H

#include "tstorage-client/client.h"

#include "BufferedIStream.h"
#include "ErrorCode.h"
#include "ResponseHeader.h"

/**
 * @brief A TStorage Get response that calls a callback on records.
 *
 * It is read from a stream and deserialized. When deserializing, it calls
 * a callback function on each received RedcordsSet.
 */
typedef struct ResponseGetWithCallback
{
	ResponseHeader base;
} ResponseGetWithCallback;

/**
 * @brief Reads a ResponseGetWithCallback from a stream.
 *
 * The function creates RecordsSet objects when records are read from the
 * stream, and calls 'callback' for each RecordsSet. (It later destroys these
 * RecordsSets, don't worry about them.)
 *
 * @param this The ResponseGetWithCallback being read.
 * @param input The stream to read the data from
 * @param payloadType payload type to use when creating the resulting RecordSet
 * on this->records
 * @param[in] callback a function to call for each retrieved RecordsSet
 * @param userData a user-provided data, passed verbatim to 'callback'
 * @return ERR_OK on success.
 * In case of failure, 'callback' will be called only on records read so far.
 * Possible failures:
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_INVALID when deserialization of a record failed.
 * ERR_LIMIT when a read record's size is greater than buffer size.
 * ERR_RECEIVE on an error in input->read.
 * ERR_UNEXPECTED on malformed response.
 *
 * @public @memberof ResponseGetWithCallback
 */
ErrorCode ResponseGetWithCallback_read(ResponseGetWithCallback* this, BufferedIStream* input, const TSCLIENT_PayloadType* payloadType, TSCLIENT_GetCallback callback, void* userData);

#endif /* RESPONSEGETWITHCALLBACK_H */
