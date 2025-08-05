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

/** @file tstorage-client\client.h
 *
 * Communications interface for TStorage.
 */

#ifndef TSTORAGE_CLIENT_H
#define TSTORAGE_CLIENT_H

#include <stddef.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Export only symbols explicitly declared with TSTORAGE_CLIENT_EXPORT. */
#ifdef TSTORAGE_CLIENT_BUILD /* enter only when building the library itself */
#ifdef _WIN32
#define TSTORAGE_CLIENT_EXPORT __declspec(dllexport)
#elif __GNUC__ >= 4
#define TSTORAGE_CLIENT_EXPORT __attribute__((visibility("default")))
#else /* __GNUC__ >= 4 */
#define TSTORAGE_CLIENT_EXPORT
#endif /* __GNUC__ >= 4 */
#else /* TSTORAGE_CLIENT_BUILD; enter when using the library */
#ifdef _WIN32
#define TSTORAGE_CLIENT_EXPORT __declspec(dllimport)
#else /* _WIN32 */
#define TSTORAGE_CLIENT_EXPORT
#endif /* _WIN32 */
#endif /* TSTORAGE_CLIENT_BUILD */

/**
 * @brief Possible results of TSCLIENT_* functions.
 */
typedef enum {
	/* TStorage network protocol's error codes, within
	   [ INT8_MIN .. INT8_MAX ]. */
	/* TODO Remove unnecessary values after implementation is ready,
	   esp. the ones lower than INT8_MIN */
	TSCLIENT_RES_OK = 0,
	TSCLIENT_RES_ERROR = -1,
	TSCLIENT_RES_INVARG = -2,
	TSCLIENT_RES_RETRY = -3,
	TSCLIENT_RES_TIMEOUT = -4,
	TSCLIENT_RES_NOMEM = -5,
	TSCLIENT_RES_IOERR = -6,
	TSCLIENT_RES_NOPERM = -7,
	TSCLIENT_RES_NOIMPL = -8,
	TSCLIENT_RES_ABORT = -9,

	TSCLIENT_RES_UNAUTHORIZED = -11,
	TSCLIENT_RES_INACTIVE = -12,

	TSCLIENT_RES_CONTINUE = -16,

	TSCLIENT_RES_INTRERROR = -126,
	TSCLIENT_RES_CONNRESET = -127,
	TSCLIENT_RES_ADDRERROR = -128,
	TSCLIENT_RES_CONNERROR = -129,
	TSCLIENT_RES_BINDERROR = -130,
	TSCLIENT_RES_SOCKERROR = -131,

	TSCLIENT_RES_INVPATH = -132,
	TSCLIENT_RES_EXIST = -133,
	TSCLIENT_RES_NOENT = -134,
	TSCLIENT_RES_NOTDIR = -135,
	TSCLIENT_RES_BUSY = -136,
	TSCLIENT_RES_NOTEMPTY = -137,
	TSCLIENT_RES_NOTOPENED = -138,
	TSCLIENT_RES_ISDIR = -139,
	TSCLIENT_RES_OPENED = -140,
	TSCLIENT_RES_CLOSED = -141,

	TSCLIENT_RES_NOTSTARTED = -256,
	TSCLIENT_RES_RUNNING = -257,
	TSCLIENT_RES_ABORTED = -258,
	TSCLIENT_RES_REDIR = -259,

	/* Error codes specific to library, outside [ INT8_MIN .. INT8_MAX ]. */
	TSCLIENT_ERR_INVALID = INT8_MAX + 1, /* errors caused by invalid input parameters */
	TSCLIENT_ERR_MEMORYLIMIT, /* memory limit exceeded */
	TSCLIENT_ERR_RESOURCE, /* errors when getting or discarding system resources */
	TSCLIENT_ERR_RECEIVE, /* errors when retrieveing data */
	TSCLIENT_ERR_SEND, /* errors when sending or connecting */
	TSCLIENT_ERR_UNEXPECTED /* unexpected responses */
} TSCLIENT_ResponseStatus;

/**
 * @brief Minimum allowed value of TSCLIENT_Key.cid.
 */
#define TSCLIENT_CID_MIN 0

/**
 * @brief Maximum allowed value of TSCLIENT_Key.cid.
 */
#define TSCLIENT_CID_MAX INT32_MAX

/**
 * @brief Minimum allowed value of TSCLIENT_Key.mid.
 */
#define TSCLIENT_MID_MIN INT64_MIN

/**
 * @brief Maximum allowed value of TSCLIENT_Key.mid.
 */
#define TSCLIENT_MID_MAX INT64_MAX

/**
 * @brief Minimum allowed value of TSCLIENT_Key.moid.
 */
#define TSCLIENT_MOID_MIN INT32_MIN

