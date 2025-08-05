/*
 * TStorage: Client library (C++)
 *
 * ChannelBase.h
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

#ifndef D_TSTORAGE_CHANNELBASE_H
#define D_TSTORAGE_CHANNELBASE_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <ratio>
#include <string>

#include "DataTypes.h"

/** @file
 * @brief Defines the base class of each `Channel<T>` and a major part of the
 * library's ABI. */

namespace tstorage {
namespace impl {

class ChannelImpl;

/**
 * @brief A base class of each Channel<T>.
 *
 * This class defines the ABI between the templated classes comprising the
 * user-exposed part of the library and the library itself. Its methods
 * abstract away implementation details of the client and are not accessible
 * for direct use. Their declarations and signatures can be safely considered
 * stable. The stability of their definitions, on the other hand, should not be
 * relied upon, as their implementations (as well as that of any other library
 * component under the `src/` directory tree) might change without prior
 * warning between versions.
 *
 * To maintain a clean dependency separation between the API and the internals,
 * the class uses a variant of the PIMPL idiom (a pointer to implementation) to
 * implement all of the methods declared below.
 *
 * The methods provide a high level error-checked primitives for data exchange
 * with the TStorage server, collectively responsible for initiating,
 * sustaining and performing TCP communication, providing access to
 * readable/writable buffers for use with PayloadSize instances, and adjusting
 * Channel settings. Their intended role is to form building blocks from which
 * the implementation of the TStorage communication protocol is composed (see
 * `<tstorageclient++/Channel.tpp>).
 *
 * For some more details regarding its implementation, please refer to the
 * documentation inside the `ChannelBase.cpp` file.
 *
 * @see `ChannelBase.cpp`
 */
class ChannelBase
{
public:
	/**
	 * @brief A default constructor.
	 *
	 * Sets up a raw channel with default settings. These settings can be adjusted
	 * afterwards, and will take effect on the next call to `Channel::connect()`
	 * if not connected, or immediately, if a connection is currently established.
	 */
	ChannelBase();
	/**
	 * @brief A default destructor.
	 *
	 * Closes the connection and frees all resources allocated to the Channel.
	 */
	~ChannelBase();

	ChannelBase(const ChannelBase&) = delete;
	/**
	 *  @brief A move constructor, moves channel resources to the other instance.
	 */
	ChannelBase(ChannelBase&&) = default;
	ChannelBase& operator=(const ChannelBase&) = delete;
	/**
	 *  @brief A move-assignment operator, moves channel resources to the other
	 *  instance.
	 */
	ChannelBase& operator=(ChannelBase&&) = default;

protected:
	/**
	 * @brief Establishes a TCP connection to a TStorage instance.
	 *
	 * The address/port pair of the TStorage instance is set by
	 * `ChannelBase::setHost()`.
	 *
	 * @see `Channel::connect()`
	 *
	 * @return The status code.
	 */
	result_t connectImpl();
	/**
	 * @brief Gracefully closes the TCP connection if still connected.
	 *
	 * @see `Channel::close()`
	 *
	 * @return Status code.
	 */
	result_t closeImpl();
	/**
	 * @brief Forcefully closes the connection.
	 */
	void abort();

	/**
	 * @brief Initiates the GET request for all records with keys lying in the
	 * `[keyMin, keyMax)` key-interval through an open channel.
	 * @param keyMin The lower vertex of the key-interval.
	 * @param keyMax The upper vertex of the key-interval.
	 * @return Status code.
	 */
	result_t writeGetRequest(const Key& keyMin, const Key& keyMax);
	/**
	 * @brief Initiates the GETACQ request for the `[keyMin, keyMax)`
	 * key-interval through an open channel.
	 * @param keyMin The lower vertex of the key-interval.
	 * @param keyMax The upper vertex of the key-interval.
	 * @return Status code.
	 */
	result_t writeGetAcqRequest(const Key& keyMin, const Key& keyMax);

	/**
	 * @brief Initiates the PUT request through an open channel.
	 * @return Status code.
	 */
	result_t writePutHeader();
	/**
	 * @brief Initiates the PUTA request through an open channel.
	 * @return Status code.
	 */
	result_t writePutAHeader();

	/**
	 * @brief Supplies the client with a writable buffer to deserialize the records
	 * into. Used with PUT requests.
	 *
	 * @param[out] oPayloadBuffer A writable memory chunk for data serialization.
	 * @param[out] oBufferSize The size of the memory chunk.
	 * @param[in] key The key of the next record to serialize.
	 * @return Status code.
	 */
	result_t obtainPutPayloadBuffer(
		void*& oPayloadBuffer, std::size_t& oBufferSize, const Key& key);
	/**
	 * @brief Supplies the client with a writable buffer to deserialize the records
	 * into. Used with PUTA requests.
	 *
	 * @param[out] oPayloadBuffer A writable memory chunk for data serialization.
	 * @param[out] oBufferSize The size of the memory chunk.
	 * @param[in] key The key of the next record to serialize.
	 * @return Status code.
	 */
	result_t obtainPutAPayloadBuffer(
		void*& oPayloadBuffer, std::size_t& oBufferSize, const Key& key);
	/**
	 * @brief Supplies the client with a writable buffer to deserialize the records
	 * into. A successful call to this method guarantees that `oBufferSize >=
	 * payloadSize`. Used with PUT requests.
	 *
	 * @param[out] oPayloadBuffer A writable memory chunk for data serialization.
	 * @param[out] oBufferSize The size of the memory chunk.
	 * @param[in] payloadSize The total size of serialized data.
	 * @param[in] key The key of the next record to serialize.
	 * @return Status code.
	 */
	result_t reservePutPayloadBuffer(void*& oPayloadBuffer,
		std::size_t& oBufferSize,
		std::size_t payloadSize,
		const Key& key);
	/**
	 * @brief Supplies the client with a writable buffer to deserialize the records
	 * into. A successful call to this method guarantees that `oBufferSize >=
	 * payloadSize`. Used with PUTA requests.
	 *
	 * @param[out] oPayloadBuffer A writable memory chunk for data serialization.
	 * @param[out] oBufferSize The size of the memory chunk.
	 * @param[in] payloadSize The total size of serialized data.
	 * @param[in] key The key of the next record to serialize.
	 * @return Status code.
	 */
	result_t reservePutAPayloadBuffer(void*& oPayloadBuffer,
		std::size_t& oBufferSize,
		std::size_t payloadSize,
		const Key& key);

