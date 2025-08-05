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

/** @file Record.c
 *
 * Implementation of the TSCLIENT_Record class.
 */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "tstorage-client/client.h"

#include "Record.h"

#include "ErrorCode.h"

/**
 * @brief Serialize the record's payload into buffer, leaving space for the
 * header.
 *
 * If the serialization would need space greater than 'size', the function
 * does not do the serialization, and returns the needed size. This allows the
 * caller to enlarge the buffer and call this function again.
 *
 * @param this The Record whose payload to serialize
 * @param[out] buffer The buffer to which to serialize the payload
 * @param size Size of the buffer in bytes
 * @param payloadType defines the record's type of payload
 * @param headerSize size of the header in bytes. Serialized payload will
 * start at 'buffer + headerSize'
 * @return Size of 'this' (whole record, not only payload!) after
 * serialization, i.e. length from the beginning of 'buffer' to the end of
 * serialized payload. It is guaranteed that
 * headerSize <= the value <= INT32_MAX.
 */
static size_t serializePayload(const TSCLIENT_Record* this, char* buffer, size_t size, const TSCLIENT_PayloadType* payloadType, size_t headerSize)
{
	/* First try serializing payload, and if succeeded, then serialize header. */
	const void* payload = (const char*)this + payloadType->offset;
	char* payloadBuffer = buffer + headerSize;

	/* 'size' may be not enough to hold even the header. */
	size_t payloadMaxSize = size < headerSize ? 0 : size - headerSize;

	size_t payloadSize = payloadType->toBytes(payload, payloadBuffer, payloadMaxSize);

	/* As required by TSCLIENT_PayloadType.toBytes: */
	assert(payloadSize <= TSCLIENT_PAYLOAD_SIZE_MAX
		   && "User's PayloadType.toBytes return value is greater than TSCLIENT_PAYLOAD_SIZE_MAX");

	size_t recSize = headerSize + payloadSize;
	assert(recSize >= headerSize);
	assert(recSize <= (size_t)INT32_MAX);
	return recSize;
}

/**
 * @brief Serializes the Record into a buffer of given size, in PUTSAFE or PUTASAFE format.
 *
 * See comments for Record_serializePutSafe and Record_serializePutASafe for
 * the formats of serialization.
 *
 * @param this The Record being serialized
 * @param[out] buffer The buffer to which to serialize
 * @param size Size of 'buffer' in bytes. May not be greater than INT32_MAX+4.
 * @param payloadType Defines the record's type of payload
 * @param withAcq 1 means PUTASAFE format, 0 means PUTSAFE.
 * @return Actual size of 'this' after serialization (always <= INT32_MAX).
 * If the value is greater than 'size', the serialization failed and the
 * caller should enlarge the buffer's size to at least the returned value and
 * try again.
 *
 * @private @memberof TSCLIENT_Record
 */
static size_t commonSerializePutSafe(const TSCLIENT_Record* this, char* buffer, size_t size, const TSCLIENT_PayloadType* payloadType, int withAcq)
{
	/* Header is recSize + mid + moid + cap + (optionally) acq */
	const size_t headerSize = sizeof(int32_t) + sizeof(int64_t) + sizeof(int32_t) + sizeof(int64_t)
							  + (withAcq ? sizeof(int64_t) : 0);

	const size_t recSize = serializePayload(this, buffer, size, payloadType, headerSize);

	if (recSize <= size) {
		/* Now it makes sense to serialize header. */
		const int32_t recSize32 = (int32_t)(recSize - sizeof(int32_t)); /* minus the recSize field itself */
		memcpy(buffer, &recSize32, sizeof(recSize32));
		memcpy(buffer += sizeof(recSize32), &this->key.mid, sizeof(this->key.mid));
		memcpy(buffer += sizeof(this->key.mid), &this->key.moid, sizeof(this->key.moid));
		memcpy(buffer += sizeof(this->key.moid), &this->key.cap, sizeof(this->key.cap));
		if (withAcq) {
			memcpy(buffer += sizeof(this->key.cap), &this->key.acq, sizeof(this->key.acq));
		}
	}

	assert(recSize <= (size_t)INT32_MAX);
	return recSize;
}

size_t Record_serializePutSafe(const TSCLIENT_Record* this, char* buffer, size_t size, const TSCLIENT_PayloadType* payloadType)
{
	return commonSerializePutSafe(this, buffer, size, payloadType, 0);
}

size_t Record_serializePutASafe(const TSCLIENT_Record* this, char* buffer, size_t size, const TSCLIENT_PayloadType* payloadType)
{
	return commonSerializePutSafe(this, buffer, size, payloadType, 1);
}

ErrorCode Record_deserialize(TSCLIENT_Record* this, const char* buffer, size_t size, const TSCLIENT_PayloadType* payloadType)
{
	static const size_t keySize = sizeof(this->key.cid) + sizeof(this->key.mid) + sizeof(this->key.moid) + sizeof(this->key.cap) + sizeof(this->key.acq);
	if (size < keySize) {
		/* Not enough data to deserialize the record's key. */
		return ERR_UNEXPECTED;
	}

	memcpy(&this->key.cid, buffer, sizeof(this->key.cid));
	memcpy(&this->key.mid, buffer += sizeof(this->key.cid), sizeof(this->key.mid));
	memcpy(&this->key.moid, buffer += sizeof(this->key.mid), sizeof(this->key.moid));
	memcpy(&this->key.cap, buffer += sizeof(this->key.moid), sizeof(this->key.cap));
	memcpy(&this->key.acq, buffer += sizeof(this->key.cap), sizeof(this->key.acq));

	size_t payloadSize = size - keySize;
	void* payload = (char*)this + payloadType->offset;
	if (payloadType->fromBytes(payload, buffer += sizeof(this->key.acq), payloadSize) != 0) {
		return ERR_INVALID;
	}

	return ERR_OK;
}
