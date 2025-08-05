/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Interfaces
{
    /// <summary>
    /// Represents the result of the GetAcq operation, including acquisition timestamp.
    /// </summary>
    public class ResponseAcq : Response
    {
        /// <summary>
        /// Initializes a new instance of the ResponseAcq.
        /// </summary>
        /// <param name="status"> Result of the response status. </param>
        /// <param name="acq"> Result of the Acq value. </param>
        public ResponseAcq(ResponseStatus status, long acq) : base(status)
        {
            Acq = acq;
        }

        public ResponseAcq() : this(ResponseStatus.OK, -1) { }

        /// <summary> Acquisition timestamp returned by the operation. </summary>
        public readonly long Acq;
    }
}