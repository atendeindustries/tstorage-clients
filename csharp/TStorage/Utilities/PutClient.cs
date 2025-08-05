/*
 * Copyright 2025 Atende Industries
 */

using TStorage.Interfaces;

namespace TStorage.Utilities
{
    using Cid = int;

    /// <summary>
    /// The class that is responsible for the functionality of the Safe Put/Puta command.
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public class PutClient<T>
    {
        private class BatchData
        {
            public RecordsSet<byte[]> Records = new();
            public int BatchSize = 0;
            public bool IsFull = false;
        }

        /// <summary>
        /// Initializes a new instance of the PutClient.
        /// </summary>
        /// <param name="networkBuffer"> The NetworkBuffer through which the data will be exchanged. </param>
        /// <param name="payloadType"> The payload type converter. </param>
        /// <param name="withAcq"> Indicates whether Acq should be the part of the Key. </param>
        public PutClient(NetworkBuffer networkBuffer, IPayloadType<T> payloadType, bool withAcq)
        {
            _networkBuffer = networkBuffer;
            _payloadType = payloadType;
            _withAcq = withAcq;
            _requestHeader = withAcq ? new(Command.PUTASAFE, 0) : new(Command.PUTSAFE, 0);
        }

        /// <summary> Loads and preprocesses data. </summary>
        /// <param name="data"> Input data. </param>
        public void LoadData(RecordsSet<T> data)
        {
            foreach (Record<T> record in data)
            {
                if (!_groupedData.TryGetValue(record.Key.Cid, out List<BatchData>? listOfBatches))
                {
                    listOfBatches = [];
                    _groupedData[record.Key.Cid] = listOfBatches;
                }

                BatchData? currentBatch = listOfBatches.Count > 0 ? listOfBatches[^1] : null;
                if (currentBatch == null || currentBatch.IsFull)
                {
                    currentBatch = new BatchData();
                    listOfBatches.Add(currentBatch);
                }

                byte[] payloadBytes = _payloadType.ToBytes(record.Value);
                int recsSizeBytesSize = sizeof(int);
                try
                {
                    checked
                    {
                        currentBatch.BatchSize += recsSizeBytesSize + Key.StructSize() - Key.CID_BYTES + payloadBytes.Length - (_withAcq ? 0 : Key.ACQ_BYTES);
                    }
                }
                catch (OverflowException)
                {
                    currentBatch.IsFull = true;
                    currentBatch = new BatchData();
                    currentBatch.BatchSize += recsSizeBytesSize + Key.StructSize() - Key.CID_BYTES + payloadBytes.Length - (_withAcq ? 0 : Key.ACQ_BYTES);
                    listOfBatches.Add(currentBatch);
                }

                currentBatch.Records.Append(new(record.Key, payloadBytes));
            }
        }

        /// <summary> Sends request through a socket. </summary>
        public void SendRequest()
        {
            _networkBuffer.Write(_requestHeader);
            _networkBuffer.Send();
        }

        /// <summary> Actual Put/Puta functionality. </summary>
        /// <returns> A Response containing the result or an error. </returns>
        public Response SendRecords()
        {
            foreach (KeyValuePair<Cid, List<BatchData>> data in _groupedData)
            {
                Cid cid = data.Key;
                List<BatchData> listOfBatches = data.Value;
                foreach (BatchData batchData in listOfBatches)
                {
                    WriteBatchInfo(cid: cid, batchSize: batchData.BatchSize);
                    foreach (Record<byte[]> record in batchData.Records)
                    {
                        WriteRecord(record);
                    }
                }
            }
            _networkBuffer.Write(END_OF_DATA_MARK);
            _networkBuffer.Send();

            return ReadConfirmation();
        }

        private void WriteBatchInfo(Cid cid, int batchSize)
        {
            _networkBuffer.Write(cid);
            _networkBuffer.Write(batchSize);
        }

        private void WriteRecord(Record<byte[]> record)
        {
            byte[] recordPayload = record.Value;
            int recSize = Key.StructSize() - Key.CID_BYTES + recordPayload.Length;
            recSize -= _withAcq ? 0 : Key.ACQ_BYTES;
            _networkBuffer.Write(recSize);
            _networkBuffer.Write(record.Key, noCid: true, noAcq: !_withAcq);
            _networkBuffer.Write(recordPayload);
        }

        private Response ReadConfirmation()
        {
            try
            {
                _networkBuffer.Read(out ResponseHeader responseHeader);
                if (responseHeader.Result != ResponseStatus.OK)
                {
                    return new(responseHeader.Result);
                }

                _networkBuffer.Read(out long acqMin);
                _networkBuffer.Read(out long acqMax);
            }
            catch (EndOfStreamException)
            {
                return new(ResponseStatus.TSCLIENT_ENDOFSTREAM);
            }
            catch (IOException)
            {
                return new(ResponseStatus.TSCLIENT_IOERROR);
            }

            return new(ResponseStatus.OK);
        }

        private readonly NetworkBuffer _networkBuffer;
        private readonly IPayloadType<T> _payloadType;
        private readonly bool _withAcq;
        private readonly RequestHeader _requestHeader;
        private const int END_OF_DATA_MARK = -1;
        private Dictionary<Cid, List<BatchData>> _groupedData = [];
    }
}