/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Interfaces
{
    /// <summary>
    /// Represents the status of an operation, indicating success or various types of failure.
    /// </summary>
    public enum ResponseStatus : int
    {
        OK = 0,

        // TSClient error code, greater than int8_t
        TSCLIENT_ERROR = 128,
        TSCLIENT_ENDOFSTREAM = 129,
        TSCLIENT_IOERROR = 130,
        TSCLIENT_OUTOFMEMORY = 131,
        TSCLIENT_ARGOUTOFRANGE = 132,
        TSCLIENT_BADRESPONSE = 133,
        TSCLIENT_CONNERROR = 134,
        TSCLIENT_SERIALIZATIONERROR = 135,

        TSTORAGE_ERROR = -1,
        TSTORAGE_INVARG = -2,
        TSTORAGE_RETRY = -3,
        TSTORAGE_TIMEOUT = -4,
        TSTORAGE_NOMEM = -5,
        TSTORAGE_IOERR = -6,
        TSTORAGE_NOPERM = -7,
        TSTORAGE_NOIMPL = -8,
        TSTORAGE_ABORT = -9,

        TSTORAGE_UNAUTHORIZED = -11,
        TSTORAGE_INACTIVE = -12,

        TSTORAGE_CONTINUE = -16,

        TSTORAGE_INTRERROR = -126,
        TSTORAGE_CONNRESET = -127,
        TSTORAGE_ADDRERROR = -128,
        TSTORAGE_CONNERROR = -129,
        TSTORAGE_BINDERROR = -130,
        TSTORAGE_SOCKERROR = -131,

        TSTORAGE_INVPATH = -132,
        TSTORAGE_EXIST = -133,
        TSTORAGE_NOENT = -134,
        TSTORAGE_NOTDIR = -135,
        TSTORAGE_BUSY = -136,
        TSTORAGE_NOTEMPTY = -137,
        TSTORAGE_NOTOPENED = -138,
        TSTORAGE_ISDIR = -139,
        TSTORAGE_OPENED = -140,
        TSTORAGE_CLOSED = -141,

        TSTORAGE_NOTSTARTED = -256,
        TSTORAGE_RUNNING = -257,
        TSTORAGE_ABORTED = -258,
        TSTORAGE_REDIR = -259
    }
}