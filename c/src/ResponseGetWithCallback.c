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

/** @file ResponseGetWithCallback.c
 *
 * Implementation of the ResponseGetWithCallback class.
 */

#include <stddef.h>
#include <string.h>

#include "tstorage-client/client.h"

#include "ResponseGetWithCallback.h"

#include "BufferedIStream.h"
#include "ErrorCode.h"
#include "RecordsSet.h"
#include "ResponseHeader.h"

ErrorCode ResponseGetWithCallback_read(ResponseGetWithCallback* this, BufferedIStream* input, const TSCLIENT_PayloadType* payloadType, TSCLIENT_GetCallback callback, void* userData)
{
	ErrorCode res = ResponseHeader_read(&this->base, input);
	if (res != ERR_OK) {
		return res;
	}

	TSCLIENT_RecordsSet records;
	res = RecordsSet_initialize(&records, payloadType);
	if (res != ERR_OK) {
		return res;
	}

	res = RecordsSet_readWithCallback(&records, input, callback, userData);

	RecordsSet_finalize(&records);

	return res;
}