/**
 * @brief Maximum allowed value of TSCLIENT_Key.moid.
 */
#define TSCLIENT_MOID_MAX INT32_MAX

/**
 * @brief Minimum allowed value of TSCLIENT_Key.cap.
 */
#define TSCLIENT_CAP_MIN INT64_MIN

/**
 * @brief Maximum allowed value of TSCLIENT_Key.cap.
 */
#define TSCLIENT_CAP_MAX INT64_MAX

/**
 * @brief Minimum allowed value of TSCLIENT_Key.acq.
 */
#define TSCLIENT_ACQ_MIN INT64_MIN

/**
 * @brief Maximum allowed value of TSCLIENT_Key.acq.
 */
#define TSCLIENT_ACQ_MAX INT64_MAX

/**
 * @brief A key structure for TStorage records.
 */
typedef struct TSCLIENT_Key
{
	/* Order changed from the network protocol, to eliminate struct padding. */
	int32_t cid;
	int32_t moid;
	int64_t mid;
	int64_t cap;
	int64_t acq;
} TSCLIENT_Key;

/**
 * @brief Converts a cap/acq timestamp to a Unix timestamp.
 *
 * @param ts a timestamp taken from 'TSCLIENT_Key.cap' or '.acq'
 * @return a Unix timestamp, equal to 'ts' rounded down to seconds
 */
TSTORAGE_CLIENT_EXPORT time_t TSCLIENT_timestampToUnix(int64_t ts);

/**
 * @brief Converts a Unix timestamp to a cap/acq timestamp.
 *
 * @param ts a Unix timestamp
 * @return a timestamp for use in 'TSCLIENT_Key.cap' or '.acq'
 */
TSTORAGE_CLIENT_EXPORT int64_t TSCLIENT_timestampFromUnix(time_t ts);

/**
 * @brief Makes a cap/acq timestamp representing the current time.
 *
 * Equivalent to:
 *
 * #include <stddef.h>
 * #include <time.h>
 * return TSCLIENT_TimeStampFromUnix(time(NULL));
 *
 * @return a timestamp for use in 'TSCLIENT_Key.cap' or 'acq'
 */
TSTORAGE_CLIENT_EXPORT int64_t TSCLIENT_timestampNow(void);

/**
 * @brief A record with a key and a payload.
 *
 * This structure has no payload field - the payload type is defined by user.
 * The user should define its own record structure that "inherits" from
 * 'TSCLIENT_Record', i.e. has 'TSCLIENT_Record' as its first element, and
 * a payload structure as its second element, e.g.:
 *
 * struct MyPayload {
 *     int measurementType;
 *     float measurementValue;
 * };
 *
 * struct MyRecord {
 *     TSCLIENT_Record base;
 *     MyPayload value;
 * };
 *
 * Then the user should provide the custom record structure's size, and the
 * payload's offset within this structure, to a payloadType that would later
 * be passed to a Channel, e.g.:
 *
 * TSCLIENT_PayLoadType payloadType = {
 *     .size = sizeof(struct MyRecord),
 *     .offset = offsetof(struct MyRecord, value),
 *     .toBytes = (...),
 *     .fromBytes = (...)
 * };
 *
 * A payloadType defined above shall then be passed to each RecordSet and
 * Channel:
 *
 * TSCLIENT_RecordsSet* recs = TSCLIENT_RecordsSet_new(&payloadType);
 * TSCLIENT_Channel* chan = TSCLIENT_Channel_new(host, port, &payloadType);
 *
 * Then the user should cast any void pointers obtained from RecordsSets to
 * their own record structure's pointer (see TSCLIENT_RecordsSet_elements).
 *
 * Note: If records without payload are desired, then the user does not need
 * to define custom structures and shall just use the TSCLIENT_Record
 * structure as follows:
 *
 * TSCLIENT_PayLoadType payloadType = {
 *     .size = sizeof(TSCLIENT_Record),
 *     .offset = sizeof(TSCLIENT_Record),
 *     .toBytes = (...),
 *     .fromBytes = (...),
 * }
 */
typedef struct TSCLIENT_Record
{
	TSCLIENT_Key key;
} TSCLIENT_Record;

