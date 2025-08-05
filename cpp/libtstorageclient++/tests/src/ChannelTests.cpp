/*
 * TStorage: Client library tests (C++)
 *
 * Copyright 2025 Atende Industries
 */

#include "ChannelTests.h"

#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <set>

#include <tstorageclient++/Channel.h>
#include <tstorageclient++/DataTypes.h>
#include <tstorageclient++/RecordsSet.h>
#include <tstorageclient++/Response.h>
#include <tstorageclient++/ResponseAcq.h>
#include <tstorageclient++/Timestamp.h>

#include "FloatPayload.h"
#include "StringPayload.h"

namespace globals {

extern std::string addr;
extern std::uint16_t port;

} /*namespace globals*/

using std::cout;
using std::endl;
using std::chrono::system_clock;
using namespace std::chrono_literals;

namespace tstorage {

/***********************
 * Utilities
 */

namespace {

std::ostream& operator<<(std::ostream& out, const Key& key)
{
	return out << "("
			   << key.cid << ", "
			   << key.mid << ", "
			   << key.moid << ", "
			   << key.cap << ", "
			   << key.acq << ")";
}

// clang-format off
bool compKeysFloatsWithAcq(const Record<float>& a, const Record<float>& b)
{
	const void* av = &a.value;
	const void* bv = &b.value;
	return a.key.cid < b.key.cid
		|| (a.key.cid == b.key.cid && (a.key.mid < b.key.mid
		|| (a.key.mid == b.key.mid && (a.key.moid < b.key.moid
		|| (a.key.moid == b.key.moid && (a.key.cap < b.key.cap
		|| (a.key.cap == b.key.cap && (a.key.acq < b.key.acq
		|| std::memcmp(av, bv, sizeof(float)) < 0))))))));
};

bool compKeysFloats(const Record<float>& a, const Record<float>& b)
{
	const void* av = &a.value;
	const void* bv = &b.value;
	return a.key.cid < b.key.cid
		|| (a.key.cid == b.key.cid && (a.key.mid < b.key.mid
		|| (a.key.mid == b.key.mid && (a.key.moid < b.key.moid
		|| (a.key.moid == b.key.moid && (a.key.cap < b.key.cap
		|| std::memcmp(av, bv, sizeof(float)) < 0))))));
};

bool compKeysStringConditionalAcq(const Record<std::string>& a, const Record<std::string>& b)
{
	return a.key.cid < b.key.cid
		|| (a.key.cid == b.key.cid && (a.key.mid < b.key.mid
		|| (a.key.mid == b.key.mid && (a.key.moid < b.key.moid
		|| (a.key.moid == b.key.moid && (a.key.cap < b.key.cap
		|| (a.key.cap == b.key.cap && (
			(a.key.cid == 1 && (a.key.acq < b.key.acq || a.value < b.value))
				|| a.value < b.value))))))));
};

bool compKeys(const Record<std::string>& a, const Record<std::string>& b)
{
	return a.key.cid < b.key.cid
		|| (a.key.cid == b.key.cid && (a.key.mid < b.key.mid
		|| (a.key.mid == b.key.mid && (a.key.moid < b.key.moid
		|| (a.key.moid == b.key.moid && (a.key.cap < b.key.cap))))));
};
// clang-format on

template<typename T, typename Func>
int compareRecordsSets(const RecordsSet<T>& sentRecords, const RecordsSet<T>& recvdRecords, Func comp)
{
	std::set<Record<T>, decltype(comp)> sortedSent(comp);
	std::set<Record<T>, decltype(comp)> sortedResponse(comp);
	sortedSent.insert(sentRecords.begin(), sentRecords.end());
	sortedResponse.insert(recvdRecords.begin(), recvdRecords.end());
	typename std::set<Record<T>>::const_iterator sent = sortedSent.begin();
	typename std::set<Record<T>>::const_iterator recvd = sortedResponse.begin();
	while (sent != sortedSent.end() && recvd != sortedResponse.end()) {
		if (comp(*sent, *recvd)) {
			cout << "[ERROR] Record (" << sent->key << ", " << sent->value
				 << ") missing from the response." << endl;
			return -1;
		}
		if (comp(*recvd, *sent)) {
			cout << "[ERROR] Unexpected record (" << recvd->key << ", " << recvd->value
				 << ") found inside the response." << endl;
			return 1;
		}
		++sent;
		++recvd;
	}
	return 0;
}

std::int32_t getTestCid(std::int32_t tid)
{
	return 0x7ffffff0 | tid;
}

Key getTestKeyMin()
{
	Key key = cKeyMin;
	key.cid = getTestCid(0);
	key.cap = Timestamp::now();
	return key;
}

Key getTestKeyMax()
{
	Key key = cKeyMax;
	return key;
}

} /*namespace*/

/**********************
 * Tests
 */

int test_channel_connect()
{
	Channel<float> channel(globals::addr, globals::port, std::make_unique<FloatPayload>());
	cout << "Connecting..." << endl;
	Response res = channel.connect();
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 1;
	}
	cout << "Closing..." << endl;
	res = channel.close();
	if (res.error()) {
		cout << "[ERROR] Close failed: " << (int)res.status() << endl;
		return 2;
	}
	cout << "Connecting again..." << endl;
	res = channel.connect();
	if (res.error()) {
		cout << "[ERROR] Connect after close failed: " << (int)res.status() << endl;
		return 3;
	}
	cout << "Closing again..." << endl;
	res = channel.close();
	if (res.error()) {
		cout << "[ERROR] Final close failed: " << (int)res.status() << endl;
		return 4;
	}
	return 0;
}

