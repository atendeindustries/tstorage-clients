/*
 * TStorage: Client library (C++)
 *
 * Buffer.h
 *   A simple, static buffer with read/write position tracking.
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

#ifndef D_TSTORAGE_BUFFER_PH
#define D_TSTORAGE_BUFFER_PH

#include <cstddef>
#include <cstdint>
#include <memory>

/** @file
 * @brief Defines a static buffer with read/write position tracking. */

namespace tstorage {
namespace impl {

/**
 * @brief A static buffer that keeps track of its last read/write locations.
 *
 * A `Buffer` manages a dynamically allocated block of memory for read/write
 * access. It also stores two additional offsets, one for each R/W access mode,
 * to keep track of the last memory access location inside the buffer. All
 * memory access is mediated by these offsets, in that the only data that is
 * legally available to read lies between the read and the write offset, and
 * the only memory addresses that can be legally writen into lie between the
 * write offset and the end of the buffer (treated as right-open intervals in
 * the address space). All other memory accesses are deemed invalid. Offsets
 * are managed manually using `readAdvance()` and `writeAdvance()`.
 *
 * This class offers no implicit error checks (not even out-of-bounds checks)
 * due to their performace impact on the hot path. To manually determine
 * whether a read or write of operation of a certain size is possible, use
 * `bytesAvailableToRead()` and `bytesOfFreeSpace()` respectively.
 *
 * As a result of some actions or unfavorable circumstances, the buffer can end
 * up in an invalid state. In this state no memory is allocated by the `Buffer`
 * object, and any future memory access will yield undefined results. To check
 * whether a buffer is valid or not, use the `valid()` method.
 */
class Buffer
{
public:
	/** @brief A default constructor, initializing the buffer in an invalid state.
	 * Does not allocate memory.*/
	Buffer();
	/** @brief An allocating constructor. The resulting buffer has capacity
	 * `initialBufferSize`. If the allocation fails due to insufficient memory,
	 * the buffer object ends up in an invalid state. */
	Buffer(std::size_t initialBufferSize);
	/** @brief The default destructor. Frees memory allocated by the buffer. */
	~Buffer() = default;

	/** @brief A copy constructor that initializes the object as an exact copy of
	 * `buf`. On memory allocation failure the object becomes invalid. */
	Buffer(const Buffer& buf);
	/** @brief A copy assignment operator that copies the state of `buf` over to
	 * the caller. On memory allocation failure the object becomes invalid. */
	Buffer& operator=(const Buffer& buf);
	/** @brief A move constructor that takes the ownership of the `buf`'s memory
	 * block. Leaves `buf` in an invalid state. */
	Buffer(Buffer&& buf) noexcept;
	/** @brief A move assignment operator that takes the ownership of the `buf`'s
	 * memory block. Leaves `buf` in an invalid state. */
	Buffer& operator=(Buffer&& buf) noexcept;

	/** @brief Returns true if the buffer is in valid state and false otherwise. */
	bool valid() const { return mBuffer.operator bool(); }
	/** @brief Returns true if the buffer is in valid state and false otherwise. */
	operator bool() const { return valid(); }
	/** @brief Returns true if the buffer is in invalid state and false otherwise. */
	bool operator!() const { return !valid(); }

	/** @brief Returns the pointer to the current read location in the buffer. */
	const void* readData(std::size_t offset = 0) const { return mBuffer.get() + mReadOffset + offset; }
	/** @brief Returns the offset of the current read location from the start of
	 * the buffer. */
	std::size_t readOffset() const { return mReadOffset; }
	/** @brief Moves the read location forward a given amount of bytes. */
	void readAdvance(const std::size_t amount) { mReadOffset += amount; }

	/** @brief Returns the pointer to the next write location in the buffer. */
	void* writeData(std::size_t offset = 0) { return mBuffer.get() + mWriteOffset + offset; }
	/** @brief Returns the offset of the next write location from the start of
	 * the buffer. */
	std::size_t writeOffset() const { return mWriteOffset; }
	/** @brief Moves the write location forward a given `amount` of bytes. */
	void writeAdvance(const std::size_t amount) { mWriteOffset += amount; }

	/** @brief Returns the maximum capacity of the buffer, i.e. the overall size
	 * of the memory block managed by the buffer. */
	std::size_t capacity() const { return mBufferSize; }
	/** @brief Returns the total amount of bytes that is available to read from
	 * the current read location. */
	std::size_t bytesAvailableToRead() const { return mWriteOffset - mReadOffset; }
	/** @brief Returns the total amount of bytes that can be safely written to the
	 * next write location. */
	std::size_t bytesOfFreeSpace() const { return mBufferSize - mWriteOffset; }


	/**
	 * @brief Attempts to reserve a specified amount of bytes inside the buffer.
	 *
	 * If necessary to accomodate `targetSize` amount of bytes in the
	 * buffer, moves its unread content to the start of the buffer, then checks
	 * whether the requested amount of memory is available.
	 *
	 * @param targetSize Amount of bytes to reserve.
	 * @return `true` if the specified amount of bytes is available to write, and
	 * `false` otherwise.
	 */
	bool reserve(std::size_t targetSize);
	/** @brief Discards the contents of the buffer by zeroing the R/W offsets. */
	void reset() { mWriteOffset = mReadOffset = 0; }

private:
	/** @brief An owning pointer to the allocated memory block. */
	std::unique_ptr<uint8_t[]> mBuffer;
	/** @brief The size of the owned memory block. Is `0` for invalid buffers. */
	std::size_t mBufferSize;
	/** @brief The offset to the location of the next legal write. */
	std::size_t mWriteOffset;
	/** @brief The offset to the location of the current byte to read. */
	std::size_t mReadOffset;
};

} /*namespace impl*/
} /*namespace tstorage*/

#endif
