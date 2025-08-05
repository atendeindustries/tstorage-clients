/*
 * TStorage: Client library (C++)
 *
 * Headers.h
 *   TStorage protocol header definitions.
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

#ifndef D_TSTORAGE_HEADERS_PH
#define D_TSTORAGE_HEADERS_PH

#include <cstdint>

#include <tstorageclient++/DataTypes.h>

/** @file
 * @brief Defines standard TStorage protocol headers. */

namespace tstorage {
namespace impl {

/** @brief Protocol-defined command types available for use. */
enum CommandType : std::int32_t {
	GET = 1,
	PUT = 5,
	PUTA = 6,
	GETACQ = 7,
};

/** @brief A standard TStorage request/response header. */
struct Header
{
	/** @brief A command ID or a status code. */
	std::int32_t id = 0;
	/** @brief The amount of extra bytes in a header. */
	std::uint64_t dataSize = 0;
};

/** @brief A TStorage request header for GET and GETACQ. */
struct HeaderKeyRange
{
	/** @brief A command ID or a status code. */
	std::int32_t id = 0;
	/** @brief The amount of extra bytes in a header. */
	std::uint64_t dataSize = 0;
	/** @brief The lower vertex of the target key-interval. */
	Key keyMin;
	/** @brief The upper vertex of the target key-interval. */
	Key keyMax;
};

/** @brief A TStorage response header for GET and GETACQ. */
struct HeaderAcq
{
	/** @brief A command ID or a status code id. */
	std::int32_t id = 0;
	/** @brief The amount of extra bytes in a header. */
	std::uint64_t dataSize = 0;
	/** @brief ACQ timestamp of the latest fully-commited database operation.
	 * (See `Channel::getAcq()`.) */
	Key::AcqT acq = 0;
};

} /*namespace impl*/
} /*namespace tstorage*/

#endif
