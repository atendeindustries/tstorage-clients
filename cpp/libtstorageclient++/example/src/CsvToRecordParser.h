/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#ifndef D_TSTORAGE_EXAMPLECSV_CSVTORECORDPARSER_H
#define D_TSTORAGE_EXAMPLECSV_CSVTORECORDPARSER_H

#include <utility>

#include <tstorageclient++/DataTypes.h>

#include "File.h"
#include "Log.h"
#include "Utils.h"

namespace tstorage {
namespace exampleCSV {

class CsvToRecordsParser
{
public:
	CsvToRecordsParser(File& file, Log log = Log{})
		: mFile(&file), mOk(true), mLog(std::move(log))
	{
	}

	bool next(Record<Bytes64>& oRecord);

	bool hasMore() const { return mOk && mFile->lineAvailable(); }
	bool ok() const { return mOk && !mFile->error(); }
	operator bool() const { return hasMore(); }

private:
	Log::Stream fail();

	File* mFile;
	bool mOk;
	Log mLog;
};

} /*namespace exampleCSV */
} /*namespace tstorage*/

#endif
