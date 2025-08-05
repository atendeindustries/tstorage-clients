/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#ifndef D_TSTORAGE_EXAMPLECSV_CSVRECORD_H
#define D_TSTORAGE_EXAMPLECSV_CSVRECORD_H

#include <array>
#include <cstddef>
#include <string>

#include <tstorageclient++/DataTypes.h>
#include "Utils.h"

namespace tstorage {
namespace exampleCSV {

class CsvRecord
{
public:
	constexpr static int cNumberOfFields = 5;
	constexpr static int cHelperArraySize = cNumberOfFields + 1;

	CsvRecord();
	CsvRecord(std::string recordLine);

	bool getCid(Key::CidT& x) const { return parseInt32(getField(0), x); }
	bool getMid(Key::MidT& x) const { return parseInt64(getField(1), x); }
	bool getMoid(Key::MoidT& x) const { return parseInt32(getField(2), x); }
	bool getCap(Key::CapT& x) const { return parseInt64(getField(3), x); }
	bool getPayload(Bytes64& x) const { return parseBytes(getField(4), x); }

	std::string getCidStr() const { return getField(0); }
	std::string getMidStr() const { return getField(1); }
	std::string getMoidStr() const { return getField(2); }
	std::string getCapStr() const { return getField(3); }
	std::string getPayloadStr() const { return getField(4); }

	const std::string& str() const { return mLine; }

	std::string getField(std::size_t i) const;
	int countFields() const;

private:
	std::string mLine;
	std::array<std::size_t, cHelperArraySize> mFieldPositions;
};

} /*namespace exampleCSV */
} /*namespace tstorage*/

#endif
