/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#ifndef D_TSTORAGE_EXAMPLECSV_COMMANDLINEOPTIONS_H
#define D_TSTORAGE_EXAMPLECSV_COMMANDLINEOPTIONS_H

#include <cstdint>
#include <string>
#include <utility>

#include <tstorageclient++/DataTypes.h>

#include "Log.h"
#include "Utils.h"

namespace tstorage {
namespace exampleCSV {

class CommandLineOptions
{
	static constexpr int cnMandatoryArgs = 13;
	static constexpr const char* const cDefaultAddr = "localhost";
	static constexpr std::uint16_t cDefaultPort = 2025;

public:
	CommandLineOptions(Log log = Log{})
		: mAddr(cDefaultAddr)
		, mPort(cDefaultPort)
		, mKeyMin(cKeyMin)
		, mKeyMax(cKeyMax)
		, mLogInit(std::move(log))
	{
	}

	bool parse(int argc, const char* const* argv);

	std::string getRecordFilePath() const { return mCsvFile; }
	std::string getAddr() const { return mAddr; }
	std::uint16_t getPort() const { return mPort; }
	Key getKeyMin() const { return mKeyMin; }
	Key getKeyMax() const { return mKeyMax; }

private:
	void parseFile(const std::string& filename) { mCsvFile = filename; }
	void parseAddr(const std::string& addr) { mAddr = addr; }
	bool parsePort(const std::string& port) { return parseUInt16(port, mPort); }

	bool parseCidMin(const std::string& sCid) { return parseInt32(sCid, mKeyMin.cid); }
	bool parseCidMax(const std::string& sCid) { return parseInt32(sCid, mKeyMax.cid); }
	bool parseMidMin(const std::string& sMid) { return parseInt64(sMid, mKeyMin.mid); }
	bool parseMidMax(const std::string& sMid) { return parseInt64(sMid, mKeyMax.mid); }
	bool parseMoidMin(const std::string& sMoid)
	{
		return parseInt32(sMoid, mKeyMin.moid);
	}
	bool parseMoidMax(const std::string& sMoid)
	{
		return parseInt32(sMoid, mKeyMax.moid);
	}
	bool parseCapMin(const std::string& sCap) { return parseInt64(sCap, mKeyMin.cap); }
	bool parseCapMax(const std::string& sCap) { return parseInt64(sCap, mKeyMax.cap); }
	bool parseAcqMin(const std::string& sAcq) { return parseInt64(sAcq, mKeyMin.acq); }
	bool parseAcqMax(const std::string& sAcq) { return parseInt64(sAcq, mKeyMax.acq); }

	std::string mAddr;
	std::uint16_t mPort;
	std::string mCsvFile;
	Key mKeyMin;
	Key mKeyMax;
	Log mLogInit;
};

} /*namespace exampleCSV */
} /*namespace tstorage*/

#endif
