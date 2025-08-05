/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include "Utils.h"

#include <cstddef>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <ios>
#include <limits>
#include <sstream>
#include <string>

namespace tstorage {
namespace exampleCSV {

std::int32_t stoi32(const std::string& str, std::size_t* pos)
{
	if (sizeof(int) == sizeof(std::int32_t)) {
		return static_cast<std::int32_t>(std::stoi(str, pos, 0));
	}
	if (sizeof(long) == sizeof(std::int32_t)) {
		return static_cast<std::int32_t>(std::stol(str, pos, 0));
	}
	return static_cast<std::int32_t>(std::stoll(str, pos, 0));
}

std::int64_t stoi64(const std::string& str, std::size_t* pos)
{
	if (sizeof(int) == sizeof(std::int64_t)) {
		return static_cast<std::int64_t>(std::stoi(str, pos, 0));
	}
	if (sizeof(long) == sizeof(std::int64_t)) {
		return static_cast<std::int64_t>(std::stol(str, pos, 0));
	}
	return static_cast<std::int64_t>(std::stoll(str, pos, 0));
}

bool allWhitespace(const std::string& str, std::size_t pos)
{
	const char* const ws = " \t\n\r\v\f";
	return str.find_first_not_of(ws, pos) == std::string::npos;
}

bool parseUInt16(const std::string& str, std::uint16_t& oVal)
{
	std::size_t pos = 0;
	unsigned long ulong = 0;
	try {
		ulong = std::stoul(str, &pos);
	} catch (std::exception& e) {
		return false;
	}
	if (ulong > std::numeric_limits<std::uint16_t>::max()) {
		return false;
	}
	oVal = ulong;
	return allWhitespace(str, pos);
}

bool parseInt32(const std::string& str, std::int32_t& oVal)
{
	std::size_t pos = 0;
	try {
		oVal = stoi32(str, &pos);
	} catch (std::exception& e) {
		return false;
	}
	return allWhitespace(str, pos);
}

bool parseInt64(const std::string& str, std::int64_t& oVal)
{
	std::size_t pos = 0;
	try {
		oVal = stoi64(str, &pos);
	} catch (std::exception& e) {
		return false;
	}
	return allWhitespace(str, pos);
}

bool parseBytes(const std::string& str, Bytes64& oVal)
{
	const char* const ws = " \t\n\r\v\f";
	std::size_t bytesPos = str.find_first_not_of(ws);
	std::size_t bytesEnd = str.find_first_of(ws, bytesPos);
	if (bytesEnd == std::string::npos) {
		bytesEnd = str.size();
	}
	if (bytesPos == std::string::npos || ((bytesEnd - bytesPos) % 2) != 0
		|| !allWhitespace(str, bytesEnd)) {
		return false;
	}
	if (str.substr(bytesPos, 2) == "0x") {
		bytesPos += 2UL;
	}

	oVal.fill(0);
	const std::size_t bytesAmt = (bytesEnd - bytesPos) / 2;
	if (bytesAmt > oVal.max_size()) {
		return false;
	}
	for (std::size_t idx = 0; idx < bytesAmt; ++idx) {
		std::uint8_t byte{};
		try {
			byte = std::stoul(str.substr(bytesPos + 2 * idx, 2), nullptr, 16);
		} catch (std::exception& e) {
			return false;
		}
		oVal[oVal.max_size() - bytesAmt + idx] = byte;
	}
	return true;
}


std::string to_string(const Bytes64& bytes)
{
	std::stringstream ss{""};
	ss << std::hex;
	for (std::uint8_t byte : bytes) {
		ss << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(byte);
	}
	return ss.str();
}

} /*namespace exampleCSV */
} /*namespace tstorage*/
