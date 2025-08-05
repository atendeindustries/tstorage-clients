/*
 * TStorage: Client library (C++)
 *
 * Timestamp.cpp
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

#include <tstorageclient++/Timestamp.h>

#include <chrono>
#include <ctime>

#include <tstorageclient++/DataTypes.h>
#include "Defines.h"

namespace tstorage {

namespace Timestamp {

TSTORAGE_EXPORT UnixTime<std::chrono::seconds> fromDateTime(const int year,
	const int month,
	const int day,
	const int hour,
	const int minute,
	const int second)
{
	std::tm time{};
	time.tm_year = year - 1900; /* NOLINT(cppcoreguidelines-avoid-magic-numbers) */
	time.tm_mon = month - 1; /* NOLINT(cppcoreguidelines-avoid-magic-numbers) */
	time.tm_mday = day;
	time.tm_hour = hour;
	time.tm_min = minute;
	time.tm_sec = second;
	const std::time_t t = std::mktime(&time);
	if (t == -1) {
		return {};
	}
	return std::chrono::time_point_cast<std::chrono::seconds>(
		UnixTime<>::clock::from_time_t(t));
}

TSTORAGE_EXPORT TimeT now()
{
	return fromUnix(std::chrono::system_clock::now());
}

} /*namespace Timestamp*/

} /*namespace tstorage*/
