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

/** @file ResponseGetConfirmation.c
 *
 * Implementation of the ResponseGetConfirmation class.
 */

#include <stddef.h>
#include <string.h>

#include "tstorage-client/client.h"

#include "ResponseGetConfirmation.h"

#include "BufferedIStream.h"
#include "ErrorCode.h"

/**
 * @brief Computes size of a success response.
 */
static size_t sizeSuccess(const ResponseGetConfirmation* this)
{
	return sizeof(this->acq);
}

/**
 * @brief Deserializes a success response.
 */
static void deserializeSuccess(ResponseGetConfirmation* this, const char* src)
{
	memcpy(&this->acq, src, sizeof(this->acq));
}

ErrorCode ResponseGetConfirmation_read(ResponseGetConfirmation* this, BufferedIStream* input)
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
