NAME
----
tstorage-example - example program that uses the tstorage-client C library

SYNOPSIS
--------
tstorage-example HOST PORT MIN_CID MIN_MID MIN_MOID MIN_CAP MIN_ACQ MAX_CID MAX_MID MAX_MOID MAX_CAP MAX_ACQ CSV_PATH

DESCRIPTION
-----------
The file example/tstorage-example.c is an example program that uses the
tstorage-client library to communicate with TStorage. It demonstrates several
different aspects of using the library by first sending records to TStorage,
then retrieving a range of records.

tstorage-example defines its own payload type: an array of 64 bytes of data,
along with functions "toBytesSomeData" and "fromBytesSomeData" that serialize
and deserialize the payload.

tstorage-example connects to a TStorage Connector that is located at hostname
HOST and listens for connections on port number PORT. Then it reads records
from a CSV file located at CSV_PATH (see INPUT FORMAT for details) and sends
them to TStorage.

Then, tstorage-example attempts to retrieve a range of records from TStorage,
bound by the provided minimum and maximum values. The parameters MIN_CID,
MIN_MID, MIN_MOID, MIN_CAP and MIN_ACQ define the lower bound, while MAX_CID,
MAX_MID, MAX_MOID, MAX_CAP and MAX_ACQ define the higher bound of retrieved
records. Note that the range is right-exclusive; the records retrieved have
the key equal or higher than the MIN_* values and lower (but not equal) than
the MAX_* values.

MIN_CID and MAX_CID are integers between 0 and INT32_MAX (2^31-1).
MIN_MID and MAX_MID are integers between INT64_MIN (-2^63) and INT64_MAX
(2^63-1).
MIN_MOID and MAX_MOID are integers between INT32_MIN (-2^31) and INT32_MAX
(2^31-1).
MIN_CAP, MAX_CAP, MIN_ACQ and MAX_ACQ are TStorage timestamps, i.e. integers
between INT64_MIN (-2^63) and INT64_MAX (2^63-1) that indicate number of
nanoseconds since 1 January 2001 (ignoring leap seconds).

The program prints the retrieved records on the standard output in a CSV format
(see OUTPUT FORMAT), and then closes the TStorage connection and exits.

RETURN VALUE
------------
The program exits with code 0 on success. If any error occurs, the program
will display an error message on standard error and exit with code 1.

INPUT FORMAT
------------
The input file CSV_PATH is a simplified CSV file (without cell quoting) with 5
columns in each row. There is no header row. Each row represents one record.
The columns are:
CID,MID,MOID,CAP,VAL

where:
1. CID is an integer between 0 and INT32_MAX-1 (2^31-2).
2. MID is an integer between INT64_MIN (-2^63) and INT64_MAX-1 (2^63-2).
3. MOID is an integer between INT32_MIN (-2^31) and INT32_MAX-1 (2^31-2).
4. CAP is a TStorage timestamp, i.e. an integer between INT64_MIN (-2^63) and
   INT64_MAX-1 (2^63-2) that indicates number of nanoseconds since 1 January
   2001 (ignoring leap seconds).
5. VAL is a string of 128 hexadecimal digits, represeting the record's payload
   of 64 bytes.

OUTPUT FORMAT
-------------
The format of the program's standard output is similar to the input file's,
with one additional column ACQ:
CID,MID,MOID,CAP,ACQ,VAL

ACQ is a TStorage timestamp, i.e. an integer between INT64_MIN (-2^63) and
INT64_MAX-1 (2^63-2) that indicates number of nanoseconds since 1 January
2001 (ignoring leap seconds).

INSTALLATION
------------

See INSTALL.