int test_channel_getacq()
{
	Channel<float> channel(globals::addr, globals::port, std::make_unique<FloatPayload>());
	cout << "Connecting..." << endl;
	Response res = channel.connect();
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 1;
	}
	ResponseAcq resAcq = channel.getAcq(cKeyMin, cKeyMax);
	if (resAcq.error()) {
		cout << "[ERROR] getACQ failed: " << (int)resAcq.status() << endl;
		return 2;
	}
	std::time_t acqTime = system_clock::to_time_t(Timestamp::toUnix(resAcq.acq()));
	cout << "Got acq: " << resAcq.acq() << " ("
		 << std::put_time(std::localtime(&acqTime), "%Y.%m.%d, %H:%M:%S") << ")" << endl;

	cout << "Closing connection..." << endl;
	res = channel.close();
	if (res.error()) {
		cout << "[ERROR] Close failed: " << (int)res.status() << endl;
		return 3;
	}
	return 0;
}

int test_channel_get_empty()
{
	Channel<float> channel(globals::addr, globals::port, std::make_unique<FloatPayload>());
	Response res = channel.connect();
	cout << "Connecting..." << endl;
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 1;
	}

	const Key keyMin = getTestKeyMin();
	const Key keyMax = getTestKeyMax();

	channel.setTimeout(3000ms);
	cout << "Calling GET on empty TStorage..." << endl;
	ResponseGet<float> resGet = channel.get(keyMin, keyMax);
	if (resGet.error()) {
		cout << "[ERROR] GET on empty TStorage failed: " << (int)resGet.status() << endl;
		return 2;
	}

	std::size_t amtRecords = resGet.records().size();
	cout << "Got " << amtRecords << " records" << endl;
	if (amtRecords > 0) {
		cout << "[ERROR] Nonempty response received" << endl;
		return 3;
	}

	cout << "Empty response confirmed. Closing connection..." << endl;
	res = channel.close();
	if (res.error()) {
		cout << "[ERROR] Close failed: " << (int)res.status() << endl;
		return 4;
	}
	return 0;
}

int test_channel_put_empty()
{
	Channel<float> channel(globals::addr, globals::port, std::make_unique<FloatPayload>());
	Response res = channel.connect();
	cout << "Connecting..." << endl;
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 1;
	}

	const Key keyMin = getTestKeyMin();
	const Key keyMax = getTestKeyMax();

	RecordsSet<float> records;

	channel.setTimeout(3000ms);
	cout << "Calling PUT with empty RecordsSet..." << endl;
	Response resPut = channel.put(records);
	if (resPut.error()) {
		cout << "[ERROR] PUT with empty RecordsSet failed: " << (int)resPut.status() << endl;
		return 2;
	}
	cout << "Calling PUTA with empty RecordsSet..." << endl;
	Response resPutA = channel.puta(records);
	if (resPut.error()) {
		cout << "[ERROR] PUTA with empty RecordsSet failed: " << (int)resPutA.status() << endl;
		return 3;
	}
	cout << "Calling GET on whole database..." << endl;
	ResponseGet<float> resGet = channel.get(keyMin, keyMax);
	if (resGet.error()) {
		cout << "[ERROR] GET on empty TStorage failed: " << (int)resGet.status() << endl;
		return 4;
	}
	std::size_t amtRecords = resGet.records().size();
	cout << "Got " << amtRecords << " records" << endl;
	if (amtRecords > 0) {
		cout << "[ERROR] Nonempty response received" << endl;
		return 5;
	}
	cout << "Empty response confirmed. Closing connection..." << endl;
	res = channel.close();
	if (res.error()) {
		cout << "[ERROR] Close failed: " << (int)res.status() << endl;
		return 6;
	}
	return 0;
}

