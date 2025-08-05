/*
 * TStorage: Client library (C++)
 *
 * ChannelImpl.cpp
 *   An implementation layer of ChannelBase's PIMPL idiom.
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

#include "ChannelImpl.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include <tstorageclient++/DataTypes.h>

#include "BatchSerializer.h"
#include "Buffer.h"
#include "Headers.h"
#include "Serializer.h"
#include "Socket.h"

namespace tstorage {
namespace impl {

constexpr std::size_t ChannelImpl::cMinBufferSize;

/**************
 * Setup
 */

result_t ChannelImpl::connect()
{
	mBuffer = Buffer(mMemoryLimit);
	if (!mBuffer) {
		return result_t::OUT_OF_MEMORY;
	}
	return mSocket.connect();
}

result_t ChannelImpl::close()
{
	if (mBuffer) {
		mBuffer = Buffer{};
	}
	return mSocket.close();
}

void ChannelImpl::setMemoryLimit(std::size_t memoryLimitBytes)
{
	mMemoryLimit = std::max(memoryLimitBytes, cMinBufferSize);
	if (mBuffer && mBuffer.capacity() != memoryLimitBytes) {
		mBuffer = Buffer(mMemoryLimit);
	}
}

void ChannelImpl::resetState()
{
	mBuffer.reset();
	mBatch = BatchSerializer(mBuffer);
}

/**************
 * Requests
 */

result_t ChannelImpl::writeKeyPairHeader(
	const CommandType cmdId, const Key& keyMin, const Key& keyMax)
{
	if (!mSocket.connectionEstablished()) {
		return result_t::NOT_CONNECTED;
	}
	if (!mBuffer) {
		return result_t::OUT_OF_MEMORY;
	}
	if (!(keyMin <= keyMax - 1)) {
		return result_t::EMPTY_KEY_RANGE;
	}
	if (!keyMin.isValid()) {
		return result_t::INVALID_KEY;
	}
	resetState();
	const HeaderKeyRange header{cmdId, 2 * Serializer::cKeySize, keyMin, keyMax};
	Serializer(mBuffer).putHeaderKeyRange(header);
	return sendBuffer();
}

result_t ChannelImpl::writeEmptyHeader(const CommandType cmdId)
{
	if (!mSocket.connectionEstablished()) {
		return result_t::NOT_CONNECTED;
	}
	if (!mBuffer) {
		return result_t::OUT_OF_MEMORY;
	}
	resetState();
	const Header header{cmdId, 0};
	Serializer(mBuffer).putHeader(header);
	return result_t::OK;
}

result_t ChannelImpl::reservePayloadBuffer(void*& oPayloadBuffer,
	std::size_t& oBufferSize,
	const std::size_t payloadSize,
	const Key& key,
	const std::size_t keySize)
{
	mExpectedPayloadSize = std::max(payloadSize, mExpectedPayloadSize);
	oPayloadBuffer = nullptr;
	oBufferSize = 0;

	std::size_t recordOffset = mBatch.getNextRecordOffset(key.cid);
	const std::size_t payloadOffset = keySize + sizeof(std::int32_t);

	if (recordOffset + payloadOffset + payloadSize > mBuffer.bytesOfFreeSpace()) {
		if (mBuffer.writeOffset() == 0) {
			return result_t::MEMORY_LIMIT_EXCEEDED;
		}

		mBatch.endBatch();
		const result_t resFlush = sendBuffer();
		if (resFlush != result_t::OK) {
			return resFlush;
		}
		if (recordOffset + payloadOffset + payloadSize > mBuffer.bytesOfFreeSpace()) {
			return result_t::MEMORY_LIMIT_EXCEEDED;
		}

		recordOffset = mBatch.getNextRecordOffset(key.cid);
	}

	const std::size_t totalOffset = recordOffset + payloadOffset;
	oPayloadBuffer = mBuffer.writeData(totalOffset);
	oBufferSize = mBuffer.bytesOfFreeSpace() - totalOffset;
	return result_t::OK;
}

result_t ChannelImpl::writeNextPutRecord(const Key& key, const std::size_t payloadSize)
{
	if (payloadSize > cMaxPayloadSize) {
		return result_t::PAYLOAD_TOO_LARGE;
	}
	Key maxKey = cKeyMax - 1;
	maxKey.acq = cKeyMax.acq;
	if (!key.isValid() || !(key <= maxKey)) {
		return result_t::INVALID_KEY;
	}
	mBatch.putRecord<BatchSerializer::ProtoT::PUT>(key, payloadSize);
	return result_t::OK;
}

result_t ChannelImpl::writeNextPutARecord(const Key& key, const std::size_t payloadSize)
{
	if (payloadSize > cMaxPayloadSize) {
		return result_t::PAYLOAD_TOO_LARGE;
	}
	if (!key.isValid() || !(key <= cKeyMax - 1)) {
		return result_t::INVALID_KEY;
	}
	mBatch.putRecord<BatchSerializer::ProtoT::PUTA>(key, payloadSize);
	return result_t::OK;
}

