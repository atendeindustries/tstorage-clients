/*
 * Copyright 2025 Atende Industries sp. z o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/** @file tstorage-example.c
 *
 * An example of using the tstorage-client C library. It demonstrates various
 * features or the library:
 * - Defining a custom payload type and its [de]serialization.
 * - Establishing a channel to a TStorage Connector service.
 * - Putting records to TStorage.
 * - Getting records from TStorage stream-wise.
 * - Handling TStorage error conditions.
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "tstorage-client/client.h"

/* Some functions return one of the following values: */
enum {
	E_OK = 0, /* Function ended successfully */
	E_FAIL = 1, /* Function failed */
	E_CLOSED = 2, /* Function failed and closed the channel */
	E_EOF = 3 /* Function reached end of file */
};

/* Values of command line arguments. */
char* programName;
char* csvPath;
char* host;
int port;
struct TSCLIENT_Key keyMin;
struct TSCLIENT_Key keyMax;

enum {
	NUM_ARGS = 14, /* Number of required command line arguments */
};

/******************
 * Parsing of command line arguments
 */

/**
 * @brief Parses an integer command line argument, with bounds checking.
 *
 * @param[in] arg string with the command-line argument
 * @param[out] val resulting value of the command line argument
 * @param min minimum allowed value of the argument, inclusive
 * @param min maximum allowed value of the argument, inclusive
 * @param[in] argName name of the command-line argument, for error printing
 * @return non-0 on success; 0 otherwise and prints error to stderr.
 */
static int parseIntArg(const char* arg, int* val, int min, int max, const char* argName)
{
	if (sscanf(arg, "%i", val) != 1 || *val < min || *val > max) {
		printf("%s: error: %s value %s invalid or out of range\n", programName, argName, arg);
		return 0;
	}
	return 1;
}

/**
 * @brief Parses an int32_t command line argument, with bounds checking.
 *
 * @param[in] arg string with the command-line argument
 * @param[out] val resulting value of the command line argument
 * @param min minimum allowed value of the argument, inclusive
 * @param min maximum allowed value of the argument, inclusive
 * @param[in] argName name of the command-line argument, for error printing
 * @return non-0 on success; 0 otherwise and prints error to stderr.
 */
static int parseInt32Arg(const char* arg, int32_t* val, int32_t min, int32_t max, const char* argName)
{
	if (sscanf(arg, "%" SCNd32, val) != 1 || *val < min || *val > max) {
		printf("%s: error: %s value %s invalid or out of range\n", programName, argName, arg);
		return 0;
	}
	return 1;
}

/**
 * @brief Parses an int64_t command line argument, with bounds checking.
 *
 * @param[in] arg string with the command-line argument
 * @param[out] val resulting value of the command line argument
 * @param min minimum allowed value of the argument, inclusive
 * @param min maximum allowed value of the argument, inclusive
 * @param[in] argName name of the command-line argument, for error printing
 * @return non-0 on success; 0 otherwise and prints error to stderr.
 */
static int parseInt64Arg(const char* arg, int64_t* val, int64_t min, int64_t max, const char* argName)
{
	if (sscanf(arg, "%" SCNd64, val) != 1 || *val < min || *val > max) {
		printf("%s: error: %s value %s invalid or out of range\n", programName, argName, arg);
		return 0;
	}
	return 1;
}

/**
 * @brief Parses command line arguments argv into global variables.
 *
 * @param argv strings with command line arguments, of length >= NUM_ARGS
 * @return non-0 on success; 0 otherwise and prints error to stderr.
 */
