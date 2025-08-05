/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Interfaces
{

    /// <summary>
    /// Represents the result of the Get operation, including acquired records.
    /// </summary>
    /// <typeparam name="T"> The type of values stored in the returned records. </typeparam>
    public class ResponseGet<T> : ResponseAcq
    {
        /// <summary>
        /// Initializes a new instance of the ResponseGet.
        /// </summary>
        /// <param name="status"> Result of the response status. </param>
        /// <param name="acq"> Result of the Acq value. </param>
        /// <param name="records"> Result of the records. </param>
        public ResponseGet(ResponseStatus status, long acq, RecordsSet<T> records) : base(status, acq)
        {
            Data = records;
        }

        /// <summary> The data retrieved by the GET operation. </summary>
        public readonly RecordsSet<T> Data;
    }
}