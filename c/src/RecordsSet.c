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

/** @file RecordsSet.c
 *
 * Implementation of the TSCLIENT_RecordsSet class.
 */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tstorage-client/client.h"

#include "RecordsSet.h"

#include "BufferedIStream.h"
#include "BufferedOStream.h"
#include "DynamicArray.h"
#include "ErrorCode.h"
#include "Record.h"

ErrorCode RecordsSet_initialize(TSCLIENT_RecordsSet* this, const TSCLIENT_PayloadType* payloadType)
{
	this->payloadType = *payloadType;

	/* Fulfill requirements of DynamicArray_initialize. */
	size_t maxCapacity = SIZE_MAX / payloadType->size;

	return DynamicArray_initialize(&this->array, payloadType->size, maxCapacity, 0);
}

void RecordsSet_finalize(TSCLIENT_RecordsSet* this)
{
	DynamicArray_finalize(&this->array);
}

TSCLIENT_RecordsSet* TSCLIENT_RecordsSet_new(const TSCLIENT_PayloadType* payloadType)
{
	TSCLIENT_RecordsSet* this = malloc(sizeof(TSCLIENT_RecordsSet));
	if (this == NULL) {
		return NULL;
	}

	ErrorCode res = RecordsSet_initialize(this, payloadType);
	if (res != ERR_OK) {
		free(this);
		return NULL;
	}

	return this;
}

void TSCLIENT_RecordsSet_destroy(TSCLIENT_RecordsSet* this)
{
	RecordsSet_finalize(this);
	free(this);
}

size_t TSCLIENT_RecordsSet_size(const TSCLIENT_RecordsSet* this)
{
	return this->array.size;
}

void* TSCLIENT_RecordsSet_elements(const TSCLIENT_RecordsSet* this)
{
	return this->array.buffer.storage;
}

int TSCLIENT_RecordsSet_append(TSCLIENT_RecordsSet* restrict this, TSCLIENT_Key* restrict key, void* restrict val)
{
	void* elem;
	ErrorCode res = DynamicArray_emplaceBack(&this->array, &elem);
	if (res != ERR_OK) {
		/* this->array's maxCapacity is set to system maximum (see TSCLIENT_RecordsSet_new), so emplaceBack is not
		   supposed to return ERR_LIMIT. BUT! If it does, set errno to ENOMEM and pretend to the user that it was
		   a relloc error - in this case the user should not care. */
		if (res != ERR_RESOURCE) {
			errno = ENOMEM;
		}
		return res;
	}

	assert(((char*)elem) + this->payloadType.size <= this->array.buffer.storage + this->array.size * this->array.elemSize
		   && "Writing past the end of this->array.storage");
	assert(((char*)elem) + sizeof(TSCLIENT_Key) <= this->array.buffer.storage + this->array.size * this->array.elemSize
		   && "Writing past the end of this->array.storage");
	memcpy(elem, key, sizeof(TSCLIENT_Key));

	assert(((char*)elem) + this->payloadType.offset + this->payloadType.size - sizeof(TSCLIENT_Key) <= this->array.buffer.storage + this->array.size * this->array.elemSize
		   && "Writing past the end of this->array.storage");
	memcpy(elem + this->payloadType.offset, val, this->payloadType.size - sizeof(TSCLIENT_Key));

	return 0;
}

/**
 * @brief Writes a batch of Records with the same cid to a stream.
 *
 * The batch's size is limited by the free space in the buffer of 'output', and
 * by INT32_MAX.
 *
 * The batch is serialized as:
 * 1. int32_t cid - the cid for all records in the batch, -1 marks end
 * 2. int32_t batchSize - size of 3..n in bytes
 * 3. Record
 * ... Record
 * n. Record
 *
 * Each Record above is serialized using 'recordSerializeFun'.
 *
 * @param payloadType defines the record's type of payload
 * @param output the stream to write the records to
 * @param recordSerializeFun function that serializes a Record
 * @param[in,out] recPtr points to the first record
 * @param[in] recordsEnd points to the last record plus one
 * @return ERR_OK on success.
 * ERR_LIMIT when a record's size is greater than buffer size; the batch is
 * not sent but previous contents of the buffer may be sent.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_SEND on an error in output->write, when flushing data.
 */