static int parseArgs(char* argv[])
{
	csvPath = argv[13]; /* CSV_PATH */
	host = argv[1]; /* HOST */

	/* Stop parsing on first incorrect argument. */
	return parseIntArg(argv[2], &port, 0, 0xffff, "PORT")
		   && parseInt32Arg(argv[3], &keyMin.cid, TSCLIENT_CID_MIN, TSCLIENT_CID_MAX, "MIN_CID")
		   && parseInt64Arg(argv[4], &keyMin.mid, TSCLIENT_MID_MIN, TSCLIENT_MID_MAX, "MIN_MID")
		   && parseInt32Arg(argv[5], &keyMin.moid, TSCLIENT_MOID_MIN, TSCLIENT_MOID_MAX, "MIN_MOID")
		   && parseInt64Arg(argv[6], &keyMin.cap, TSCLIENT_CAP_MIN, TSCLIENT_CAP_MAX, "MIN_CAP")
		   && parseInt64Arg(argv[7], &keyMin.acq, TSCLIENT_ACQ_MIN, TSCLIENT_ACQ_MAX, "MIN_ACQ")
		   && parseInt32Arg(argv[8], &keyMax.cid, TSCLIENT_CID_MIN, TSCLIENT_CID_MAX, "MAX_CID")
		   && parseInt64Arg(argv[9], &keyMax.mid, TSCLIENT_MID_MIN, TSCLIENT_MID_MAX, "MAX_MID")
		   && parseInt32Arg(argv[10], &keyMax.moid, TSCLIENT_MOID_MIN, TSCLIENT_MOID_MAX, "MAX_MOID")
		   && parseInt64Arg(argv[11], &keyMax.cap, TSCLIENT_CAP_MIN, TSCLIENT_CAP_MAX, "MAX_CAP")
		   && parseInt64Arg(argv[12], &keyMax.acq, TSCLIENT_ACQ_MIN, TSCLIENT_ACQ_MAX, "MAX_ACQ");
}

/**
 * @brief Prints command line usage information.
 */
static void showUsage(void)
{
	printf("Usage: %s HOST PORT MIN_CID MIN_MID MIN_MOID MIN_CAP MIN_ACQ MAX_CID MAX_MID MAX_MOID MAX_CAP MAX_ACQ CSV_PATH\n",
		programName);
}

/******************
 * Definition of the specific payload type.
 *
 * The payload contains 64 bytes of unspecified data.
 */

enum { PAYLOAD_LENGTH = 64 };
static const size_t payloadSize = sizeof(char) * PAYLOAD_LENGTH;

struct SomeDataRecord
{
	TSCLIENT_Record base;
	char value[PAYLOAD_LENGTH];
};

static size_t toBytesSomeData(const void* restrict val, void* restrict buffer, size_t size)
{
	if (size >= payloadSize) {
		char* payload = (char*)val;
		memcpy(buffer, payload, payloadSize);
	}

	return payloadSize;
}

static int fromBytesSomeData(void* restrict val, const void* restrict buffer, size_t size)
{
	if (size != payloadSize) {
		return 1;
	}

	char* payload = (char*)val;
	memcpy(payload, buffer, size);

	return 0;
}

static TSCLIENT_PayloadType SomeDataPayloadType = {
	.size = sizeof(struct SomeDataRecord),
	.offset = offsetof(struct SomeDataRecord, value),
	.toBytes = &toBytesSomeData,
	.fromBytes = &fromBytesSomeData};

/******************
 * Putting records from CSV to TStorage.
 */

/**
 * @brief Sends records to TStorage channel.
 *
 * @return E_OK on success, otherwise E_FAIL or E_CLOSED and prints error on
 * stderr.
 */
