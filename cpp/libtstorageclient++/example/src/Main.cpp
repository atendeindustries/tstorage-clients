/*
 * TStorage: example client (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include <cstddef>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#include <tstorageclient++/Channel.h>
#include <tstorageclient++/DataTypes.h>
#include <tstorageclient++/PayloadType.h>
#include <tstorageclient++/RecordsSet.h>
#include <tstorageclient++/Response.h>
#include <tstorageclient++/ResponseAcq.h>
#include <tstorageclient++/Timestamp.h>

#include "CommandLineOptions.h"
#include "CsvToRecordParser.h"
#include "File.h"
#include "Log.h"
#include "Utils.h"

using namespace tstorage;
using namespace tstorage::exampleCSV;

void printHelp()
{
	using std::cout;
	using std::endl;

	cout << "Usage: tstorageclient-upload-csv <addr> <port> <cidMin>\n"
			"<midMin> <moidMin> <capMin> <acqMin> <cidMax> <midMax>\n"
			"<moidMax> <capMax> <acqMax> <csvFile>\n"
		 << endl;

	cout << "Sends the content of the CSV file located in <csvFile> to a TStorage\n"
			"instance at <addr>:<port>, then performs a GET operation to obtain all\n"
			"records in the right-open key-interval [ <*Min>, <*Max> ) and display\n"
			"them on the standard output in the CSV format.\n"
		 << endl;
}

class BytesPayload : public PayloadType<Bytes64>
{
public:
	std::size_t toBytes(const Bytes64& val,
		void* const outputBuffer,
		const std::size_t bufferSize) override
	{
		if (bufferSize >= val.max_size()) {
			std::memcpy(outputBuffer, val.data(), val.max_size());
		}
		return val.max_size();
	}

	bool fromBytes(Bytes64& oVar,
		const void* const payloadBuffer,
		const std::size_t payloadSize) override
	{
		if (payloadSize != oVar.max_size()) {
			return false;
		}
		std::memcpy(oVar.data(), payloadBuffer, oVar.max_size());
		return true;
	}
};

int main(int argc, char* argv[])
{
	Log loggerOpts("while parsing command line options:");

	CommandLineOptions opts(loggerOpts);
	if (!opts.parse(argc, argv)) {
		printHelp();
		return -1;
	}

	File file(opts.getRecordFilePath());
	if (file.error()) {
		Log{}.error() << "file '" << file.name() << "' does not exist";
		return -1;
	}

	RecordsSet<Bytes64> recordsSet{};
	Record<Bytes64> record{};
	CsvToRecordsParser parser(file, Log(std::string("in '" + file.name() + "':")));
	while (parser) {
		if (!parser.next(record)) {
			return -1;
		}
		recordsSet.append(record);
	}
	file.close();

	Channel<Bytes64> channel(
		opts.getAddr(), opts.getPort(), std::make_unique<BytesPayload>());

	Response resConnect = channel.connect();
	if (resConnect.error()) {
		Log{}.error() << "Connect failed with error code " << (int)resConnect.status();
		return -1;
	}
	Response resPut = channel.put(recordsSet);
	if (resConnect.error()) {
		Log{}.error() << "PUT failed with error code " << (int)resPut.status();
		return -1;
	}

	ResponseAcq resGet = channel.getStream(
		opts.getKeyMin(), opts.getKeyMax(), [](const RecordsSet<Bytes64>& batch) {
			for (const Record<Bytes64>& rec : batch) {
				/* clang-format off */
				std::cout << rec.key.cid << ","
					<< rec.key.mid << ","
					<< rec.key.moid << ","
					<< rec.key.cap << ","
					<< rec.key.acq << ","
					<< to_string(rec.value) << "\n";
				/* clang-format on */
			}
		});
	std::cout.flush();

	if (resGet.error()) {
		Log{}.error() << "GET failed with error code " << (int)resGet.status();
		return -1;
	}

	Response resClose = channel.close();
	if (resClose.error()) {
		Log{}.error() << "Close connection failed with error code "
					  << (int)resGet.status();
		return -1;
	}
	return 0;
}
