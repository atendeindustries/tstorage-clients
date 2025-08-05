/*
 * TStorage: Client library (C++)
 *
 * ChannelImpl.h
 *   The implementation layer of ChannelBase's PIMPL idiom.
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

#ifndef D_TSTORAGE_CHANNELIMPL_PH
#define D_TSTORAGE_CHANNELIMPL_PH

#include <cstddef>
#include <cstdint>
#include <string>

#include <tstorageclient++/DataTypes.h>

#include "BatchSerializer.h"
#include "Buffer.h"
#include "Headers.h"
#include "Serializer.h"
#include "Socket.h"

/** @file
 * @brief Provides low-level implementation details of the `Channel<T>` class.
 * */

namespace tstorage {
namespace impl {

/**
 * @brief An implementation of TStorage communication protocol.
 *
 * This class implements the ChannelBase interface. Internally, it performs
 * buffered I/O over TCP sockets using the TStorage communication protocol to
 * talk with a given TStorage instance. It operates in a payload-type agnostic
 * way, using raw byte-stream payload representations stored in an internal
 * buffer of a fixed (but adjustable) length. Also, it supplies its
 * `Channel<T>` children with readable and writable fragments of its internal
 * buffer to de-/serialize the data from/into.
 *
 * The channel defers its resource acquisition (socket FDs, memory) to the
 * first `connect()` call. Analogously, the resources are freed on the
 * subsequent `close()` or `abort()` call.
 *
 * In order to reduce the amount of syscalls, all communication with the server
 * is buffered. On write, all data is first written to the internal buffer. The
 * buffer is flushed only in two cases: (1) when the buffer's total memory
 * usage approaches its maximum capacity; and (2) on an outbound message
 * termination. On receive, we do not bound the amount of data fetched during a
 * single syscall. While reading and deserializing the server's response, we
 * first access the excess data from the last `Socket::recv()` call located in
 * the internal buffer, and then, only when the buffer is about to run out of
 * local data to read, we perform a syscall to receive the next portion of the
 * response.
 *
 * The one detail that was not mentioned in the documentation of `Channel<T>`
 * or `ChannelBase` is that the basic data type underlying the PUT/A protocol
 * is not an individual record; it is, instead, a batch of records. A good way
 * to think of such a batch is as a list of records sharing the same CID. To
 * construct the batch inside the internal buffer and keep track of its current
 * state we employ a `BatchSerializer` object. This `ChannelImpl` component is
 * responsible for appending each record with a matching CID to the current
 * batch. It also decides when to end the batch and start a new one; this
 * occurs automatically when the internal buffer is about to overflow or when
 * we acquire a record with a differing CID value. For more information, see
 * `BatchSerializer`.
 */
class ChannelImpl final
{
private:
	/** @brief The default size of the internal buffer. */
	static constexpr std::size_t cInitialBufferSize = 64L * 1024;  // 64 KiB
	/** @brief The minimal size of the internal buffer. */
	static constexpr std::size_t cMinBufferSize = 128;	// 128B
	/** @brief The maximal payload size acceptable by TStorage. */
	static constexpr std::size_t cMaxPayloadSize = 32UL * 1024 * 1024;
	/** @brief Initial value of a payload size hint for some internal
	 * optimizations (see `reservePutPayloadBuffer()`). */
	static constexpr std::size_t cInitialExpectedPayloadSize = 8;

public:

	/*******************
	 * Public methods
	 */

	/**
	 * @brief A default constructor.
	 *
	 * Initializes the object with default values.
	 */
	ChannelImpl()
		: mExpectedPayloadSize(cInitialExpectedPayloadSize)
		, mMemoryLimit(cInitialBufferSize)
		, mBatch(mBuffer)
	{
	}

	/** @brief Forcefully closes the connection. Used on errors. */
	void abort() { mSocket.abort(); }
	/**
	 * @brief Discards the data currently stored in the buffer (does not affect
	 * its capacity).
	 */
	void resetBuffer();
	/**
	 * @brief Prepares the channel for work by allocating memory for internal
	 * buffer and connecting with the host.
	 *
	 * The possible error codes are:
	 *  - `result_t::BAD_ADDRESS`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNREFUSED`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::OUT_OF_MEMORY`
	 *  - `result_t::SETOPT_ERROR`
	 *  - `result_t::SIGNAL`
	 *  - `result_t::SOCKET_ERROR`
	 *
	 * @return `result_t::OUT_OF_MEMORY` if the allocation failed, the result of
	 * `Socket::connect()` otherwise.
	 */
	result_t connect();
	/**
	 * @brief Instructs the socket to close the connection gracefully and returns
	 * the result.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNERROR`
	 *  - `result_t::NOT_CONNECTED`
	 *
	 * @return The status code.
	 */
	result_t close();

