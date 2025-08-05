/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Interfaces
{
    /// <summary>
    /// Represents the result of an operation, encapsulating a status.
    /// Used as a base response type for various operations, indicating success, failure,
    /// or other outcome as represented by <see cref="ResponseStatus"/>
    /// </summary>
    public class Response
    {
        /// <summary>
        /// Initializes a new instance of the Response.
        /// </summary>
        /// <param name="status"> Result of the response status. </param>
        public Response(ResponseStatus status)
        {
            Status = status;
        }

        /// <summary> The status of the response. </summary>
        public readonly ResponseStatus Status;
    }
}