/**
 * @brief Interface for user-defined payloads.
 *
 * Each TStorage RecordsSet and Channel has an associated payload type,
 * defined by user by passing a 'TSCLIENT_PayLoadType' structure as
 * a 'payloadType' parameter to 'TSCLIENT_RecordsSet_new' or
 * 'TSCLIENT_Channel_new'. 'payloadType' defines the size of the record with
 * payload ('size'), offset of the payload within a record ('offset'), and
 * functions to serialize ('toBytes') and deserialize ('fromBytes') a single
 * payload structure to/from a bytestream. When communicating with TStorage,
 * the Channel uses these values and functions to send or retrieve Records.
 *
 * As mentioned above, 'size' defines size of each record with payload. This
 * is not to say that variable-sized payloads are not allowed: a payload
 * structure may e.g. contain a pointer to a variable-sized memory area, that
 * is allocated and filled by the 'fromBytes' function. This way the payload
 * structure remains constant in size, as it only contains the pointer, but
 * the payload itself may have any size.
 *
 * But care must be taken in the above case - the library does not know about
 * any additional allocated memory in the payload, so it is the responsibility
 * of the user to free the memory allocated for each payload in any
 * RecordsSets sent to or retrieved from a Channel.
 */
typedef struct TSCLIENT_PayloadType
{
	/**
	 * @brief Size of the record with payload structure in bytes.
	 *
	 * A record is composed of a Key and a payload. This is the size of the
	 * whole record structure including the key and the payload.
	 *
	 * The size may be defined as sizeof(TSCLIENT_Record) to support empty
	 * payloads. In this case, records' keys are the only meaningful data
	 * stored in TStorage.
	 *
	 * See comments for TSCLIENT_Record.
	 */
	size_t size;

	/**
	 * @brief Offset of the payload structure in an encapsulating Record.
	 *
	 * See comments for TSCLIENT_Record.
	 */
	size_t offset;

	/**
	 * @brief Serializes a user-provided payload to a sequence of bytes.
	 *
	 * This function is called during sending records via a Channel (i.e.
	 * during TSCLIENT_Channel_put and TSCLIENT_Channel_puta). It serializes
	 * the user-defined payload structure 'val' to a sequence of bytes
	 * starting at '*buffer', writing no more than 'size' bytes.
	 *
	 * If it occurs that the number of bytes needed to serialize 'val' is
	 * greater than 'size', this function may abort the serialization and
	 * leave the contents of 'buffer' in an undetermined state. This will
	 * cause the library to increase the buffer's size and try again.
	 *
	 * It always returns a number of bytes needed to serialize 'val' in full.
	 * The returned value must not be greater than TSCLIENT_PAYLOAD_SIZE_MAX.
	 *
	 * @param[in] val pointer to a user-defined payload structure. May be NULL
	 * to indicate empty payload.
	 * @param[out] buffer pointer to a buffer to which 'val' shall be
	 * serialized.
	 * @param size size of 'buffer' in bytes
	 * @return Number of bytes needed to serialize 'val' (may be 0 if 'val'
	 * is NULL, i.e. an empty payload).
	 */
	size_t (*toBytes)(const void* restrict val, void* restrict buffer, size_t size);

	/**
	 * @brief Deserializes a sequence of bytes into a user-defined payload.
	 *
	 * This function is called during receiving records from a Channel (i.e.
	 * during TSCLIENT_Channel_get and TSCLIENT_Channel_getStream), once on
	 * each retrieved Record. It deserializes a sequence of bytes starting at
	 * '*buffer' of length 'size', into a user-defined payload structure
	 * pointed to by '*val'. (The size of the 'val' structure is constant for
	 * a given payload type.)
	 *
	 * This function may allocate additional memory and store pointers to it
	 * in the payload structure, but it is then the user's responsibility to
	 * free such allocated memory for all records created during communication
	 * via a Channel that uses this payload type.
	 *
	 * Should this function fail to deserialize, either because of invalid
	 * format of data in '*buffer', or because of a memory allocation error or
	 * any other reason - then it should indicate the failure by returning
	 * non-zero. This will cause the failed payload to not be included in
	 * a resulting RecordsSet, will close the Channel, and cause the
	 * get/getStream function to fail with an error.
	 *
	 * Failure indication described above is not mandatory. This function may
	 * indicate a failure by any other means, e.g. by setting an external
	 * variable or putting a special value in the payload '*val' itself - and
	 * return 0 to indicate success. This will result in retrieving all
	 * records from TStorage and not closing the Channel, and the user may
	 * then examine the external variable or payloads of the returned Records
	 * to determine which payloads deserialized successfully.
	 *
	 * @param[out] val pointer to a user-defined payload structure. 'buffer'
	 * will be deserialized here.
	 * @param[in] buffer pointer to bytes to deserialize.
	 * @param size number of bytes in 'buffer' to deserialize. It may be 0 to
	 * indicate empty payload.
	 * @return Returns 0 on success, or non-0 to indicate an error during
	 * deserialization.
	 */
	int (*fromBytes)(void* restrict val, const void* restrict buffer, size_t size);
} TSCLIENT_PayloadType;