static ErrorCode writeCidBatch(const TSCLIENT_PayloadType* payloadType, BufferedOStream* output, Record_serialize_fun recordSerializeFun, const char** recPtr, const char* recordsEnd)
{
	assert(*recPtr < recordsEnd && "Function called for empty range of records");

	const TSCLIENT_Record* recStart = (const TSCLIENT_Record*)*recPtr;

	int32_t batchSize = 0; /* size of the currently-serialized batch */
	static const size_t headerSize = sizeof(recStart->key.cid) + sizeof(batchSize); /* size of batch header */

	char* buffer;
	ErrorCode res = BufferedOStream_reserveFlushing(output, &buffer, headerSize);
	if (res != ERR_OK) {
		return res;
	}

	size_t writePos = headerSize;

	int recTooLarge = 0; /* Encountered a record that won't ever fit in the buffer? */
	int needFlush = 0; /* Finished a batch because buffer space ended? */

	for (; *recPtr < recordsEnd; (*recPtr) += payloadType->size) {
		const TSCLIENT_Record* rec = (const TSCLIENT_Record*)*recPtr;

		assert((size_t)batchSize + headerSize == writePos);

		if (rec->key.cid != recStart->key.cid) {
			/* CIDs differ, this batch needs to end now. */
			break;
		}

		/* Try serializing a record without enlarging the buffer. */
		size_t freeSize = BufferedOStream_sizeReserved(output);
		assert(freeSize >= writePos);

		freeSize -= writePos;
		const size_t recSize = recordSerializeFun(rec, buffer + writePos, freeSize, payloadType);
		assert(recSize <= (size_t)INT32_MAX);
		if (batchSize > INT32_MAX - (int32_t)recSize) {
			/* The record won't fit in the batch */
			break;
		}

		if (recSize > freeSize) {
			/* Check if the record's size exceeds the buffer's maximum size. */
			if (recSize > BufferedOStream_bufferSize(output) - headerSize) {
				/* Even a chunk only with this one record won't fit! */
				recTooLarge = 1;
				break;
			}

			/* Try serializing a record with enlarging the buffer. */
			res = BufferedOStream_reserve(output, &buffer, writePos + recSize);
			if (res != ERR_OK) { /* ERR_LIMIT */
				/* This record does not fit in the stream's buffer's free space. */
				needFlush = 1;
				break;
			}
			size_t recSize2 = recordSerializeFun(rec, buffer + writePos, recSize, payloadType);
			assert((recSize2 == recSize) && "User's payloadType.toBytes function returns different size for the same record");
		}

		batchSize += (int32_t)recSize;
		writePos += recSize;
	}

	if (recTooLarge) {
		return ERR_LIMIT;
	}

	if (batchSize == 0) {
		/* Didn't serialize any record - there was no space in the buffer. */
		assert(*recPtr == (char*)recStart);
		needFlush = 1; /* Ensure there is space the next time */
	} else {
		assert(*recPtr > (char*)recStart);
		memcpy(buffer, &recStart->key.cid, sizeof(recStart->key.cid));
		memcpy(buffer + sizeof(recStart->key.cid), &batchSize, sizeof(batchSize));
		BufferedOStream_confirm(output, writePos);
	}

	return needFlush ? BufferedOStream_flush(output) : ERR_OK;
}

/**
 * @brief Writes the RecordsSet to a stream, in batches.
 *
 * Each batch comprises a sequence of Records with the
 * same cid. A batch is serialized as:
 * 1. int32_t cid - the cid for all records in the batch, -1 marks end
 * 2. int32_t batchSize - size of 3..n in bytes
 * 3. Record
 * ... Record
 * n. Record
 *
 * Each Record above is serialized using 'recordSerializeFun'.
 *
 * The last batch has 1. cid = -1 to mark end of transmission.
 *
 * The records in 'this' are not sorted before serializing. To minimize the
 * number of batches, the caller should sort the RecordsSet so that the
 * records are grouped by the same cid.
 *
 * @param this The RecordsSet being written
 * @param output The stream to write the records to
 * @param recordSerializeFun function that serializes a Record
 * @return ERR_OK on success.
 * ERR_LIMIT when a record's size is greater than buffer size; record batches
 * may be sent incomplete, the cid = -1 end marker is not sent.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_SEND on an error in output->write, when flushing data.
 */
static ErrorCode commonWrite(const TSCLIENT_RecordsSet* this, BufferedOStream* output, Record_serialize_fun recordSerializeFun)
{
	const size_t size = TSCLIENT_RecordsSet_size(this);
	const size_t recSize = this->payloadType.size;

	const char* recPtr = TSCLIENT_RecordsSet_elements(this);
	/* We use 'recPtr' as an iterator. 'recordsEnd' marks end of iteration. */
	const char* const recordsEnd = recPtr + size * recSize;

	ErrorCode res;

	/* Write records in batches that have the same CID. */
	while (recPtr < recordsEnd) {
		res = writeCidBatch(&this->payloadType, output, recordSerializeFun, &recPtr, recordsEnd);
		if (res != ERR_OK) {
			return res;
		}
	}

	/* Write the end mark, i.e. cid == -1 */
	const int32_t cid = -1;
	char* buf;
	res = BufferedOStream_reserveFlushing(output, &buf, sizeof(cid));
	if (res != ERR_OK) {
		return res;
	}

	memcpy(buf, &cid, sizeof(cid));
	BufferedOStream_confirm(output, sizeof(cid));

	return ERR_OK;
}

