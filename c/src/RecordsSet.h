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
 * Interface of the TSCLIENT_RecordsSet class. Supplements the public API.
 */

#ifndef RECORDSSET_H
#define RECORDSSET_H

#include "tstorage-client/client.h"

#include "BufferedIStream.h"
#include "BufferedOStream.h"
#include "DynamicArray.h"
#include "ErrorCode.h"

/**
 * @brief
 *
 * A dynamically expandable set of TSCLIENT_Record objects.
 */
struct TSCLIENT_RecordsSet
{
	TSCLIENT_PayloadType payloadType;
	DynamicArray array;
};

/**
 * @brief Initializes a RecordsSet.
 *
 * @param this The RecordsSet being intialized
 * @param payloadType defines type of payload carried by records stored in
 * this RecordsSet
 * @return ERR_OK on success. On failure, returns ERR_RESOURCE and sets errno(3)
 * to any of the errors specified for malloc(3).
 *
 * @public @memberof TSCLIENT_RecordsSet
 */
ErrorCode RecordsSet_initialize(TSCLIENT_RecordsSet* this, const TSCLIENT_PayloadType* payloadType);

/**
 * @brief Frees the RecordsSet previously initialized.
 *
 * @param this The RecordsSet being finalized.
 *
 * @public @memberof TSCLIENT_RecordsSet
 */
void RecordsSet_finalize(TSCLIENT_RecordsSet* this);

typedef ErrorCode (*RecordsSet_write_fun)(const TSCLIENT_RecordsSet* this, BufferedOStream* output);

/**
 * @brief Writes the RecordsSet to a stream, in PUTSAFE format.
 *
 * In the PUTSAFE format, each batch comprises a sequence of Records with the
 * same cid. A batch is serialized as:
 * 1. int32_t cid - the cid for all records in the batch, -1 marks end
 * 2. int32_t batchSize - size of entries 3..n below, in bytes
 * 3. Record
 * ... Record
 * n. Record
 *
 * Each Record above is serialized as:
 * 1. int32_t recSize - size of entries 2..5 below, in bytes
 * 2. int64_t mid
 * 3. int32_t moid
 * 4. int64_t cap
 * 5. char* payload
 *
 * The last batch has 1. cid = -1 to mark end of transmission.
 *
 * The records in 'this' are not sorted before serializing. To minimize the
 * number of batches, the caller should sort the RecordsSet so that the
 * records are grouped by the same cid.
 *
 * @param this The RecordsSet being written
 * @param output The stream to write the records to
 * @return ERR_OK on success.
 * ERR_LIMIT when a record's size is greater than buffer size; record batches
 * may be written incomplete, the cid = -1 end marker is not written.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_SEND on an error in output->write, when flushing data.
 *
 * @public @memberof TSCLIENT_RecordsSet
 */
ErrorCode RecordsSet_writePutSafe(const TSCLIENT_RecordsSet* this, BufferedOStream* output);

/**
 * @brief Writes the RecordsSet to a stream, in PUTASAFE format.
 *
 * In the PUTASAFE format, each batch comprises a sequence of Records with the
 * same cid. A batch is serialized as:
 * 1. int32_t cid - the cid for all records in the batch, -1 marks end
 * 2. int32_t batchSize - size of entries 3..n below, in bytes
 * 3. Record
 * ... Record
 * n. Record
 *
 * Each Record above is serialized as:
 * 1. int32_t recSize - size of entries 2..6 below, in bytes
 * 2. int64_t mid
 * 3. int32_t moid
 * 4. int64_t cap
 * 5. int64_t acq
 * 6. char* payload
 *
 * The last batch has 1. cid = -1 to mark end of transmission.
 *
 * The records in 'this' are not sorted before serializing. To minimize the
 * number of batches, the caller should sort the RecordsSet so that the
 * records are grouped by the same cid.
 *
 * @param this The RecordsSet being written
 * @param output The stream to write the records to
 * @return ERR_OK on success.
 * ERR_LIMIT when a record's size is greater than buffer size; record batches
 * may be written incomplete, the cid = -1 end marker is not written.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_SEND on an error in output->write, when flushing data.
 *
 * @public @memberof TSCLIENT_RecordsSet
 */
ErrorCode RecordsSet_writePutASafe(const TSCLIENT_RecordsSet* this, BufferedOStream* output);

/**
 * @brief Reads a RecordsSet from a stream.
 *
 * This function reads Records from the stream, deserializes them, and puts
 * them in 'this'. It expects the following bytestream format:
 * 1. int32_t recSize - size of entries 2..7 below, in bytes
 * 2. int32_t cid
 * 3. int64_t mid
 * 4. int32_t moid
 * 5. int64_t cap
 * 6. int64_t acq
 * 7. char* payload
 *
 * The last record has 1. recSize = 0 to indicate end of transmission.
 *
 * @param this The RecordsSet being filled with records
 * @param input The stream to read the data from
 * @return ERR_OK on success. In case of failure, 'this' may be filled with
 * a subset of read records.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_INVALID when deserialization of a record failed.
 * ERR_LIMIT when the read data's size is greater than buffer size.
 * ERR_RECEIVE on an error in input->read.
 * ERR_UNEXPECTED on malformed response.
 *
 * @public @memberof TSCLIENT_RecordsSet
 */
ErrorCode RecordsSet_read(TSCLIENT_RecordsSet* this, BufferedIStream* input);

/**
 * @brief Reads records from a stream with a callback.
 *
 * This function reads records from the stream, deserializes them, and puts
 * them in 'this', in chunks - each chunk contains at least one Record. Then
 * it calls 'callback' on 'this'. After that, 'this' is emptied and the
 * process continues until all records are read and processed.
 *
 * The function expects the following bytestream format:
 * 1. int32_t recSize - size of entries 2..7 below, in bytes
 * 2. int32_t cid
 * 3. int64_t mid
 * 4. int32_t moid
 * 5. int64_t cap
 * 6. int64_t acq
 * 7. char* payload
 *
 * The last record has 1. recSize = 0 to indicate end of transmission.
 *
 * @param this The RecordsSet being filled with records
 * @param input The stream to read the data from
 * @param[in] callback a function to call for each chunk of Records
 * @param userData a user-provided data, passed verbatim to 'callback'
 * @return ERR_OK on success.
 * In case of failure, 'callback' will be called only on records received
 * so far. Possible failures:
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_INVALID when deserialization of a record failed.
 * ERR_LIMIT when the read data's size is greater than buffer size.
 * ERR_RECEIVE on an error in input->read.
 * ERR_UNEXPECTED on malformed response.
 *
 * @public @memberof TSCLIENT_RecordsSet
 */
ErrorCode RecordsSet_readWithCallback(TSCLIENT_RecordsSet* this, BufferedIStream* input, TSCLIENT_GetCallback callback, void* userData);

#endif /* RECORDSSET_H */