int test_channel_put()
{
	Channel<float> channel(globals::addr, globals::port, std::make_unique<FloatPayload>());
	channel.setTimeout(3000ms);

	const Key keyMin = getTestKeyMin();
	const Key keyMax = getTestKeyMax();

	cout << "Preparing records to send..." << endl;
	RecordsSet<float> records;
	std::random_device r{};
	std::mt19937 mt{r()};
	std::uniform_real_distribution<float> rand(-1.0F, 1.0F);
	for (long int i = 0; i < 10; ++i) {
		records.append(Key(getTestCid(1), 20, i, Timestamp::now()), rand(mt));
	}
	for (long int i = 0; i < 10; ++i) {
		records.append(Key(getTestCid(2), 20, i, Timestamp::now()), rand(mt));
	}
	for (long int i = 0; i < 10; ++i) {
		records.append(Key(getTestCid(3), 20, i, Timestamp::now()), rand(mt));
	}

	Response res = channel.connect();
	cout << "Connecting..." << endl;
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 1;
	}

	cout << "Sending records..." << endl;
	Response resPut = channel.put(records);
	if (resPut.error()) {
		cout << "[ERROR] PUT failed: " << (int)resPut.status() << endl;
		return 2;
	}
	cout << "Fetching all records from the database..." << endl;
	ResponseGet<float> resGet = channel.get(keyMin, keyMax);
	if (resGet.error()) {
		cout << "[ERROR] GET failed: " << (int)resGet.status() << endl;
		return 3;
	}

	cout << "Comparing sent records with the response..." << endl;
	int resCompare = compareRecordsSets(records, resGet.records(), compKeysFloats);
	if (resCompare != 0) {
		return 4;
	}
	cout << "Both record sets are equal. Closing connection..." << endl;
	res = channel.close();
	if (res.error()) {
		cout << "[ERROR] Close failed: " << (int)res.status() << endl;
		return 5;
	}
	return 0;
}

int test_channel_puta()
{
	Channel<float> channel(globals::addr, globals::port, std::make_unique<FloatPayload>());
	channel.setTimeout(3000ms);

	const Key keyMin = getTestKeyMin();
	const Key keyMax = getTestKeyMax();

	cout << "Preparing records to send..." << endl;
	RecordsSet<float> records;
	Timestamp::UnixTime<> baseDate = Timestamp::fromDateTime(2010, 04, 01);
	std::random_device r{};
	std::mt19937 mt{r()};
	std::uniform_real_distribution<float> rand(-1.0F, 1.0F);
	for (long int i = 0; i < 10; ++i) {
		Key::AcqT acq = Timestamp::fromUnix(baseDate + i * 1min);
		records.append(Key(getTestCid(1), 20, i, Timestamp::now(), acq), rand(mt));
	}
	for (long int i = 0; i < 10; ++i) {
		Key::AcqT acq = Timestamp::fromUnix(baseDate + (i + 30) * 1min);
		records.append(Key(getTestCid(2), 20, i, Timestamp::now(), acq), rand(mt));
	}
	for (long int i = 0; i < 10; ++i) {
		Key::AcqT acq = Timestamp::fromUnix(baseDate + (i + 60) * 1min + 10us);
		records.append(Key(getTestCid(3), 20, i, Timestamp::now(), acq), rand(mt));
	}

	Response res = channel.connect();
	cout << "Connecting..." << endl;
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 1;
	}

	cout << "Sending records..." << endl;
	Response resPut = channel.puta(records);
	if (resPut.error()) {
		cout << "[ERROR] PUTA failed: " << (int)resPut.status() << endl;
		return 2;
	}
	cout << "Fetching all records from the database..." << endl;
	ResponseGet<float> resGet = channel.get(keyMin, keyMax);
	if (resGet.error()) {
		cout << "[ERROR] GET failed: " << (int)resGet.status() << endl;
		return 3;
	}

	cout << "Comparing sent records with the response..." << endl;
	int resCompare = compareRecordsSets(records, resGet.records(), compKeysFloatsWithAcq);
	if (resCompare != 0) {
		return 4;
	}
	cout << "Both record sets are equal. Closing connection..." << endl;
	res = channel.close();
	if (res.error()) {
		cout << "[ERROR] Close failed: " << (int)res.status() << endl;
		return 5;
	}
	return 0;
}

int test_channel_put_variable_payload_size()
{
	Channel<std::string> channel(globals::addr, globals::port, std::make_unique<StringPayload>());
	channel.setTimeout(3000ms);

	const Key keyMin = getTestKeyMin();
	const Key keyMax = getTestKeyMax();

	cout << "Preparing records to send..." << endl;
	RecordsSet<std::string> records;
	RecordsSet<std::string> recordsWithAcq;
	std::array<const char*, 10> payloads{
		"A recipe for a cake:",
		"1 (18.25-ounce) package chocolate cake mix",
		"1 can prepared coconut-pecan frosting",
		"3/4 cup vegetable oil",
		"4 large eggs",
		"1 cup semi-sweet chocolate chips",
		"3/4 cup butter or margarine",
		"1 2/3 cup granulated sugar",
		"2 cups all-purpose flour",
		"Don't forget garnishes!"};

	for (long int i = 0; i < 10; ++i) {
		records.append(Key(getTestCid(1), 20, 320 + i, Timestamp::now()), payloads[i]);
	}
	for (long int i = 0; i < 10; ++i) {
		recordsWithAcq.append(Key(getTestCid(2), 20, 3, Timestamp::now(), 1000 * i), payloads[i]);
	}

	Response res = channel.connect();
	cout << "Connecting..." << endl;
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 1;
	}

	cout << "Sending records..." << endl;
	Response resPut = channel.put(records);
	if (resPut.error()) {
		cout << "[ERROR] PUT failed: " << (int)resPut.status() << endl;
		return 2;
	}
	cout << "Sending second batch via PUTA..." << endl;
	Response resPutA = channel.puta(recordsWithAcq);
	if (resPutA.error()) {
		cout << "[ERROR] PUTA failed: " << (int)resPutA.status() << endl;
		return 3;
	}

	cout << "Fetching all records from the database..." << endl;
	ResponseGet<std::string> resGet = channel.get(keyMin, keyMax);
	if (resGet.error()) {
		cout << "[ERROR] GET failed: " << (int)resGet.status() << endl;
		return 4;
	}

	for (const auto& record : recordsWithAcq) {
		records.append(record);
	}

	cout << "Comparing sent records with the response..." << endl;
	int resCompare = compareRecordsSets(records, resGet.records(), compKeysStringConditionalAcq);
	if (resCompare != 0) {
		return 4;
	}
	cout << "Both record sets are equal. Closing connection..." << endl;
	res = channel.close();
	if (res.error()) {
		cout << "[ERROR] Close failed: " << (int)res.status() << endl;
		return 5;
	}
	return 0;
}

