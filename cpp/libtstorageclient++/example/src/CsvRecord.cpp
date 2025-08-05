/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include "CsvRecord.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <utility>

namespace tstorage {
namespace exampleCSV {

constexpr int CsvRecord::cNumberOfFields;
constexpr int CsvRecord::cHelperArraySize;

CsvRecord::CsvRecord() : mFieldPositions{}
{
	mFieldPositions.fill(std::string::npos);
}

CsvRecord::CsvRecord(std::string recordLines)
	: mLine(std::move(recordLines)), mFieldPositions{}
{
	mFieldPositions.fill(std::string::npos);
	mFieldPositions[0] = 0;
	std::size_t pos = mLine.find(',');
	for (int i = 1; i < cHelperArraySize && pos != std::string::npos; ++i) {
		mFieldPositions[i] = pos + 1;
		pos = mLine.find(',', pos + 1);
	}
}

std::string CsvRecord::getField(std::size_t i) const
{
	std::size_t start = mFieldPositions[i];
	const std::size_t end = mFieldPositions[i + 1] - 1;
	if (start == std::string::npos) {
		start = mLine.size();
	}
	return mLine.substr(start, end - start);
}

int CsvRecord::countFields() const
{
	if (mLine.empty()) {
		return 0;
	}
	int nFields = cHelperArraySize
		- static_cast<int>(/* narrowing long->int conversions, harmless */
			std::count(
				mFieldPositions.begin(), mFieldPositions.end(), std::string::npos));
	if (nFields < cHelperArraySize) {
		return nFields;
	}
	return nFields
		+ static_cast<int>(
			std::count(mLine.begin() + static_cast<long>(mFieldPositions.back()),
				mLine.end(),
				static_cast<unsigned char>(',')));
}

} /*namespace exampleCSV */
} /*namespace tstorage*/
