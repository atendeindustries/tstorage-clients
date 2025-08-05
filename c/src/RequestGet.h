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

/** @file RequestGet.h
 *
 * Interface of the RequestGet class.
 */

#ifndef REQUESTGET_H
#define REQUESTGET_H

#include "tstorage-client/client.h"

#include "BufferedOStream.h"
#include "ErrorCode.h"
#include "KeyRange.h"
#include "RequestHeader.h"

/**
 * @brief A TStorage Get and GetAcq request.
 *
 * It is serialized and written to a stream.
 */
typedef struct RequestGet
{
	RequestHeader base;
	KeyRange keyRange;
} RequestGet;

void RequestGet_initializeGet(RequestGet* this, const TSCLIENT_Key* restrict keyMin, const TSCLIENT_Key* restrict keyMax);

void RequestGet_initializeGetAcq(RequestGet* this, const TSCLIENT_Key* restrict keyMin, const TSCLIENT_Key* restrict keyMax);

/**
 * @brief Writes the RequestGet onto a stream.
 *
 * @param this The RequestGet being written
 * @param output The stream to write the records to
 * @return ERR_OK on success.
 * ERR_LIMIT when the request's size is greater than buffer size.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_SEND on an error in output->write, when flushing data.
 *
 * @public @memberof RequestGet
 */
ErrorCode RequestGet_write(const RequestGet* this, BufferedOStream* output);

#endif /* REQUESTGET_H */
