/*
 * TStorage: Client library (C++)
 *
 * Channel.tpp
 *   A high level implementation of the `Channel<T>` interface.
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

#ifndef D_TSTORAGE_CHANNEL_TPP
#define D_TSTORAGE_CHANNEL_TPP

#ifndef D_TSTORAGE_CHANNEL_H
#error __FILE__ was included from outside of "Channel.h"
#include "Channel.h"  // clangd integration
#endif

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <ratio>
#include <string>

#include "ChannelBase.h"
#include "DataTypes.h"
#include "PayloadType.h"
#include "RecordsSet.h"
#include "Response.h"
#include "ResponseAcq.h"
#include "ResponseGet.h"

/** @file
 * @brief Contains the high-level implementation of `Channel<T>`. */

namespace tstorage {

template<typename T>
Channel<T>::Channel(const std::string& hostname,
	const std::uint16_t port,
	std::unique_ptr<PayloadType<T>> payloadType)
	: ChannelBase(), mPayloadType(std::move(payloadType))
{
	setHost(hostname, port);
}

template<typename T>
Channel<T>::Channel(const std::string& hostname,
	const std::uint16_t port,
	std::unique_ptr<PayloadType<T>> payloadType,
	std::size_t memoryLimitBytes)
	: Channel<T>(hostname, port, payloadType)
{
	setMemoryLimit(memoryLimitBytes);
}

template<typename T>
Channel<T>::~Channel()
{
	(void)close();
}

template<typename T>
Response Channel<T>::connect()
{
	return Response(connectImpl());
}

template<typename T>
Response Channel<T>::close()
{
	return Response(closeImpl());
}

template<typename T>
void Channel<T>::setTimeout(const std::chrono::duration<std::int64_t, std::milli> timeout)
{
	setTimeoutImpl(timeout);
}

template<typename T>
void Channel<T>::setMemoryLimit(const std::size_t memoryLimitBytes)
{
	setMemoryLimitImpl(memoryLimitBytes);
}

/**************
 * Put
 */

template<typename T>
template<typename Channel<T>::ProtoT PutProtocol>
result_t Channel<T>::putImpl(const RecordsSet<T>& recordSet)
{
	constexpr PutMethods impl = putMethods(PutProtocol);
	result_t res = (this->*impl.writeHeader)();
	if (res != result_t::OK) {
		abort();
		return res;
	}

	typename RecordsSet<T>::const_iterator recordIter = recordSet.begin();
	typename RecordsSet<T>::const_iterator endIter = recordSet.end();
	while (res == result_t::OK && recordIter != endIter) {
		res = serializeAndWriteRecord<PutProtocol>(*recordIter);
		if (res == result_t::OK) {
			++recordIter;
		}
	}

	if (res == result_t::PAYLOAD_TOO_LARGE || res == result_t::INVALID_KEY) {
		if (putFinalize<PutProtocol>() == result_t::OK) {
			(void)close();
		} else {
			abort();
		}
		return res;
	}
	if (res != result_t::OK) {
		abort();
		return res;
	}
	return putFinalize<PutProtocol>();
}

template<typename T>
template<typename Channel<T>::ProtoT PutProtocol>
result_t Channel<T>::serializeAndWriteRecord(const Record<T>& record)
{
	constexpr PutMethods impl = putMethods(PutProtocol);
	void* buffer{};
	std::size_t bufferSize{};
	std::size_t payloadSize{};

	result_t res = (this->*impl.obtainPayloadBuffer)(buffer, bufferSize, record.key);
	if (res != result_t::OK) {
		return res;
	}
	payloadSize = mPayloadType->toBytes(record.value, buffer, bufferSize);
	if (bufferSize < payloadSize) {
		res = (this->*impl.reservePayloadBuffer)(
			buffer, bufferSize, payloadSize, record.key);
		if (res != result_t::OK) {
			return res;
		}
		payloadSize = mPayloadType->toBytes(record.value, buffer, bufferSize);
	}
	return (this->*impl.writeNextRecord)(record.key, payloadSize);
}

template<typename T>
template<typename Channel<T>::ProtoT PutProtocol>
result_t Channel<T>::putFinalize()
{
	constexpr PutMethods impl = putMethods(PutProtocol);
	result_t res = writeFin();
	if (res != result_t::OK) {
		abort();
		return res;
	}

	res = (this->*impl.readResult)();
	if (res != result_t::OK) {
		abort();
		return res;
	}
	return result_t::OK;
}

template<typename T>
struct Channel<T>::PutMethods
{
	result_t (Channel<T>::*writeHeader)();
	result_t (Channel<T>::*obtainPayloadBuffer)(
		void*& oBuffer, std::size_t& oBufferSize, const Key& key);
	result_t (Channel<T>::*reservePayloadBuffer)(void*& oBuffer,
		std::size_t& oBufferSize,
		std::size_t payloadSize,
		const Key& key);
	result_t (Channel<T>::*writeNextRecord)(const Key& key, std::size_t payloadSize);
	result_t (Channel<T>::*readResult)();
};

template<typename T>
constexpr typename Channel<T>::PutMethods Channel<T>::putMethods(
	const Channel<T>::ProtoT tag)
{
	switch (tag) {
		case PUT:
			return PutMethods{
				&Channel<T>::writePutHeader,
				&Channel<T>::obtainPutPayloadBuffer,
				&Channel<T>::reservePutPayloadBuffer,
				&Channel<T>::writeNextPutRecord,
				&Channel<T>::readPutResult,
			};
		case PUTA:
			return PutMethods{
				&Channel<T>::writePutAHeader,
				&Channel<T>::obtainPutAPayloadBuffer,
				&Channel<T>::reservePutAPayloadBuffer,
				&Channel<T>::writeNextPutARecord,
				&Channel<T>::readPutAResult,
			};
	}
}

template<typename T>
Response Channel<T>::put(const RecordsSet<T>& data)
{
	return Response(putImpl<ProtoT::PUT>(data));
}

template<typename T>
Response Channel<T>::puta(const RecordsSet<T>& data)
{
	return Response(putImpl<ProtoT::PUTA>(data));
}

/**************
 * Get
 */

template<typename T>
ResponseAcq Channel<T>::getAcq(const Key& keyMin, const Key& keyMax)
{
	result_t res = writeGetAcqRequest(keyMin, keyMax);
	if (res != result_t::OK) {
		abort();
		return ResponseAcq(res);
	}

	Key::AcqT acq{};
	res = readAcq(acq);
	if (res != result_t::OK) {
		abort();
		return ResponseAcq(res);
	}

	return ResponseAcq(res, acq);
}

template<typename T>
ResponseGet<T> Channel<T>::get(const Key& keyMin, const Key& keyMax)
{
	result_t res = writeGetRequest(keyMin, keyMax);
	if (res != result_t::OK) {
		abort();
		return ResponseGet<T>(res);
	}

	res = readResponse();
	if (res != result_t::OK) {
		abort();
		return ResponseGet<T>(res);
	}

	RecordsSet<T> recordSet{};
	while (true) {
		res = recvAndDeserializeRecordTo(recordSet);
		if (res == result_t::END_OF_STREAM) {
			break;
		}
		if (res != result_t::OK) {
			abort();
			return ResponseGet<T>(res, std::move(recordSet), 0);
		}
	}

	Key::AcqT acq{};
	res = readGetResult(acq);
	if (res != result_t::OK) {
		abort();
		return ResponseGet<T>(res, std::move(recordSet), 0);
	}
	return ResponseGet<T>(res, std::move(recordSet), acq);
}

template<typename T>
ResponseAcq Channel<T>::getStream(const Key& keyMin,
	const Key& keyMax,
	const std::function<void(RecordsSet<T>&)>& callback)
{
	result_t res = writeGetRequest(keyMin, keyMax);
	if (res != result_t::OK) {
		abort();
		return ResponseAcq(res);
	}

	res = readResponse();
	if (res != result_t::OK) {
		abort();
		return ResponseAcq(res);
	}

	while (res == result_t::OK) {
		RecordsSet<T> recordSet{};
		res = recvAndDeserializeBatchTo(recordSet);
		callback(recordSet);
	}
	if (res != result_t::END_OF_STREAM) {
		abort();
		return ResponseAcq(res);
	}

	Key::AcqT acq{};
	res = readGetResult(acq);
	if (res != result_t::OK) {
		abort();
		return ResponseAcq(res);
	}
	return ResponseAcq(res, acq);
}

template<typename T>
result_t Channel<T>::recvAndDeserializeBatchTo(RecordsSet<T>& recordSet)
{
	result_t res = result_t::OK;
	while (res == result_t::OK) {
		res = recvAndDeserializeRecordTo(recordSet);
	}
	if (res == result_t::MEMORY_LIMIT_EXCEEDED) {
		if (recordSet.size() == 0) {
			return result_t::MEMORY_LIMIT_EXCEEDED;
		}
		return result_t::OK;
	}
	return res;
}

template<typename T>
result_t Channel<T>::recvAndDeserializeRecordTo(RecordsSet<T>& recordSet)
{
	Record<T> record{};
	const void* payloadBuffer{};
	std::size_t payloadSize{};

	result_t res = readNextRecordData(record.key, payloadBuffer, payloadSize);
	if (res != result_t::OK) {
		return res;
	}

	if (!mPayloadType->fromBytes(record.value, payloadBuffer, payloadSize)) {
		return result_t::DESERIALIZATION_ERROR;
	}

	recordSet.append(std::move(record));
	return res;
}

} /*namespace tstorage*/

#endif
