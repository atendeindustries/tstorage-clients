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

/** @file RecordsSet.h
 *
 * Interface of the TSClient_Channel class. Supplements the public API.
 */

#ifndef CHANNEL_H
#define CHANNEL_H

#include "tstorage-client/client.h"

#include "BufferedIStream.h"
#include "BufferedOStream.h"
#include "DynamicBuffer.h"
#include "Socket.h"

struct TSCLIENT_Channel
{
	char* host;
	int port;
	TSCLIENT_PayloadType payloadType;
	DynamicBuffer buffer;
	Socket sock;
	BufferedIStream input;
	BufferedOStream output;
};

#endif /* CHANNEL_H */
