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

/** @file RequestGet.c
 *
 * Implementation of the RequestGett class.
 */

#include "RequestGet.h"

#include "BufferedOStream.h"
#include "ErrorCode.h"
#include "KeyRange.h"
#include "MsgType.h"
#include "RequestHeader.h"

static inline size_t serializeSize(const RequestGet* this)
{
	return KeyRange_serializeSize(&this->keyRange);
}

void RequestGet_initializeGet(RequestGet* this, const TSCLIENT_Key* restrict keyMin, const TSCLIENT_Key* restrict keyMax)
{
	this->base.cmd = MSG_GET;
	this->base.size = serializeSize(this);
	this->keyRange.min = keyMin;
	this->keyRange.max = keyMax;
}

void RequestGet_initializeGetAcq(RequestGet* this, const TSCLIENT_Key* restrict keyMin, const TSCLIENT_Key* restrict keyMax)
{
	this->base.cmd = MSG_GETACQ;
	this->base.size = serializeSize(this);
	this->keyRange.min = keyMin;
	this->keyRange.max = keyMax;
}

ErrorCode RequestGet_write(const RequestGet* this, BufferedOStream* output)
{
	ErrorCode res = RequestHeader_write(&this->base, output);
	if (res != ERR_OK) {
		return res;
	}

	return KeyRange_write(&this->keyRange, output);
}
