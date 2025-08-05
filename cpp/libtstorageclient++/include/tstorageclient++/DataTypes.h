/*
 * TStorage: Client library (C++)
 *
 * DataTypes.h
 *   A collection of fundamental data types for the TStorage protocol.
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

#ifndef D_TSTORAGE_DATATYPES_H
#define D_TSTORAGE_DATATYPES_H

#include <cstdint>
#include <limits>
#include <utility>

#include <tstorageclient++/Timestamp.h>

namespace tstorage {

/** @file
 *  @brief Defines the fundamental TStorage datatypes. */

/**
 * @brief An enumeration type containing status codes reported by each TStorage
 * operation.
 *
 * The values of this enum conform to the convention by which every non-error
 * status code has a non-negative value and every error code has a negative
 * value. The value `0` (`result_t::OK`) generally means that the operation
 * succeeded and there is nothing of significance to report. In the current
 * implementation we only distinguish between failure (value `-1`) and succcess
 * (value `0`) on the server side. The client/network errors are reported by
 * means of status codes with values less or equal `-513`, while their
 * non-error status codes have values greater or equal `512`.
 */
enum class result_t : std::int32_t {
	/** @brief Success, nothing to report. */
	OK = 0,
	/** @brief Failure, general server-side error. */
	ERROR = -1,

	/** @brief A key with either CID < 0 or a field with a maximal value
	 * (unreachable by GET) encountered. */
	INVALID_KEY = -513,
	/** @brief A key pair spanning an empty right-open key-interval has been
	 * passed to a GET/GETACQ request. */
	EMPTY_KEY_RANGE = -514,
	/** @brief A payload with size over the hard limit of 32MB encountered. */
	PAYLOAD_TOO_LARGE = -515,
	/** @brief Payload deserialization failed. */
	DESERIALIZATION_ERROR = -516,
	/** @brief For GET requests: response exceeds in size the user-set limit.
	 * For other requests: the size of a serialized record exceeds the user-ser
	 * limit.*/
	MEMORY_LIMIT_EXCEEDED = -517,
	/** @brief Client internal buffers not allocated due to insufficient memory.
	 * */
	OUT_OF_MEMORY = -518,

	/** @brief Received data does not conform to the TStorage communication
	 * protocol. */
	BAD_RESPONSE = -519,

	/** @brief General TCP socket error. */
	CONNERROR = -521,
	/** @brief The host under a given address does not accept TCP connections on
	 * the given port. */
	CONNREFUSED = -522,
	/** @brief No host under a given address. */
	BAD_ADDRESS = -523,
	/** @brief No connection established, operation not performed. */
	NOT_CONNECTED = -524,
	/** @brief Cannot obtain a socket file descriptor/handle. */
	SOCKET_ERROR = -525,
	/** @brief Failed to set socket options. */
	SETOPT_ERROR = -526,

	/** @brief Internal status code, signals the end of the GET response. */
	END_OF_STREAM = 512,

	/** @brief Received RST packet, connection reset by peer. */
	CONNRESET = 520,
	/** @brief Received/sent FIN packet, connection has closed gracefully. */
	CONNCLOSED = 521,
	/** @brief Connection timed out. */
	CONNTIMEOUT = 522,
	/** @brief Operation interrupted by a UNIX signal/system interrupt. */
	SIGNAL = 523,
};

/** @brief A maximal error code of client fatal errors. */
static constexpr std::int32_t cClientFatals = -513;
/** @brief A minimal error code of client non-fatal errors. */
static constexpr std::int32_t cClientIssues = 512;

