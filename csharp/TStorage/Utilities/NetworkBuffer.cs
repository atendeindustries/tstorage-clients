/*
 * Copyright 2025 Atende Industries
 */

using TStorage.Interfaces;

namespace TStorage.Utilities
{
    /// <summary>
    /// A network buffer that manges a communication with Socket.
    /// </summary>
    public class NetworkBuffer : IDisposable
    {
        /// <summary>
        /// Initializes a new instance of the NetworkBuffer class with the specified stream and buffer size.
        /// </summary>
        /// <param name="stream"> The underlying stream. </param>
        /// <param name="bufferSize"> Maximum size of the buffered stream. </param>
        /// <exception cref="ArgumentNullException"></exception>
        /// <exception cref="ArgumentOutOfRangeException"></exception>
        public NetworkBuffer(Stream stream, int bufferSize)
        {
            _bufferedStream = new BufferedStream(stream, bufferSize);
            _binaryReader = new BinaryReader(_bufferedStream, System.Text.Encoding.UTF8, leaveOpen: true);
            _binaryWriter = new BinaryWriter(_bufferedStream, System.Text.Encoding.UTF8, leaveOpen: true);
        }

        /// <summary> Sends data to the stream. Flushes buffered stream. </summary>
        public void Send()
        {
            _bufferedStream.Flush();
        }

        /// <summary> Resets streams. </summary>
        public void Reset()
        {
            _bufferedStream.Position = 0;
            _bufferedStream.SetLength(0);
        }

        /// <summary> Read data from the stream. </summary>
        /// <param name="buffer"> The buffer that will receive the read value. </param>
        /// <param name="count"> The buffer that will receive the read value. </param>
        /// <returns> A number of read bytes. </returns>
        /// <exception cref="EndOfStreamException"></exception>
        /// <exception cref="ObjectDisposedException"></exception>
        /// <exception cref="IOException"></exception>
        public int Read(out byte[] buffer, int count)
        {
            buffer = _binaryReader.ReadBytes(count);
            if (buffer.Length != count)
            {
                throw new EndOfStreamException();
            }
            return buffer.Length;
        }

        /// <summary> Read Key from the stream. </summary>
        /// <param name="key">The variable that will receive the read value.</param>
        /// <returns> A number of read bytes. </returns>
        /// <exception cref="EndOfStreamException"></exception>
        /// <exception cref="ObjectDisposedException"></exception>
        /// <exception cref="IOException"></exception>
        public int Read(out Key key)
        {
            Read(out int cid);
            Read(out long mid);
            Read(out int moid);
            Read(out long cap);
            Read(out long acq);
            key = new(cid, mid, moid, cap, acq);
            return Key.StructSize();
        }

        /// <summary> Read ResponseHeaderAcq from the stream. </summary>
        /// <param name="responseHeaderAcq">The variable that will receive the read value.</param>
        /// <returns> A number of read bytes. </returns>
        /// <exception cref="EndOfStreamException"></exception>
        /// <exception cref="ObjectDisposedException"></exception>
        /// <exception cref="IOException"></exception>
        public int Read(out ResponseHeaderAcq responseHeaderAcq)
        {
            Read(out int responseStatus);
            Read(out ulong size);
            long acq = -1;
            if ((ResponseStatus)responseStatus == ResponseStatus.OK)
            {
                Read(out acq);
            }

            responseHeaderAcq = new((ResponseStatus)responseStatus, size, acq);
            return ResponseHeaderAcq.StructSize();
        }

        /// <summary> Read ResponseHeader from the stream. </summary>
        /// <param name="responseHeader">The variable that will receive the read value.</param>
        /// <returns> A number of read bytes. </returns>
        /// <exception cref="EndOfStreamException"></exception>
        /// <exception cref="ObjectDisposedException"></exception>
        /// <exception cref="IOException"></exception>
        public int Read(out ResponseHeader responseHeader)
        {
            Read(out int responseStatus);
            Read(out ulong size);
            responseHeader = new((ResponseStatus)responseStatus, size);
            return ResponseHeader.StructSize();
        }

        /// <summary> Read int from the stream. </summary>
        /// <param name="value">The variable that will receive the read value.</param>
        /// <returns> A number of read bytes. </returns>
        /// <exception cref="EndOfStreamException"></exception>
        /// <exception cref="ObjectDisposedException"></exception>
        /// <exception cref="IOException"></exception>
        public int Read(out int value)
        {
            value = _binaryReader.ReadInt32();
            return sizeof(int);
        }

        /// <summary> Read long from the stream. </summary>
        /// <param name="value">The variable that will receive the read value.</param>
        /// <returns> A number of read bytes. </returns>
        /// <exception cref="EndOfStreamException"></exception>
        /// <exception cref="ObjectDisposedException"></exception>
        /// <exception cref="IOException"></exception>
        public int Read(out long value)
        {
            value = _binaryReader.ReadInt64();
            return sizeof(long);
        }

        /// <summary> Read ulong from the stream. </summary>
        /// <param name="value">The variable that will receive the read value.</param>
        /// <returns> A number of read bytes. </returns>
        /// <exception cref="EndOfStreamException"></exception>
        /// <exception cref="ObjectDisposedException"></exception>
        /// <exception cref="IOException"></exception>
        public int Read(out ulong value)
        {
            value = _binaryReader.ReadUInt64();
            return sizeof(ulong);
        }

        /// <summary> Write Key to the buffered stream. </summary>
        /// <param name="key">The value to be written.</param>
        /// <param name="noCid">Decides whether Cid should be written.</param>
        /// <param name="noAcq">Decides whether Acq should be written.</param>
        public void Write(Key key, bool noCid = false, bool noAcq = false)
        {
            if (!noCid)
            {
                Write(key.Cid);
            }
            Write(key.Mid);
            Write(key.Moid);
            Write(key.Cap);
            if (!noAcq)
            {
                Write(key.Acq);
            }
        }

        /// <summary> Write RequestHeader to the buffered stream. </summary>
        /// <param name="requestHeader">The value to be written.</param>
        public void Write(RequestHeader requestHeader)
        {
            Write((int)requestHeader.Cmd);
            Write(requestHeader.Size);
        }

        /// <summary> Write int to the buffered stream. </summary>
        /// <param name="value">The value to be written.</param>
        public void Write(int value)
        {
            _binaryWriter.Write(value);
        }

        /// <summary> Write long to the buffered stream. </summary>
        /// <param name="value">The value to be written.</param>
        public void Write(long value)
        {
            _binaryWriter.Write(value);
        }

        /// <summary> Write ulong to the buffered stream. </summary>
        /// <param name="value">The value to be written.</param>
        public void Write(ulong value)
        {
            _binaryWriter.Write(value);
        }

        /// <summary> Write buffer to the buffered stream. </summary>
        /// <param name="buffer">The buffer to be written.</param>
        public void Write(byte[] buffer)
        {
            _bufferedStream.Write(buffer);
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
                    _binaryReader.Dispose();
                    _bufferedStream.Dispose();
                }
            }
        }
        private bool _isDisposed = false;

        private readonly BufferedStream _bufferedStream;
        private readonly BinaryReader _binaryReader;
        private readonly BinaryWriter _binaryWriter;
    }
}