/*
 * Copyright 2025 Atende Industries
 */

using TStorage.Interfaces;

namespace TStorage.Utilities
{
    /// <summary>
    /// The class that is responsible for the functionality of the Get command.
    /// </summary>
    /// <typeparam name="T"> The type of values stored in the returned records. </typeparam>
    public class GetClient<T>
    {
        /// <summary>
        /// Initializes a new instance of the GetClient.
        /// </summary>
        /// <param name="networkBuffer"> The NetworkBuffer through which the data will be exchanged. </param>
        /// <param name="payloadType"> The payload type converter. </param>
        /// <param name="memoryLimit"> The byte limit of the TStorage response. </param>
        public GetClient(NetworkBuffer networkBuffer, IPayloadType<T> payloadType, int memoryLimit)
        {
            _networkBuffer = networkBuffer;
            _memoryLimit = memoryLimit;
            _payloadType = payloadType;
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

        /// <summary> Actual Get functionality. </summary>
        /// <returns> A ResponseGet containing the result or an error. </returns>
        public ResponseGet<T> GetRecords()
        {
            RecordsSet<T> resultRecords = new();
            ResponseHeaderAcq responseHeaderAcq;
            try
            {
                int numberOfBytesRead = 0;
                numberOfBytesRead += _networkBuffer.Read(out ResponseHeader responseHeader);
                if (responseHeader.Result != ResponseStatus.OK)
                {
                    return new(responseHeader.Result, default, new());
                }

                while (true)
                {
                    numberOfBytesRead += _networkBuffer.Read(out int recordSize);
                    if (recordSize == END_OF_DATA_MARK)
                    {
                        _networkBuffer.Read(out responseHeaderAcq);
                        break;
                    }
                    if (recordSize < Key.StructSize())
                    {
                        return new(ResponseStatus.TSCLIENT_BADRESPONSE, default, resultRecords);
                    }
                    if (recordSize > (_memoryLimit - numberOfBytesRead - Key.StructSize()))
                    {
                        return new(ResponseStatus.TSCLIENT_OUTOFMEMORY, default, resultRecords);
                    }

                    numberOfBytesRead += _networkBuffer.Read(out Key key);
                    int payloadSize = recordSize - Key.StructSize();
                    numberOfBytesRead += _networkBuffer.Read(out byte[] payloadBytes, payloadSize);
                    T? payloadT = _payloadType.FromBytes(payloadBytes);
                    if (payloadT == null)
                    {
                        return new(ResponseStatus.TSCLIENT_SERIALIZATIONERROR, default, resultRecords);
                    }
                    resultRecords.Append(new(key, payloadT));
                }
            }
            catch (EndOfStreamException)
            {
                return new(ResponseStatus.TSCLIENT_ENDOFSTREAM, default, resultRecords);
            }
            catch (IOException)
            {
                return new(ResponseStatus.TSCLIENT_IOERROR, default, resultRecords);
            }

            return new(responseHeaderAcq.Result, responseHeaderAcq.Acq, resultRecords);
        }

        /// <summary> Actual GetStream functionality. </summary>
        /// <param name="callback"> User-defined function called with each received chunk. </param>
        /// <returns> A ResponseAcq containing the result or an error. </returns>
        public ResponseAcq GetRecordsStream(GetCallback<T> callback)
        {
            RecordsSet<T> records = new();
            ResponseHeaderAcq responseHeaderAcq;
            try
            {
                int numberOfBytesRead = 0;
                numberOfBytesRead += _networkBuffer.Read(out ResponseHeader responseHeader);
                if (responseHeader.Result != ResponseStatus.OK)
                {
                    return new(responseHeader.Result, default);
                }

                while (true)
                {
                    numberOfBytesRead += _networkBuffer.Read(out int recordSize);
                    if (recordSize >= _memoryLimit - numberOfBytesRead)
                    {
                        callback(records);
                        records.Clear();
                        numberOfBytesRead = 0;
                    }

                    if (recordSize == END_OF_DATA_MARK)
                    {
                        _networkBuffer.Read(out responseHeaderAcq);
                        callback(records);
                        break;
                    }

                    if (recordSize < Key.StructSize())
                    {
                        callback(records);
                        return new(ResponseStatus.TSCLIENT_BADRESPONSE, default);
                    }

                    numberOfBytesRead += _networkBuffer.Read(out Key key);
                    int payloadSize = recordSize - Key.StructSize();
                    numberOfBytesRead += _networkBuffer.Read(out byte[] payloadBytes, payloadSize);
                    T? payloadT = _payloadType.FromBytes(payloadBytes);
                    if (payloadT == null)
                    {
                        callback(records);
                        return new(ResponseStatus.TSCLIENT_SERIALIZATIONERROR, default);
                    }
                    records.Append(new(key, payloadT));
                }
            }
            catch (EndOfStreamException)
            {
                callback(records);
                return new(ResponseStatus.TSCLIENT_ENDOFSTREAM, default);
            }
            catch (IOException)
            {
                callback(records);
                return new(ResponseStatus.TSCLIENT_IOERROR, default);
            }
            catch
            {
                callback(records);
                throw;
            }

            return new(responseHeaderAcq.Result, responseHeaderAcq.Acq);
        }

        private readonly NetworkBuffer _networkBuffer;
        private readonly int _memoryLimit;
        private readonly IPayloadType<T> _payloadType;
        private static readonly RequestHeader _requestHeader = new(Command.GET, (ulong)Key.StructSize() * 2);
        private const int END_OF_DATA_MARK = 0;
    }
}