/**
 * @brief maximum size of a serialized payload.
 *
 * This is the maximum size that a record's payload can have after
 * serialization. The user's TSCLIENT_PayloadType.toBytes function must return
 * a value not greater than this - longer payloads are not allowed.
 *
 * (Technically speaking, this is maximum size of a payload to fit in a PutA
 * request's batch that contains one record.)
 *
 * Note that the TStorage database may impose a stricter limit on payload
 * size (it is currently 32 MB). Payloads longer than that will be sent in a
 * Put/PutA request, but may be rejected by the database.
 */
#define TSCLIENT_PAYLOAD_SIZE_MAX (INT32_MAX - sizeof(int32_t) * 2 - sizeof(int64_t) * 3)

/**
 * @brief A set or records to put or get from a TStorage Channel.
 *
 * A RecordsSet holds a number of Records. It has a defined payload type, and
 * all Records inside must have payloads of that type.
 *
 * This structure may be created by user, or may be a result of a calling
 * 'TSCLIENT_Channel_get' or 'TSCLIENT_Channel_getStream'. It can be passed as
 * input to 'TSCLIENT_Channel_put' and 'TSCLIENT_Channel_puta'.
 *
 * Caution: if record payloads (as defined by a RecordsSet's payloadType)
 * contain pointers to additional allocated memory, then it is the user's
 * responsibility to free such memory on each record inside that RecordsSet.
 */
typedef struct TSCLIENT_RecordsSet TSCLIENT_RecordsSet;

/**
 * @brief Creates a new empty RecordsSet.
 *
 * A RecordsSet created this way should later be destroyed using
 * 'TSCLIENT_RecordsSet_destroy' when it is no longer needed.
 *
 * @param payloadType defines type of payload carried by records stored in
 * this RecordsSet.
 * @return a pointer to the new RecordsSet. On error, the function returns
 * NULL and sets errno(3) to any of the errors specified for malloc(3).
 *
 * @public @memberof TSCLIENT_RecordsSet
 */
TSTORAGE_CLIENT_EXPORT TSCLIENT_RecordsSet* TSCLIENT_RecordsSet_new(const TSCLIENT_PayloadType* payloadType);

/**
 * @brief Frees memory allocated by a RecordsSet.
 *
 * Caution: if record payloads (as defined by this RecordsSet's payloadType)
 * contain pointers to additional allocated memory, then it is the user's
 * responsibility to free such memory on each record inside this RecordsSet
 * before calling this function.
 *
 * @param this the RecordsSet to destroy
 *
 * @public @memberof TSCLIENT_RecordsSet
 */
TSTORAGE_CLIENT_EXPORT void TSCLIENT_RecordsSet_destroy(TSCLIENT_RecordsSet* this);

/**
 * @brief Returns number of Records in a RecordsSet.
 *
 * @param this the RecordsSet
 * @return number of Records in 'this'
 *
 * @public @memberof TSCLIENT_RecordsSet
 */
TSTORAGE_CLIENT_EXPORT size_t TSCLIENT_RecordsSet_size(const TSCLIENT_RecordsSet* this);

/**
 * @brief Returns a pointer to Records in a RecordsSet.
 *
 * This function may be called on an RecordsSet whose size (see
 * TSCLIENT_RecordsSet_size) is 0, as long as the returned pointer is not
 * dereferenced.
 *
 * @param this the RecordsSet
 * @return a pointer to the first Record in 'this'. It should be cast
 * to the user's record pointer's type (see comments for TSCLIENT_Record)
 * before iterating over it, e.g.:
 *
 * size_t sz = TSCLIENT_RecordsSet_size(aRecordsSet);
 * MyRecord* elems = TSCLIENT_RecordsSet_elements(aRecordsSet);
 * for (int i = 0; i < sz; ++i) {
 *     doSomething(&elems[i]);
 * }
 *
 * @public @memberof TSCLIENT_RecordsSet
 */
TSTORAGE_CLIENT_EXPORT void* TSCLIENT_RecordsSet_elements(const TSCLIENT_RecordsSet* this);

/**
 * @brief Appends a record to a RecordsSet.
 *
 * @param this the recordsSet being added to
 * @param key the new record's key
 * @param[in] val pointer to the new record's payload structure. It must match
 * this RecordsSet's defined payloadType.
 * @return On success returns 0. On error, the function returns non-zero and
 * sets errno(3) to any of the errors specified for malloc(3).
 *
 * @public @memberof TSCLIENT_RecordsSet
 */
TSTORAGE_CLIENT_EXPORT int TSCLIENT_RecordsSet_append(TSCLIENT_RecordsSet* restrict this, TSCLIENT_Key* restrict key, void* restrict val);

/**
 * @brief A TStorage communications channel.
 */
typedef struct TSCLIENT_Channel TSCLIENT_Channel;

