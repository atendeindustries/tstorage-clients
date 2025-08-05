/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include <cmath>
#include <cstdint>

#include "../Utils.h"

#include <catch2/catch_test_macros.hpp>

using namespace tstorage::exampleCSV;

TEST_CASE("Utils: parse status", "[field]")
{
	std::int32_t x{};
	std::int64_t y{};
	Bytes64 z{};
	SECTION("int32_t success")
	{
		REQUIRE(parseInt32("0", x));
		REQUIRE(parseInt32("0000", x));
		REQUIRE(parseInt32("123", x));
		REQUIRE(parseInt32("54312", x));
		REQUIRE(parseInt32("-54312", x));
		REQUIRE(parseInt32("0xAD", x));
		REQUIRE(parseInt32("-0xAD", x));
		REQUIRE(parseInt32("0765", x));
		REQUIRE(parseInt32("-0765", x));
		REQUIRE(parseInt32("+0765", x));
		REQUIRE(parseInt32("  123", x));
		REQUIRE(parseInt32("  123   ", x));
	}
	SECTION("int64_t success")
	{
		REQUIRE(parseInt64("0", y));
		REQUIRE(parseInt64("0000", y));
		REQUIRE(parseInt64("123", y));
		REQUIRE(parseInt64("54312", y));
		REQUIRE(parseInt64("-54312", y));
		REQUIRE(parseInt64("0xAD", y));
		REQUIRE(parseInt64("-0xAD", y));
		REQUIRE(parseInt64("0765", y));
		REQUIRE(parseInt64("-0765", y));
		REQUIRE(parseInt64("+0765", y));
		REQUIRE(parseInt64("0x1000000000000000", y));
		REQUIRE(parseInt64(" \n123", y));
		REQUIRE(parseInt64("  123\t\n  ", y));
	}
	SECTION("bytes success")
	{
		REQUIRE(parseBytes("1234", z));
		REQUIRE(parseBytes("abcd", z));
		REQUIRE(parseBytes("00002200", z));
		REQUIRE(
			parseBytes("0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0"
					   "123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF",
				z));
		REQUIRE(
			parseBytes("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0"
					   "123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef",
				z));
		REQUIRE(
			parseBytes("0123456789abcdef0123456789abcdef0123456789ABCDEF0123456789ABCDEF0"
					   "123456789ABCDEF0123456789aBCdef0123456789abcdef0123456789abcdef",
				z));
		REQUIRE(
			parseBytes("00000000000000000000000000000000000000000000000000000000000000000"
					   "0000000000000000000000000000000000000000000000000000000000000ef",
				z));
		REQUIRE(
			parseBytes("00000000000000000000000000000000000000000000000000000000000000000"
					   "000000000000000000000000000000000000000000000000000000000000000",
				z));
		REQUIRE(
			parseBytes("00000000000000000000000000000000000000000000000000000000000000000"
					   "0000000000000000000000000000000000000000000000000000000000000",
				z));
		REQUIRE(
			parseBytes("f0000000000000000000000000000000000000000000000000000000000000000"
					   "0000000000000000000000000000000000000000000000000000000000000",
				z));
		REQUIRE(
			parseBytes("ff000000000000000000000000000000000000000000000000000000000000000"
					   "0000000000000000000000000000000000000000000000000000000000000",
				z));
		REQUIRE(
			parseBytes("  "
					   "ff000000000000000000000000000000000000000000000000000000000000000"
					   "0000000000000000000000000000000000000000000000000000000000000  ",
				z));
		REQUIRE(parseBytes("00", z));
	}
	SECTION("int32_t failure")
	{
		REQUIRE(!parseInt32("", x));
		REQUIRE(!parseInt32("Text", x));
		REQUIRE(!parseInt32("123x", x));
		REQUIRE(!parseInt32("+-123", x));
		REQUIRE(!parseInt32("-+123", x));
		REQUIRE(!parseInt32("0AD", x));
		REQUIRE(!parseInt32("-xAD", x));
		REQUIRE(!parseInt32("08765", x));
		REQUIRE(!parseInt32("0x1000000032", x));
		REQUIRE(!parseInt32("23-2", x));
		REQUIRE(!parseInt32("0xx1234", x));
	}
	SECTION("int64_t failure")
	{
		REQUIRE(!parseInt64("", y));
		REQUIRE(!parseInt64("Text", y));
		REQUIRE(!parseInt64("123x", y));
		REQUIRE(!parseInt64("+-123", y));
		REQUIRE(!parseInt64("-+123", y));
		REQUIRE(!parseInt64("0AD", y));
		REQUIRE(!parseInt64("-xAD", y));
		REQUIRE(!parseInt64("08765", y));
		REQUIRE(!parseInt64("0x100000000000000064", y));
		REQUIRE(!parseInt64("23-2", y));
		REQUIRE(!parseInt64("0xx1234", y));
	}
	SECTION("bytes failure")
	{
		REQUIRE(!parseBytes("1", z));
		REQUIRE(!parseBytes("324", z));
		REQUIRE(!parseBytes("abc", z));
		REQUIRE(!parseBytes("abcdefgh", z));
		REQUIRE(!parseBytes("0.1", z));
		REQUIRE(!parseBytes("", z));
		REQUIRE(!parseBytes(
			"0001020304050607080910111213141516171819202122232425262728293031323334353637"
			"383940414243444546474849505152535455565758596061626364",
			z));
		REQUIRE(!parseBytes(
			"0001020304050607080910111213141516171819202122232425262728293031323334353637"
			"383940414243444546474849505152535455565758596061626364656667",
			z));
		REQUIRE(
			!parseBytes("0001020304050607080910111213141516171819202122232425262728293031"
						"32333435363738394041424344454647484950515253545556575859606162f",
				z));
		REQUIRE(
			!parseBytes("00010203040506070809101112131415161718192021222324 "
						"2526272829303132333435363738394041424344454647484950515253545556"
						"575859606162",
				z));
	}
}

