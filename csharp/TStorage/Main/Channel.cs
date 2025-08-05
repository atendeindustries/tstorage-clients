/*
 * Copyright 2025 Atende Industries
 */

using System.Net.Sockets;
using TStorage.Interfaces;
using TStorage.Utilities;

namespace TStorage.Main
{
    /// <summary>
    /// A generic channel for sending and receiving data over a network connection to TStorage.
    /// This class manages the lifecycle of a network connection and supports operations
    /// like `Get`, `Put`, `Puta`, `GetAcq`, and streaming data with `GetStream`.
    /// This class supports only little-endian byte order.
    /// </summary>
    /// <typeparam name="T"> The record's payload type. </typeparam>
    public class Channel<T> : IChannel<T>, IDisposable
    {
        /// <summary> Constructs a Channel with connection parameters. </summary>
        /// <param name="host"> The remote host. </param>
        /// <param name="port"> The remote port. </param>
        /// <param name="payloadType"> The payload type converter. </param>
        /// <exception cref="ArgumentNullException"> When payloadType is null. </exception>
        public Channel(string host, int port, IPayloadType<T> payloadType)
        {
            ArgumentNullException.ThrowIfNull(payloadType, nameof(payloadType));

            _host = host;
            _port = port;
            _payloadType = payloadType;
        }

        /// <summary> Opens a connection with TStorage. </summary>
        /// <returns> A Response indicating success or failure. </returns>
        public Response Connect()
        {
            if (Connected)
            {
                return new Response(ResponseStatus.TSCLIENT_ERROR);
            }

            _socket ??= CreateSocket();

            try
            {
                _socket.Connect(_host, _port);
            }
            catch (SocketException)
            {
                return new(ResponseStatus.TSCLIENT_CONNERROR);
            }

            _networkBuffer = new NetworkBuffer(new NetworkStream(_socket, ownsSocket: false), MemoryLimit);

            return new Response(ResponseStatus.OK);
        }

        /// <summary> Closes a connection with TStorage. </summary>
        /// <returns> A Response indicating success or failure. </returns>
        public Response Close()
        {
            Dispose();
            return new(ResponseStatus.OK);
        }

        /// <summary> Retrieves records within a key range [min,max). </summary>
        /// <param name="keyMin"> The minimum key. </param>
        /// <param name="keyMax"> The maximum key. </param>
        /// <returns>
        /// A ResponseGet indicating success ResponseStatus.OK. In case of error, the last successfully fetched data will be attached.
        /// Other possible statuses:
        /// ResponseStatus.TSCLIENT_ENDOFSTREAM - when not available data yet is read.
        /// ResponseStatus.TSCLIENT_IOERROR - when I/O error occurs.
        /// ResponseStatus.TSCLIENT_OUTOFMEMORY - when MemoryLimit will be reached.
        /// ResponseStatus.TSCLIENT_SERIALIZATIONERROR - when received data cannot be serialized.
        /// ResponseStatus.TSTORAGE_ - when TStorage returns an error.
        /// </returns>
        public ResponseGet<T> Get(Key keyMin, Key keyMax)
        {
            if (!Connected || _networkBuffer is null)
            {
                return new(ResponseStatus.TSCLIENT_ERROR, default, new());
            }

            GetClient<T> getClient = new(_networkBuffer, _payloadType, MemoryLimit);
            getClient.SendRequest(keyMin, keyMax);
            return getClient.GetRecords();
        }

        /// <summary> Sends records to TStorage using safe PUT. </summary>
        /// <param name="data"> The records to be sent. </param>
        /// <returns> A Response indicating success ResponseStatus.OK.
        /// In case of error:
        /// ResponseStatus.TSCLIENT_ENDOFSTREAM - when not available data yet is read.
        /// ResponseStatus.TSCLIENT_IOERROR - when I/O error occurs.
        /// ResponseStatus.TSTORAGE_ - when TStorage returns an error.
        /// </returns>
        public Response Put(RecordsSet<T> data)
        {
            if (!Connected || _networkBuffer is null)
            {
                return new(ResponseStatus.TSCLIENT_ERROR);
            }

            PutClient<T> putClient = new(_networkBuffer, _payloadType, withAcq: false);
            putClient.LoadData(data);
            putClient.SendRequest();
            return putClient.SendRecords();
        }

