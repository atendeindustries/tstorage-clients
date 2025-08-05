/*
 * TStorage: Client library tests (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include "SerializerTests.h"

#include <cstddef>
#include <cstdint>

#include <cstring>
#include <tstorageclient++/DataTypes.h>

#include "Asserts.h"
#include "Buffer.h"
#include "Serializer.h"

namespace tstorage {
using namespace impl;

std::ostream& operator<<(std::ostream& out, const Key& key)
{
	return out << "Key("
			   << key.cid << ", "
			   << key.mid << ", "
			   << key.moid << ", "
			   << key.cap << ", "
			   << key.acq << ")";
}

int test_serializer_put()
{
	Buffer buffer(256);
	Serializer serializer(buffer);

	serializer.putInt32(2);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 4)
	ASSERT_MEM_EQ(buffer.readData(), "\x02\x00\x00\x00", 4)
	buffer.readAdvance(4);

	serializer.putInt32(-2);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 4)
	ASSERT_MEM_EQ(buffer.readData(), "\xfe\xff\xff\xff", 4)
	buffer.readAdvance(4);

	serializer.putUInt32(17);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 4)
	ASSERT_MEM_EQ(buffer.readData(), "\x11\x00\x00\x00", 4)
	buffer.readAdvance(4);

	serializer.putInt64(256);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 8)
	ASSERT_MEM_EQ(buffer.readData(), "\x00\x01\x00\x00\x00\x00\x00\x00", 8)
	buffer.readAdvance(8);

	serializer.putUInt64(17);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 8)
	ASSERT_MEM_EQ(buffer.readData(), "\x11\x00\x00\x00\x00\x00\x00\x00", 8)
	buffer.readAdvance(8);

	Key key(1, 2, 3, 4, 5);
	serializer.putKey(key);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 32)
	ASSERT_MEM_EQ(buffer.readData(),
		"\x01\x00\x00\x00"
		"\x02\x00\x00\x00\x00\x00\x00\x00"
		"\x03\x00\x00\x00"
		"\x04\x00\x00\x00\x00\x00\x00\x00"
		"\x05\x00\x00\x00\x00\x00\x00\x00",
		32)
	buffer.readAdvance(32);

	serializer.putAbbrevKey(key);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 28)
	ASSERT_MEM_EQ(buffer.readData(),
		"\x02\x00\x00\x00\x00\x00\x00\x00"
		"\x03\x00\x00\x00"
		"\x04\x00\x00\x00\x00\x00\x00\x00"
		"\x05\x00\x00\x00\x00\x00\x00\x00",
		28)
	buffer.readAdvance(28);

	serializer.putAbbrevKeyWithoutAcq(key);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 20)
	ASSERT_MEM_EQ(buffer.readData(),
		"\x02\x00\x00\x00\x00\x00\x00\x00"
		"\x03\x00\x00\x00"
		"\x04\x00\x00\x00\x00\x00\x00\x00",
		20)
	buffer.readAdvance(20);
	return 0;
}

int test_serializer_get()
{
	Buffer buffer(256);
	Serializer serializer(buffer);

	serializer.putKey(Key(150, 250, 300, 1000, 2000));

	std::int32_t x = serializer.getInt32();
	ASSERT_EQ(x, 150)

	std::int64_t y = serializer.getInt64();
	ASSERT_EQ(y, 250)

	x = serializer.getInt32();
	ASSERT_EQ(x, 300)

	serializer.putUInt32(1024);
	serializer.putUInt64(1'000'000'000'000);

	y = serializer.getInt64();
	ASSERT_EQ(y, 1000)

	y = serializer.getInt64();
	ASSERT_EQ(y, 2000)

	std::uint32_t z{};
	std::uint64_t w{};
	z = serializer.getUInt32();
	ASSERT_EQ(z, 1024)
	w = serializer.getUInt64();
	ASSERT_EQ(w, 1'000'000'000'000)

	serializer.putInt32(1);
	serializer.putInt64(2);
	serializer.putInt32(3);
	serializer.putInt64(4);
	serializer.putInt64(5);

	Key key = serializer.getKey();
	ASSERT_EQ(key, Key(1, 2, 3, 4, 5))

	key = Key(6, 7, 8, 9, 10);
	serializer.putKey(key);
	key = serializer.getKey();
	ASSERT_EQ(key, Key(6, 7, 8, 9, 10))
	return 0;
}

int test_serializer_buffer()
{
	Buffer buffer(256);
	Serializer serializer(buffer);

	for (std::size_t i = 0; i < 8; ++i) {
		serializer.putInt32(i);
	}

	const void* bufStart = buffer.readData();
	const void* dataBuf = serializer.getDataBuffer(8);
	ASSERT_EQ(dataBuf, bufStart)
	ASSERT_MEM_EQ(dataBuf, "\x00\0\0\0\x01\0\0\0", 8)

	dataBuf = serializer.getDataBuffer(8);
	ASSERT_MEM_EQ(dataBuf, "\x02\0\0\0\x03\0\0\0", 8)

	int32_t x = serializer.getInt32();
	ASSERT_EQ(x, 4)
	x = serializer.getInt32();
	ASSERT_EQ(x, 5)

	ASSERT_EQ(buffer.bytesAvailableToRead(), 8)
	char* str = static_cast<char*>(buffer.writeData());
	strlcpy(str, "0123456789ABCDEF", buffer.capacity());
	serializer.confirmPayloadBuffer(16);
	ASSERT_EQ(buffer.bytesAvailableToRead(), 24)

	dataBuf = serializer.getDataBuffer(24);
	ASSERT_MEM_EQ(dataBuf,
		"\x06\0\0\0\x07\0\0\0"
		"0123456789ABCDEF",
		8)
	return 0;
}

} /*namespace tstorage*/
