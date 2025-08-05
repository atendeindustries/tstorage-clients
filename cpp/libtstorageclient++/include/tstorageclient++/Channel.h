/*
 * TStorage: Client library (C++)
 *
 * Channel.h
 *   An implementation of the TStorage communication protocol.
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

#ifndef D_TSTORAGE_CHANNEL_H
#define D_TSTORAGE_CHANNEL_H

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <ratio>
#include <string>
#include <type_traits>

#include "DataTypes.h"
#include "PayloadType.h"
#include "RecordsSet.h"
#include "Response.h"
#include "ResponseAcq.h"
#include "ResponseGet.h"

#include "ChannelBase.h"

/** @file
 * @brief Defines the `Channel<T>` class. */

namespace tstorage {

using impl::ChannelBase;

/**
 * @brief A TCP communication channel and client for TStorage.
 *
 * Allows to send and receive data from a TStorage instance via TCP. It
 * supports the following commands:
 *  - `PUT`    - store a set of key-payload pairs,
 *  - `PUTA`   - store a set of key-payload pairs with prescribed acquisition
 *               timestamps,
 *  - `GET`    - fetch a set of key-payload pairs contained in the given key
 *               interval,
 *  - `GETACQ` - fetch the server-side acquisition time of the last
 *               fully-committed record inside a given key interval.
 *
 * Each command is executed by the corresponding method, which returns a
 * subobject of type `Response` containing the status code of the operation and
 * the requested data (if any). On success, the channel is kept open, allowing
 * to string commands together during a single TCP session. On failure, the
 * connection is closed automatically and a response with a
 * `tstorage::result_t` status code indicating an error is returned. For a list
 * of error codes, consult the documentation of individual methods and the
 * `DataTypes.h` header file.
 *
 * Copying objects of this class is explicitly disallowed.
 *
 * @tparam T Internal data type of the payload.
 */
template<typename T>
class Channel final : public ChannelBase
{
	static_assert(std::is_default_constructible<T>::value,
		"The `Channel` parameter type is not default-constructible");

	static_assert(std::is_move_constructible<T>::value,
		"The `Channel` parameter type is not move-constructible");

public:
	/**
	 * @brief Constructs a communication channel with a TStorage server under
	 * a given address.
	 *
	 * @param hostname Hostname of the target TStorage server.
	 * @param port Port under which the TStorage server accepts connections.
	 *
	 * @param payloadType A unique_ptr to a PayloadType instance. The ownership
	 * of the underlying object is transferred to Channel. It's role is to
	 * de-/serialize records' payloads.
	 */
	Channel(const std::string& hostname,
		std::uint16_t port,
		std::unique_ptr<PayloadType<T>> payloadType);

	/**
	 * @brief Constructs a communication channel with a TStorage server under
	 * a given address and with a given memory limit for GET requests.
	 *
	 * @param hostname Hostname of the target TStorage server.
	 * @param port Port under which the TStorage server accepts connections.
	 *
	 * @param payloadType A unique_ptr to a PayloadType instance. The ownership
	 * of the underlying object is transferred to Channel. It's role is to
	 * de-/serialize records' payloads.
	 *
	 * @param memoryLimitBytes Size of the internal buffer used for PUT/A and GET
	 * requests, and an upper limit for the amount of incoming data during
	 * realization of GET requests.
	 */
	Channel(const std::string& hostname,
		std::uint16_t port,
		std::unique_ptr<PayloadType<T>> payloadType,
		std::size_t memoryLimitBytes);

	/**
	 * @brief Closes the channel if still open and frees all allocated resources.
	 */
	~Channel();

	Channel(const Channel&) = delete;
	/** @brief A move constructor, moves channel resources to the other instance.
	 * */
	Channel(Channel&&) = default;
	Channel& operator=(const Channel&) noexcept = delete;
	/** @brief A move-assignment operator, moves channel resources to the other
	 *  instance. */
	Channel& operator=(Channel&&) noexcept = default;