	/**
	 * @brief Writes the previously serialized record through the `Channel`. Used
	 * with PUT protocol.
	 *
	 * Serializes the key together with some other metadata of the record and
	 * enqueues the resulting byte-stream for sending. Called after serializing the
	 * payload to a `Channel`-supplied buffer.
	 *
	 * @param key The key associated to the serialized record.
	 * @param payloadSize The size of the serialized record's payload.
	 * @return Status code.
	 */
	result_t writeNextPutRecord(const Key& key, std::size_t payloadSize);
	/**
	 * @brief Writes the previously serialized record through the `Channel`. Used
	 * with PUTA protocol.
	 *
	 * Serializes the key together with some other metadata of the record and
	 * enqueues the resulting byte-stream for sending. Called after serializing the
	 * payload to a `Channel`-supplied buffer.
	 *
	 * @param key The key associated to the serialized record.
	 * @param payloadSize The size of the serialized record's payload.
	 * @return Status code.
	 */
	result_t writeNextPutARecord(const Key& key, std::size_t payloadSize);
	/**
	 * @brief Finalizes the PUT/A request.
	 * @return Status code.
	 */
	result_t writeFin();

	/**
	 * @brief Reads the header of the server's response to GET requests.
	 *
	 * Used right after `writeGetRequest()` only.
	 *
	 * @return Status code.
	 */
	result_t readResponse();
	/**
	 * @brief Reads the result of the previous PUT command as reported by the
	 * server.
	 *
	 * Only used directly after a successful call to `writeFin()`.
	 *
	 * @return Status code.
	 */
	result_t readPutResult();
	/**
	 * @brief Reads the result of the previous PUTA command as reported by the
	 * server.
	 *
	 * Only used directly after a successful call to `writeFin()`.
	 *
	 * @return Status code.
	 */
	result_t readPutAResult();
	/**
	 * @brief Deserializes the key, reads the payload size and supplies a read-only
	 * buffer to raw payload data of the next received record to deserialize by the
	 * client.
	 *
	 * Used in a loop directly after a successful call to `readResponse()` during
	 * GET request processing as long as there are records available to read.
	 *
	 * @param[out] oKey The key of the deserialized record.
	 * @param[out] oPayloadPtr A pointer to a read-only memory block containing raw
	 * payload data to deserialize.
	 * @param[out] oPayloadSize The length of the returned memory block.
	 *
	 * @return Status code.
	 */
	result_t readNextRecordData(
		Key& oKey, const void*& oPayloadPtr, std::size_t& oPayloadSize);
	/**
	 * @brief Reads the result of the previous GET command as reported by the
	 * server.
	 *
	 * Only used directly after `readNextRecordData()` returns
	 * `result_t::END_OF_STREAM`.
	 *
	 * @param[out] oAcq The ACQ timestamp returned by the server.
	 * @return Status code.
	 */
	result_t readGetResult(Key::AcqT& oAcq);
	/**
	 * @brief Reads the result of the previous GETACQ command as reported by the
	 * server.
	 *
	 * Only used directly after a successful call to `writeGetAcqRequest()`.
	 *
	 * @param[out] oAcq The ACQ timestamp returned by the server.
	 * @return Status code.
	 */
	result_t readAcq(Key::AcqT& oAcq);

	/**
	 * @brief Sets the timeout of the underlying TCP socket to a given value (for
	 * both send and receive).
	 *
	 * The change takes effect instantaneously.
	 *
	 * @see `Channel::setTimeout()`
	 *
	 * @param timeout The timeout in milliseconds.
	 */
	void setTimeoutImpl(std::chrono::duration<std::int64_t, std::milli> timeout);
	/**
	 * @brief Sets the memory limit for GET requests.
	 *
	 * The change takes effect instantaneously.
	 *
	 * @see `Channel::setMemoryLimit()`
	 *
	 * @param memoryLimitBytes The new memory limit in bytes.
	 */
	void setMemoryLimitImpl(std::size_t memoryLimitBytes);
	/**
	 * @brief Sets the address/port pair of the target server.
	 *
	 * The change takes effect exactly when a new connection is established.
	 *
	 * @see `Channel::setHost()`
	 *
	 * @param addr The address of the server.
	 * @param port The server's port.
	 */
	void setHost(const std::string& addr, std::uint16_t port);

private:
	/**
	 * @brief A PIMPL.
	 */
	std::unique_ptr<ChannelImpl> mImpl;
};

} /*namespace impl*/
} /*namespace tstorage*/

#endif