/**
 * @brief Creates a new Channel for communications with a single TStorage
 * connector.
 *
 * @param[in] host hostname of the TStorage connector
 * @param port TCP port number of the TStorage connector
 * @param payloadType defines type of payload sent or received using this
 * connection
 * @return pointer to a new Channel. It should be destroyed when no longer
 * needed with 'TSCLIENT_Channel_destroy', to free allocated memory associated
 * with it.
 * On error, this function returns NULL and sets errno(3) to any of the errors
 * specified for malloc(3).
 *
 * @public @memberof TSCLIENT_Channel
 */
TSTORAGE_CLIENT_EXPORT TSCLIENT_Channel* TSCLIENT_Channel_new(const char* host, int port, const TSCLIENT_PayloadType* payloadType);

/**
 * @brief Frees memory allocated by a Channel.
 *
 * This function should be called on every Channel created with
 * 'TSCLIENT_Channel_new'.
 *
 * 'this' should be in a disconnected state when calling this function.
 *
 * @param this the Channel to destroy
 *
 * @public @memberof TSCLIENT_Channel
 */
TSTORAGE_CLIENT_EXPORT void TSCLIENT_Channel_destroy(TSCLIENT_Channel* this);

/**
 * @brief Set the Channel's socket timeout.
 *
 * This function sets the receive and send timeouts on the underlying socket
 * of the Channel 'this'. The channel must be opened.
 *
 * @param this the Channel being configured
 * @param[in] timeout the value of the timeout, see timeval(3p)
 * @return On success returns 0. On error returns non-zero and sets 'errno' to
 * one of:
 * - ENOTSOCK - 'this' is not opened.
 * - any of the errors specified for setsockopt(3).
 *
 * @public @memberof TSCLIENT_Channel
 */
TSTORAGE_CLIENT_EXPORT int TSCLIENT_Channel_setTimeout(TSCLIENT_Channel* this, struct timeval* timeout);

/**
 * @brief Set the Channel's size limit when getting records.
 *
 * This function sets the upper size limit for data transmitted to or received
 * from TStorage in one go. This includes not only the records in their
 * serialized form, but also other metadata sent to or received from TStorage.
 * In other words, this sets the maximum buffer size for data transmitted or
 * received.
 *
 * In case of TSCLIENT_Channel_put, TSCLIENT_Channel_puta and
 * TSCLIENT_Channel_getStream this means, approximately, maximum size of a
 * single Record allowed to send or receive.
 *
 * In case of TSCLIENT_Channel_get and TSCLIENT_Channel_getAcq this means
 * maximum amount of data retrieved from TStorage during the whole function
 * call.
 *
 * If a user wants to send or retrieve records of size N (incl. key), they
 * shall setMemoryLimit to a value greater than N by a certain amount (1 KB is
 * enough), otherwise TSCLIENT_Channel_get*|put* functions will fail when
 * sending or retrieving that record.
 *
 * If any of the TSCLIENT_Channel_* functions receives or sends an amount of
 * data that exceeds the memoryLimit, that function will fail with
 * TSCLIENT_ERR_MEMORYLIMIT, and, in the case of "Get" functions, will produce
 * a RecordsSet containing records received up to that point.
 *
 * @param this the Channel being configured
 * @param size maximum size of sent/received data in bytes
 */
TSTORAGE_CLIENT_EXPORT void TSCLIENT_Channel_setMemoryLimit(TSCLIENT_Channel* this, size_t size);

/**
 * @brief Connects a Channel to a TStorage connector.
 *
 * This function establishes a TCP connecion of the Channel 'this' to
 * a TStorage connector service, using parameters provided when 'this' was
 * created (see 'TSCLIENT_Channel_new').
 *
 * 'this' should be in a disconnected state when calling this function.
 *
 * @param this the Channel to connect with
 * @return On success returns TSCLIENT_RES_OK. Otherwise, it may return:
 * - If 'this' was already opened before, the function returns
 *   TSCLIENT_ERR_INVALID.
 * - If hostname resolution failed, the function returns TSCLIENT_ERR_RECEIVE
 *   and sets errno(3) to any of the errors specified for getaddrinfo(3).
 * - If creation of a socket failed, the function returns
 *   TSCLIENT_ERR_RESOURCE and sets errno(3) to any of the errors specified
 *   for socket(2).
 * - If connection to the host failed, the function returns TSCLIENT_ERR_SEND
 *   and sets errno(3) to any of the errors specified for connect(2).
 *
 * @public @memberof TSCLIENT_Channel
 */
TSTORAGE_CLIENT_EXPORT TSCLIENT_ResponseStatus TSCLIENT_Channel_connect(TSCLIENT_Channel* this);

