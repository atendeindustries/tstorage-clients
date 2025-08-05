/*
 * TStorage: Client library (C++)
 *
 * BatchSerializer.cpp
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

#include "BatchSerializer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

#include <tstorageclient++/DataTypes.h>

#include "Serializer.h"

namespace tstorage {
namespace impl {

template<BatchSerializer::ProtoT PutProtocol>
void BatchSerializer::putRecord(const Key& key, const std::size_t payloadSize)
{
	if (key.cid != mCid && mCid >= 0) {
		endBatch();
	}
	Serializer serializer(*mBuffer);
	if (empty()) {
		mCid = key.cid;
		serializer.putInt32(key.cid);
		serializer.putInt32(0);
	}

	std::int32_t recordSize{};
	if (PutProtocol == BatchSerializer::PUT) {
		recordSize =
			static_cast<std::int32_t>(Serializer::cAbbrevKeySizeWithoutAcq + payloadSize);
		serializer.putInt32(recordSize);
		serializer.putAbbrevKeyWithoutAcq(key);
	} else {
		recordSize = static_cast<std::int32_t>(Serializer::cAbbrevKeySize + payloadSize);
		serializer.putInt32(recordSize);
		serializer.putAbbrevKey(key);
	}
	serializer.confirmPayloadBuffer(payloadSize);
	mBatchSize += static_cast<std::int32_t>(sizeof(std::int32_t) + recordSize);
}

void BatchSerializer::endBatch()
{
	Serializer::put<std::int32_t>(mBatchSize, mBatchSizeField);
	mBatchSize = 0;
	mBatchSizeField = mBuffer->writeData(cBatchSizeOffset);
}

std::size_t BatchSerializer::getNextRecordOffset(Key::CidT cid) const
{
	return (empty() || cid != mCid) ? cBatchHeaderSize : 0;
}

/***************
 * Explicit template instantiations
 */

template void BatchSerializer::putRecord<BatchSerializer::ProtoT::PUT>(
	const Key& key, std::size_t payloadSize);
template void BatchSerializer::putRecord<BatchSerializer::ProtoT::PUTA>(
	const Key& key, std::size_t payloadSize);

} /*namespace impl*/
} /*namespace tstorage*/