	/**
	 * @brief Establishes connection to the server.
	 *
	 * Attempts to connect to the TStorage server under the `hostname:port` pair
	 * passed to the constructor of the channel and allocates all necessary
	 * resources to support communications. If the connection is already
	 * established, the call is silently ignored and a success code is
	 * returned. On failure, returns an error code.
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
	 * @return A `Response` instance with the status code of the executed operation.
	 */
	Response connect();
	/**
	 * @brief Closes the current connection.
	 *
	 * Attempts to close the current connection. If the connection on the current
	 * channel hasn't been established yet, or `close()` was called and the
	 * connection was not reestablished again before the current call, the call
	 * is silently ignored and a `result_t::NOT_CONNECTED` response is issued. If
	 * the connection was broken before closing it or closing the connection
	 * fails in any way, the call to `close()` reports a `result_t::CONNERROR`
	 * response. Otherwise, the connection is successfully closed and success
	 * value is returned.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNERROR`
	 *  - `result_t::NOT_CONNECTED`
	 *
	 * @return A `Response` instance with the status code of the executed operation.
	 */
	Response close();
	/**
	 * @brief Sets the timeout for channel send/receive operations.
	 *
	 * @param timeout Timeout in milliseconds.
	 */
	void setTimeout(std::chrono::duration<std::int64_t, std::milli> timeout);
	/**
	 * @brief Sets the maximal memory usage of the channel.
	 *
	 * Use this setting to limit memory usage of a given Channel<T> instance.
	 * Each Channel<T> object uses a heap-allocated internal buffer for data
	 * de-/serialization, the maximal size of which is controlled by the
	 * `memoryLimitBytes` parameter. By default, the buffer's size is 64KiB.
	 *
	 * - For `get()` requests: a response exceeding the maximal internal buffer's
	 * capacity in size is cut short and the connection is broken to save
	 * bandwidth.
	 *
	 * - For `getStream()` requests: a response is split into chunks of size not
	 * exceeding the maximal capacity of the Channel's internal buffer.
	 *
	 * This setting also limits the size of internal buffers used by `put()` and
	 * `puta()` operations. This does not limit the amount of records sent during
	 * a single PUT/A push request, though one important caveat to bear in mind
	 * is that the whole payload, together with extra 48 bytes (56 bytes in case
	 * of PUTA) of constant overhead must fit inside the buffer. If the buffer is
	 * too short, the operation will fail with `result_t::MEMORY_LIMIT_EXCEEDED`
	 * error code inside the returned `Response`.
	 *
	 * For example, the minimal capacity of the internal buffer required to send
	 * 1 million records with `float` payloads via PUT protocol is `48B + 4B =
	 * 52B`. The user should invoke `channel.setMemoryLimit(x);` for `x >= 52` at
	 * any point before calling `channel.put(...)` to give the operation a chance
	 * to succeed.
	 *
	 * @see get()
	 * @see getStream()
	 * @see put()
	 *
	 * @param memoryLimitBytes New memory limit in bytes.
	 */
	void setMemoryLimit(std::size_t memoryLimitBytes);