/**
 * @brief Closes a Channel's connection to a TStorage connector.
 *
 * This function closes a connection of the Channel 'this' to a TStorage
 * connector service, previously opened with 'TSCLIENT_Channel_connect'.
 *
 * A Channel thus disconnected may be reused by calling
 * 'TSCLIENT_Channel_connect', even if the close operation ended with an error.
 *
 * @param this the Channel to disconnect
 * @return On success returns TSCLIENT_RES_OK.
 * - If 'this' was already closed before, the function returns
 *   TSCLIENT_ERR_INVALID.
 * - If closing the channel failed, the function returns TSCLIENT_RES_RESOURCE
 *   and sets errno(3) to any of the errors specified for close(2).
 *
 * @public @memberof TSCLIENT_Channel
 */
TSTORAGE_CLIENT_EXPORT TSCLIENT_ResponseStatus TSCLIENT_Channel_close(TSCLIENT_Channel* this);

/**
 * @brief Performs a GET query on a Channel, returns a set of records.
 *
 * This function retrieves all records from TStorage whose keys satisfy
 * 'keyMin' <= key < 'keyMax', and creates a RecordsSet 'data'
 * with the results.
 *
 * @param this the Channel for communications
 * @param[in] keyMin the low bound of the query
 * @param[in] keyMax the high bound of the query
 * @param[out] acq On success, the function stores the actual max ACQ value
 * here
 * @param[out] data the function stores a pointer to the resulting
 * RecordsSet here. It is the user's responsibility to destroy '*data' using
 * 'TSCLIENT_RecordsSet_destroy' when no longer needed. If this function
 * fails, '*data' may be set to NULL, or it may contain a subset of records it
 * would have retrieved if it had succeeded.
 * @return On success returns TSCLIENT_RES_OK. If any failure occured, the
 * channel gets closed and the function returns a different value. Values
 * within [ INT8_MIN .. INT8_MAX ] are error codes from the TStorage server,
 * while other values indicate errors on the client side.
 * - If sending data to the underlying socket failed, the function returns
 *   TSCLIENT_ERR_SEND and sets errno(3) to any of the errors specified for
 *   send(2).
 * - If receiving data from the underlying socket failed, the function returns
 *   TSCLIENT_ERR_RECEIVE and sets errno(3) to:
 *   - 0 to indicate that the server closed the connection when more data was
 *     expected, or
 *   - any of the errors specified for recv(2).
 * - If a response from TStorage was malformed, the function returns
 *   TSCLIENT_ERR_UNEXPECTED.
 * - If allocation of memory for deserialization of records failed, the
 *   function returns TSCLIENT_ERR_RESOURCE and sets errno(3) to any of the
 *   errors specified for malloc(3).
 * - If a retrieved payload failed to deserialize (see
 *   TSCLIENT_Payload.fromBytes), the function returns TSCLIENT_ERR_INVALID.
 * - If the size of a request to be sent to, or size of data retrieved from
 *   TStorage, exceeds the channel's memory limit (see
 *   TSCLIENT_Channel_setMemoryLimit), the function returns
 *   TSCLIENT_ERR_MEMORYLIMIT.
 * - If an error occured on the side of TStorage, the function returns an
 *   error code sent by TStorage, which may be any value from
 *   TSCLIENT_ResponseStatus within the [ INT8_MIN .. INT8_MAX ] range.
 *
 * @public @memberof TSCLIENT_Channel
 */
TSTORAGE_CLIENT_EXPORT TSCLIENT_ResponseStatus TSCLIENT_Channel_get(TSCLIENT_Channel* restrict this,
	const TSCLIENT_Key* restrict keyMin,
	const TSCLIENT_Key* restrict keyMax,
	int64_t* acq,
	TSCLIENT_RecordsSet** restrict data);

/**
 * @brief Performs a PUT query on a Channel.
 *
 * This function sends all records from a provided RecordsSet 'data', to
 * TStorage. The records' provided ACQ key values are ignored as they are
 * assigned by TStorage for all incoming records. (Compare
 * 'TSCLIENT_Channel_puta').
 *
 * @param this the Channel for communications
 * @param[in] data the RecordsSet to send. Its payload type must be the same
 * as this Channel's.
 * @return On success returns TSCLIENT_RES_OK. If any failure occured, the
 * channel gets closed and the function returns a different value. Values
 * within [ INT8_MIN .. INT8_MAX ] are error codes from the TStorage server,
 * while other values indicate errors on the client side.
 * - If sending data to the underlying socket failed, the function returns
 *   TSCLIENT_ERR_SEND and sets errno(3) to any of the errors specified for
 *   send(2).
 * - If receiving data from the underlying socket failed, the function returns
 *   TSCLIENT_ERR_RECEIVE and sets errno(3) to:
 *   - 0 to indicate that the server closed the connection when more data was
 *     expected, or
 *   - any of the errors specified for recv(2).
 * - If a response from TStorage was malformed, the function returns
 *   TSCLIENT_ERR_UNEXPECTED.
 * - If the size of a record in 'data', after serialization, or the size of a
 *   retrieved response, exceeds the channel's memory limit (see
 *   TSCLIENT_Channel_setMemoryLimit), the function returns
 *   TSCLIENT_ERR_MEMORYLIMIT.
 * - If allocation of memory for [de]serialization of data failed, the
 *   function returns TSCLIENT_ERR_RESOURCE and sets errno(3) to any of the
 *   errors specified for malloc(3).
 * - If an error occured on the side of TStorage, the function returns an
 *   error code sent by TStorage, which may be any value from
 *   TSCLIENT_ResponseStatus within the [ INT8_MIN .. INT8_MAX ] range.
 *
 * @public @memberof TSCLIENT_Channel
 */