int test_channel_get()
{
	Channel<float> channel(globals::addr, globals::port, std::make_unique<FloatPayload>());
	channel.setTimeout(3000ms);
	channel.setMemoryLimit(128UL * 1024);

	Timestamp::TimeT now = Timestamp::now();

	cout << "Preparing records to send..." << endl;
	RecordsSet<float> records;
	for (long int i = 0; i < 10; ++i) {
		for (long int j = 0; j < 10; ++j) {
			for (long int k = 0; k < 10; ++k) {
				records.append(Key(getTestCid(i + 1), j, k, now, 0), 0.0F);
			}
		}
	}

	Response res = channel.connect();
	cout << "Connecting..." << endl;
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 1;
	}

	cout << "Sending records..." << endl;
	Response resPut = channel.puta(records);
	if (resPut.error()) {
		cout << "[ERROR] PUTA failed: " << (int)resPut.status() << endl;
		return 2;
	}

	const Key keyMin(getTestCid(3), 6, 8, now, 0);
	const Key keyMax(getTestCid(6), 8, 9, now + 1, 1);
	RecordsSet<float> expectedResponse;
	for (const auto& record : records) {
		if (keyMin <= record.key && record.key <= keyMax - 1) {
			expectedResponse.append(record);
		}
	}

	cout << "Fetching records from the right-open key range " << keyMin << ", " << keyMax << "..." << endl;
	ResponseGet<float> resGet = channel.get(keyMin, keyMax);
	if (resGet.error()) {
		cout << "[ERROR] GET failed: " << (int)resGet.status() << endl;
		return 3;
	}

	cout << "Comparing the response with the expected outcome..." << endl;
	int resCompare = compareRecordsSets(expectedResponse, resGet.records(), compKeysFloats);
	if (resCompare != 0) {
		return 4;
	}
	cout << "Both record sets are equal. Closing connection..." << endl;
	res = channel.close();
	if (res.error()) {
		cout << "[ERROR] Close failed: " << (int)res.status() << endl;
		return 5;
	}
	return 0;
}

int test_channel_many_records()
{
	Channel<float> channel(globals::addr, globals::port, std::make_unique<FloatPayload>());
	channel.setTimeout(3000ms);
	channel.setMemoryLimit(64UL * 1024 * 1024);

	const Key keyMin = getTestKeyMin();
	const Key keyMax = getTestKeyMax();

	cout << "Preparing records to send..." << endl;
	RecordsSet<float> records;
	std::random_device r{};
	std::mt19937 mt{r()};
	std::uniform_real_distribution<float> rand(-1.0F, 1.0F);
	for (long int i = 0; i < 100'000; ++i) {
		records.append(
			Key(getTestCid(i % 7), i % 47, i % 23, Timestamp::now(), 1000 * i),
			rand(mt));
	}

	Response res = channel.connect();
	cout << "Connecting..." << endl;
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 1;
	}

	cout << "Sending records..." << endl;
	Response resPut = channel.puta(records);
	if (resPut.error()) {
		cout << "[ERROR] PUTA failed: " << (int)resPut.status() << endl;
		return 2;
	}

	cout << "Fetching all records from the database..." << endl;
	ResponseGet<float> resGet = channel.get(keyMin, keyMax);
	if (resGet.error()) {
		cout << "[ERROR] GET failed: " << (int)resGet.status() << endl;
		return 3;
	}

	cout << "Closing connection..." << endl;
	res = channel.close();
	if (res.error()) {
		cout << "[ERROR] Close failed: " << (int)res.status() << endl;
		return 4;
	}

	cout << "Comparing sent records with the response..." << endl;
	int resCompare = compareRecordsSets(records, resGet.records(), compKeysFloatsWithAcq);
	if (resCompare != 0) {
		return 5;
	}
	cout << "Both record sets are equal." << endl;
	return 0;
}