/**
 * @brief The type of database primary keys used to uniquely identify records.
 *
 * Each key is 32 bytes in length and consists of 5 named fields:
 *  - CID - client ID, a non-negative integer,
 *  - MID - measurement device ID,
 *  - MOID - measured object ID,
 *  - CAP - measurement capture time,
 *  - ACQ - database acquisition time.
 *
 * Of the 5 fields, only the first 4 are specified by the user for a database
 * store operation. The remaining one, ACQ timestamp, is attached to each
 * record by TStorage upon database arrival, and its value is fetched from an
 * internal monotone timer of the database system.
 *
 * In contrast, all 5 fields can be used to query the server, e.g. by asking
 * the database to fetch all records received by the database at most 1 day ago
 * (meaning all records with ACQ timestamp between exactly 1 day ago and now).
 * One could use CAP instead of ACQ for a similar query, although the semantics
 * of such query would differ significantly, for example when TStorage is used
 * to store archival data.
 *
 * Both CAP and ACQ fields use TStorage internal timestamp format. To convert a
 * standad UNIX time to this format, use the conversion functions contained in
 * the `tstorage::Timestamp` namespace.
 *
 * If the CID field of the key is negative, the key is deemed invalid and is
 * rejected by all methods of the `Channel` class. Supplying it to one of these
 * methods results in an immediate exit upon encounter with an error response
 * return value.
 *
 * @see `Timestamp.h`
 */
struct Key
{
	/** @brief An integral type used to represent CIDs. */
	using CidT = std::int32_t;
	/** @brief An integral type used to represent MIDs. */
	using MidT = std::int64_t;
	/** @brief An integral type used to represent MOIDs. */
	using MoidT = std::int32_t;
	/** @brief An integral type used to represent CAPs. */
	using CapT = Timestamp::TimeT;
	/** @brief An integral type used to represent ACQs. */
	using AcqT = Timestamp::TimeT;

	/**
	 * @brief Default constructor, zero-initializes the key.
	 */
	Key() noexcept : Key(cCidInvalid, 0, 0, 0, 0) {}

	/**
	 * @brief A constructor.
	 *
	 * Requires passing at least CID, MID and MOID. The default CAP is the
	 * current time (TStorage internal format). The default ACQ is `Key::cAcqMax`
	 * to guard against sending incorrectly initialized data using PUTA command.
	 *
	 * @param pCid Initial client ID value.
	 * @param pMid Initial measurement device ID value.
	 * @param pMoid Initial measured quantity ID value.
	 * @param pCap Initial capture time (in TStorage internal format).
	 * @param pAcq Initial acquisition time (in TStorage internal format).
	 */
	Key(CidT pCid,
		MidT pMid,
		MoidT pMoid,
		CapT pCap = Timestamp::now(),
		AcqT pAcq = cAcqMax) noexcept
		: cid(pCid), mid(pMid), moid(pMoid), cap(pCap), acq(pAcq)
	{
	}

	/** @brief Client ID */
	CidT cid;
	/** @brief Measurement device ID */
	MidT mid;
	/** @brief Measured quantity ID */
	MoidT moid;
	/** @brief Capture time (TStorage internal timestamp) */
	CapT cap;
	/** @brief Database acquisition time (TStorage internal timestamp) */
	AcqT acq;

	/**
	 * @brief Checks if the key is valid.
	 * @return `true` if the key is a valid key, `false` otherwise.
	 */
	bool isValid() const { return cid >= 0; }

	/** @brief An invalid CID value. */
	static constexpr CidT cCidInvalid = std::numeric_limits<CidT>::min();
	/** @brief Minimal CID value. */
	static constexpr CidT cCidMin = 0;
	/** @brief Maximal CID value. */
	static constexpr CidT cCidMax = std::numeric_limits<CidT>::max();
	/** @brief Minimal MID value. */
	static constexpr MidT cMidMin = std::numeric_limits<MidT>::min();
	/** @brief Maximal MID value. */
	static constexpr MidT cMidMax = std::numeric_limits<MidT>::max();
	/** @brief Minimal MOID value. */
	static constexpr MoidT cMoidMin = std::numeric_limits<MoidT>::min();
	/** @brief Maximal MOID value. */
	static constexpr MoidT cMoidMax = std::numeric_limits<MoidT>::max();
	/** @brief Minimal CAP value. */
	static constexpr CapT cCapMin = std::numeric_limits<CapT>::min();
	/** @brief Maximal CAP value. */
	static constexpr CapT cCapMax = std::numeric_limits<CapT>::max();
	/** @brief Minimal ACQ value. */
	static constexpr AcqT cAcqMin = std::numeric_limits<AcqT>::min();
	/** @brief Maximal ACQ value. */
	static constexpr AcqT cAcqMax = std::numeric_limits<AcqT>::max();
};

