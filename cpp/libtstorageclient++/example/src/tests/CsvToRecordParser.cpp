/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include <tstorageclient++/DataTypes.h>

#include "../CsvToRecordParser.h"
#include "../File.h"
#include "../Log.h"
#include "../Utils.h"

#include "TmpDir.h"

using namespace tstorage;
using namespace tstorage::exampleCSV;

std::ostream& operator<<(std::ostream& ostr, const Key& key)
{
	return ostr << "Key(" << key.cid << ", " << key.mid << ", " << key.moid << ", "
				<< key.cap << ", " << key.acq << ")";
}

#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_adapters.hpp>
#include <catch2/generators/catch_generators_random.hpp>
#include <catch2/internal/catch_unique_ptr.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

using namespace Catch::Generators;
using namespace Catch::Matchers;

namespace {

class RecordGenerator : public IGenerator<Record<Bytes64>>
{
	static constexpr Key::CidT cUpperCid = 100;
	static constexpr Key::MidT cUpperMid = 1000;
	static constexpr Key::MoidT cUpperMoid = 50;
	static constexpr Key::CapT cUpperCap = 1000000;

public:
	RecordGenerator()
		: mRand{std::random_device{}()}
		, mDistCid(0, cUpperCid)
		, mDistMid(0, cUpperMid)
		, mDistMoid(0, cUpperMoid)
		, mDistCap(0, cUpperCap)
	{
		RecordGenerator::next();
	}

	bool next() override
	{
		mRecord.key.cid = mDistCid(mRand);
		mRecord.key.mid = mDistMid(mRand);
		mRecord.key.moid = mDistMoid(mRand);
		mRecord.key.cap = mDistCap(mRand);
		for (std::uint8_t& byte : mRecord.value) {
			byte = mRand();
		}
		return true;
	}

	const Record<Bytes64>& get() const override { return mRecord; }


private:
	Record<Bytes64> mRecord;
	std::minstd_rand mRand;
	std::uniform_int_distribution<Key::CidT> mDistCid;
	std::uniform_int_distribution<Key::MidT> mDistMid;
	std::uniform_int_distribution<Key::MoidT> mDistMoid;
	std::uniform_int_distribution<Key::CapT> mDistCap;
};

GeneratorWrapper<Record<Bytes64>> record()
{
	return GeneratorWrapper<Record<Bytes64>>(
		Catch::Detail::make_unique<RecordGenerator>());
}

std::string validCsvLine(const Record<Bytes64>& rec, char delim)
{
	std::ostringstream ss{};
	ss << rec.key.cid << delim;
	ss << rec.key.mid << delim;
	ss << rec.key.moid << delim;
	ss << rec.key.cap << delim;
	ss << to_string(rec.value) << '\n';
	return ss.str();
}

std::string validCsvLineMangled(const Record<Bytes64>& rec, char delim)
{
	std::ostringstream ss{};
	ss << rec.key.cid << delim;
	ss << " " << rec.key.mid << delim;
	ss << rec.key.moid << " " << delim;
	ss << "\t" << rec.key.cap << "   \t " << delim;
	ss << to_string(rec.value) << "\t\n";
	return ss.str();
}

File createTmpCsv(
	const TmpDir& dir, const std::string& name, const std::vector<Record<Bytes64>>& recs)
{
	constexpr float cMangleThreshold = 0.9F;
	std::string tmpFile = dir.pathToFile(name);
	std::ofstream fs(tmpFile);
	REQUIRE(fs.good());
	std::minstd_rand rand(std::random_device{}());
	std::uniform_real_distribution<float> fDist(0.0F, 1.0F);
	for (const Record<Bytes64>& rec : recs) {
		if (fDist(rand) > cMangleThreshold) {
			fs << validCsvLineMangled(rec, ',');
		} else {
			fs << validCsvLine(rec, ',');
		}
	}
	fs.close();

	File file(tmpFile);
	REQUIRE(!file.error());
	return file;
}

File createTmpCsvInjectLine(const TmpDir& dir,
	const std::string& name,
	std::size_t breakPoint,
	const std::string& line,
	const std::vector<Record<Bytes64>>& recs)
{
	std::string tmpFile = dir.pathToFile(name);
	std::ofstream fs(tmpFile);
	REQUIRE(fs.good());
	for (std::size_t i = 0; i < breakPoint; ++i) {
		fs << validCsvLine(recs[i], ',');
	}
	fs << line << '\n';
	for (std::size_t i = breakPoint; i < recs.size(); ++i) {
		fs << validCsvLine(recs[i], ',');
	}
	fs.close();

	File file(tmpFile);
	REQUIRE(!file.error());
	return file;
}

} /*namespace*/

