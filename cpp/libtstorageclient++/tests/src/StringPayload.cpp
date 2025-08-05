/*
 * TStorage: Client library tests (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include "StringPayload.h"
#include <cstring>

namespace tstorage {

std::size_t StringPayload::toBytes(
	const std::string& val, void* outputBuffer, std::size_t bufferSize)
{
	if (bufferSize >= val.length()) {
		memcpy(outputBuffer, val.c_str(), val.length());
	}
	return val.length();
	;
}

bool StringPayload::fromBytes(
	std::string& oVar, const void* payloadBuffer, std::size_t payloadSize)
{
	oVar = std::string(static_cast<const char*>(payloadBuffer), payloadSize);
	return true;
}

} /*namespace tstorage*/