int test_channel_large_payloads()
{
	Channel<std::string> channel(globals::addr, globals::port, std::make_unique<StringPayload>());
	channel.setTimeout(3000ms);
	channel.setMemoryLimit(256UL * 1024 * 1024);

	const Key keyMin = getTestKeyMin();
	const Key keyMax = getTestKeyMax();
	int batchNo = 1;

	for (int size = 32 * 1024; size <= 32 * 1024 * 1024; size *= 4) {
		cout << "Testing payload size = " << size / 1024 << "KB" << endl;
		cout << "Preparing records to send..." << endl;
		RecordsSet<std::string> records;
		for (long int i = 0; i < 5; ++i) {
			records.append(
				Key(getTestCid(1), (7 * i) % 47, batchNo, Timestamp::now(), 1000 * i),
				size, '\0');
		}
		Response res = channel.connect();
		cout << "Connecting..." << endl;
		if (res.error()) {
			cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
			return 1;
		}

		cout << "Sending records..." << endl;
		Response resPut = channel.puta(records);
		if (resPut.error()) {
			cout << "[ERROR] PUTA failed: " << (int)resPut.status() << endl;
			return 2;
		}

		cout << "Fetching records from the database with a given payload size..." << endl;
		Key batchKeyMin = keyMin;
		Key batchKeyMax = keyMax;
		batchKeyMin.moid = batchNo;
		batchKeyMax.moid = batchNo + 1;
		ResponseGet<std::string> resGet = channel.get(batchKeyMin, batchKeyMax);
		if (resGet.error()) {
			cout << "[ERROR] GET failed: " << (int)resGet.status() << endl;
			return 3;
		}
		++batchNo;

		cout << "Closing connection..." << endl;
		res = channel.close();
		if (res.error()) {
			cout << "[ERROR] Close failed: " << (int)res.status() << endl;
			return 4;
		}

		cout << "Comparing sent records with the response..." << endl;
		int resCompare = compareRecordsSets(records, resGet.records(), compKeys);
		if (resCompare != 0) {
			return 5;
		}
		cout << "Both record sets are equal." << endl;
	}
	return 0;
}

int test_channel_mixed_payloads()
{
	Channel<float> channel(globals::addr, globals::port, std::make_unique<FloatPayload>());
	channel.setTimeout(3000ms);
	channel.setMemoryLimit(512UL * 1024);

	const Key keyMin = getTestKeyMin();
	const Key keyMax = getTestKeyMax();

	cout << "Preparing records to send..." << endl;
	RecordsSet<float> records;
	std::random_device r{};
	std::mt19937 mt{r()};
	std::uniform_real_distribution<float> rand(-1.0F, 1.0F);
	for (long int i = 0; i < 100; ++i) {
		records.append(Key(getTestCid(1), 2, 3, Timestamp::now(), 1000 * i), rand(mt));
	}

	Response res = channel.connect();
	cout << "Connecting..." << endl;
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 1;
	}

	cout << "Sending records..." << endl;
	Response resPut = channel.puta(records);
	if (resPut.error()) {
		cout << "[ERROR] PUTA failed: " << (int)resPut.status() << endl;
		return 2;
	}

	cout << "Closing connection..." << endl;
	res = channel.close();
	if (res.error()) {
		cout << "[ERROR] Close failed: " << (int)res.status() << endl;
		return 3;
	}

	cout << "Preparing records with different payload type..." << endl;
	Channel<std::string> channel2(globals::addr, globals::port, std::make_unique<StringPayload>());
	RecordsSet<std::string> recordsStrings;
	std::string payload = "oak";
	for (long int i = 0; i < 100; ++i) {
		recordsStrings.append(Key(getTestCid(1), 5, 3, Timestamp::now(), 1000 * i), payload);
		payload += "oak";
	}

	res = channel2.connect();
	cout << "Connecting..." << endl;
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 4;
	}

	cout << "Sending records with different payload type..." << endl;
	resPut = channel2.puta(recordsStrings);
	if (resPut.error()) {
		cout << "[ERROR] PUTA failed: " << (int)resPut.status() << endl;
		return 5;
	}

	cout << "Closing connection..." << endl;
	res = channel2.close();
	if (res.error()) {
		cout << "[ERROR] Close failed: " << (int)res.status() << endl;
		return 6;
	}

	res = channel.connect();
	cout << "Connecting back..." << endl;
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 7;
	}

	cout << "Fetching all records from the database..." << endl;
	ResponseGet<float> resGet = channel.get(keyMin, keyMax);
	if (resGet.success()) {
		cout << "[ERROR] GET succeded (expected deserialization error)" << endl;
		return 8;
	}
	if (resGet.status() != result_t::DESERIALIZATION_ERROR) {
		cout << "[ERROR] GET exited with error different from a deserialization error: "
			 << (int)resGet.status() << endl;
		return 9;
	}
	cout << "Detected deserialization error" << endl;

	cout << "Closing connection..." << endl;
	res = channel.close();
	if (res.success()) {
		cout << "[ERROR] Connection still open after error" << endl;
		return 10;
	}
	if (res.status() != result_t::NOT_CONNECTED) {
		cout << "[ERROR] Connection close failure: " << (int)res.status() << endl;
		return 11;
	}
	cout << "Disconnect on deserialization confirmed" << endl;
	return 0;
}

