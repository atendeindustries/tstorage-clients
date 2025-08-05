/*
 * TStorage: Client library (C++)
 *
 * Serializer.cpp
 *   A thin wrapper over `Buffer` implementing protocol-specific data
 *   serialization methods.
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

#include "Serializer.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <tstorageclient++/DataTypes.h>

#include "Buffer.h"
#include "Headers.h"

namespace tstorage {
namespace impl {
namespace {

/*
 * @brief Checks if the architecture is big-endian.
 *
 * Comes at no cost due to compiler optimizations (GCC/Linux).
 */
bool is_be()
{
	constexpr std::uint32_t dword = 0x01020304;
	return dword == htonl(dword);
}

} /*namespace*/

template<std::size_t Bytes>
void Serializer::swapBytes(const void* const src, void* const dest)
{
	const uint8_t* srcBytes = static_cast<const uint8_t*>(src);
	uint8_t* destBytes = static_cast<uint8_t*>(dest);
	for (std::size_t i = 0; i < Bytes; ++i) {
		destBytes[i] = srcBytes[Bytes - i - 1];
	}
}

template<typename T>
void Serializer::get(T& oValue, const void* addr)
{
	constexpr std::size_t size = sizeof(T);
	if (is_be()) {
		swapBytes<size>(addr, &oValue);
	} else {
		memcpy(&oValue, addr, size);
	}
}

template<typename T>
void Serializer::get(T& oValue)
{
	assert(mBuffer->bytesAvailableToRead() >= sizeof(T));
	get(oValue, mBuffer->readData());
	mBuffer->readAdvance(sizeof(T));
}

std::int32_t Serializer::peekInt32()
{
	std::int32_t val{};
	peek<std::int32_t>(val);
	return val;
}

std::int32_t Serializer::getInt32()
{
	std::int32_t val{};
	get<std::int32_t>(val);
	return val;
}

std::int64_t Serializer::getInt64()
{
	std::int64_t val{};
	get<std::int64_t>(val);
	return val;
}

std::uint32_t Serializer::getUInt32()
{
	std::uint32_t val{};
	get<std::uint32_t>(val);
	return val;
}

std::uint64_t Serializer::getUInt64()
{
	std::uint64_t val{};
	get<std::uint64_t>(val);
	return val;
}

Key Serializer::getKey()
{
	Key key;
	get<Key::CidT>(key.cid);
	get<Key::MidT>(key.mid);
	get<Key::MoidT>(key.moid);
	get<Key::CapT>(key.cap);
	get<Key::AcqT>(key.acq);
	return key;
}

Header Serializer::getHeader()
{
	Header header{};
	get<std::int32_t>(header.id);
	get<std::uint64_t>(header.dataSize);
	return header;
}

HeaderAcq Serializer::getHeaderAcq()
{
	HeaderAcq header{};
	get<std::int32_t>(header.id);
	get<std::uint64_t>(header.dataSize);
	get<Key::AcqT>(header.acq);
	return header;
}

const void* Serializer::getDataBuffer(const std::size_t size)
{
	const void* dataBuffer = mBuffer->readData();
	if (size > mBuffer->bytesAvailableToRead()) {
		return nullptr;
	}
	mBuffer->readAdvance(size);
	return dataBuffer;
}

template<typename T>
void Serializer::put(const T value, void* addr)
{
	constexpr std::size_t size = sizeof(T);
	if (is_be()) { /* big-endian */
		swapBytes<size>(&value, addr);
	} else { /* little-endian */
		memcpy(addr, &value, size);
	} /* a conditional gets optimized out */
}

template<typename T>
void Serializer::put(const T value)
{
	assert(mBuffer->bytesOfFreeSpace() >= sizeof(T));
	put<T>(value, mBuffer->writeData());
	mBuffer->writeAdvance(sizeof(T));
}

void Serializer::putInt32(const std::int32_t value)
{
	put<std::int32_t>(value);
}

void Serializer::putInt64(const std::int64_t value)
{
	put<std::int64_t>(value);
}

void Serializer::putUInt32(const std::uint32_t value)
{
	put<std::uint32_t>(value);
}

void Serializer::putUInt64(const std::uint64_t value)
{
	put<std::uint64_t>(value);
}

void Serializer::putKey(const Key& key)
{
	put<Key::CidT>(key.cid);
	put<Key::MidT>(key.mid);
	put<Key::MoidT>(key.moid);
	put<Key::CapT>(key.cap);
	put<Key::AcqT>(key.acq);
}

void Serializer::putAbbrevKey(const Key& key)
{
	put<Key::MidT>(key.mid);
	put<Key::MoidT>(key.moid);
	put<Key::CapT>(key.cap);
	put<Key::AcqT>(key.acq);
}

void Serializer::putAbbrevKeyWithoutAcq(const Key& key)
{
	put<Key::MidT>(key.mid);
	put<Key::MoidT>(key.moid);
	put<Key::CapT>(key.cap);
}

void Serializer::putHeader(const Header& header)
{
	put<std::int32_t>(header.id);
	put<std::uint64_t>(header.dataSize);
}

void Serializer::putHeaderKeyRange(const HeaderKeyRange& header)
{
	put<std::int32_t>(header.id);
	put<std::uint64_t>(header.dataSize);
	put<Key::CidT>(header.keyMin.cid);
	put<Key::MidT>(header.keyMin.mid);
	put<Key::MoidT>(header.keyMin.moid);
	put<Key::CapT>(header.keyMin.cap);
	put<Key::AcqT>(header.keyMin.acq);
	put<Key::CidT>(header.keyMax.cid);
	put<Key::MidT>(header.keyMax.mid);
	put<Key::MoidT>(header.keyMax.moid);
	put<Key::CapT>(header.keyMax.cap);
	put<Key::AcqT>(header.keyMax.acq);
}

void Serializer::confirmPayloadBuffer(const std::size_t size)
{
	mBuffer->writeAdvance(size);
}

template void Serializer::get<std::int32_t>(std::int32_t&, const void*);
template void Serializer::get<std::int64_t>(std::int64_t&, const void*);
template void Serializer::get<std::uint32_t>(std::uint32_t&, const void*);
template void Serializer::get<std::uint64_t>(std::uint64_t&, const void*);

template void Serializer::put<std::int32_t>(std::int32_t, void*);
template void Serializer::put<std::int64_t>(std::int64_t, void*);
template void Serializer::put<std::uint32_t>(std::uint32_t, void*);
template void Serializer::put<std::uint64_t>(std::uint64_t, void*);

} /*namespace impl*/
} /*namespace tstorage*/
