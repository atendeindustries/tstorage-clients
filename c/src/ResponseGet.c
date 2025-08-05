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

/** @file ResponseGet.c
 *
 * Implementation of the ResponseGet class.
 */

#include <stddef.h>
#include <string.h>

#include "tstorage-client/client.h"

#include "ResponseGet.h"

#include "BufferedIStream.h"
#include "ErrorCode.h"
#include "RecordsSet.h"
#include "ResponseHeader.h"

ErrorCode ResponseGet_read(ResponseGet* this, BufferedIStream* input, const TSCLIENT_PayloadType* payloadType)
{
	ErrorCode res = ResponseHeader_read(&this->base, input);
	if (res != ERR_OK) {
		return res;
	}

	this->records = TSCLIENT_RecordsSet_new(payloadType);
	if (this->records == NULL) {
		return ERR_RESOURCE;
	}

	res = RecordsSet_read(this->records, input);
	if (res != ERR_OK) {
		/* It's nicer to not pass an empty RecordsSet to user. */
		if (TSCLIENT_RecordsSet_size(this->records) == 0) {
			TSCLIENT_RecordsSet_destroy(this->records);
			this->records = NULL;
		}
	}

	return res;
}