/**
 * @brief A field-wise equality operator.
 *
 * @param k1 Left-hand side of the equality operator.
 * @param k2 Right-hand side of the equality operator.
 * @return `true` if `k1.x == k2.x` for all fields `x` of `Key`, `false`
 * otherwise.
 */
bool operator==(const Key& k1, const Key& k2);
/**
 * @brief A field-wise non-equality operator.
 *
 * @param k1 Left-hand side of the non-equality operator.
 * @param k2 Right-hand side of the non-equality operator.
 * @return `true` if `k1.x != k2.x` for any field `x` of `Key`, `false`
 * otherwise.
 */
bool operator!=(const Key& k1, const Key& k2);
/**
 * @brief A field-wise less-or-equal operator.
 *
 * @param k1 Left-hand side of the operator.
 * @param k2 Right-hand side of the operator.
 * @return `true` if `k1.x <= k2.x` for all fields `x` of `Key`, `false`
 * otherwise.
 */
bool operator<=(const Key& k1, const Key& k2);
/**
 * @brief A field-wise greater-or-equal operator.
 *
 * @param k1 Left-hand side of the operator.
 * @param k2 Right-hand side of the operator.
 * @return `true` if `k1.x >= k2.x` for all fields `x` of `Key`, `false`
 * otherwise.
 */
bool operator>=(const Key& k1, const Key& k2);
/**
 * @brief A lexicographical less-than operator.
 *
 * @param k1 Left-hand side of the operator.
 * @param k2 Right-hand side of the operator.
 * @return `true` if the last differing entries `x` of `(cid, mid, moid, cap,
 * acq)` tuples of the keys satisfy `k1.x < k2.x`, `false` otherwise.
 */
bool operator<(const Key& k1, const Key& k2);
/**
 * @brief A lexicographical greater-than operator.
 *
 * @param k1 Left-hand side of the operator.
 * @param k2 Right-hand side of the operator.
 * @return `true` if the last differing entries `x` of `(cid, mid, moid, cap,
 * acq)` tuples of the keys satisfy `k1.x > k2.x`, `false` otherwise.
 */
bool operator>(const Key& k1, const Key& k2);

/**
 * @brief Returns a key with fields equal to the fields of `key` incremented by
 * `x`. Used to form closed key-intervals from right-open ones.
 *
 * @param key A key to increment.
 * @param x A value to increment the `key` by.
 * @return A new key with each field incremented by `x`.
 */
Key operator+(const Key& key, int x);
/**
 * @brief Returns a key with fields equal to the fields of `key` decremented by
 * `x`. Used to form right-open key-intervals from closed ones.
 *
 * @param key A key to decrement.
 * @param x A value to decrement the `key` by.
 * @return A new key with each field decremented by `x`.
 */
Key operator-(const Key& key, int x);

/** @brief A minimal Key. */
static const Key cKeyMin =
	Key(Key::cCidMin, Key::cMidMin, Key::cMoidMin, Key::cCapMin, Key::cAcqMin);
/** @brief A maximal Key. */
static const Key cKeyMax =
	Key(Key::cCidMax, Key::cMidMax, Key::cMoidMax, Key::cCapMax, Key::cAcqMax);

/**
 * @brief A record type, containing a key and a payload of type T.
 *
 * @tparam T Payload type.
 */
template<typename T>
struct Record
{
	/**
	 * @brief Default constructor. zero-initializes the key and
	 * default-constructs the payload.
	 * */
	Record() : value{} {}

	/**
	 * @brief A constructor. Constructs the key by value, forwards the rest of
	 * the arguments to a constructor of the payload.
	 *
	 * @param pKey Key.
	 * @param args Arguments forwarded to the constructor of the payload.
	 * @tparam U... Types of the arguments passed to the payload's constructor.
	 */
	template<typename... U>
	Record(Key pKey, U&&... args) : key(pKey), value(std::forward<U>(args)...)
	{
	}

	/** @brief Key of the record. */
	Key key;
	/** @brief Payload of the record. */
	T value;
};

} /*namespace tstorage*/

#endif
