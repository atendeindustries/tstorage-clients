/*
 * TStorage: Client library (C++)
 *
 * ChannelBase.cpp
 *  A base class of `Channel<T>` providing the API to the library.
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

#include <tstorageclient++/ChannelBase.h>

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ratio>
#include <string>

#include <tstorageclient++/DataTypes.h>

#include "ChannelImpl.h"
#include "Defines.h"

namespace tstorage {
namespace impl {

/**
 * @file
 * @brief Implements `ChannelBase` methods as calls to its PIMPL object.
 *
 * Here we include some more details regarding the usage of the `ChannelBase`
 * class.
 *
 * The objects of this class have a rich internal state which requires careful
 * management in order for the channel to function properly. Apart from the
 * possibility of being "open" or "closed" according to whether or not the
 * channel remains successfully connected to a TStorage instance, the possible
 * internal states mimic the communication phases defined by the TStorage
 * protocol.
 *
 * - While being "open", the channel can transition into one of PUT, PUTA, GET,
 * GETACQ states by initiating communication with TStorage by means of one of
 * the `writePutHeader()`, `writePutAHeader()`, `writeGetRequest()` or
 * `writeGetAcqRequest()` method calls.
 *
 * - While in PUT state, the channel can call `obtainPutPayloadBuffer()` or
 * `reservePutPayloadBuffer()` to reserve a buffer for payload serialization
 * purposes, `writeNextPutRecord()` to begin processing the next record, or
 * `writeFin()` to end the PUT request at the last record that was written
 * to the channel. The last of these calls moves the channel into
 * PUT_RESPONSE state, in which it can call `readPutResult()` to receive
 * the confirmation of a successful database store operation or an error code.
 * This call brings the channel back to the "open" state.
 *
 * - The PUTA state and the PUTA_RESPONSE state behave analogously.
 *
 * - While in GET state, the channel can call `readResponse()` to check if the
 * server accepts the GET request. On success, the channel transitions into the
 * GET_RECORDS state, allowing it to call `readNextRecordData()` repeatedly to
 * deserialize records sent in response. When `readNextRecordData()` signals
 * the end of stream by returning `result_t::END_OF_STREAM`, the channel
 * transitions into GET_RESULT state. The call to `readGetResult()` retrieves
 * the last ACQ timestamp guaranteeing data consistency together with the
 * operation status code, bringing the channel back to "open" state.
 *
 * - While in GETACQ state, the channel can call `readAcq()` to retrieve the last
 * ACQ timestamp guaranteeing data consistency inside the given key-interval on
 * success, and an error code on failure. This brings the channel back to the
 * "open" state.
 *
 * If any of the above methods returns an error code, the channel switches to
 * its ERROR state. In this state, it is well advised to `close()` or, better
 * yet, `abort()` the connection. This brings the channel back to the initial,
 * "closed" state. Otherwise, the behavior of the channel becomes highly
 * context sensitive and implementation dependent; please refer to the
 * documentation of the Channel internals for clues for how to react to some
 * specific errors.
 *
 * For any given state, calling any other method that is not included in its
 * associated list above (apart from `set*()` `connectImpl()`, `closeImpl()`
 * and `abort()`) leaves the channel in an invalid state, and from now on one
 * cannot make any sensible guarantees on its behavior.
 */

TSTORAGE_EXPORT ChannelBase::ChannelBase() : mImpl(std::make_unique<ChannelImpl>())
{
}

TSTORAGE_EXPORT ChannelBase::~ChannelBase() = default;

TSTORAGE_EXPORT result_t ChannelBase::connectImpl()
{
	return mImpl->connect();
}

TSTORAGE_EXPORT result_t ChannelBase::closeImpl()
{
	return mImpl->close();
}

TSTORAGE_EXPORT void ChannelBase::abort()
{
	mImpl->abort();
}

TSTORAGE_EXPORT result_t ChannelBase::writeGetRequest(
	const Key& keyMin, const Key& keyMax)
{
	return mImpl->writeGetRequest(keyMin, keyMax);
}

