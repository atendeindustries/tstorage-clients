/*
 * TStorage: Client library (C++)
 *
 * DataTypes.cpp
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

#include <tstorageclient++/DataTypes.h>

#include "Defines.h"

namespace tstorage {

TSTORAGE_EXPORT bool operator==(const Key& k1, const Key& k2)
{
	return k1.cid == k2.cid
		   && k1.mid == k2.mid
		   && k1.moid == k2.moid
		   && k1.cap == k2.cap
		   && k1.acq == k2.acq;
}

TSTORAGE_EXPORT bool operator!=(const Key& k1, const Key& k2)
{
	return k1.cid != k2.cid
		   || k1.mid != k2.mid
		   || k1.moid != k2.moid
		   || k1.cap != k2.cap
		   || k1.acq != k2.acq;
}

TSTORAGE_EXPORT bool operator<=(const Key& k1, const Key& k2)
{
	return k1.cid <= k2.cid
		   && k1.mid <= k2.mid
		   && k1.moid <= k2.moid
		   && k1.cap <= k2.cap
		   && k1.acq <= k2.acq;
}

TSTORAGE_EXPORT bool operator>=(const Key& k1, const Key& k2)
{
	return k1.cid >= k2.cid
		   && k1.mid >= k2.mid
		   && k1.moid >= k2.moid
		   && k1.cap >= k2.cap
		   && k1.acq >= k2.acq;
}

// clang-format off
TSTORAGE_EXPORT bool operator<(const Key& k1, const Key& k2)
{
	return k1.cid < k2.cid
		|| (k1.cid == k2.cid && (k1.mid < k2.mid
		|| (k1.mid == k2.mid && (k1.moid < k2.moid
		|| (k1.moid == k2.moid && (k1.cap < k2.cap
		|| (k1.cap == k2.cap && k1.acq < k2.acq)))))));
}

TSTORAGE_EXPORT bool operator>(const Key& k1, const Key& k2)
{
	return k1.cid > k2.cid
		|| (k1.cid == k2.cid && (k1.mid > k2.mid
		|| (k1.mid == k2.mid && (k1.moid > k2.moid
		|| (k1.moid == k2.moid && (k1.cap > k2.cap
		|| (k1.cap == k2.cap && k1.acq > k2.acq)))))));
}
// clang-format on

TSTORAGE_EXPORT Key operator+(const Key& key, const int x)
{
	return {key.cid + x, key.mid + x, key.moid + x, key.cap + x, key.acq + x};
}

TSTORAGE_EXPORT Key operator-(const Key& key, const int x)
{
	return {key.cid - x, key.mid - x, key.moid - x, key.cap - x, key.acq - x};
}

} /*namespace tstorage*/
