/*
 * TStorage: Client library (C++)
 *
 * Timestamp.h
 *   Some utility functions around TStorage timestamps.
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

#ifndef D_TSTORAGE_TIMESTAMP_H
#define D_TSTORAGE_TIMESTAMP_H

#include <chrono>
#include <cstdint>

/** @file
 * @brief Provides TStorage internal timestamp utilities. */

namespace tstorage {

/** @brief Number of nanoseconds that elapsed from epoch to January 1st, 2001.
 * */
static constexpr std::int64_t cNanosFrom1970To2001 = 978307200000000000;

/**
 * @brief A namespace with conversion functions between UNIX and TStorage
 * internal timestamps.
 *
 * A TStorage internal timestamp is a 64-bit integer containing the number of
 * nanoseconds which elapsed since January 1, 2001, 12:00 AM UTC. This
 * namespace offers several utility functions for time conversion
 * between the standard C++ format and the TStorage internal one.
 */
namespace Timestamp {

/** @brief TStorage timestamp datatype */
using TimeT = std::int64_t;

/**
 * @brief Alias for std::chrono::time_point<std::chrono::system_clock, UnitT>>
 * returned from std::chrono::system_clock::now.
 */
template<typename UnitT = std::chrono::seconds>
using UnixTime = std::chrono::time_point<std::chrono::system_clock, UnitT>;

/**
 * @brief Converts TStorage internal timestamp to standard UNIX timestamp.
 *
 * @tparam UnitT Time unit (see `std::chrono`).
 * @param tstorageTimestamp The TStorage internal timestamp to convert.
 * @return UNIX timestamp corresponding to the argument.
 */
template<typename UnitT = std::chrono::seconds>
UnixTime<UnitT> toUnix(TimeT tstorageTimestamp)
{
	using namespace std::chrono;
	using namespace std::chrono_literals;
	return time_point_cast<UnitT>(
		UnixTime<nanoseconds>{(tstorageTimestamp + cNanosFrom1970To2001) * 1ns});
}

/**
 * @brief Converts a standard UNIX timestamp to a TStorage internal timestamp.
 *
 * @tparam UnitT Time unit (from std::chrono).
 * @param unixTimestamp UNIX timestamp to convert.
 * @return TStorage internal timestamp corresponding to the argument.
 */
template<typename UnitT = std::chrono::seconds>
TimeT fromUnix(UnixTime<UnitT> unixTimestamp)
{
	using namespace std::chrono;
	return time_point_cast<nanoseconds>(unixTimestamp).time_since_epoch().count()
		   - cNanosFrom1970To2001;
}

/**
 * @brief Returns the UNIX timestamp corresponding to a given date.
 *
 * To specify the date/time with a larger resolution, add an additional
 * `std::chrono::duration` to the result, using e.g. `100ns` from
 * `std::chrono_literals`.
 *
 * @param year Year.
 * @param month Month, with `1` corresponding to January.
 * @param day Day, counting from `1`.
 * @param hour Hours, in a 24-hour format.
 * @param minute Minutes.
 * @param second Seconds.
 * @return UNIX timestamp corresponding to the current time.
 */
/* NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers) */
UnixTime<std::chrono::seconds> fromDateTime(int year = 2001,
	int month = 1,
	int day = 1,
	int hour = 0,
	int minute = 0,
	int second = 0);
/* NOLINTEND(cppcoreguidelines-avoid-magic-numbers) */

/**
 * @brief Returns the current time according to the system clock as a TStorage
 * internal timestamp.
 *
 * @return TStorage internal timestamp corresponding to the current time.
 */
TimeT now();

}; /*namespace Timestamp*/

} /*namespace tstorage*/

#endif