	/**
	 * @brief Initiates a GET request.
	 *
	 * @see `writeKeyPairHeader()`
	 *
	 * @param keyMin The lower vertex of the target key-interval.
	 * @param keyMax The upper vertex of the target key-interval.
	 * @return The status code.
	 */
	inline result_t writeGetRequest(const Key& keyMin, const Key& keyMax);
	/**
	 * @brief Initiates a GETACQ request.
	 *
	 * @see `writeKeyPairHeader()`
	 *
	 * @param keyMin The lower vertex of the target key-interval.
	 * @param keyMax The upper vertex of the target key-interval.
	 * @return The status code.
	 */
	inline result_t writeGetAcqRequest(const Key& keyMin, const Key& keyMax);
	/**
	 * @brief Initiates a PUT request.
	 * @see `writeEmptyHeader()`
	 * @return The status code.
	 */
	inline result_t writePutHeader();
	/**
	 * @brief Initiates a PUTA request.
	 * @see `writeEmptyHeader()`
	 * @return The status code.
	 */
	inline result_t writePutAHeader();

	/**
	 * @brief Retrieves a pointer to a writable memory block of the internal
	 * buffer for payload serialization. Used with PUT protocol.
	 *
	 * Since the required size of the memory block is known only after payload
	 * serialization (as per `PayloadType::toBytes()`), the size of the
	 * reserved memory block is chosen according to a simple heuristic, namely it
	 * is equal to the maximal payload size encountered by the channel to date
	 * (starting from `ChannelImpl::cInitialExpectedPayloadSize`).
	 *
	 * Used during the first serialization attempt of a given record, when the actual
	 * payload size is not known.
	 *
	 * @see `reservePayloadBuffer()`
	 * @param[out] oPayloadBuffer A writable memory block to serialize payload into.
	 * @param[out] oBufferSize The size of the reserved memory block.
	 * @param[in] key The key of the record that is currently serialized.
	 * @return The status code.
	 */
	inline result_t obtainPutPayloadBuffer(
		void*& oPayloadBuffer, std::size_t& oBufferSize, const Key& key);
	/**
	 * @brief Retrieves a pointer to a writable memory block of the internal
	 * buffer for payload serialization. Used with PUTA protocol.
	 *
	 * @see `obtainPutPayloadBuffer()`
	 * @param[out] oPayloadBuffer A writable memory block to serialize payload into.
	 * @param[out] oBufferSize The size of the reserved memory block.
	 * @param[in] key The key of the record that is currently serialized.
	 * @return The status code.
	 */
	inline result_t obtainPutAPayloadBuffer(
		void*& oPayloadBuffer, std::size_t& oBufferSize, const Key& key);
	/**
	 * @brief Reserves a writable memory block of a certain size inside the
	 * internal buffer for payload serialization. Used with PUT protocol.
	 *
	 * Used during the second serialization attempt of a given record, only when
	 * the first returned a pointer to a memory block of insufficient size. The
	 * actual payload size is now known and given as an argument to the method
	 * call.
	 *
	 * The call to this method guarantees that, on success, `oBufferSize >=
	 * payloadSize`.
	 *
	 * @see `reservePayloadBuffer()`
	 * @param[out] oPayloadBuffer A writable memory block to serialize payload into.
	 * @param[out] oBufferSize The size of the reserved memory block.
	 * @param[in] payloadSize The requested size of the memory block.
	 * @param[in] key The key of the record that is currently serialized.
	 * @return The status code.
	 */
	inline result_t reservePutPayloadBuffer(void*& oPayloadBuffer,
		std::size_t& oBufferSize,
		std::size_t payloadSize,
		const Key& key);
	/**
	 * @brief Reserves a writable memory block of a certain size inside the
	 * internal buffer for payload serialization. Used with PUTA protocol.
	 *
	 * @see `reservePutPayloadBuffer()`
	 * @param[out] oPayloadBuffer A writable memory block to serialize payload into.
	 * @param[out] oBufferSize The size of the reserved memory block.
	 * @param[in] payloadSize The requested size of the memory block.
	 * @param[in] key The key of the record that is currently serialized.
	 * @return The status code.
	 */
	inline result_t reservePutAPayloadBuffer(void*& oPayloadBuffer,
		std::size_t& oBufferSize,
		std::size_t payloadSize,
		const Key& key);

