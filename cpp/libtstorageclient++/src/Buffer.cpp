/*
 * TStorage: Client library (C++)
 *
 * Buffer.cpp
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

#include "Buffer.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <new>
#include <utility>

namespace tstorage {
namespace impl {

Buffer::Buffer() : mBuffer(nullptr), mBufferSize(0), mWriteOffset(0), mReadOffset(0)
{
}

Buffer::Buffer(const std::size_t initialBufferSize)
	: mBuffer(new(std::nothrow) uint8_t[initialBufferSize])
	, mBufferSize(initialBufferSize)
	, mWriteOffset(0)
	, mReadOffset(0)
{
}

Buffer::Buffer(const Buffer& buf)
	: mBuffer(buf ? new(std::nothrow) uint8_t[buf.mBufferSize] : nullptr)
	, mBufferSize(0)
	, mWriteOffset(0)
	, mReadOffset(0)
{
	if (mBuffer) {
		memcpy(mBuffer.get(), buf.mBuffer.get(), buf.capacity());
		mBufferSize = buf.capacity();
		mWriteOffset = buf.writeOffset();
		mReadOffset = buf.readOffset();
	}
}

Buffer::Buffer(Buffer&& buf) noexcept
	: mBuffer(std::move(buf.mBuffer))
	, mBufferSize(std::move(buf.mBufferSize))
	, mWriteOffset(std::move(buf.mWriteOffset))
	, mReadOffset(std::move(buf.mReadOffset))
{
	buf.mBufferSize = 0;
	buf.mWriteOffset = 0;
	buf.mReadOffset = 0;
}

Buffer& Buffer::operator=(const Buffer& buf)
{
	if (this != &buf) {
		if (mBufferSize != buf.mBufferSize) {
			mBuffer.reset(buf ? new (std::nothrow) uint8_t[buf.mBufferSize] : nullptr);
			if (mBuffer == nullptr) {
				mBufferSize = 0;
				mWriteOffset = 0;
				mReadOffset = 0;
			} else {
				mBufferSize = buf.mBufferSize;
				mWriteOffset = buf.mWriteOffset;
				mReadOffset = buf.mReadOffset;
			}
		}
		memcpy(mBuffer.get(), buf.mBuffer.get(), capacity());
	}
	return *this;
}

Buffer& Buffer::operator=(Buffer&& buf) noexcept
{
	if (this != &buf) {
		mBuffer = std::move(buf.mBuffer);
		mBufferSize = buf.mBufferSize;
		mWriteOffset = buf.mWriteOffset;
		mReadOffset = buf.mReadOffset;
		buf.mBufferSize = 0;
		buf.mWriteOffset = 0;
		buf.mReadOffset = 0;
	}
	return *this;
}

bool Buffer::reserve(std::size_t targetSize)
{
	const std::size_t bytesAvailable = bytesAvailableToRead();
	const std::size_t bytesFree = bytesOfFreeSpace();

	if (bytesAvailable + targetSize > mBufferSize) {
		return false;
	}
	if (targetSize > bytesFree) {
		memmove(mBuffer.get(), readData(), bytesAvailable);
		mWriteOffset = bytesAvailable;
		mReadOffset = 0;
	}
	return true;
}

} /*namespace impl*/
} /*namespace tstorage*/