TSTORAGE_CLIENT_EXPORT TSCLIENT_ResponseStatus TSCLIENT_Channel_put(TSCLIENT_Channel* restrict this, const TSCLIENT_RecordsSet* restrict data);

/**
 * @brief Performs a PUTA query on a Channel.
 *
 * This function sends all records from a provided RecordsSet 'data', to
 * TStorage. The records' provided ACQ key values are inserted to the database
 * intact. (Compare 'TSCLIENT_Channel_put').
 *
 * @param this the Channel for communications
 * @param[in] data the RecordsSet to send. Its payload type must be the same
 * as this Channel's.
 * @return On success returns TSCLIENT_RES_OK. If any failure occured, the
 * channel gets closed and the function returns a different value. Values
 * within [ INT8_MIN .. INT8_MAX ] are error codes from the TStorage server,
 * while other values indicate errors on the client side.
 * - If sending data to the underlying socket failed, the function returns
 *   TSCLIENT_ERR_SEND and sets errno(3) to any of the errors specified for
 *   send(2).
 * - If receiving data from the underlying socket failed, the function returns
 *   TSCLIENT_ERR_RECEIVE and sets errno(3) to:
 *   - 0 to indicate that the server closed the connection when more data was
 *     expected, or
 *   - any of the errors specified for recv(2).
 * - If a response from TStorage was malformed, the function returns
 *   TSCLIENT_ERR_UNEXPECTED.
 * - If the size of a record in 'data', after serialization, or the size of a
 *   retrieved response, exceeds the channel's memory limit (see
 *   TSCLIENT_Channel_setMemoryLimit), the function returns
 *   TSCLIENT_ERR_MEMORYLIMIT.
 * - If allocation of memory for [de]serialization of data failed, the
 *   function returns TSCLIENT_ERR_RESOURCE and sets errno(3) to any of the
 *   errors specified for malloc(3).
 * - If an error occured on the side of TStorage, the function returns an
 *   error code sent by TStorage, which may be any value from
 *   TSCLIENT_ResponseStatus within the [ INT8_MIN .. INT8_MAX ] range.
 *
 * @public @memberof TSCLIENT_Channel
 */
TSTORAGE_CLIENT_EXPORT TSCLIENT_ResponseStatus TSCLIENT_Channel_puta(TSCLIENT_Channel* restrict this, const TSCLIENT_RecordsSet* restrict data);

/**
 * @brief Performs a GET_ACQ query on a Channel.
 *
 * This function retrieves a maximum ACQ for all records from TStorage whose
 * keys satisfy 'keyMin' <= key < 'keyMax'.
 *
 * @param this the Channel for communications
 * @param[in] keyMin the low bound of the query
 * @param[in] keyMax the high bound of the query
 * @param[out] acq On success, the function stores the actual max ACQ value
 * here
 * @return On success returns TSCLIENT_RES_OK. If any failure occured, the
 * channel gets closed and the function returns a different value. Values
 * within [ INT8_MIN .. INT8_MAX ] are error codes from the TStorage server,
 * while other values indicate errors on the client side.
 * - If sending data to the underlying socket failed, the function returns
 * TSCLIENT_ERR_SEND and sets errno(3) to any of the errors specified for
 * send(2).
 * - If receiving data from the underlying socket failed, the function returns
 *   TSCLIENT_ERR_RECEIVE and sets errno(3) to:
 *   - 0 to indicate that the server closed the connection when more data was
 *     expected, or
 *   - any of the errors specified for recv(2).
 * - If a response from TStorage was malformed, the function returns
 *   TSCLIENT_ERR_UNEXPECTED.
 * - If allocation of memory for [de]serialization of data failed, the
 *   function returns TSCLIENT_ERR_RESOURCE and sets errno(3) to any of the
 *   errors specified for malloc(3).
 * - If the size of a request to be sent to, or size of data retrieved from
 *   TStorage, exceeds the channel's memory limit (see
 *   TSCLIENT_Channel_setMemoryLimit), the function returns
 *   TSCLIENT_ERR_MEMORYLIMIT.
 * - If an error occured on the side of TStorage, the function returns an
 *   error code sent by TStorage, which may be any value from
 *   TSCLIENT_ResponseStatus within the [ INT8_MIN .. INT8_MAX ] range.
 *
 * @public @memberof TSCLIENT_Channel
 */