	/**
	 * @brief Stores a given set of records in a TStorage instance.
	 *
	 * Sends the `RecordsSet<T>` `data` to a TStorage instance over a previously
	 * established TCP connection, serializing the payload of each record using
	 * the `PayloadType::toBytes()` call. This method uses the PUT storage
	 * protocol intended for regular usage, where the server stamps each incoming
	 * record with its own acquisition time timestamp (ACQ). The `key.acq` field
	 * of each record inside `data` remains unused.
	 *
	 * Not all records can be sent to TStorage; some key-payload combinations
	 * result in invalid records. If an invalid record is encountered at any
	 * point during the PUT command execution, the `put()` method will exit
	 * early, but not after trying to send all the valid records from `data()` up
	 * to the record just before the first invalid one; the records after that
	 * are not sent. The method will then return an error response signalling
	 * what exactly was at fault.
	 *
	 * For each record's key `key`, the fields `key.cid`, `key.mid`, `key.moid`
	 * and `key.cap` cannot attain maximal values. If this happens to be the
	 * case, the corresponding record is deemed invalid and, when encountered,
	 * the `put()` response will contain a `result_t::INVALID_KEY` error code.
	 *
	 * As for the payload, its size (as reported by `PayloadType::toBytes()`)
	 * cannot exceed 32MiB. Otherwise, the record is deemed invalid and, when
	 * encountered, the `put()` response will contain a
	 * `result_t::PAYLOAD_TOO_LARGE` error code. Overall, the records size is
	 * bound by the internal's buffer capacity set by a `setMemoryLimit()` call.
	 * On the event when the method attempts to serialize a record with size
	 * exceeding the buffer's capacity, the operation is aborted and a `Response`
	 * with the `result_t::MEMORY_LIMIT_EXCEEDED` error code is returned. See
	 * `setMemoryLimit()` for details.
	 *
	 * The connection to TStorage must be established (see `connect()`) for the
	 * `put()` request to succeed. Otherwise, the method will return an error
	 * response with `result_t::NOT_CONNECTED`.
	 *
	 * Due to the way the PUT protocol works, this method is a bit more efficient
	 * for `data` containing longer consecutive runs of records with the same CID
	 * value. In particular, given a set of records having the same payloads (but
	 * differing in keys) the maximal performance is reached when the records are
	 * sorted by their CID values.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNRESET`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::ERROR`
	 *  - `result_t::INVALID_KEY`
	 *  - `result_t::MEMORY_LIMIT_EXCEEDED`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::PAYLOAD_TOO_LARGE`
	 *  - `result_t::SIGNAL`
	 *  - `result_t::OUT_OF_MEMORY`
	 *
	 * @param data A set of valid records to store in the TStorage instance.
	 *
	 * @return A `Response` instance with the status code of the executed operation.
	 */
	Response put(const RecordsSet<T>& data);
	/**
	 * @brief Stores a given set of records in a TStorage instance with
	 * user-supplied acquisition times (ACQ).
	 *
	 * Sends the RecordsSet<T> `data` to a TStorage instance over a previously
	 * established TCP connection, serializing the payload of each record using
	 * the `PayloadType::toBytes()` call. This method uses the PUTA storage
	 * protocol intended for maintenance/testing work, where the server's own
	 * acquisition timestamps are ignored for the sake of user supplied data. The
	 * records are passed for storage as they are, without any special treatment
	 * of their `key.acq` field or other restrictions on key-payload pairs
	 * (excluding some reserved keys and payload size limit, see below). This
	 * operation allows to, say, make an exact copy of a whole chunk of the
	 * database and send it to another instance of TStorage. Be wary that
	 * irresponsible usage might lead to an inconsistencies in the database, e.g.
	 * duplicates or distorted data, like capture times larger than acquisition
	 * times.
	 *
	 * Not all records can be sent to TStorage; some key-payload combinations
	 * result in invalid records. If an invalid record is encountered at any
	 * point during the PUT command execution, the `put()` method will exit
	 * early, but not after trying to send all the valid records from `data()` up
	 * to the record just before the first invalid one; the records after that
	 * are not sent. The method will then return an error response signalling
	 * what exactly was at fault.
	 *
	 * For each record's key `key`, the fields `key.cid`, `key.mid`, `key.moid`,
	 * `key.cap` and `key.acq` cannot attain maximal values. If this happens to
	 * be the case, the corresponding record is deemed invalid and, when
	 * encountered, the `put()` response will contain a `result_t::INVALID_KEY`
	 * error code.
	 *
	 * As for the payload, its size (as reported by `PayloadType::toBytes()`)
	 * cannot exceed 32MB. Otherwise, the record is deemed invalid and, when
	 * encountered, the `put()` response will contain a
	 * `result_t::PAYLOAD_TOO_LARGE` error code. Overall, the records size is
	 * bound by the internal's buffer capacity set by a `setMemoryLimit()` call.
	 * On the event when the method attempts to serialize a record with size
	 * exceeding the buffer's capacity, the operation is aborted and a `Response`
	 * with the `result_t::MEMORY_LIMIT_EXCEEDED` error code is returned. See
	 * `setMemoryLimit()` for details.
	 *
	 * The connection to TStorage must be established (see `connect()`) for the
	 * `put()` request to succeed. Otherwise, the method will return an error
	 * response with `result_t::NOT_CONNECTED`
	 *
	 * Due to the way the PUTA protocol works, this method is a bit more efficient
	 * for `data` containing longer consecutive runs of records with the same CID
	 * value. In particular, given a set of records having the same payloads (but
	 * differing in keys) the maximal performance is reached when the records are
	 * sorted by their CID values.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNRESET`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::ERROR`
	 *  - `result_t::INVALID_KEY`
	 *  - `result_t::MEMORY_LIMIT_EXCEEDED`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::PAYLOAD_TOO_LARGE`
	 *  - `result_t::SIGNAL`
	 *  - `result_t::OUT_OF_MEMORY`
	 *
	 * @param data A set of valid records to store in the TStorage instance.
	 *
	 * @return A `Response` instance with the status code of the executed operation.
	 */
	Response puta(const RecordsSet<T>& data);
	/**
	 * @brief Retrieves a set of records from a TStorage instance.
	 *
	 * Sends a request to a TStorage instance asking for data in the key-interval
	 * `[keyMin, keyMax)`. A record lies in this interval if and only if each
	 * field `x` of its key satisfies `keyMin.x <= key.x && key.x < keyMax.x`.
	 * Upon a successful request, a (possibly empty) RecordsSet<T> containing
	 * exactly the records from the given key-interval is constructed and
	 * returned along with a success code and the key-interval's last full-commit
	 * acquisition time (ACQ) timestamp (see `getAcq()`). The payloads are
	 * deserialized using `PayloadType::fromBytes()`. On error, the connection
	 * is forcefully broken, the ACQ field of the response is
	 * default-initialized, and all the records obtained up to this point while
	 * processing the request are gathered into a RecordsSet<T> structure and
	 * returned as a part of the response with an error code.
	 *
	 * Note that the `RecordsSet<T>` returned in case of error might be
	 * incomplete, i.e. it might contain only a fraction of all records present
	 * in the database contained in the supplied key-interval. The records
	 * obtained this way might not be sorted according to ACQ, hence it is not
	 * safe to assume that another fetch starting at the maximal ACQ among the
	 * keys received up to this point will complete the request.
	 *
	 * It may happen that the total amount of bytes received via the channel
	 * exceeds the memory limit set by the call to
	 * `Channel<T>::setMemoryLimit()`. This event is treated as an error. In this
	 * case, the GET operation is aborted and, the corresponding error code
	 * returned in `ResponseGet<T>` is `result_t::MEMORY_LIMIT_EXCEEDED`.
	 *
	 * This method assumes that all records within the specified key-interval
	 * have payloads with the same data type, meaning that they are deserialized
	 * to values of the same data type, namely the type represented by the class
	 * template parameter `T`. On encountering a record not conforming to the
	 * user-supplied deserialization protocol (signalled by
	 * `PayloadType::fromBytes() == false`) the method returns an error
	 * response with `result_t::DESERIALIZATION_ERROR`.
	 *
	 * The individual keys passed to the method and the key-interval they form
	 * are validated before the query is made. If the CID field of one of the
	 * keys is negative, the method exits with a default-initialized response
	 * containing the `result_t::INVALID_KEY` error code. If the corresponding
	 * right-open key-interval is empty, the method exits with
	 * `result_t::EMPTY_KEY_RANGE` response. In both cases, no contact with the
	 * server is made.
	 *
	 * One has to be wary that the server signals an error if `keyMin.acq` is
	 * greater than the `acq` field of the result of a successful call to
	 * `channel.getAcq()`, or, in other words, if the client attempts to "reach
	 * into the future" be means of GET requests. This does not apply to
	 * `keyMax.acq`; in case `keyMax.acq` is greater than the current full-commit
	 * ACQ retrieved by the server, the upper limit of the interval is
	 * essentailly brought down to the current ACQ for consistency with future
	 * GETs.
	 *
	 * The connection to TStorage must be established (see `connect()`) for the
	 * `put()` request to succeed. Otherwise, the method will return an error
	 * response with `result_t::NOT_CONNECTED`
	 *
	 * The possible error codes are:
	 *  - `result_t::BAD_RESPONSE`
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNRESET`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::DESERIALIZATION_ERROR`
	 *  - `result_t::EMPTY_KEY_RANGE`
	 *  - `result_t::ERROR`
	 *  - `result_t::INVALID_KEY`
	 *  - `result_t::MEMORY_LIMIT_EXCEEDED`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *  - `result_t::OUT_OF_MEMORY`
	 *
	 * @param keyMin The lower vertex of the right-open key-interval
	 * containing the desired data.
	 *
	 * @param keyMax The upper vertex of the right-open key-interval
	 * containing the desired data.
	 *
	 * @return `ResponseGet<T>(result_t::OK, records, acq)` with a valid ACQ and
	 * a `RecordsSet<T>` `records` containing all fetched records on receiving a
	 * confirmation of successful database fetch, `ResponseGet<T>(status)` on
	 * error in case when no meaningful contact with the server was made,
	 * `ResponseGet<T>(status, partialFetchResult)` otherwise.
	 */
	ResponseGet<T> get(const Key& keyMin, const Key& keyMax);
	/**
	 * @brief Queries the TStorage instance for the last full-commit acquisition
	 * time (ACQ) timestamp for the given key-interval.
	 *
	 * The ACQ timestamp returned by this query provides a consistency guarantee
	 * for the data returned by every future GET request for the given
	 * key-interval: the state of the database within the given key-interval up
	 * to the returned ACQ will remain fixed throughout the database's lifetime.
	 * In more concrete terms, each two `get(keyMin, keyMax)` responses will
	 * always contain the same sets of records whose `record.key.acq` is less
	 * than `getAcq(keyMin, keyMax).acq()`.
	 *
	 * The individual keys passed to the method and the key-interval they form
	 * are validated before the query is made. If the CID field of one of the
	 * keys is negative, the method exits with a default-initialized response
	 * containing the `result_t::INVALID_KEY` error code. If the corresponding
	 * right-open key-interval is empty, the method exits with
	 * `result_t::EMPTY_KEY_RANGE` response. In both cases, no contact with the
	 * server is made.
	 *
	 * In the event that the key-interval "points to the future", i.e.
	 * `keyMin.acq` is strictly larger than the servers time, the server signals
	 * a `result_t::ERROR`.
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNRESET`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::EMPTY_KEY_RANGE`
	 *  - `result_t::ERROR`
	 *  - `result_t::INVALID_KEY`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *
	 * @param keyMin The lower vertex of the right-open reference key-interval.
	 * @param keyMax The upper vertex of the right-open reference key-interval.
	 *
	 * @return `ResponseAcq``(result_t::OK, acq)` on receiving the `acq` timestamp,
	 * `ResponseAcq``(err)` with an error code `err` otherwise.
	 */
	ResponseAcq getAcq(const Key& keyMin, const Key& keyMax);
	/**
	 * @brief Batch-streams a set of records from a TStorage instance through
	 * a callback function.
	 *
	 * Acts essentially the same as the standard get(), except it retrieves the
	 * response in batches of maximal size set by a call to `setMemoryLimit()`.
	 * These batches are processed one at a time using the `callback` callable in
	 * their entirety before fetching another one, thereby avoiding a large
	 * memory overhead inherent in processing entire responses at once.
	 * Especially useful if the expected size of the response is very large.
	 *
	 * In contrast to `get()`, this method does not place any limit on size of
	 * GET responses. Instead, the memory limit set by `setMemoryLimit()` governs
	 * the size of a single record batch that is processed before fetching
	 * another one.
	 *
	 * The method may return a response with status code
	 * `result_t::MEMORY_LIMIT_EXCEEDED`, however, if the size of any of the
	 * received records does not fit inside the specified memory limit. This
	 * might occur if, for instance, the client receives a record with payload
	 * size 32MB and the memory limit is set to 4MB. To mitigate this error, set
	 * a higher memory limit, preferably a bit higher than the maximum payload
	 * size of 32MB (see `setMemoryLimit()`).
	 *
	 * The method acts otherwise in the same way as its standard `get()`
	 * counterpart. Please refer to the `get()` documentation for more info.
	 *
	 * @see get()
	 *
	 * The possible error codes are:
	 *  - `result_t::CONNCLOSED`
	 *  - `result_t::CONNERROR`
	 *  - `result_t::CONNRESET`
	 *  - `result_t::CONNTIMEOUT`
	 *  - `result_t::DESERIALIZATION_ERROR`
	 *  - `result_t::EMPTY_KEY_RANGE`
	 *  - `result_t::ERROR`
	 *  - `result_t::INVALID_KEY`
	 *  - `result_t::MEMORY_LIMIT_EXCEEDED`
	 *  - `result_t::NOT_CONNECTED`
	 *  - `result_t::SIGNAL`
	 *  - `result_t::OUT_OF_MEMORY`
	 *
	 * @param keyMin The lower vertex of the right-open key-interval
	 * containing the desired data.
	 *
	 * @param keyMax The upper vertex of the right-open key-interval
	 * containing the desired data.
	 *
	 * @param callback A callable that will be called on each batch of records
	 * forming the response.
	 *
	 * @return `ResponseAcq(result_t::OK, acq)` with a valid ACQ timestamp on a
	 * successfully completed database fetch, `ResponseAcq(err)` with an error
	 * code `err` otherwise.
	 */
	ResponseAcq getStream(const Key& keyMin,
		const Key& keyMax,
		const std::function<void(RecordsSet<T>&)>& callback);

private:

	/**************************
	 * Implementation details
	 */

	/** @brief A dispatch table containing function pointers to several library
	 * functions appropriate for a specific PUT protocol. */
	struct PutMethods;
	/** @brief A PUT protocol tag. */
	enum ProtoT {
		PUT,
		PUTA,
	};

	/**
	 * @brief Returns a `PutMethods` dispatch table for a specific protocol.
	 * @param tag Protocol tag.
	 * @return A preset dispatch table.
	 */
	static constexpr PutMethods putMethods(ProtoT tag);

	/**
	 * @brief A common implementation of a PUT/A request.
	 *
	 * Currently, the only difference between the implementations of `put()` and
	 * `puta()` lies in the names of some library functions called at some
	 * crucial places in the code. We use the template parameter to disambiguate
	 * between the two sets of functions by referring to the results of a
	 * `putMethods()` static method call.
	 *
	 * @tparam Tag A protocol tag.
	 * @param recordSet The set of records to store in the database.
	 * @return The status code to pass to the user inside the `Response` of
	 * `put()`/`puta()`.
	 */
	template<ProtoT Tag>
	result_t putImpl(const RecordsSet<T>& recordSet);
	/**
	 * @brief Serialize and write a single record.
	 *
	 * @tparam Tag A protocol tag.
	 * @param record A record to send.
	 * @return An internal status code.
	 */
	template<ProtoT Tag>
	result_t serializeAndWriteRecord(const Record<T>& record);
	/**
	 * @brief Write an end-of-stream marker and receive the command status code
	 * from the server.
	 *
	 * @tparam Tag A protocol tag.
	 * @return An internal status code.
	 */
	template<ProtoT Tag>
	result_t putFinalize();

	/**
	 * @brief Prepare the next batch of records from a GET response for
	 * processing inside a `getStream()` callback.
	 *
	 * @param[in, out] recordSet The set of records to append the results to.
	 * @return An internal status code.
	 */
	result_t recvAndDeserializeRecordTo(RecordsSet<T>& recordSet);
	/**
	 * @brief Append the next record from a GET response to a given
	 * `RecordsSet<T>`.
	 *
	 * @param[in, out] recordSet The set of records to append the record to.
	 * @return An internal status code.
	 */
	result_t recvAndDeserializeBatchTo(RecordsSet<T>& recordSet);


	/****************
	 * Class fields
	 */

	/**
	 * @brief An object of type `PayloadType<T>` responsible for serialization
	 * and deserialization of record payloads from and to objects of type `T`.
	 */
	std::unique_ptr<PayloadType<T>> mPayloadType;
};

} /*namespace tstorage*/

#include "Channel.tpp"

#endif