int test_channel_get_with_memory_limit()
{
	Channel<float> channel(globals::addr, globals::port, std::make_unique<FloatPayload>());
	channel.setTimeout(3000ms);
	channel.setMemoryLimit(64UL * 1024);

	const Key keyMin = getTestKeyMin();
	const Key keyMax = getTestKeyMax();

	cout << "Preparing records to send..." << endl;
	RecordsSet<float> records;
	std::random_device r{};
	std::mt19937 mt{r()};
	std::uniform_real_distribution<float> rand(-1.0F, 1.0F);
	for (long int i = 0; i < 100; ++i) {
		records.append(Key(getTestCid(1), 2, 3, Timestamp::now(), 1000 * i), rand(mt));
	}

	cout << "Connecting..." << endl;
	Response res = channel.connect();
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 1;
	}

	cout << "Sending records..." << endl;
	Response resPut = channel.puta(records);
	if (resPut.error()) {
		cout << "[ERROR] PUTA failed: " << (int)resPut.status() << endl;
		return 2;
	}

	channel.setMemoryLimit(512);
	cout << "Memory limit set: 512B" << endl;
	cout << "Fetching all records from the database..." << endl;
	ResponseGet<float> resGet = channel.get(keyMin, keyMax);
	if (resGet.success()) {
		cout << "[ERROR] Whole database retrieved, "
			 << resGet.records().size() << " records, "
			 << resGet.records().size() * (32 + 4 + sizeof(float)) + 12 + 4 + 20
			 << "B fetched with a limit of 512B" << endl;
		return 3;
	}
	if (resGet.status() != result_t::MEMORY_LIMIT_EXCEEDED) {
		cout << "[ERROR] GET failed with error code " << (int)resGet.status()
			 << " other than MEMORY_LIMIT_EXCEEDED" << endl;
		return 4;
	}
	cout << "Memory limit violation detected: fetched only "
			 << resGet.records().size() << " records, "
			 << resGet.records().size() * (32 + 4 + sizeof(float)) + 12 + 4 + 20
			 << "B fetched with a limit of 512B" << endl;

	cout << "Reconnecting..." << endl;
	res = channel.connect();
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 5;
	}

	std::size_t threshold = resGet.records().size();
	cout << "Fetching first " << threshold << " records (the least amount exceeding memory limit)..." << endl;
	Key eKeyMax = keyMax;
	eKeyMax.acq = threshold * 1000;
	ResponseGet<float> resGet2 = channel.get(keyMin, eKeyMax);
	if (resGet2.success()) {
		cout << "[ERROR] Whole response retrieved, "
			 << resGet2.records().size() << " records, "
			 << resGet2.records().size() * (32 + 4 + sizeof(float)) + 12 + 4 + 20
			 << "B fetched with a limit of 512B" << endl;
		return 6;
	}
	if (resGet2.status() != result_t::MEMORY_LIMIT_EXCEEDED) {
		cout << "[ERROR] GET failed with error code " << (int)resGet2.status()
			 << " other than MEMORY_LIMIT_EXCEEDED" << endl;
		return 7;
	}
	cout << "Memory limit violation detected again: success. Fetched only "
			 << resGet2.records().size() << " records, "
			 << resGet2.records().size() * (32 + 4 + sizeof(float)) + 12 + 4 + 20
			 << "B fetched with a limit of 512B" << endl;

	cout << "Reconnecting yet again..." << endl;
	res = channel.connect();
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 8;
	}

	cout << "Fetching first " << threshold - 1 << " records (should not exceed memory limit)..." << endl;
	eKeyMax.acq = (threshold - 1) * 1000;
	ResponseGet<float> resGet3 = channel.get(keyMin, eKeyMax);
	if (resGet3.error()) {
		cout << "[ERROR] Final GET failed: " << (int)resGet.status() << endl;
		return 9;
	}
	cout << "Fetch successful: fetched "
			 << resGet3.records().size() << " records, "
			 << resGet3.records().size() * (32 + 4 + sizeof(float)) + 12 + 4 + 20
			 << "B fetched with a limit of 512B" << endl;

	cout << "Closing connection..." << endl;
	res = channel.close();
	if (res.error()) {
		cout << "[ERROR] Close failed: " << (int)res.status() << endl;
		return 10;
	}
	return 0;
}