	/**
	 * @brief Serializes the current record's metadata. Used with PUT protocol.
	 *
	 * The possible error codes are:
	 *  - `result_t::INVALID_KEY`
	 *  - `result_t::PAYLOAD_TOO_LARGE`
	 *
	 * @param key The key of the record that is currently serialized.
	 * @param payloadSize Payload size in bytes.
	 * @return The status code.
	 */
	result_t writeNextPutRecord(const Key& key, std::size_t payloadSize);
	/**
	 * @brief Serializes the current record's metadata. Used with PUTA protocol.
	 *
	 * In case the internal buffer is about to overflow, ends the current data
	 * batch and sends it over to TStorage.
	 *
	 * The possible error codes are:
	 *  - `result_t::INVALID_KEY`
	 *  - `result_t::PAYLOAD_TOO_LARGE`
	 *
	 * @param key The key of the record that is currently serialized.
	 * @param payloadSize Payload size in bytes.
	 * @return The status code.
	 */
	result_t writeNextPutARecord(const Key& key, std::size_t payloadSize);
	/**
	 * @brief Finalizes a PUT/A request.
	 *
	 * Ends the current data batch, places the end-of-stream marker at the
	 * end of the buffer to complete the request and sends it over to TStorage,
	 * finishing the PUT/A request.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNRESET`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @return The status code.
	 */
	result_t writeFin();

	/**
	 * @brief Reads a TStorage response header. Used with GET.
	 *
	 * The PUT, PUTA and GET client requests prompt the server to respond with a
	 * standard `Response` header which indicates whether a server encountered an
	 * error or not, and if so, of which kind. This method retrieves it.
	 *
	 * The possible error codes are:
	 *  - `result_t::BAD_RESPONSE`
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::ERROR`
	 *  - `result_t::MEMORY_LIMIT_EXCEEDED`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @return The status code.
	 */
	result_t readResponse();
	/**
	 * @brief Reads a TStorage response header. Used with PUT.
	 * @see `readResponse()`
	 * @return The status code.
	 */
	result_t readPutResult() { return readResponse(); }
	/**
	 * @brief Reads a TStorage response header. Used with PUTA.
	 * @see `readResponse()`
	 * @return The status code.
	 */
	result_t readPutAResult() { return readResponse(); }
	/**
	 * @brief Retrieves the next record from the server.
	 *
	 * The payload is supplied in a raw byte-stream form. It's located in a
	 * read-only memory block pointed to by `oPayloadPtr`. This pointer might
	 * become invalid after the next read operation; not deserializing the
	 * payload beforehand risks loss of data.
	 *
	 * The possible error codes are:
	 *  - `result_t::BAD_RESPONSE`
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::END_OF_STREAM`
	 *  - `result_t::MEMORY_LIMIT_EXCEEDED`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @param[out] oKey The key of the read record.
	 * @param[out] oPayloadPtr The memory address under which the raw payload is located.
	 * @param[out] oPayloadSize The length of the memory block representing the payload.
	 * @return The status code.
	 */
	result_t readNextRecordData(
		Key& oKey, const void*& oPayloadPtr, std::size_t& oPayloadSize);
	/**
	 * @brief Retrieves the ACQ timestamp returned by the server upon completion
	 * of a GET request.
	 * @see `readAcq()`
	 * @param oAcq The ACQ timestamp.
	 * @return The status code.
	 */
	result_t readGetResult(Key::AcqT& oAcq) { return readAcq(oAcq); }
	/**
	 * @brief Retrieves the result of GETACQ query.
	 *
	 * The possible error codes are:
	 *  - `result_t::BAD_RESPONSE`
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::ERROR`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @param oAcq The ACQ timestamp of the last transaction confirmed by the server.
	 * @return The status code.
	 */
	result_t readAcq(Key::AcqT& oAcq);