        /// <summary> Sends records to TStorage using safe PUTA. </summary>
        /// <param name="data"> The records to be sent. </param>
        /// <returns> A Response indicating success ResponseStatus.OK.
        /// In case of error:
        /// ResponseStatus.TSCLIENT_ENDOFSTREAM - when not available data yet is read.
        /// ResponseStatus.TSCLIENT_IOERROR - when I/O error occurs.
        /// ResponseStatus.TSTORAGE_ - when TStorage returns an error.
        /// </returns>
        public Response Puta(RecordsSet<T> data)
        {
            if (!Connected || _networkBuffer is null)
            {
                return new(ResponseStatus.TSCLIENT_ERROR);
            }

            PutClient<T> putClient = new(_networkBuffer, _payloadType, withAcq: true);
            putClient.LoadData(data);
            putClient.SendRequest();
            return putClient.SendRecords();
        }

        /// <summary> Gets acquisition value within a key range. </summary>
        /// <param name="keyMin"> The minimum key. </param>
        /// <param name="keyMax"> The maximum key. </param>
        /// <returns> A ResponseAcq indicating success ResponseStatus.OK.
        /// In case of error:
        /// ResponseStatus.TSCLIENT_ENDOFSTREAM - when not available data yet is read.
        /// ResponseStatus.TSCLIENT_IOERROR - when I/O error occurs.
        /// ResponseStatus.TSTORAGE_ - when TStorage returns an error.
        /// </returns>
        public ResponseAcq GetAcq(Key keyMin, Key keyMax)
        {
            if (!Connected || _networkBuffer is null)
            {
                return new(ResponseStatus.TSCLIENT_ERROR, 0);
            }

            GetAcqClient getAcqClient = new(_networkBuffer);
            getAcqClient.SendRequest(keyMin, keyMax);

            return getAcqClient.GetAcq();
        }

        /// <summary> Retrieves records within a key range. </summary>
        /// <param name="keyMin"> The minimum key. </param>
        /// <param name="keyMax"> The maximum key. </param>
        /// <param name="callback"> User-defined function called with each received chunk. </param>
        /// <returns>
        /// A ResponseAcq indicating success ResponseStatus.OK. In case of error, last callback with correctly fetched data will be called.
        /// Other possible statuses:
        /// ResponseStatus.TSCLIENT_ENDOFSTREAM - when not available data yet is read.
        /// ResponseStatus.TSCLIENT_IOERROR - when I/O error occurs.
        /// ResponseStatus.TSCLIENT_SERIALIZATIONERROR - when received data cannot be serialized.
        /// ResponseStatus.TSCLIENT_BADRESPONSE - when received data is not as expected.
        /// ResponseStatus.TSTORAGE_ - when TStorage returns an error.
        /// </returns>
        public ResponseAcq GetStream(Key keyMin, Key keyMax, GetCallback<T> callback)
        {
            if (!Connected || _networkBuffer is null)
            {
                return new(ResponseStatus.TSCLIENT_ERROR, 0);
            }

            GetClient<T> getClient = new(_networkBuffer, _payloadType, MemoryLimit);
            getClient.SendRequest(keyMin, keyMax);

            return getClient.GetRecordsStream(callback);
        }

        private Socket CreateSocket()
        {
            return new(SocketType.Stream, ProtocolType.Tcp)
            {
                LingerState = new LingerOption(true, 10),
                Blocking = true,
                ReceiveBufferSize = 128 * 1024,
                SendBufferSize = 128 * 1024,
                NoDelay = false,
                ReceiveTimeout = NetTimeout
            };
        }

        /// <summary> Connection timeout in milliseconds. </summary>
        /// <remarks>
        /// Cannot be modified when Channel is Connected.
        /// </remarks>
        public int NetTimeout
        {
            get => _netTimeout;
            set
            {
                if (Connected)
                {
                    throw new InvalidOperationException("Cannot modify NetTimeout when Channel is Connected.");
                }
                _netTimeout = value;
            }
        }
        private int _netTimeout = 30000;

        /// <summary> The byte limit of the TStorage response. </summary>
        /// <remarks>
        /// Cannot be modified when Channel is Connected.
        /// </remarks>
        public int MemoryLimit
        {
            get => _memoryLimit;
            set
            {
                if (Connected)
                {
                    throw new InvalidOperationException("Cannot modify MemoryLimit when Channel is Connected.");
                }
                _memoryLimit = value;
            }
        }
        private int _memoryLimit = 67108864;

        /// <summary> Connection state. </summary>
        public bool Connected
        {
            get { return _socket is not null && _socket.Connected; }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_isDisposed)
            {
                _isDisposed = true;
                if (disposing)
                {
                    _networkBuffer?.Dispose();
                    _socket?.Dispose();
                    _networkBuffer = null;
                    _socket = null;
                }
            }
        }
        private bool _isDisposed = false;

        private readonly string _host;
        private readonly int _port;
        private Socket? _socket;
        private NetworkBuffer? _networkBuffer;
        private readonly IPayloadType<T> _payloadType;
    }
}