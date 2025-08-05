/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Interfaces
{
    public enum Command : int
    {
        DEFAULT = 0,
        GET = 1,
        PUTSAFE = 5,
        PUTASAFE = 6,
        GETACQ = 7,
    };

    /// <summary>
    /// Request struct for TStorage communication.
    /// </summary>
    public readonly struct RequestHeader
    {
        /// <summary>
        /// Initializes a new instance of the RequestHeader.
        /// </summary>
        /// <param name="cmd"> Requested command type. </param>
        /// <param name="size"> Size of header'a additional data. </param>
        public RequestHeader(Command cmd, ulong size)
        {
            Cmd = cmd;
            Size = size;
        }

        /// <summary>
        /// Requested command type
        /// </summary>
        public readonly Command Cmd;

        /// <summary>
        /// Size of header'a additional data
        /// </summary>
        public readonly ulong Size;

        /// <summary>
        /// Returns the total RequestHeader size in bytes.
        /// </summary>
        public static int StructSize()
        {
            return CMD_BYTES + SIZE_BYTES;
        }

        /// <summary>
        /// Returns cmd size in bytes.
        /// </summary>
        public const int CMD_BYTES = sizeof(int);

        /// <summary>
        /// Returns Size size in bytes.
        /// </summary>
        public const int SIZE_BYTES = sizeof(ulong);
    }


    /// <summary>
    /// Response struct for TStorage communication.
    /// </summary>
    public readonly struct ResponseHeader
    {
        /// <summary>
        /// Initializes a new instance of the ResponseHeader.
        /// </summary>
        /// <param name="result"> Result of the response. </param>
        /// <param name="size"> Size of header'a additional data. </param>
        public ResponseHeader(ResponseStatus result, ulong size)
        {
            Result = result;
            Size = size;
        }

        /// <summary>
        /// Result of the response
        /// </summary>
        public readonly ResponseStatus Result;

        /// <summary>
        /// Size of header'a additional data
        /// </summary>
        public readonly ulong Size;

        /// <summary>
        /// Returns the total ResponseHeader size in bytes.
        /// </summary>
        public static int StructSize()
        {
            return RESULT_BYTES + SIZE_BYTES;
        }

        /// <summary>
        /// Returns Result size in bytes.
        /// </summary>
        public const int RESULT_BYTES = sizeof(int);

        /// <summary>
        /// Returns Size size in bytes.
        /// </summary>
        public const int SIZE_BYTES = sizeof(ulong);
    }

    /// <summary>
    /// Response with acq struct for TStorage communication.
    /// </summary>
    public readonly struct ResponseHeaderAcq
    {
        /// <summary>
        /// Initializes a new instance of the ResponseHeaderAcq.
        /// </summary>
        /// <param name="result"> Result of the response. </param>
        /// <param name="size"> Size of header'a additional data. </param>
        /// <param name="acq"> Result Acq. </param>
        public ResponseHeaderAcq(ResponseStatus result, ulong size, long acq)
        {
            Result = result;
            Size = size;
            Acq = acq;
        }

        /// <summary> Result of the response </summary>
        public readonly ResponseStatus Result;

        /// <summary> Size of header'a additional data </summary>
        public readonly ulong Size;

        /// <summary> Result Acq </summary>
        public readonly long Acq;

        /// <summary> Returns the total ResponseHeaderAcq size in bytes. </summary>
        public static int StructSize()
        {
            return RESULT_BYTES + SIZE_BYTES + ACQ_BYTES;
        }

        /// <summary> Returns Result size in bytes. </summary>
        public const int RESULT_BYTES = sizeof(int);

        /// <summary> Returns Size size in bytes. </summary>
        public const int SIZE_BYTES = sizeof(ulong);

        /// <summary> Returns Acq size in bytes. </summary>
        public const int ACQ_BYTES = sizeof(long);
    }
}