	/**
	 * @brief Sets the timeout of the underlying socket (for both receive and
	 * send operations).
	 * @see `Channel::setTimeout()`
	 * @param timeout Timeout in milliseconds.
	 */
	void setTimeoutMs(std::int64_t timeout) { mSocket.setTimeoutMs(timeout); }
	/**
	 * @brief Sets the target address/port pair of the underlying socket.
	 * @see `Channel::setHost()`
	 * @param addr Hostname of the target TStorage instance.
	 * @param port Port that the target TStorage instance is listening to.
	 */
	void setHost(const std::string& addr, std::uint16_t port)
	{
		mSocket.setHost(addr, port);
	}
	/**
	 * @brief Sets the new size of the internal buffer.
	 *
	 * After a successful call to this method it is guaranteed that the internal
	 * buffer will have the size of exactly `memoryLimitBytes` whenever it is
	 * active.
	 *
	 * The buffer is active only in the interval between `connect()` and the
	 * subsequent `close()`. When inactive, the memory block holding the buffer
	 * contents is not allocated.
	 *
	 * @see `Channel::setMemoryLimit()`
	 * @param memoryLimitBytes The new size of the internal buffer in bytes.
	 */
	void setMemoryLimit(std::size_t memoryLimitBytes);

private:

	/*******************
	 * Private methods
	 */


	/**
	 * @brief Serializes and sends a request header for command `cmdId` involving
	 * a right-open key-interval `[keyMin, keyMax)`.
	 *
	 * Used for initiating GET and GETACQ requests.
	 *
	 * Performs validity checks for the individual keys and the key-interval. If
	 * any of the arguments is invalid, a `result_t::INVALID_KEY` is returned. If
	 * the specified key-range is empty, a `result_t::EMPTY_KEY_RANGE` is
	 * returned.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNRESET`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::EMPTY_KEY_RANGE`
	 *  - `result_t::INVALID_KEY`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::OUT_OF_MEMORY`
	 *  - `result_t::SIGNAL`
	 *
	 * @param cmdId Command ID (see `Headers.h`).
	 * @param keyMin The lower vertex of the key-interval.
	 * @param keyMax The upper vertex of the key-interval.
	 * @return The status code.
	 */
	result_t writeKeyPairHeader(CommandType cmdId, const Key& keyMin, const Key& keyMax);
	/**
	 * @brief Serializes and sends a request header for command `cmdId` without
	 * additional data.
	 *
	 * Used for initiating PUT and PUTA requests.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNRESET`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::OUT_OF_MEMORY`
	 *  - `result_t::SIGNAL`
	 *
	 * @param cmdId Command ID (see `Headers.h`).
	 * @return The status code.
	 */
	result_t writeEmptyHeader(CommandType cmdId);

	/**
	 * @brief Returns a contiguous memory block of length `payloadSize` for
	 * payload serialization purposes.
	 *
	 * The returned memory address points to the exact spot inside the output
	 * buffer where the next record's payload should be located. The contents of
	 * the block will be sent as a payload to the connected TStorage instance
	 * as-is, without any further checks or copies. Not respecting the bounds of
	 * the memory block might corrupt the whole request, so it is well-advised to
	 * handle the buffer with extra care.
	 *
	 * The requested amount of bytes might not be available inside the internal
	 * buffer at the time of the method call. In this case, the method checks
	 * whether it exceeds the total buffer capacity; in this case,
	 * `result_t::MEMORY_LIMIT_EXCEEDED` is returned. Otherwise, in case the
	 * internal buffer is about to overflow but the requested amount of bytes
	 * would be available if the buffer was empty, it ends the current data batch
	 * and sends it over to TStorage, discarding the buffer's content and
	 * enabling its reuse.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNRESET`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::MEMORY_LIMIT_EXCEEDED`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @param[out] oPayloadBuffer The reserved memory block.
	 * @param[out] oBufferSize The memory block length in bytes.
	 * @param[in] payloadSize The minimal length of the memory block to reserve
	 * (in bytes).
	 * @param[in] key The key of the currently processed record, the payload of
	 * which will be the content of the reserved memory block.
	 * @param[in] keySize The size of the key in bytes.
	 * @return The status code.
	 */
	result_t reservePayloadBuffer(void*& oPayloadBuffer,
		std::size_t& oBufferSize,
		std::size_t payloadSize,
		const Key& key,
		std::size_t keySize);

	/**
	 * @brief Flushes the content of the internal buffer to the connected
	 * TStorage instance.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNRESET`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @return The status code.
	 */
	result_t sendBuffer();
	/**
	 * @brief Makes sure that `amountBytes` of data is available to read inside
	 * the internal buffer, fetching it from the connected TStorage instance if
	 * necessary.
	 *
	 * The possible error codes are:
	 *  - `result_t::BAD_RESPONSE`
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::MEMORY_LIMIT_EXCEEDED`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @param amountBytes Amount of bytes to make available to read.
	 * @return The status code.
	 */
	result_t requestData(std::size_t amountBytes);
	/**
	 * @brief Ignores `amountBytes` of incoming data, skipping them inside the
	 * internal buffer if present, then fetching and ignoring the rest.
	 *
	 * The possible error codes are:
	 *  - `result_t::BAD_RESPONSE`
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @param amountBytes Amount of bytes to skip.
	 * @return The status code.
	 */
	result_t skip(std::size_t amountBytes);

