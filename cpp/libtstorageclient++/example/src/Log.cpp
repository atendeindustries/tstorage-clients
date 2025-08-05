/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include "Log.h"

#include <utility>

namespace tstorage {
namespace exampleCSV {

Log::Stream::~Stream()
{
	if (mStream != nullptr) {
		*mStream << "\n\n";
	}
}

Log::Stream::Stream(Log::Stream&& other) noexcept : mStream(other.mStream)
{
	other.mStream = nullptr;
}

Log::Stream& Log::Stream::operator=(Log::Stream&& other) noexcept
{
	std::swap(mStream, other.mStream);
	return *this;
}

} /*namespace exampleCSV */
} /*namespace tstorage*/