int test_channel_get_stream()
{
	Channel<float> channel(globals::addr, globals::port, std::make_unique<FloatPayload>());
	channel.setTimeout(3000ms);

	const Key keyMin = getTestKeyMin();
	const Key keyMax = getTestKeyMax();

	cout << "Preparing records to send..." << endl;
	RecordsSet<float> records;
	std::random_device r{};
	std::mt19937 mt{r()};
	std::uniform_real_distribution<float> rand(-1.0F, 1.0F);
	for (long int i = 0; i < 10000; ++i) {
		records.append(Key(getTestCid(1), 2, 3, Timestamp::now(), 1000 * i), rand(mt));
	}

	cout << "Connecting..." << endl;
	Response res = channel.connect();
	if (res.error()) {
		cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
		return 1;
	}

	cout << "Sending records..." << endl;
	Response resPut = channel.puta(records);
	if (resPut.error()) {
		cout << "[ERROR] PUTA failed: " << (int)resPut.status() << endl;
		return 2;
	}

	channel.setMemoryLimit(512);
	cout << "Memory limit set: 512B" << endl;

	RecordsSet<float> recvdRecords;
	unsigned int batchesAmt = 0;
	auto callback = [&batchesAmt, &recvdRecords](RecordsSet<float>& batch) {
		++batchesAmt;
		for (const auto& record : batch) {
			recvdRecords.append(std::move(record));
		}
	};

	cout << "Gathering whole database through getStream..." << endl;
	ResponseAcq resGet = channel.getStream(keyMin, keyMax, callback);
	if (resGet.error()) {
		cout << "[ERROR] Stream GET failed: " << (int)resGet.status() << endl;
		return 3;
	}
	if (batchesAmt == 1) {
		cout << "[ERROR] Stream GET ran through only a single batch of records" << endl;
		return 4;
	}
	cout << "Stream GET call successfull, " << batchesAmt << " batches processed." << endl;

	cout << "Comparing sent records with the response..." << endl;
	int resCompare = compareRecordsSets(records, recvdRecords, compKeysFloatsWithAcq);
	if (resCompare != 0) {
		for (const auto& r : recvdRecords) {
			cout << "g: " << r.key << ", " << r.value << endl;
		}
		return 5;
	}
	cout << "Both record sets are equal. Closing connection..." << endl;
	res = channel.close();
	if (res.error()) {
		cout << "[ERROR] Close failed: " << (int)res.status() << endl;
		return 6;
	}
	return 0;
}

int test_channel_put_bad_cid()
{
	Channel<float> channel(globals::addr, globals::port, std::make_unique<FloatPayload>());
	channel.setTimeout(3000ms);
	channel.setMemoryLimit(128UL * 1024);

	for (int cid = -3; cid < 0; ++cid) {
		const Key keyMin = getTestKeyMin();
		const Key keyMax = getTestKeyMax();

		cout << "Preparing records to send (injecting a record with cid=" << cid << ")..." << endl;
		RecordsSet<float> records;
		RecordsSet<float> recordsSent;
		std::random_device r{};
		std::mt19937 mt{r()};
		std::uniform_real_distribution<float> rand(-1.0F, 1.0F);
		for (long int i = 0; i < 50; ++i) {
			Record<float> rec(Key(getTestCid(1), 2, 3, Timestamp::now(), 1000 * i), rand(mt));
			records.append(rec);
			recordsSent.append(rec);
		}
		records.append(Key(cid, 2 - cid, 3, Timestamp::now(), 50000), rand(mt));
		for (long int i = 51; i < 100; ++i) {
			records.append(Key(getTestCid(1), 2, 3, Timestamp::now(), 1000 * i), rand(mt));
		}

		cout << "Connecting..." << endl;
		Response res = channel.connect();
		if (res.error()) {
			cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
			return 1;
		}

		cout << "Sending records..." << endl;
		Response resPut = channel.puta(records);
		if (resPut.success()) {
			cout << "[ERROR] PUTA succeeded" << endl;
			return 2;
		}
		if (resPut.status() != result_t::INVALID_KEY) {
			cout << "[ERROR] Invalid key not detected" << endl;
			return 3;
		}
		cout << "Invalid key detected as expected. Closing..." << endl;
		res = channel.close();
		if (res.status() != result_t::NOT_CONNECTED) {
			cout << "[ERROR] Connection wasn't aborted on error: " << (int)res.status() << endl;
			return 4;
		}
		cout << "Reconnecting..." << endl;
		res = channel.connect();
		if (res.error()) {
			cout << "[ERROR] Reconnect failed: " << (int)res.status() << endl;
			return 5;
		}
		cout << "Fetching all records from the database..." << endl;
		ResponseGet<float> resGet = channel.get(keyMin, keyMax);
		if (resGet.error()) {
			cout << "[ERROR] GET failed: " << (int)resGet.status() << endl;
			return 6;
		}
		cout << "Comparing sent records with the response..." << endl;
		int resCompare = compareRecordsSets(recordsSent, resGet.records(), compKeysFloatsWithAcq);
		if (resCompare != 0) {
			return 7;
		}
		cout << "All records up to the one with an invalid key were"
			 << " stored in the database. Closing connection..." << endl;
		res = channel.close();
		if (res.error()) {
			cout << "[ERROR] Close failed: " << (int)res.status() << endl;
			return 8;
		}
	}
	return 0;
}