ErrorCode RecordsSet_writePutSafe(const TSCLIENT_RecordsSet* this, BufferedOStream* output)
{
	return commonWrite(this, output, Record_serializePutSafe);
}

ErrorCode RecordsSet_writePutASafe(const TSCLIENT_RecordsSet* this, BufferedOStream* output)
{
	return commonWrite(this, output, Record_serializePutASafe);
}

ErrorCode RecordsSet_read(TSCLIENT_RecordsSet* this, BufferedIStream* input)
{
	ErrorCode res;

	for (;;) {
		int32_t recSize32;
		char* buffer;
		res = BufferedIStream_reserve(input, &buffer, sizeof(recSize32));
		if (res != ERR_OK) {
			break;
		}

		memcpy(&recSize32, buffer, sizeof(recSize32));
		size_t recSize = (size_t)recSize32;
		if (recSize == 0) {
			/* No more records to receive. */
			res = ERR_OK;
			break;
		}

		res = BufferedIStream_reserve(input, &buffer, recSize);
		if (res != ERR_OK) {
			break;
		}

		TSCLIENT_Record* rec;
		res = DynamicArray_emplaceBack(&this->array, (void**)(&rec));
		if (res != ERR_OK) {
			break;
		}

		res = Record_deserialize(rec, buffer, recSize, &this->payloadType);
		if (res != ERR_OK) {
			DynamicArray_removeLast(&this->array);
			break;
		}
	}

	return res;
}

/**
 * @brief Reads data from stream, calls callback when stream buffer full.
 *
 * If the stream reserve function returns a "buffer full" error, this function
 * calls 'callback' on the RecordsSet 'this', then empties it, frees space in
 * the stream's buffer, and tries to receive again.
 *
 * @param this The RecordsSet being filled with records
 * @param input The stream to read the data from
 * @param[out] buffer Pointer to the received data in the stream's buffer
 * @param size Requested size of the data to read. May be 0.
 * @param[in] callback A function to call for each chunk of Records
 * @param userData A user-provided data, passed verbatim to 'callback'
 * @param[in,out] readSize Tracks the number of read bytes to confirm from
 * the stream.
 * @return ERR_OK on success.
 * ERR_LIMIT when 'size' is greater than free (previously unclaimed) space to
 * the end of buffer.
 * ERR_RESOURCE on realloc(3) error. Check errno(3).
 * ERR_RECEIVE on an error in input->read.
 */
static ErrorCode reserveWithCallback(TSCLIENT_RecordsSet* this, BufferedIStream* input, char** buffer, size_t size, TSCLIENT_GetCallback callback, void* userData)
{
	ErrorCode res = BufferedIStream_reserve(input, buffer, size);
	if (res == ERR_LIMIT) {
		if (TSCLIENT_RecordsSet_size(this) > 0) {
			callback(userData, this);
		}
		BufferedIStream_confirm(input);
		DynamicArray_empty(&this->array);

		res = BufferedIStream_reserve(input, buffer, size);
	}

	return res;
}

ErrorCode RecordsSet_readWithCallback(TSCLIENT_RecordsSet* this, BufferedIStream* input, TSCLIENT_GetCallback callback, void* userData)
{
	ErrorCode res;

	for (;;) {
		int32_t recSize32;
		char* buffer;
		res = reserveWithCallback(this, input, &buffer, sizeof(recSize32), callback, userData);
		if (res != ERR_OK) {
			break;
		}

		memcpy(&recSize32, buffer, sizeof(recSize32));

		size_t recSize = (size_t)recSize32;
		if (recSize == 0) {
			/* 0 indicates no more records to receive. */
			res = ERR_OK;
			break;
		}

		res = reserveWithCallback(this, input, &buffer, recSize, callback, userData);
		if (res != ERR_OK) {
			break;
		}

		TSCLIENT_Record* rec;
		res = DynamicArray_emplaceBack(&this->array, (void**)(&rec));
		if (res != ERR_OK) {
			break;
		}

		res = Record_deserialize(rec, buffer, recSize, &this->payloadType);
		if (res != ERR_OK) {
			DynamicArray_removeLast(&this->array);
			break;
		}
	}

	/* Even if something failed, we call 'callback' on all records received so
	   far. */
	if (TSCLIENT_RecordsSet_size(this) > 0) {
		callback(userData, this);
	}
	DynamicArray_empty(&this->array);

	return res;
}
