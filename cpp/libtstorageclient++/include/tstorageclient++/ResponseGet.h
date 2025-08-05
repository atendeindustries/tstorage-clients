/*
 * TStorage: Client library (C++)
 *
 * ResponseGet
 *   A response class containing the acq and a set of records returned by GET
 *   requests.
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

#ifndef D_TSTORAGE_RESPONSEGET_H
#define D_TSTORAGE_RESPONSEGET_H

#include "DataTypes.h"
#include "RecordsSet.h"
#include "ResponseAcq.h"

/** @file
 * @brief Defines a TStorage GET operation response type. */

namespace tstorage {

/**
 * @brief A TStorage GET query response containing its status code, an ACQ
 * timestamp and a `RecordsSet` with data.
 *
 * The only TStorage database operation which sends back actual data is
 * the GET query. It is issued by `Channel::get()` and, indirectly, by
 * `Channel::getStream()`. Only the first of these methods returns an actual
 * `ResponseGet<T>` object. More details can be found in the documentation of
 * the respective methods.
 *
 * Note:
 *  - The ACQ timestamp is valid if and only if a call to `error()` returns
 *  `false`. In this case neither validity or completeness of the query
 *  response, i.e. the `RecordsSet` returned by `records()`, cannot be
 *  guaranteed in general (for precise guarantees, see `Channel::get()`).
 *  - Objects of this class are mutable. The `records()` method accesses the
 *  underlying `RecordsSet` direcly.
 *
 *  @see `Channel::get()`
 *  @see `Channel::getStream()`
 *
 *  @tparam T Payload type of received records.
 */
template<typename T>
class ResponseGet : public ResponseAcq
{
public:
	/**
	 * @brief A constructor used for error responses, initializing the ACQ
	 * timestamp with some default value and keeping the records container empty.
	 *
	 * @param errorCode The error code of the query.
	 */
	ResponseGet(result_t errorCode)
		: ResponseAcq(errorCode) {}

	/**
	 * @brief A constructor used to store the timestamp returned in response to a
	 * successful query. The records container is initialized as a copy of the
	 * argument.
	 *
	 * @param statusCode The statusCode of the response.
	 * @param records The initial records set.
	 * @param acq The returned ACQ timestamp.
	 */
	ResponseGet(result_t statusCode, const RecordsSet<T>& records, Key::AcqT acq)
		: ResponseAcq(statusCode, acq), mResponse(records) {}

	/**
	 * @brief A constructor used to store the timestamp returned in response to a
	 * successful query. The records container is move-initialized from the
	 * argument.
	 *
	 * @param statusCode The statusCode of the response.
	 * @param records The initial records set.
	 * @param acq The returned ACQ timestamp.
	 */
	ResponseGet(result_t statusCode, RecordsSet<T>&& records, Key::AcqT acq)
		: ResponseAcq(statusCode, acq), mResponse(std::move(records)) {}

	/**
	 * @brief Accesses the underlying records container.
	 * @return A reference to the underlying `RecordsSet<T>`.
	 */
	RecordsSet<T>& records() { return mResponse; }
	/**
	 * @brief Accesses the underlying records container (read-only).
	 * @return A const reference to the underlying `RecordsSet<T>`.
	 */
	const RecordsSet<T>& records() const { return mResponse; }

private:
	/** @brief The set of records received in response to the GET query. */
	RecordsSet<T> mResponse;
};

} /*namespace tstorage*/

#endif
