/*
 * Copyright 2025 Atende Industries
 */

using TStorage.Interfaces;

namespace TStorage.Utilities
{
    /// <summary>
    /// The class that is responsible for the functionality of the GetAcq command.
    /// </summary>
    public class GetAcqClient
    {
        /// <summary>
        /// Initializes a new instance of the GetAcqClient.
        /// </summary>
        /// <param name="networkBuffer"> The NetworkBuffer through which the data will be exchanged. </param>
        public GetAcqClient(NetworkBuffer networkBuffer)
        {
            _networkBuffer = networkBuffer;
        }

        /// <summary> Sends request through a socket. </summary>
        /// <param name="keyMin"> The minimum key. </param>
        /// <param name="keyMax"> The maximum key. </param>
        public void SendRequest(Key keyMin, Key keyMax)
        {
            _networkBuffer.Write(_requestHeader);
            _networkBuffer.Write(keyMin);
            _networkBuffer.Write(keyMax);
            _networkBuffer.Send();
        }

        /// <summary>
        /// Actual GetAcq functionality.
        /// Reads result Acq data through the NetworkBuffer.
        /// </summary>
        /// <returns> A ResponseAcq containing result or error. </returns>
        public ResponseAcq GetAcq()
        {
            ResponseHeaderAcq responseHeaderAcq;
            try
            {
                _networkBuffer.Read(out responseHeaderAcq);
            }
            catch (EndOfStreamException)
            {
                return new(ResponseStatus.TSCLIENT_ENDOFSTREAM, default);
            }
            catch (IOException)
            {
                return new(ResponseStatus.TSCLIENT_IOERROR, default);
            }

            return new(responseHeaderAcq.Result, responseHeaderAcq.Acq);
        }

        private readonly NetworkBuffer _networkBuffer;
        private static readonly RequestHeader _requestHeader = new(Command.GETACQ, (ulong)Key.StructSize() * 2);
    }
}