	/**
	 * @brief Discards the content of the internal buffer and resets state of
	 * internal serializers.
	 */
	void resetState();


	/****************
	 * Class fields
	 */

	/**
	 * @brief The internal buffer object.
	 *
	 * Remains uninitialized until the first call to `connect()`. After that, it
	 * owns a memory block of length `mMemoryLimit` until subsequent `close()`,
	 * which resets the buffer object to its initial state.
	 */
	Buffer mBuffer;
	/**
	 * @brief A Socket object managing network I/O.
	 *
	 * Used for talking with TStorage instance. Explicit sends and receives using
	 * this object are found only in `sendBuffer()`, `requestData()` and
	 * `skip()` methods.
	 */
	Socket mSocket;
	/**
	 * @brief A hint used to suggest a size of the payload buffer block that is
	 * likely to accomodate the whole payload after serialization when the actual
	 * size of the payload byte-stream is not known.
	 *
	 * It is equal to the maximum of `cInitialExpectedPayloadSize` and the
	 * maximal payload size encountered during the lifetime of the Channel.
	 *
	 * @see `obtainPutPayloadBuffer()`
	 * @see `obtainPutAPayloadBuffer()`
	 */
	std::size_t mExpectedPayloadSize;
	/**
	 * @brief The size of the internal buffer in bytes.
	 *
	 * It limits the total size of the response to the `get()` query and the size
	 * of a single batch of records received as part of the `getStream()` query.
	 *
	 * @see `Channel::get()`
	 * @see `Channel::getStream()`
	 */
	std::size_t mMemoryLimit;
	/**
	 * @brief An object tracking the current PUT/A batch state inside the send
	 * buffer and managing overall single-batch serialization.
	 */
	BatchSerializer mBatch;
};


/********************************
 * Inline method implementation
 */

result_t ChannelImpl::writeGetRequest(const Key& keyMin, const Key& keyMax)
{
	return writeKeyPairHeader(CommandType::GET, keyMin, keyMax);
}

result_t ChannelImpl::writeGetAcqRequest(const Key& keyMin, const Key& keyMax)
{
	return writeKeyPairHeader(CommandType::GETACQ, keyMin, keyMax);
}

result_t ChannelImpl::writePutHeader()
{
	const result_t res = writeEmptyHeader(CommandType::PUT);
	mBatch.endBatch();
	return res;
}

result_t ChannelImpl::writePutAHeader()
{
	const result_t res = writeEmptyHeader(CommandType::PUTA);
	mBatch.endBatch();
	return res;
}

result_t ChannelImpl::obtainPutPayloadBuffer(
	void*& oPayloadBuffer, std::size_t& oBufferSize, const Key& key)
{
	return reservePayloadBuffer(oPayloadBuffer, oBufferSize, mExpectedPayloadSize, key,
		Serializer::cAbbrevKeySizeWithoutAcq);
}

result_t ChannelImpl::obtainPutAPayloadBuffer(
	void*& oPayloadBuffer, std::size_t& oBufferSize, const Key& key)
{
	return reservePayloadBuffer(oPayloadBuffer, oBufferSize, mExpectedPayloadSize, key,
		Serializer::cAbbrevKeySize);
}

result_t ChannelImpl::reservePutPayloadBuffer(void*& oPayloadBuffer,
	std::size_t& oBufferSize,
	std::size_t payloadSize,
	const Key& key)
{
	return reservePayloadBuffer(oPayloadBuffer, oBufferSize, payloadSize, key,
		Serializer::cAbbrevKeySizeWithoutAcq);
}

result_t ChannelImpl::reservePutAPayloadBuffer(void*& oPayloadBuffer,
	std::size_t& oBufferSize,
	std::size_t payloadSize,
	const Key& key)
{
	return reservePayloadBuffer(
		oPayloadBuffer, oBufferSize, payloadSize, key, Serializer::cAbbrevKeySize);
}

} /*namespace impl*/
} /*namespace tstorage*/

#endif
