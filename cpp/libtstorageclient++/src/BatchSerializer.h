/*
 * TStorage: Client library (C++)
 *
 * BatchSerializer.h
 *   An information expert around PUT/A batch processing.
 *
 * Copyright 2025 Atende Industries
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
 */

#ifndef D_TSTORAGE_BATCHSERIALZIER_PH
#define D_TSTORAGE_BATCHSERIALZIER_PH

#include <cstddef>
#include <cstdint>

#include <tstorageclient++/DataTypes.h>

#include "Buffer.h"

/** @file
 * @brief Defines a batch serialization information expert object. */

namespace tstorage {
namespace impl {

/**
 * @brief A stateful serializer for PUT/A record batches.
 *
 * Its main purpose is to keep track of the batch header data and, by comparing
 * it against the incoming records, to decide when to end the current batch and
 * start serializing a new one.
 *
 * Ending a batch involves writing an end-of-batch marker to the stream, then
 * reaching back to a memorized location inside the batch header
 * (`mBatchSizeField`) and overwriting this header field with the size of the
 * batch in bytes. Starting a new batch boils down to simply writing a new
 * header to the stream.
 *
 * The header consists of the CID of all the records inside a batch, and the
 * total size of the batch in bytes (excluding the header). Due to CID being
 * present in the header, we "abbreviate" the records' keys by serializing them
 * without their CIDs. As we serialize records on the fly, the size of the
 * batch is only known after we decide to end a batch. This is where
 * `mBatchSizeField` comes into play. At every moment it points to a memory
 * address of the size field of the current batch. This size field is
 * zero-initialized at the start of a new batch, and is updated only after we
 * decide to end the current one, when its size becomes known.
 *
 * The batch serializer ends the batch if and only if:
 *  - the next record has a CID differing from the CID of the current batch;
 *  - the user requests it explicitly.
 *
 * When a new batch is started, the serializer awaits further records. After
 * getting a new record and serializing its payload, call `putRecord()` to
 * append it to the current batch. The offset of the record itself inside the
 * buffer, which can be nonzero, is essential for payload serialization. It can
 * be queried by `getNextRecordOffset()`.
 */
class BatchSerializer
{
public:
	/** @brief A PUT protocol tag. */
	enum ProtoT {
		PUT,
		PUTA,
	};

	/** @brief The size of a batch header in bytes. */
	static constexpr std::size_t cBatchHeaderSize =
		sizeof(Key::CidT) + sizeof(std::int32_t);
	/** @brief The offset of the size field from the beginning of the batch. */
	static constexpr std::size_t cBatchSizeOffset = sizeof(Key::CidT);

	/**
	 * @brief A constructor. Sets up the initial state of the serializer.
	 * @param buf A buffer to serialize batches into.
	 */
	BatchSerializer(Buffer& buf)
		: mBuffer(&buf)
		, mBatchSizeField(mBuffer->writeData(cBatchSizeOffset))
		, mCid(-1)
		, mBatchSize(0)
	{
	}

	/**
	 * @brief Appends the next record byte-stream to the current batch, ending it
	 * and starting a new one if necessary.
	 *
	 * @tparam PutProtocol A protocol tag which determines the shape of the key
	 * to send along with the rest of the record. Can be either PUT or PUTA.
	 * @param key The record's key.
	 * @param payloadSize The size of the record's payload.
	 */
	template<ProtoT PutProtocol>
	void putRecord(const Key& key, std::size_t payloadSize);
	/** @brief Ends the current batch. */
	void endBatch();

	/** @brief Returns the offset to the next record data inside the current batch
	 * from the buffer's next write location. Depends on next record's `cid`
	 * value. */
	std::size_t getNextRecordOffset(Key::CidT cid) const;
	/** @brief Returns `true` if the batch has no records, `false` otherwise. */
	bool empty() const { return mBatchSize == 0; }

private:
	/** @brief A pointer to the underlying `Buffer`. */
	Buffer* mBuffer;
	/** @brief A pointer to the size field of the current batch header. */
	void* mBatchSizeField;
	/** @brief CID of the current batch. */
	Key::CidT mCid;
	/** @brief The size of the current batch. Increases with each appended record.
	 * */
	std::int32_t mBatchSize;
};

} /*namespace impl*/
} /*namespace tstorage*/

#endif
