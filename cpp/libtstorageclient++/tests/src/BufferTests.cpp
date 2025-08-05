/*
 * TStorage: Client library tests (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include "BufferTests.h"

#include <cstdlib>
#include <cstring>
#include <string>

#include "Asserts.h"
#include "Buffer.h"

using namespace std;

namespace tstorage {
using namespace impl;

int test_buffer_create()
{
	Buffer buffer(128);
	ASSERT_EQ(buffer.capacity(), 128);

	buffer = Buffer(32);
	ASSERT_EQ(buffer.capacity(), 32);
	return 0;
}

int test_buffer_heads()
{
	Buffer buffer(256);

	ASSERT_EQ(buffer.readData(), buffer.writeData())
	ASSERT_EQ(buffer.bytesAvailableToRead(), 0)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 256)
	char* cStr = static_cast<char*>(buffer.writeData());
	strlcpy(cStr, "0123456789ABCDEF", buffer.bytesOfFreeSpace());
	buffer.writeAdvance(8);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 8)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 248)

	std::string word = static_cast<const char*>(buffer.readData());
	ASSERT_EQ(word, "0123456789ABCDEF")

	cStr = static_cast<char*>(buffer.writeData());
	strlcpy(cStr, "0123456789ABCDEF", buffer.bytesOfFreeSpace());
	buffer.writeAdvance(16);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 24)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 232)

	word = static_cast<const char*>(buffer.readData());
	ASSERT_EQ(word, "012345670123456789ABCDEF")

	buffer.readAdvance(8);
	word = static_cast<const char*>(buffer.readData());
	ASSERT_EQ(word, "0123456789ABCDEF")
	ASSERT_EQ(buffer.bytesAvailableToRead(), 16)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 232)

	buffer.readAdvance(8);
	word = static_cast<const char*>(buffer.readData());
	ASSERT_EQ(word, "89ABCDEF")
	ASSERT_EQ(buffer.bytesAvailableToRead(), 8)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 232)

	cStr = static_cast<char*>(buffer.writeData());
	strlcpy(cStr, "OK", buffer.bytesOfFreeSpace());
	buffer.writeAdvance(2);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 10)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 230)

	word = static_cast<const char*>(buffer.readData());
	ASSERT_EQ(word, "89ABCDEFOK")
	buffer.readAdvance(10);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 0)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 230)

	word = static_cast<const char*>(buffer.readData());
	ASSERT_EQ(word.empty(), true)

	buffer.reset();
	ASSERT_EQ(buffer.readData(), buffer.writeData())
	ASSERT_EQ(buffer.bytesAvailableToRead(), 0)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 256)
	return 0;
}

int test_buffer_reserve()
{
	Buffer buffer(256);
	char* cStr = static_cast<char*>(buffer.writeData());
	strlcpy(cStr, "0123456789ABCDEF", buffer.bytesOfFreeSpace());
	buffer.writeAdvance(16);

	ASSERT_EQ(buffer.reserve(16), true);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 16)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 240)
	ASSERT_EQ(buffer.readOffset(), 0)

	cStr = static_cast<char*>(buffer.writeData());
	strlcpy(cStr, "0123456789ABCDEF", buffer.bytesOfFreeSpace());
	buffer.writeAdvance(16);

	const char* ccStr = static_cast<const char*>(buffer.readData());
	std::string word = std::string(ccStr, buffer.bytesAvailableToRead());
	ASSERT_EQ(word, "0123456789ABCDEF0123456789ABCDEF")

	buffer.readAdvance(24);
	ccStr = static_cast<const char*>(buffer.readData());
	word = std::string(ccStr, buffer.bytesAvailableToRead());
	ASSERT_EQ(word, "89ABCDEF")
	ASSERT_EQ(buffer.bytesAvailableToRead(), 8)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 224)
	ASSERT_NEQ(buffer.readOffset(), 0)

	ASSERT_EQ(buffer.reserve(160), true);
	ccStr = static_cast<const char*>(buffer.readData());
	word = std::string(ccStr, buffer.bytesAvailableToRead());
	ASSERT_EQ(word, "89ABCDEF")
	ASSERT_EQ(buffer.bytesAvailableToRead(), 8)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 224)
	ASSERT_NEQ(buffer.readOffset(), 0)

	ASSERT_EQ(buffer.reserve(240), true);
	ccStr = static_cast<const char*>(buffer.readData());
	word = std::string(ccStr, buffer.bytesAvailableToRead());
	ASSERT_EQ(word, "89ABCDEF")
	ASSERT_EQ(buffer.bytesAvailableToRead(), 8)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 248)
	ASSERT_EQ(buffer.readOffset(), 0)

	cStr = static_cast<char*>(buffer.writeData());
	strlcpy(cStr, "0123456789ABCDEF", buffer.bytesOfFreeSpace());
	buffer.writeAdvance(16);

	ccStr = static_cast<const char*>(buffer.readData());
	word = std::string(ccStr, buffer.bytesAvailableToRead());
	ASSERT_EQ(word, "89ABCDEF0123456789ABCDEF")
	ASSERT_EQ(buffer.bytesAvailableToRead(), 24)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 232)
	ASSERT_EQ(buffer.readOffset(), 0)

	ASSERT_EQ(buffer.reserve(233), false);
	ccStr = static_cast<const char*>(buffer.readData());
	word = std::string(ccStr, buffer.bytesAvailableToRead());
	ASSERT_EQ(word, "89ABCDEF0123456789ABCDEF")
	ASSERT_EQ(buffer.bytesAvailableToRead(), 24)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 232)
	ASSERT_EQ(buffer.readOffset(), 0)

	buffer.readAdvance(1);
	ASSERT_EQ(buffer.reserve(233), true);
	ccStr = static_cast<const char*>(buffer.readData());
	word = std::string(ccStr, buffer.bytesAvailableToRead());
	ASSERT_EQ(word, "9ABCDEF0123456789ABCDEF")
	ASSERT_EQ(buffer.bytesAvailableToRead(), 23)
	ASSERT_EQ(buffer.bytesOfFreeSpace(), 233)
	ASSERT_EQ(buffer.readOffset(), 0)
	return 0;
}

} /*namespace tstorage*/