int test_channel_put_key_out_of_range()
{
	Channel<float> channel(globals::addr, globals::port, std::make_unique<FloatPayload>());
	channel.setTimeout(3000ms);
	channel.setMemoryLimit(128UL * 1024);

	for (int testCase = 0; testCase < 6; ++testCase) {
		const Key keyMin = getTestKeyMin();
		const Key keyMax = getTestKeyMax();

		if (testCase < 5) {
			cout << "Preparing records to send using PUTA (injecting"
				 << " a record such that its key has a maximal"
				 << " projection to coordinate " << testCase + 1 << ")..." << endl;
		} else {
			cout << "Preparing records to send using PUT (injecting"
				 << " a record such that its key has a maximal"
				 << " projection to coordinate 5, which remains unused by PUT)..." << endl;
		}
		RecordsSet<float> records;
		RecordsSet<float> recordsSent;
		std::random_device r{};
		std::mt19937 mt{r()};
		std::uniform_real_distribution<float> rand(-1.0F, 1.0F);
		Record<float> rec{};
		for (long int i = 0; i < 50; ++i) {
			Key key{};
			if (testCase == 1) {
				key = Key(getTestCid(10), 2, 3, Timestamp::now(), 1000 * i);
			} else {
				key = Key(getTestCid(1), 2 + testCase, 3, Timestamp::now(), 1000 * i);
			}
			rec = Record<float>(key, rand(mt));
			records.append(rec);
			recordsSent.append(rec);
		}
		switch (testCase) {
			case 0:
				records.append(Key(cKeyMax.cid, 2, 3, Timestamp::now(), 50000), rand(mt));
				break;
			case 1:
				records.append(Key(getTestCid(10), cKeyMax.mid, 3, Timestamp::now(), 50000), rand(mt));
				break;
			case 2:
				records.append(Key(getTestCid(1), 4, cKeyMax.moid, Timestamp::now(), 50000), rand(mt));
				break;
			case 3:
				records.append(Key(getTestCid(1), 5, 3, cKeyMax.cap, Timestamp::now()), rand(mt));
				break;
			case 4:
				records.append(Key(getTestCid(1), 6, 3, Timestamp::now(), cKeyMax.acq), rand(mt));
				break;
			case 5:
				rec = Record<float>(Key(getTestCid(1), 7, 3, Timestamp::now(), cKeyMax.acq), rand(mt));
				records.append(rec);
				recordsSent.append(rec);
				break;
			default:
				break;
		}
		for (long int i = 51; i < 100; ++i) {
			Key key{};
			if (testCase == 1) {
				key = Key(getTestCid(10), 2, 3, Timestamp::now(), 1000 * i);
			} else {
				key = Key(getTestCid(1), 2 + testCase, 3, Timestamp::now(), 1000 * i);
			}
			rec = Record<float>(key, rand(mt));
			records.append(rec);
			if (testCase == 5) {
				recordsSent.append(rec);
			}
		}

		cout << "Connecting..." << endl;
		Response res = channel.connect();
		if (res.error()) {
			cout << "[ERROR] Connect failed: " << (int)res.status() << endl;
			return 1;
		}

		cout << "Sending records..." << endl;
		Response resPut = result_t::OK;
		if (testCase == 5) {
			resPut = channel.put(records);
			if (resPut.error()) {
				cout << "[ERROR] PUT failed: " << (int)resPut.status() << endl;
				return 2;
			}
			cout << "Records saved successfully. Closing..." << endl;
			res = channel.close();
			if (res.error()) {
				cout << "[ERROR] Close failed: " << (int)res.status() << endl;
				return 3;
			}
		} else {
			resPut = channel.puta(records);
			if (resPut.success()) {
				cout << "[ERROR] PUTA succeeded" << endl;
				return 4;
			}
			if (resPut.status() != result_t::INVALID_KEY) {
				cout << "[ERROR] Invalid key not detected" << endl;
				return 5;
			}
			cout << "Invalid key detected as expected. Closing..." << endl;
			res = channel.close();
			if (res.status() != result_t::NOT_CONNECTED) {
				cout << "[ERROR] Connection wasn't aborted on error: " << (int)res.status() << endl;
				return 6;
			}
		}
		cout << "Reconnecting..." << endl;
		res = channel.connect();
		if (res.error()) {
			cout << "[ERROR] Reconnect failed: " << (int)res.status() << endl;
			return 7;
		}
		cout << "Fetching all records from the database..." << endl;
		ResponseGet<float> resGet = channel.get(keyMin, keyMax);
		if (resGet.error()) {
			cout << "[ERROR] GET failed: " << (int)resGet.status() << endl;
			return 8;
		}
		cout << "Comparing sent records with the response..." << endl;
		int resCompare{};
		if (testCase == 5) {
			resCompare = compareRecordsSets(recordsSent, resGet.records(), compKeysFloats);
			if (resCompare != 0) {
				for (const auto& r : resGet.records()) {
					cout << "K" << r.key << endl;
				}
				return 9;
			}
			cout << "All records were stored in the database. Closing connection..." << endl;
		} else {
			resCompare = compareRecordsSets(recordsSent, resGet.records(), compKeysFloatsWithAcq);
			if (resCompare != 0) {
				for (const auto& r : resGet.records()) {
					cout << "K" << r.key << endl;
				}
				return 9;
			}
			cout << "All records up to the one with an invalid key were"
				 << " stored in the database. Closing connection..." << endl;
		}
		res = channel.close();
		if (res.error()) {
			cout << "[ERROR] Close failed: " << (int)res.status() << endl;
			return 10;
		}
	}
	return 0;
}

} /*namespace tstorage*/
