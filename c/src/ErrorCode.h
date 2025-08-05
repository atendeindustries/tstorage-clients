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

/** @file ErrorCode.h
 *
 * Values to use as return values of functions.
 *
 * The purpose of this header is to simplify code dependencies: we avoid
 * including "tstorage-client/client.h" in all sources that use error codes.
 */

#ifndef ERRORCODE_H
#define ERRORCODE_H

#include "tstorage-client/client.h"

/**
 * @brief Return values of functions that are not part of the public API.
 *
 * Caution: Some ErrorCode values are passed verbatim as results of public API
 * functions that return TSCLIENT_ResponseStatus. Maintain compatibility
 * between ErrorCode and TSCLIENT_ResponseStatus.
 */
typedef enum ErrorCode {
	ERR_OK = TSCLIENT_RES_OK,
	ERR_INVALID = TSCLIENT_ERR_INVALID,
	ERR_LIMIT = TSCLIENT_ERR_MEMORYLIMIT,
	ERR_RESOURCE = TSCLIENT_ERR_RESOURCE,
	ERR_RECEIVE = TSCLIENT_ERR_RECEIVE,
	ERR_SEND = TSCLIENT_ERR_SEND,
	ERR_UNEXPECTED = TSCLIENT_ERR_UNEXPECTED
} ErrorCode;

#endif /* ERRORCODE_H */