TSTORAGE_CLIENT_EXPORT TSCLIENT_ResponseStatus TSCLIENT_Channel_getAcq(TSCLIENT_Channel* restrict this,
	const TSCLIENT_Key* restrict keyMin,
	const TSCLIENT_Key* restrict keyMax,
	int64_t* acq);

/**
 * @brief Function called when a TSCLIENT_Channel_getStream fetches records.
 *
 * The user provides a function of this type as the 'callback' parameter when
 * calling 'TSCLIENT_Channel_getStream', together with a 'userData' parameter
 * which is a user-supplied data structure. This function will then be called
 * every time a set of records is received by the get request.
 *
 * The function shall process each record in the provided RecordSet 'data'.
 * Caution 1: if record payloads (as defined in a payloadType passed to
 * a Channel) contain pointers to additional allocated memory, then it is
 * the user's responsibility to free such memory on each record inside 'data'
 * before returning from this function.
 * Caution 2: the function shall not destroy the records in 'data', nor 'data'
 * itself.
 *
 * @param userData the value of 'userData' provided when calling
 * 'TSCLIENT_Channel_getStream', may be whatever the user wishes, even NULL
 * @param[in] data a RecordsSet with records received from TStorage. Its
 * payload type is the same as the payload type of the Channel's on which
 * 'TSCLIENT_Channel_getStream' is being called.
 */
typedef void (*TSCLIENT_GetCallback)(void* userData, TSCLIENT_RecordsSet* data);

/**
 * @brief Performs a GET query on a Channel, calls callback on results.
 *
 * This function retrieves all records from TStorage whose keys satisfy
 * 'keyMin' <= key < 'keyMax', and calls 'callback' on each set of retrieved
 * records. This allows to perform operations on all retrieved records, such
 * as passing them to an output stream, without allocating a chunk of memory
 * to store all of them.
 *
 * @param this the Channel for communications
 * @param[in] keyMin the low bound of the query
 * @param[in] keyMax the high bound of the query
 * @param[in] callback a function to call for each retrieved RecordsSet
 * @param userData a user-provided data, passed verbatim to 'callback'
 * @param[out] acq On success, the function stores the actual max ACQ value
 * here
 * @return On success returns TSCLIENT_RES_OK. If any failure occured, the
 * channel gets closed, 'callback' may be called on any records retrieved up
 * to that point, and the function returns a different value. Values within
 * [ INT8_MIN .. INT8_MAX ] are error codes from the TStorage server, while
 * other values indicate errors on the client side.
 * - If sending data to the underlying socket failed, the function returns
 *   TSCLIENT_ERR_SEND and sets errno(3) to any of the errors specified for
 *   send(2).
 * - If receiving data from the underlying socket failed, the function returns
 *   TSCLIENT_ERR_RECEIVE and sets errno(3) to:
 *   - 0 to indicate that the server closed the connection when more data was
 *     expected, or
 *   - any of the errors specified for recv(2).
 * - If a response from TStorage was malformed, the function returns
 *   TSCLIENT_ERR_UNEXPECTED.
 * - If allocation of memory for deserialization of records failed, the
 *   function returns TSCLIENT_ERR_RESOURCE and sets errno(3) to any of the
 *   errors specified for malloc(3).
 * - If a retrieved payload failed to deserialize (see
 *   TSCLIENT_Payload.fromBytes), the function returns TSCLIENT_ERR_INVALID.
 * - If the size of a request to be sent to, or size of a record retrieved
 *   from TStorage, exceeds the channel's memory limit (see
 *   TSCLIENT_Channel_setMemoryLimit), the function returns
 *   TSCLIENT_ERR_MEMORYLIMIT.
 * - If an error occured on the side of TStorage, the function returns an
 *   error code sent by TStorage, which may be any value from
 *   TSCLIENT_ResponseStatus within the [ INT8_MIN .. INT8_MAX ] range.
 *
 * @public @memberof TSCLIENT_Channel
 */
TSTORAGE_CLIENT_EXPORT TSCLIENT_ResponseStatus TSCLIENT_Channel_getStream(TSCLIENT_Channel* restrict this,
	const TSCLIENT_Key* restrict keyMin,
	const TSCLIENT_Key* restrict keyMax,
	TSCLIENT_GetCallback callback,
	void* userData,
	int64_t* acq);

#ifdef __cplusplus
}
#endif

#endif /* TSTORAGE_CLIENT_H */
