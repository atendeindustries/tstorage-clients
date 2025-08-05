/*
 * TStorage: Client library tests (C++)
 *
 * Copyright 2025 Atende Industries
 */

#ifndef D_TSTORAGE_FLOATPAYLOAD_TESTS_PH
#define D_TSTORAGE_FLOATPAYLOAD_TESTS_PH

#include <tstorageclient++/PayloadType.h>

namespace tstorage {

class FloatPayload : public PayloadType<float>
{
public:
	std::size_t toBytes(
		const float& val, void* outputBuffer, std::size_t bufferSize) override;
	bool fromBytes(
		float& oVar, const void* payloadBuffer, std::size_t payloadSize) override;
};

} /*namespace tstorage*/

#endif