TSTORAGE_EXPORT result_t ChannelBase::writeGetAcqRequest(
	const Key& keyMin, const Key& keyMax)
{
	return mImpl->writeGetAcqRequest(keyMin, keyMax);
}

TSTORAGE_EXPORT result_t ChannelBase::writePutHeader()
{
	return mImpl->writePutHeader();
}

TSTORAGE_EXPORT result_t ChannelBase::writePutAHeader()
{
	return mImpl->writePutAHeader();
}

TSTORAGE_EXPORT result_t ChannelBase::obtainPutPayloadBuffer(
	void*& oPayloadBuffer, std::size_t& oBufferSize, const Key& key)
{
	return mImpl->obtainPutPayloadBuffer(oPayloadBuffer, oBufferSize, key);
}

TSTORAGE_EXPORT result_t ChannelBase::obtainPutAPayloadBuffer(
	void*& oPayloadBuffer, std::size_t& oBufferSize, const Key& key)
{
	return mImpl->obtainPutAPayloadBuffer(oPayloadBuffer, oBufferSize, key);
}

TSTORAGE_EXPORT result_t ChannelBase::reservePutPayloadBuffer(
	void*& oPayloadBuffer, std::size_t& oBufferSize, std::size_t payloadSize, const Key& key)
{
	return mImpl->reservePutPayloadBuffer(oPayloadBuffer, oBufferSize, payloadSize, key);
}

TSTORAGE_EXPORT result_t ChannelBase::reservePutAPayloadBuffer(
	void*& oPayloadBuffer, std::size_t& oBufferSize, std::size_t payloadSize, const Key& key)
{
	return mImpl->reservePutAPayloadBuffer(oPayloadBuffer, oBufferSize, payloadSize, key);
}

TSTORAGE_EXPORT result_t ChannelBase::writeNextPutRecord(
	const Key& key, const std::size_t payloadSize)
{
	return mImpl->writeNextPutRecord(key, payloadSize);
}

TSTORAGE_EXPORT result_t ChannelBase::writeNextPutARecord(
	const Key& key, const std::size_t payloadSize)
{
	return mImpl->writeNextPutARecord(key, payloadSize);
}

TSTORAGE_EXPORT result_t ChannelBase::writeFin()
{
	return mImpl->writeFin();
}

TSTORAGE_EXPORT result_t ChannelBase::readResponse()
{
	return mImpl->readResponse();
}

TSTORAGE_EXPORT result_t ChannelBase::readPutResult()
{
	return mImpl->readPutResult();
}

TSTORAGE_EXPORT result_t ChannelBase::readPutAResult()
{
	return mImpl->readPutAResult();
}

TSTORAGE_EXPORT result_t ChannelBase::readNextRecordData(
	Key& oKey, const void*& oPayloadPtr, std::size_t& oPayloadSize)
{
	return mImpl->readNextRecordData(oKey, oPayloadPtr, oPayloadSize);
}

TSTORAGE_EXPORT result_t ChannelBase::readGetResult(Key::AcqT& oAcq)
{
	return mImpl->readGetResult(oAcq);
}

TSTORAGE_EXPORT result_t ChannelBase::readAcq(Key::AcqT& oAcq)
{
	return mImpl->readAcq(oAcq);
}

TSTORAGE_EXPORT void ChannelBase::setTimeoutImpl(
	const std::chrono::duration<std::int64_t, std::milli> timeout)
{
	mImpl->setTimeoutMs(timeout.count());
}

TSTORAGE_EXPORT void ChannelBase::setMemoryLimitImpl(const std::size_t memoryLimitBytes)
{
	mImpl->setMemoryLimit(memoryLimitBytes);
}

TSTORAGE_EXPORT void ChannelBase::setHost(const std::string& addr, const std::uint16_t port)
{
	mImpl->setHost(addr, port);
}

} /*namespace impl*/
} /*namespace tstorage*/
