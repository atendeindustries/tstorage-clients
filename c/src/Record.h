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

/** @file Record.h
 *
 * Interface of the TSCLIENT_Record class. Supplements the public API.
 */

#ifndef RECORD_H
#define RECORD_H

#include <stddef.h>
#include <stdint.h>

#include "tstorage-client/client.h"

#include "ErrorCode.h"

typedef size_t (*Record_serialize_fun)(const TSCLIENT_Record* this, char* buffer, size_t size, const TSCLIENT_PayloadType* payloadType);

/**
 * @brief Serializes the Record into a buffer of given size, in PUTSAFE format.
 *
 * If the serialization would need space greater than 'size', the function
 * does not do the serialization, and returns the needed size. This allows the
 * caller to enlarge the buffer and call this function again.
 *
 * In the PUTSAFE format, a Record is serialized as:
 * 1. int32_t recSize - size of entries 2..5 below, in bytes
 * 2. int64_t mid
 * 3. int32_t moid
 * 4. int64_t cap
 * 5. char* payload
 *
 * @param this The Record being serialized
 * @param[out] buffer The buffer to which to serialize
 * @param size Size of 'buffer' in bytes. May not be greater than INT32_MAX+4.
 * @param payloadType Defines the record's type of payload
 * @return Actual size of 'this' after serialization (always <= INT32_MAX).
 * If the value is greater than 'size', the serialization failed and the
 * caller should enlarge the buffer's size to at least the returned value and
 * try again.
 *
 * @public @memberof TSCLIENT_Record
 */
size_t Record_serializePutSafe(const TSCLIENT_Record* this, char* buffer, size_t size, const TSCLIENT_PayloadType* payloadType);

/**
 * @brief Serializes the Record into a buffer of given size, in PUTSAFE format.
 *
 * If the serialization would need space greater than 'size', the function
 * does not do the serialization, and returns the needed size. This allows the
 * caller to enlarge the buffer and call this function again.
 *
 * In the PUTSAFE format, a Record is serialized as:
 * 1. int32_t recSize - size of entries 2..6 below, in bytes
 * 2. int64_t mid
 * 3. int32_t moid
 * 4. int64_t cap
 * 5. int64_t acq
 * 6. char* payload
 *
 * @param this The Record being serialized
 * @param[out] buffer The buffer to which to serialize
 * @param size Size of 'buffer' in bytes. May not be greater than INT32_MAX+4.
 * @param payloadType Defines the record's type of payload
 * @return Actual size of 'this' after serialization (always <= INT32_MAX).
 * If the value is greater than 'size', the serialization failed and the
 * caller should enlarge the buffer's size to at least the returned value and
 * try again.
 *
 * @public @memberof TSCLIENT_Record
 */
size_t Record_serializePutASafe(const TSCLIENT_Record* this, char* buffer, size_t size, const TSCLIENT_PayloadType* payloadType);

/**
 * @brief Deserializes the Record from a buffer of given size.
 *
 * @param this The Record being serialized
 * @param[in] buffer The buffer from which to deserialize
 * @param size Size of 'buffer' in bytes.
 * @param payloadType Defines the record's type of payload
 * @return ERR_OK on success
 * ERR_INVALID when deserialization of a record failed
 * ERR_UNEXPECTED on malformed data
 *
 * @public @memberof TSCLIENT_Record
 */
ErrorCode Record_deserialize(TSCLIENT_Record* this, const char* buffer, size_t size, const TSCLIENT_PayloadType* payloadType);

#endif /* RECORD_H */
