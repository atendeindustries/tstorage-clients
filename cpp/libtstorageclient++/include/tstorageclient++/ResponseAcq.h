/*
 * TStorage: Client library (C++)
 *
 * ResponseAcq.h
 *   A response class containing the an timestamp returned by the server on
 *   GET and GETACQ commands.
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

#ifndef D_TSTORAGE_RESPONSEACQ_H
#define D_TSTORAGE_RESPONSEACQ_H

#include "DataTypes.h"
#include "Response.h"

/** @file
 * @brief Defines a TStorage GETACQ operation response type. */

namespace tstorage {

/**
 * @brief A TStorage query response containing a status code and an ACQ
 * timestamp.
 *
 * An ACQ timestamp is send back to the client in response to successful GET
 * and GET_ACQ server queries handled by `Channel::get()`,
 * `Channel:getStream()` and `Channel::getAcq()` methods. For the meaning of
 * the returned timestamp, consult the documentation of the respective methods.
 *
 * Note:
 *  - The stored ACQ timestamp is valid if and only if a call to `error()`
 *  returns `false`.
 *  - Objects of this class are immutable.
 *
 * @see `Channel::get()`
 * @see `Channel::getStream()`
 * @see `Channel::getAcq()`
 */
class ResponseAcq : public Response
{
public:
	/**
	 * @brief A constructor used for error responses, initializing the ACQ
	 * timestamp with some default value.
	 *
	 * @param errorCode The error code of the query.
	 */
	ResponseAcq(result_t errorCode)
		: Response(errorCode), mAcq(Key::cAcqMax) {}

	/**
	 * @brief A constructor used to store the timestamp returned in response to a
	 * successful query.
	 *
	 * @param statusCode The statusCode of the response.
	 * @param acq The returned ACQ timestamp.
	 */
	ResponseAcq(result_t statusCode, Key::AcqT acq)
		: Response(statusCode), mAcq(acq) {}

	/**
	 * @brief Reads the stored ACQ timestamp.
	 * @return A copy of the stored timestamp.
	 */
	Key::AcqT acq() const { return mAcq; }

private:
	/** @brief The returned ACQ timestamp. */
	Key::AcqT mAcq;
};

} /*namespace tstorage*/

#endif