TEST_CASE("CsvToRecordParser: valid CSV", "[file][parser]")
{
	TmpDir tmpDir;
	REQUIRE(tmpDir.valid());

	constexpr std::size_t nRecords = 100;
	constexpr std::size_t nChunks = 3;
	auto recs = GENERATE(take(nChunks, chunk(nRecords, record())));

	File file = createTmpCsv(tmpDir, "valid.csv", recs);
	std::string filename = file.name();

	INFO("Fn: " << filename);
	Record<Bytes64> rec{};
	std::stringstream ss;
	Log log(ss);
	CsvToRecordsParser csvParser(file, log);
	std::size_t i = 0;
	while (csvParser && i < recs.size()) {
		bool ok = csvParser.next(rec);
		INFO("Key: " << rec.key);
		INFO("Val: " << to_string(rec.value));
		INFO("Msg: \"" << ss.str() << "\"");
		REQUIRE((ok || i == recs.size() - 1));
		REQUIRE(rec.key == recs[i].key);
		REQUIRE(rec.value == recs[i].value);
		++i;
	}
	INFO("Itr: " << i);
	INFO("Msg: \"" << ss.str() << "\"");
	REQUIRE(i == nRecords);
	REQUIRE(csvParser.ok());
	REQUIRE(!csvParser.hasMore());
	file.close();
	REQUIRE(std::remove(filename.c_str()) == 0);
	REQUIRE(tmpDir.remove());
}

TEST_CASE("CsvToRecordParser: invalid CSV", "[file][parser]")
{
	TmpDir tmpDir;
	REQUIRE(tmpDir.valid());

	constexpr std::size_t nRecords = 100;
	constexpr std::size_t nChunks = 1;
	auto recs = GENERATE(take(nChunks, chunk(nRecords, record())));

	std::ostringstream ss{};
	auto randIdx = GENERATE(take(3, random<std::size_t>(0, 99)));

	SECTION("Not enough fields")
	{
		auto line = GENERATE(as<std::string>{},
			"0,1",
			"0,1,2",
			",,,"
			"",
			"0,1,2,3",
			"0:1:2:3:4.0");
		ss << line;
	}

	SECTION("Too many fields")
	{
		auto line =
			GENERATE(as<std::string>{}, "0,1,2,3,4,5", "0,1,2,3,4,5,6", ",,,,,,,,,,");
		ss << line;
	}

	SECTION("Invalid data")
	{
		auto line = GENERATE(as<std::string>{},
			"1.3,1,2,3,4.0",
			"0a,1,2,3,4.0",
			"0,\"text\",2,3,4.0",
			"0,1,+-2,3,4.0",
			"0,1,2,0xG,4.0",
			"0,1,2,3 5,4.0");
		ss << line;
	}

	std::string extraLine = ss.str();
	File file = createTmpCsvInjectLine(tmpDir, "invalid.csv", randIdx, extraLine, recs);
	std::string filename = file.name();

	INFO("Fn: " << filename);
	INFO("Idx: " << randIdx);
	INFO("Line: " << extraLine);
	Record<Bytes64> rec{};
	std::stringstream sslog{};
	Log log(sslog);
	CsvToRecordsParser csvParser(file, log);
	std::size_t i = 0;
	while (csvParser && i < recs.size()) {
		bool ok = csvParser.next(rec);
		INFO("Key: " << rec.key);
		INFO("Val: " << to_string(rec.value));
		INFO("Msg: \"" << sslog.str() << "\"");
		if (i != randIdx) {
			REQUIRE(ok);
			REQUIRE(rec.value == recs[i].value);
		} else {
			REQUIRE(!ok);
		}
		++i;
	}
	INFO("Msg: \"" << sslog.str() << "\"");
	REQUIRE(i == randIdx + 1);
	REQUIRE(!csvParser.ok());
	file.close();
	REQUIRE(std::remove(filename.c_str()) == 0);
	REQUIRE(tmpDir.remove());
}

TEST_CASE("CsvToRecordParser: file IO error", "[file][parser]")
{
	File file{};
	std::stringstream sslog{};
	CsvToRecordsParser csvParser(file, Log{sslog});
	Record<Bytes64> rec{};
	bool ok = csvParser.next(rec);
	INFO("Msg: \"" << sslog.str() << "\"");
	REQUIRE(!ok);
	REQUIRE(!csvParser.ok());
	REQUIRE(rec.key == Key{});
	REQUIRE(rec.value == Bytes64{});
}
