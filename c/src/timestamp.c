/*
 * Copyright 2025 Atende Industries sp. z o.o.
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
 *
 */

/** @file timestamp.c
 *
 * Functions for conversion from Unix timestamp time_t to TStorage timestamp
 * type.
 *
 * TStorage timestamp is number of nanoseconds since the TStorage epoch, i.e.
 * 1 Jan 2001 00:00:00. TStorage timestamps do not take leap seconds into
 * account.
 */

#include <stdint.h>
#include <time.h>

#include "tstorage-client/client.h"

/* We assume POSIX.1-2001 conformance: time_t is number of seconds (not
 * counting leap seconds) since Unix epoch.
 */

/* Difference between the TStorage epoch (1 Jan 2001 00:00:00) and Unix epoch
 * (1 Jan 1970 00:00:00), in seconds. The value ignores leap seconds.
 */
#define EPOCH_DIFF ((time_t)978307200)

/* 10^9 */
#define TEN_9 1000000000

time_t TSCLIENT_timestampToUnix(int64_t ts)
{
	return (time_t)(ts / TEN_9) + EPOCH_DIFF;
}

int64_t TSCLIENT_timestampFromUnix(time_t ts)
{
	return ((int64_t)ts - EPOCH_DIFF) * TEN_9;
}

int64_t TSCLIENT_timestampNow(void)
{
	return TSCLIENT_timestampFromUnix(time(NULL));
}