static int putRecords(TSCLIENT_Channel* chan, TSCLIENT_RecordsSet* records)
{
	TSCLIENT_ResponseStatus tsRes = TSCLIENT_Channel_put(chan, records);
	switch (tsRes) {
		case TSCLIENT_RES_OK:
			break;
		case TSCLIENT_ERR_SEND:
			fprintf(stderr, "%s: put error: socket send failed: %s\n", programName, strerror(errno));
			return E_CLOSED;
		case TSCLIENT_ERR_RECEIVE:
			if (errno == 0) {
				fprintf(stderr, "%s: put error: socket receive got less data than expected\n", programName);
			} else {
				fprintf(stderr, "%s: put error: socket receive failed: %s\n", programName, strerror(errno));
			}
			fprintf(stderr, "%s: put error: socket receive failed: %s\n", programName, strerror(errno));
			return E_CLOSED;
		case TSCLIENT_ERR_UNEXPECTED:
			fprintf(stderr, "%s: put error: received malformed response from TStorage\n", programName);
			return E_CLOSED;
		case TSCLIENT_ERR_MEMORYLIMIT:
			fprintf(stderr, "%s: put error: received too large response from TStorage\n", programName);
			return E_CLOSED;
		case TSCLIENT_ERR_RESOURCE:
			fprintf(stderr, "%s: put error: memory allocation failed: %s\n", programName, strerror(errno));
			return E_CLOSED;
		default:
			fprintf(stderr, "%s: put error: received error code %i\n", programName, tsRes);
			return E_CLOSED;
	}

	return E_OK;
}

/**
 * @brief Reads the next record from CSV stream.
 *
 * @return E_OK or E_EOF on success, otherwise E_FAIL and prints error on
 * stderr.
 */
static int getRecordFromCsvStream(struct SomeDataRecord* record, FILE* csvStream)
{
	int res = E_OK;
	errno = 0;

	int sRes = fscanf(csvStream, " %" SCNi32 " , %" SCNi64 " , %" SCNi32 " , %" SCNi64 " , ",
		&record->base.key.cid,
		&record->base.key.mid,
		&record->base.key.moid,
		&record->base.key.cap);

	if (sRes == EOF) {
		res = E_EOF;
	} else if (sRes != 4) {
		res = E_FAIL;
	} else {
		for (int i = 0; i < PAYLOAD_LENGTH; ++i) {
			sRes = fscanf(csvStream, "%2hhx\n", &record->value[i]);
			if (sRes != 1) {
				res = E_FAIL;
				break;
			}
		}
	}

	if (res == E_FAIL) {
		if (errno != 0) {
			fprintf(stderr, "%s: file error: cannot read from %s: %s\n", programName, csvPath, strerror(errno));
		} else {
			fprintf(stderr, "%s: file error: invalid data format in %s\n", programName, csvPath);
		}
	}

	return res;
}

/**
 * @brief Reads records from CSV stream and sends them to TStorage channel.
 *
 * @return E_OK on success, otherwise E_FAIL or E_CLOSED and prints error on
 * stderr.
 */
static int putRecordsFromCsvStream(TSCLIENT_Channel* chan, FILE* csvStream)
{
	/* Maximum size of the RecordsSet; when more records are coming from
	   csvStream, the set will be sent and then emptied so that it is never
	   larger than this value. Set to less than SIZE_MAX to limit memory
	   usage. */
	static const size_t maxRecordsSetSize = SIZE_MAX;

	TSCLIENT_RecordsSet* records = TSCLIENT_RecordsSet_new(&SomeDataPayloadType);
	if (records == NULL) {
		fprintf(stderr, "%s: error: cannot create a RecordsSet: %s\n", programName, strerror(errno));
		return E_FAIL;
	}

	int res;

	for (;;) {
		struct SomeDataRecord record;
		res = getRecordFromCsvStream(&record, csvStream);
		if (res != E_OK) {
			if (res == E_EOF) {
				res = E_OK;
			}
			break;
		}

		res = TSCLIENT_RecordsSet_append(records, &record.base.key, &record.value);
		if (res != E_OK) {
			fprintf(stderr, "%s: error: cannot add a Record to a RecordsSet: %s\n", programName, strerror(errno));
			break;
		}

		if (TSCLIENT_RecordsSet_size(records) < maxRecordsSetSize) {
			continue;
		}

		/* Flush the records and empty the set. */
		res = putRecords(chan, records);
		if (res != E_OK) {
			break;
		}

		/* We don't have TSCLIENT_RecordsSet_empty() in the API. */
		TSCLIENT_RecordsSet_destroy(records);
		records = TSCLIENT_RecordsSet_new(&SomeDataPayloadType);
		if (records == NULL) {
			fprintf(stderr, "%s: error: cannot create a RecordsSet: %s\n", programName, strerror(errno));
			res = E_FAIL;
			break; /* leaving with records == NULL ! */
		}
	}

	if (records != NULL) {
		/* Some records may left not sent. */
		if (res == E_OK && TSCLIENT_RecordsSet_size(records) > 0) {
			res = putRecords(chan, records);
		}

		TSCLIENT_RecordsSet_destroy(records);
	}

	return res;
}

