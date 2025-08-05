/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include "CommandLineOptions.h"

#include <string>

#include "Log.h"

namespace tstorage {
namespace exampleCSV {

bool CommandLineOptions::parse(const int argc, const char* const* const argv)
{
	Log log = mLogInit;
	if (argc != cnMandatoryArgs + 1) {
		if (argc > 1) {
			log.error() << "Invalid number of arguments of arguments given (got "
						<< argc - 1 << ", expected " << cnMandatoryArgs << ")";
		}
		return false;
	}

	/* NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers) */
	parseAddr(argv[1]);
	if (!parsePort(argv[2])) {
		log.error() << "Invalid port (got " << argv[2] << ")";
		return false;
	}
	if (!parseCidMin(argv[3])) {
		log.error() << "Invalid lower CID (got " << argv[3] << ")";
		return false;
	}
	if (!parseMidMin(argv[4])) {
		log.error() << "Invalid lower MID (got " << argv[4] << ")";
		return false;
	}
	if (!parseMoidMin(argv[5])) {
		log.error() << "Invalid lower MOID (got " << argv[5] << ")";
		return false;
	}
	if (!parseCapMin(argv[6])) {
		log.error() << "Invalid lower CAP (got " << argv[6] << ")";
		return false;
	}
	if (!parseAcqMin(argv[7])) {
		log.error() << "Invalid lower ACQ (got " << argv[7] << ")";
		return false;
	}
	if (!parseCidMax(argv[8])) {
		log.error() << "Invalid upper CID (got " << argv[8] << ")";
		return false;
	}
	if (!parseMidMax(argv[9])) {
		log.error() << "Invalid upper MID (got " << argv[9] << ")";
		return false;
	}
	if (!parseMoidMax(argv[10])) {
		log.error() << "Invalid upper MOID (got " << argv[10] << ")";
		return false;
	}
	if (!parseCapMax(argv[11])) {
		log.error() << "Invalid upper CAP (got " << argv[11] << ")";
		return false;
	}
	if (!parseAcqMax(argv[12])) {
		log.error() << "Invalid upper ACQ (got " << argv[12] << ")";
		return false;
	}
	parseFile(argv[13]);
	/* NOLINTEND(cppcoreguidelines-avoid-magic-numbers) */

	return true;
}

} /*namespace exampleCSV */
} /*namespace tstorage*/
