/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include "CsvToRecordParser.h"

#include <cerrno>
#include <cstring>
#include <string>

#include <tstorageclient++/DataTypes.h>

#include "CsvRecord.h"
#include "Log.h"
#include "Utils.h"

namespace tstorage {
namespace exampleCSV {

using diff_t = std::string::difference_type;

bool CsvToRecordsParser::next(Record<Bytes64>& oRecord)
{
	if (!hasMore()) {
		return ok();
	}

	CsvRecord record = CsvRecord(mFile->readLine());

	if (!ok()) {
		fail() << "file IO error occurred (" << strerror(errno) << ")";
		return false;
	}

	const int nFields = record.countFields();
	if (nFields > CsvRecord::cNumberOfFields) {
		fail() << "the record has too many fields" << " (" << nFields << ", expected "
			   << CsvRecord::cNumberOfFields << ")";
		return false;
	}

	if (nFields < CsvRecord::cNumberOfFields) {
		fail() << "the record has not enough fields" << " (" << nFields << ", expected "
			   << CsvRecord::cNumberOfFields << ")";
		return false;
	}

	if (!record.getCid(oRecord.key.cid)) {
		fail()
			<< "couldn't parse CID: '" << record.getCidStr()
			<< "' (expected a decimal, hexadecimal or octal integer of length 32 bits)";
		return false;
	}

	if (!record.getMid(oRecord.key.mid)) {
		fail()
			<< "couldn't parse MID: '" << record.getMidStr()
			<< "' (expected a decimal, hexadecimal or octal integer of length 64 bits)";
		return false;
	}

	if (!record.getMoid(oRecord.key.moid)) {
		fail()
			<< "couldn't parse MOID: '" << record.getMoidStr()
			<< "' (expected a decimal, hexadecimal or octal integer of length 32 bits)";
		return false;
	}

	if (!record.getCap(oRecord.key.cap)) {
		fail()
			<< "couldn't parse CAP: '" << record.getCapStr()
			<< "' (expected a decimal, hexadecimal or octal integer of length 64 bits)";
		return false;
	}

	if (!record.getPayload(oRecord.value)) {
		fail() << "couldn't parse payload: '" << record.getPayloadStr()
			   << "' (expected a hexadecimal number of length at most 64 bytes)";
		return false;
	}

	return true;
}

Log::Stream CsvToRecordsParser::fail()
{
	mOk = false;
	return mLog.error() << "@ " << mFile->lineNumber() << " > ";
}

} /*namespace exampleCSV */
} /*namespace tstorage*/
