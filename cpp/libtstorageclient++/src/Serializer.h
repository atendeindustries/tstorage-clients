/*
 * TStorage: Client library (C++)
 *
 * Serializer.h
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

#ifndef D_TSTORAGE_SERIALIZER_PH
#define D_TSTORAGE_SERIALIZER_PH

#include <cstddef>
#include <cstdint>

#include <tstorageclient++/DataTypes.h>

#include "Buffer.h"
#include "Headers.h"

/** @file
 * @brief Defines a `Serializer` class supplying protocol-specific
 * serialization primitives. */

namespace tstorage {
namespace impl {

/**
 * @brief A wrapper class which performs TStorage data marshalling from/to its
 * underlying `Buffer` object.
 *
 * Handles both little-endian and big-endian architectures.
 *
 * Both `put` and `get` methods do not make any out-of-bounds checks in the
 * release builds. We do place some asserts, however, to facilitate diagnosing
 * violations of memory safety guarantees provided by the implementation.
 */
class Serializer final
{
public:
	/** @brief The size of `Key` in bytes after serialization. */
	static constexpr std::size_t cKeySize = 32;
	/** @brief The size of `Key` without its CID field after serialization. */
	static constexpr std::size_t cAbbrevKeySize = 28;
	/** @brief The size of `Key` without its CID and ACQ fields after serialization. */
	static constexpr std::size_t cAbbrevKeySizeWithoutAcq = 20;
	/** @brief The size of a standard message header after serialization. */
	static constexpr std::size_t cHeaderSize = 12;

	/** @brief A constructor. Stores a pointer to the `Buffer` object used. */
	explicit Serializer(Buffer& buffer) : mBuffer(&buffer) {}

	/** @brief Reads an `std::int32_t` from the buffer without advancing the read
	 * location pointer. */
	std::int32_t peekInt32();
	/** @brief Advances the read location pointer as if we read an `std::int32_t`.
	 */
	void confirmInt32() { mBuffer->readAdvance(sizeof(std::int32_t)); }

	/** @brief Reads and returns the next `std::int32_t` from the buffer. */
	std::int32_t getInt32();
	/** @brief Reads and returns the next `std::int64_t` from the buffer. */
	std::int64_t getInt64();
	/** @brief Reads and returns the next `std::uint32_t` from the buffer. */
	std::uint32_t getUInt32();
	/** @brief Reads and returns the next `std::uint64_t` from the buffer. */
	std::uint64_t getUInt64();
	/** @brief Reads and returns the next `Key` from the buffer. */
	Key getKey();
	/** @brief Reads and returns a standard header from the buffer. */
	Header getHeader();
	/** @brief Reads and returns an ACQ response from the buffer. */
	HeaderAcq getHeaderAcq();
	/** @brief Returns an address to the fragment of the buffer containing the
	 * payload with size `size`. Treats the buffer as read and advances the read
	 * location pointer accordingly. */
	const void* getDataBuffer(std::size_t size);

	/** @brief Serializes `value` into the next available write location. */
	void putInt32(std::int32_t value);
	/** @brief Serializes `value` into the next available write location. */
	void putInt64(std::int64_t value);
	/** @brief Serializes `value` into the next available write location. */
	void putUInt32(std::uint32_t value);
	/** @brief Serializes `value` into the next available write location. */
	void putUInt64(std::uint64_t value);
	/** @brief Serializes a whole `key` into the next available write location. */
	void putKey(const Key& key);
	/** @brief Serializes `key` without its CID field into the next available
	 * write location. */
	void putAbbrevKey(const Key& key);
	/** @brief Serializes `key` without its CID and ACQ fields into the next
	 * available write location. */
	void putAbbrevKeyWithoutAcq(const Key& key);
	/** @brief Serializes the standard `header` into the next available
	 * write location. */
	void putHeader(const Header& header);
	/** @brief Serializes the `header` with a pair of `Key`s into the next
	 * available write location. */
	void putHeaderKeyRange(const HeaderKeyRange& header);
	/** @brief Treats the subsequent payload buffer of size `size` as correctly
	 * serialized, advancing the next write location accordingly. */
	void confirmPayloadBuffer(std::size_t size);

	/** @brief Deserializes data from a read-only memory block at `addr` into a
	 * variable `oValue` of type `T`. */
	template<typename T>
	static void get(T& oValue, const void* addr);

	/** @brief Serializes `value` of type `T` into a writable memory block at
	 * `addr`. */
	template<typename T>
	static void put(T value, void* addr);

private:
	/**
	 * @brief Copies 'src' to 'dest' with opposite endianness.
	 *
	 * The amount of bytes to copy is made a template parameter to enable some
	 * compile-time optimizations of the inner loop. The template gets instantiated
	 * only for sizes of C++ base types (currently `[u]int(32|64)_t` only).
	 *
	 * @param src Source memory-block address, read-only.
	 * @param dest Destination memory-block address, writable.
	 */
	template<std::size_t Bytes>
	static void swapBytes(const void* src, void* dest);

	/** @brief Deserializes data from the underlying buffer's current read
	 * location into a variable `oValue` of type `T`, then advances the read
	 * location. */
	template<typename T>
	void get(T& oValue);

	/** @brief Deserializes data from the underlying buffer's current read
	 * location into a variable `oValue` of type `T` without advancing the read
	 * location. */
	template<typename T>
	void peek(T& oValue) { get(oValue, mBuffer->readData()); }

	/** @brief Serializes `value` of type `T` into the next available write
	 * location of the underlying buffer, advancing the next write location
	 * accordingly. */
	template<typename T>
	void put(T value);

	/** @brief The underlying `Buffer` object. */
	Buffer* mBuffer;
};

} /*namespace impl*/
} /*namespace tstorage*/

#endif