TEST_CASE("Utils: parse", "[field]")
{
	std::int32_t x{};
	std::int64_t y{};
	Bytes64 z = Bytes64{};
	Bytes64 r = Bytes64{};
	/*NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)*/
	SECTION("int32_t")
	{
		REQUIRE((x = 1, parseInt32("0", x), x) == 0);
		REQUIRE((x = 1, parseInt32("0000", x), x) == 0);
		REQUIRE((x = 0, parseInt32("123", x), x) == 123);
		REQUIRE((x = 0, parseInt32("54312", x), x) == 54312);
		REQUIRE((x = 0, parseInt32("-54312", x), x) == -54312);
		REQUIRE((x = 0, parseInt32("0xAD", x), x) == 0xAD);
		REQUIRE((x = 0, parseInt32("-0xAD", x), x) == -0xAD);
		REQUIRE((x = 0, parseInt32("0765", x), x) == 0765);
		REQUIRE((x = 0, parseInt32("-0765", x), x) == -0765);
		REQUIRE((x = 0, parseInt32("+0765", x), x) == 0765);
		REQUIRE((x = 0, parseInt32("  123", x), x) == 123);
		REQUIRE((x = 0, parseInt32("  123   ", x), x) == 123);
	}
	SECTION("int64_t")
	{
		REQUIRE((y = 1, parseInt64("0", y), y) == 0);
		REQUIRE((y = 1, parseInt64("0000", y), y) == 0);
		REQUIRE((y = 0, parseInt64("123", y), y) == 123);
		REQUIRE((y = 0, parseInt64("54312", y), y) == 54312);
		REQUIRE((y = 0, parseInt64("-54312", y), y) == -54312);
		REQUIRE((y = 0, parseInt64("0xAD", y), y) == 0xAD);
		REQUIRE((y = 0, parseInt64("-0xAD", y), y) == -0xAD);
		REQUIRE((y = 0, parseInt64("0765", y), y) == 0765);
		REQUIRE((y = 0, parseInt64("-0765", y), y) == -0765);
		REQUIRE((y = 0, parseInt64("+0765", y), y) == 0765);
		REQUIRE((y = 0, parseInt64("0x1000000000000000", y), y) == 0x1000000000000000);
		REQUIRE((y = 0, parseInt64(" \n123", y), y) == 123);
		REQUIRE((y = 0, parseInt64("  123\t\n  ", y), y) == 123);
	}
	SECTION("bytes")
	{
		SECTION("empty")
		{
			REQUIRE(parseBytes("00", z));
			REQUIRE(z == r);
		}
		SECTION("one")
		{
			REQUIRE(parseBytes("01", z));
			r[63] = 1;
			REQUIRE(z == r);
		}
		SECTION("hex")
		{
			REQUIRE(parseBytes("abcdef", z));
			r[61] = 0xab;
			r[62] = 0xcd;
			r[63] = 0xef;
			REQUIRE(z == r);
		}
		SECTION("full")
		{
			REQUIRE(parseBytes(
				"000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20212223"
				"2425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f",
				z));
			for (unsigned int i = 0; i < 64; ++i) {
				r[i] = i;
			}
			REQUIRE(z == r);
		}
	}
	/*NOLINTEND(cppcoreguidelines-avoid-magic-numbers)*/
}