/**
 * @brief Reads records from CSV file and sends them to TStorage channel.
 *
 * @return E_OK on success, otherwise E_FAIL or E_CLOSED and prints error on
 * stderr.
 */
static int putRecordsFromCsvFile(TSCLIENT_Channel* chan)
{
	FILE* csvStream = fopen(csvPath, "r");
	if (csvStream == NULL) {
		fprintf(stderr, "%s: file error: cannot open '%s': %s\n", programName, csvPath, strerror(errno));
		return E_FAIL;
	}

	int res = putRecordsFromCsvStream(chan, csvStream);

	if (fclose(csvStream) != 0) {
		fprintf(stderr, "%s: file error: cannot close '%s': %s\n", programName, csvPath, strerror(errno));
		if (res == E_OK) { /* might contain E_CLOSED, don't want to overwrite that */
			res = E_FAIL;
		}
	}

	return res;
}

/******************
 * Getting records from TStorage and printing them.
 */

/**
 * @brief Callback function for getStream.
 */
void printRecords(void* userData, TSCLIENT_RecordsSet* data)
{
	struct SomeDataRecord* recs = TSCLIENT_RecordsSet_elements(data);

	for (size_t i = 0; i < TSCLIENT_RecordsSet_size(data); ++i) {
		printf("%" PRId32 ",%" PRId64 ",%" PRId32 ",%" PRId64 ",%" PRId64 ",",
			recs[i].base.key.cid, recs[i].base.key.mid, recs[i].base.key.moid,
			recs[i].base.key.cap, recs[i].base.key.acq);
		for (int j = 0; j < PAYLOAD_LENGTH; ++j) {
			printf("%02hhx", recs[i].value[j]);
		}
		printf("\n");
	}
}

/**
 * @brief Retrieves records from TStorage channel and prints them.
 *
 * @return E_OK on success, otherwise E_FAIL or E_CLOSED and prints error on
 * stderr.
 */
static int getRecordsAndPrint(TSCLIENT_Channel* chan)
{
	int64_t realAcq;
	TSCLIENT_ResponseStatus tsRes = TSCLIENT_Channel_getStream(chan, &keyMin, &keyMax,
		printRecords, NULL, &realAcq);
	switch (tsRes) {
		case TSCLIENT_RES_OK:
			break;
		case TSCLIENT_ERR_SEND:
			fprintf(stderr, "%s: getStream error: socket send failed: %s\n", programName, strerror(errno));
			return E_CLOSED;
		case TSCLIENT_ERR_RECEIVE:
			if (errno == 0) {
				fprintf(stderr, "%s: getStream error: socket receive got less data than expected\n", programName);
			} else {
				fprintf(stderr, "%s: getStream error: socket receive failed: %s\n", programName, strerror(errno));
			}
			return E_CLOSED;
		case TSCLIENT_ERR_UNEXPECTED:
			fprintf(stderr, "%s: getStream error: received malformed record from TStorage\n", programName);
			return E_CLOSED;
		case TSCLIENT_ERR_RESOURCE:
			fprintf(stderr, "%s: getStream error: memory allocation failed: %s\n", programName, strerror(errno));
			return E_CLOSED;
		case TSCLIENT_ERR_INVALID:
			fprintf(stderr, "%s: getStream error: received unexpected payload from TStorage\n", programName);
			return E_CLOSED;
		case TSCLIENT_ERR_MEMORYLIMIT:
			fprintf(stderr, "%s: getStream error: received too large record from TStorage\n", programName);
			return E_CLOSED;
		default:
			fprintf(stderr, "%s: getStream error: received error code %i\n", programName, tsRes);
			return E_CLOSED;
	}

	if (realAcq != keyMax.acq) {
		fprintf(stderr, "%s: Warning: actual max Acq is %" PRId64 "\n", programName, realAcq);
	}

	return E_OK;
}