result_t ChannelImpl::writeFin()
{
	mBatch.endBatch();
	if (mBuffer.bytesOfFreeSpace() < sizeof(std::int32_t)) {
		const result_t resSend = sendBuffer();
		if (resSend != result_t::OK) {
			return resSend;
		}
		mBuffer.reset();
	}
	Serializer(mBuffer).putInt32(-1);
	return sendBuffer();
}

/**************
 * Responses
 */

result_t ChannelImpl::readResponse()
{
	mBuffer.reset();
	const result_t resData = requestData(Serializer::cHeaderSize);
	if (resData != result_t::OK) {
		return resData;
	}

	Serializer serializer(mBuffer);
	const Header response = serializer.getHeader();
	const result_t resSkip = skip(response.dataSize);
	if (resSkip != result_t::OK) {
		return resSkip;
	}

	const std::int32_t tstorageErrorCode = response.id;
	if (tstorageErrorCode != 0) {
		return result_t::ERROR;
	}
	return result_t::OK;
}

result_t ChannelImpl::readNextRecordData(
	Key& oKey, const void*& oPayloadPtr, std::size_t& oPayloadSize)
{
	const result_t resData = requestData(sizeof(std::int32_t));
	if (resData != result_t::OK) {
		return resData;
	}

	Serializer serializer(mBuffer);
	const std::int32_t recordSize = serializer.peekInt32();
	if (recordSize == 0) {
		serializer.confirmInt32();
		return result_t::END_OF_STREAM;
	}

	oPayloadSize = recordSize - Serializer::cKeySize;
	if (oPayloadSize > cMaxPayloadSize) {
		return result_t::BAD_RESPONSE;
	}

	const result_t resKeyPayload = requestData(sizeof(std::int32_t) + recordSize);
	if (resKeyPayload != result_t::OK) {
		return resKeyPayload;
	}

	serializer.confirmInt32();
	oKey = serializer.getKey();
	if (!oKey.isValid() || !(oKey <= cKeyMax - 1)) {
		return result_t::BAD_RESPONSE;
	}

	oPayloadPtr = serializer.getDataBuffer(oPayloadSize);
	return result_t::OK;
}

result_t ChannelImpl::readAcq(Key::AcqT& oAcq)
{
	const result_t resData = requestData(Serializer::cHeaderSize + sizeof(oAcq));
	if (resData != result_t::OK) {
		return resData;
	}

	Serializer serializer(mBuffer);
	const HeaderAcq response = serializer.getHeaderAcq();

	const std::int32_t tstorageErrorCode = response.id;
	if (tstorageErrorCode != 0) {
		const result_t resSkip = skip(response.dataSize);
		if (resSkip != result_t::OK) {
			return resSkip;
		}
		return result_t::ERROR;
	}
	if (response.dataSize < 8) {
		return result_t::BAD_RESPONSE;
	}

	oAcq = response.acq;
	const result_t resSkip = skip(response.dataSize - 8);
	if (resSkip != result_t::OK) {
		return resSkip;
	}
	return result_t::OK;
}

/**************
 * Helpers
 */

result_t ChannelImpl::sendBuffer()
{
	std::size_t amountSent = 0;
	const result_t res =
		mSocket.send(mBuffer.readData(), mBuffer.bytesAvailableToRead(), amountSent);
	if (res != result_t::OK) {
		return res;
	}
	resetState();
	return result_t::OK;
}

result_t ChannelImpl::requestData(const std::size_t amountBytes)
{
	const std::size_t bytesAvailableToRead = mBuffer.bytesAvailableToRead();
	if (amountBytes <= bytesAvailableToRead) {
		return result_t::OK;
	}

	const std::size_t amountBytesMissing = amountBytes - bytesAvailableToRead;
	if (amountBytesMissing > mBuffer.bytesOfFreeSpace()) {
		(void)mBuffer.reserve(amountBytesMissing);
		return result_t::MEMORY_LIMIT_EXCEEDED;
	}

	std::size_t recvd = 0;
	const result_t resFetch = mSocket.recvAtLeast(
		mBuffer.writeData(), mBuffer.bytesOfFreeSpace(), amountBytesMissing, recvd);

	if (resFetch != result_t::OK) {
		return resFetch;
	}
	mBuffer.writeAdvance(recvd);
	if (recvd < amountBytesMissing) {
		return result_t::BAD_RESPONSE;
	}
	return result_t::OK;
}

result_t ChannelImpl::skip(const std::size_t amountBytes)
{
	const std::size_t bytesAvailableToRead = mBuffer.bytesAvailableToRead();
	if (amountBytes <= bytesAvailableToRead) {
		mBuffer.readAdvance(amountBytes);
		return result_t::OK;
	}

	const std::size_t amountBytesToFetch = amountBytes - bytesAvailableToRead;
	mBuffer.readAdvance(bytesAvailableToRead);

	std::size_t amountSkipped{};
	const result_t res = mSocket.skipExactly(amountBytesToFetch, amountSkipped);
	if (res != result_t::OK) {
		return res;
	}
	if (amountSkipped < amountBytesToFetch) {
		return result_t::BAD_RESPONSE;
	}
	return res;
}

} /*namespace impl*/
} /*namespace tstorage*/
