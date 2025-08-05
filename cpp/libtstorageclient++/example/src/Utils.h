/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#ifndef D_TSTORAGE_EXAMPLECSV_UTILS_H
#define D_TSTORAGE_EXAMPLECSV_UTILS_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace tstorage {
namespace exampleCSV {

using Bytes64 = std::array<uint8_t, 64>;

std::int32_t stoi32(const std::string& str, std::size_t* pos = nullptr);
std::int64_t stoi64(const std::string& str, std::size_t* pos = nullptr);

bool allWhitespace(const std::string& str, std::size_t pos);

bool parseUInt16(const std::string& str, std::uint16_t& oVal);
bool parseInt32(const std::string& str, std::int32_t& oVal);
bool parseInt64(const std::string& str, std::int64_t& oVal);
bool parseBytes(const std::string& str, Bytes64& oVal);

std::string to_string(const Bytes64& bytes);

} /*namespace exampleCSV */
} /*namespace tstorage*/

#endif
