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

/** @file ResponsePut.c
 *
 * Implementation of the ResponsePut class.
 */

#include <stddef.h>
#include <string.h>

#include "tstorage-client/client.h"

#include "ResponsePut.h"

#include "BufferedIStream.h"
#include "ErrorCode.h"
#include "ResponseHeader.h"

/**
 * @brief Computes size of a success response.
 */
static size_t sizeSuccess(const ResponsePut* this)
{
	return sizeof(this->acqMin) + sizeof(this->acqMax);
}

/**
 * @brief Deserializes a success response.
 */
static void deserializeSuccess(ResponsePut* this, const char* src)
{
	memcpy(&this->acqMin, src, sizeof(this->acqMin));
	memcpy(&this->acqMax, src += sizeof(this->acqMin), sizeof(this->acqMax));
}

ErrorCode ResponsePut_read(ResponsePut* this, BufferedIStream* input)
{
	ErrorCode res = ResponseHeader_read(&this->base, input);
	if (res != ERR_OK) {
		return res;
	}

	if ((TSCLIENT_ResponseStatus)this->base.result == TSCLIENT_RES_OK) {
		if (this->base.size < sizeSuccess(this)) {
			/* Received malformed data! */
			return ERR_UNEXPECTED;
		}

		deserializeSuccess(this, this->base.data);
	}

	return ERR_OK;
}
