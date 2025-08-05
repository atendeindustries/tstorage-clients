/*
 * TStorage: Client library tests (C++)
 *
 * Copyright 2025 Atende Industries
 */

#ifndef D_TSTORAGE_STRINGPAYLOAD_TESTS_PH
#define D_TSTORAGE_STRINGPAYLOAD_TESTS_PH

#include <string>

#include <tstorageclient++/PayloadType.h>

namespace tstorage {

class StringPayload : public PayloadType<std::string>
{
public:
	std::size_t toBytes(
		const std::string& val, void* outputBuffer, std::size_t bufferSize) override;
	bool fromBytes(
		std::string& oVar, const void* payloadBuffer, std::size_t payloadSize) override;
};

} /*namespace tstorage*/

#endif