/**
 * @brief Sends and retrieves records through an open TStorage channel.
 *
 * @return E_OK on success, otherwise E_FAIL or E_CLOSED and prints error on
 * stderr.
 */
static int communicateAll(TSCLIENT_Channel* chan)
{
	/* Set channel timeout to support waiting times when send/receiving
	   1 million records. */
	struct timeval timeout = {
		.tv_sec = 150};
	int res = TSCLIENT_Channel_setTimeout(chan, &timeout);
	if (res) {
		return E_FAIL;
	}

	res = putRecordsFromCsvFile(chan);
	if (res != E_OK) {
		return res;
	}
	return getRecordsAndPrint(chan);
}

/**
 * @brief Opens TStorage channel and sends and retrieves records.
 *
 * Always closes the channel afterwards.
 *
 * @return E_OK on success, otherwise E_FAIL and prints error on stderr.
 */
static int openAndCommunicateAll(TSCLIENT_Channel* chan)
{
	TSCLIENT_ResponseStatus tsRes = TSCLIENT_Channel_connect(chan);
	switch (tsRes) {
		case TSCLIENT_ERR_INVALID:
			fprintf(stderr, "%s: channel connect error: channel was already open\n", programName);
			return E_FAIL;
		case TSCLIENT_ERR_RECEIVE:
			fprintf(stderr, "%s: channel connect error: getaddrinfo failed: %s\n", programName, strerror(errno));
			return E_FAIL;
		case TSCLIENT_ERR_RESOURCE:
			fprintf(stderr, "%s: channel connect error: socket open failed: %s\n", programName, strerror(errno));
			return E_FAIL;
		case TSCLIENT_ERR_SEND:
			fprintf(stderr, "%s: channel connect error: socket connect failed: %s\n", programName, strerror(errno));
			return E_FAIL;
		default:
			assert(tsRes == TSCLIENT_RES_OK);
	}

	int res = communicateAll(chan);

	if (res == E_CLOSED) {
		res = E_FAIL;
	} else {
		tsRes = TSCLIENT_Channel_close(chan);
		switch (tsRes) {
			case TSCLIENT_ERR_INVALID:
				fprintf(stderr, "%s: channel close error: it was already closed\n", programName);
				return E_FAIL;
			case TSCLIENT_ERR_RESOURCE:
				fprintf(stderr, "%s: channel close error: socket close failed: %s\n", programName, strerror(errno));
				return E_FAIL;
			default:
				assert(tsRes == TSCLIENT_RES_OK);
		}
	}

	return res;
}

int main(char argc, char* argv[])
{
	programName = argv[0];

	if (argc < NUM_ARGS) {
		showUsage();
		return 1;
	}

	if (!parseArgs(argv)) {
		return 1;
	}

	TSCLIENT_Channel* chan = TSCLIENT_Channel_new(host, port, &SomeDataPayloadType);
	if (chan == NULL) {
		fprintf(stderr, "%s: error: cannot create TStorage channel: %s\n", programName, strerror(errno));
		return 1;
	}

	/* Uncomment to limit the channel's memory usage to a value of choice. */
	/* TSCLIENT_Channel_setMemoryLimit(chan, value_of_choice); */

	int res = openAndCommunicateAll(chan);

	TSCLIENT_Channel_destroy(chan);

	return res != E_OK;
}
