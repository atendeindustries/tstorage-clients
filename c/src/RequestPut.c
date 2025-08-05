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

/** @file RequestPut.c
 *
 * Implementation of the RequestPut class.
 */

#include "RequestPut.h"

#include "BufferedOStream.h"
#include "ErrorCode.h"
#include "MsgType.h"
#include "RecordsSet.h"
#include "RequestHeader.h"

void RequestPut_initializePutSafe(RequestPut* this, const TSCLIENT_RecordsSet* records)
{
	this->base.cmd = MSG_PUTSAFE;
	this->base.size = 0;
	this->records = records;
}

void RequestPut_initializePutASafe(RequestPut* this, const TSCLIENT_RecordsSet* records)
{
	this->base.cmd = MSG_PUTASAFE;
	this->base.size = 0;
	this->records = records;
}

ErrorCode RequestPut_write(const RequestPut* this, BufferedOStream* output, RecordsSet_write_fun recordsSetSerializeFun)
{
	ErrorCode res = RequestHeader_write(&this->base, output);
	if (res != ERR_OK) {
		return res;
	}

	return recordsSetSerializeFun(this->records, output);
}
