/*
 * TStorage: Client library tests (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include "FloatPayload.h"

namespace tstorage {

std::size_t FloatPayload::toBytes(
	const float& val, void* outputBuffer, std::size_t bufferSize)
{
	if (bufferSize >= sizeof(float)) {
		*static_cast<float*>(outputBuffer) = val;
	}
	return sizeof(float);
}

bool FloatPayload::fromBytes(
	float& oVar, const void* payloadBuffer, std::size_t payloadSize)
{
	if (payloadSize != sizeof(float)) {
		return false;
	}
	oVar = *static_cast<const float*>(payloadBuffer);
	return true;
}

} /*namespace tstorage*/
