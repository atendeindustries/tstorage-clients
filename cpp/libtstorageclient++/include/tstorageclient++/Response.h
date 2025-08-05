/*
 * TStorage: Client library (C++)
 *
 * Response.h
 *   A generic response class containing the status code of a given operation.
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

#ifndef D_TSTORAGE_RESPONSE_H
#define D_TSTORAGE_RESPONSE_H

#include <cstdint>
#include "DataTypes.h"

/** @file
 * @brief Defines a generic TStorage operation response type. */

namespace tstorage {

/**
 * @brief A TStorage query response containing the status code of the invoked
 * operation.
 *
 * This minimal response is returned by PUT and PUTA requests sent to the
 * server via `Channel::put()` and `Channel::puta()`.
 *
 * Note:
 *  - Objects of this class are immutable.
 */
class Response
{
public:
	/**
	 * @brief A constructor assigning a status code.
	 * @param status The status code returned by the server.
	 */
	Response(result_t status) : mStatus(status) {}

	/**
	 * @brief Checks if the invoked operation completed successfully.
	 * @return `true` on success, `false` otherwise.
	 */
	operator bool() const { return success(); }
	/**
	 * @brief Checks if the invoked operation failed.
	 * @return `true` on failure, `false` if there were no errors.
	 */
	bool operator!() const { return error(); }

	/**
	 * @brief Returns the status code of the response.
	 * @return The status code.
	 */
	result_t status() const { return mStatus; }
	/**
	 * @brief Checks if the invoked operation completed successfully.
	 * @return `true` on success, `false` otherwise.
	 */
	bool success() const { return mStatus == result_t::OK; }
	/**
	 * @brief Checks if the invoked operation failed.
	 * @return `true` on failure, `false` if there were no errors.
	 */
	bool error() const { return mStatus != result_t::OK; }
	/**
	 * @brief Checks if the invoked operation encountered a serious error.
	 * @return `true` on fatal failure, `false` on success or warnings.
	 */
	bool fatal() const { return error() && static_cast<std::int32_t>(mStatus) < 0; }
	/**
	 * @brief Checks if the invoked operation encountered a non-fatal error or a
	 * warning.
	 * @return `true` on recoverable error, `false` on success or fatal errors.
	 */
	bool issue() const { return error() && static_cast<std::int32_t>(mStatus) > 0; }
	/**
	 * @brief Checks if the server returned an error response.
	 * @return `true` on server's error response, `false` on success or
	 * client-side error.
	 */
	bool serverSideError() const
	{
		return error() && static_cast<std::int32_t>(mStatus) > cClientFatals
			   && static_cast<std::int32_t>(mStatus) < cClientIssues;
	}
	/**
	 * @brief Checks if the operation failed on the client side.
	 * @return `true` on client error, `false` on success or server-reported
	 * error.
	 */
	bool clientSideError() const { return error() && !serverSideError(); }

private:
	/** @brief The status code. */
	result_t mStatus;
};

} /*namespace tstorage*/

#endif
