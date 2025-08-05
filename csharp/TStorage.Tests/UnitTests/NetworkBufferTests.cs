using System.Runtime.InteropServices;
using TStorage.Interfaces;
using TStorage.Utilities;

namespace TStorage.Tests.UnitTests
{
    public class NetworkBufferTests
    {
        private static void WriteResponseHeaderAcq(NetworkBuffer networkBuffer, ResponseHeaderAcq responseHeaderAcq)
        {
            networkBuffer.Write((int)responseHeaderAcq.Result);
            networkBuffer.Write(responseHeaderAcq.Size);
            networkBuffer.Write(responseHeaderAcq.Acq);
        }

        private static void WriteResponseHeader(NetworkBuffer networkBuffer, ResponseHeader responseHeader)
        {
            networkBuffer.Write((int)responseHeader.Result);
            networkBuffer.Write(responseHeader.Size);
        }

        private static void ReadRequestHeader(NetworkBuffer networkBuffer, out RequestHeader requestHeader)
        {
            networkBuffer.Read(out int command);
            networkBuffer.Read(out ulong size);
            requestHeader = new((Command)command, size);
        }

        [Fact]
        public void WriteReadOfInt_DataShouldBeWrittenAndReadCorrectly()
        {
            // Arrange
            int providedValue = 99;
            int rawBufferSize = sizeof(int);
            using var memoryStream = new MemoryStream(new byte[rawBufferSize + 1], index: 0, count: rawBufferSize + 1);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act and assert
            networkBuffer.Write(providedValue);
            networkBuffer.Send();
            Assert.Equal((int)memoryStream.Position, Marshal.SizeOf(providedValue));

            memoryStream.Position = 0;
            int numberOfBytesRead = networkBuffer.Read(out int resultValue);
            Assert.Equal(sizeof(int), numberOfBytesRead);
            Assert.Equal(providedValue, resultValue);
        }

        [Fact]
        public void WriteReadOfLong_DataShouldBeWrittenAndReadCorrectly()
        {
            // Arrange
            long providedValue = 99L;
            int rawBufferSize = sizeof(long);
            using var memoryStream = new MemoryStream(new byte[rawBufferSize + 1], index: 0, count: rawBufferSize + 1);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act and assert
            networkBuffer.Write(providedValue);
            networkBuffer.Send();
            Assert.Equal((int)memoryStream.Position, Marshal.SizeOf(providedValue));

            memoryStream.Position = 0;
            int numberOfBytesRead = networkBuffer.Read(out long resultValue);
            Assert.Equal(sizeof(long), numberOfBytesRead);
            Assert.Equal(providedValue, resultValue);
        }

        [Fact]
        public void WriteReadOfULong_DataShouldBeWrittenAndReadCorrectly()
        {
            // Arrange
            ulong providedValue = 99UL;
            int rawBufferSize = sizeof(ulong);
            using var memoryStream = new MemoryStream(new byte[rawBufferSize + 1], index: 0, count: rawBufferSize + 1);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act and assert
            networkBuffer.Write(providedValue);
            networkBuffer.Send();
            Assert.Equal((int)memoryStream.Position, Marshal.SizeOf(providedValue));

            memoryStream.Position = 0;
            int numberOfBytesRead = networkBuffer.Read(out ulong resultValue);
            Assert.Equal(sizeof(ulong), numberOfBytesRead);
            Assert.Equal(providedValue, resultValue);
        }

        [Fact]
        public void WriteReadOfKey_DataShouldBeWrittenAndReadCorrectly()
        {
            // Arrange
            Key providedValue = new(1, 2, 3, 4, 5);
            int rawBufferSize = Key.StructSize();
            using var memoryStream = new MemoryStream(new byte[rawBufferSize + 1], index: 0, count: rawBufferSize + 1);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act and assert
            networkBuffer.Write(providedValue);
            networkBuffer.Send();
            Assert.Equal((int)memoryStream.Position, Key.StructSize());

            memoryStream.Position = 0;
            int numberOfBytesRead = networkBuffer.Read(out Key resultValue);
            Assert.Equal(Key.StructSize(), numberOfBytesRead);
            Assert.Equal(providedValue, resultValue);
        }

        [Fact]
        public void WriteOfKeyNoCid_DataShouldBeWrittenCorrectly()
        {
            // Arrange
            Key providedValue = new(1, 2, 3, 4, 5);
            int rawBufferSize = Key.StructSize();
            using var memoryStream = new MemoryStream(new byte[rawBufferSize + 1], index: 0, count: rawBufferSize + 1);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act
            networkBuffer.Write(providedValue, noCid: true);
            networkBuffer.Send();

            // Assert
            Assert.Equal((int)memoryStream.Position, Key.StructSize() - Key.CID_BYTES);
        }

        [Fact]
        public void WriteOfKeyNoAcq_DataShouldBeWrittenCorrectly()
        {
            // Arrange
            Key providedValue = new(1, 2, 3, 4, 5);
            int rawBufferSize = Key.StructSize();
            using var memoryStream = new MemoryStream(new byte[rawBufferSize + 1], index: 0, count: rawBufferSize + 1);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act
            networkBuffer.Write(providedValue, noAcq: true);
            networkBuffer.Send();

            // Assert
            Assert.Equal((int)memoryStream.Position, Key.StructSize() - Key.ACQ_BYTES);
        }

        [Fact]
        public void WriteOfKeyNoAcqNoCid_DataShouldBeWrittenCorrectly()
        {
            // Arrange
            Key providedValue = new(1, 2, 3, 4, 5);
            int rawBufferSize = Key.StructSize();
            using var memoryStream = new MemoryStream(new byte[rawBufferSize + 1], index: 0, count: rawBufferSize + 1);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act
            networkBuffer.Write(providedValue, noAcq: true, noCid: true);
            networkBuffer.Send();

            // Assert
            Assert.Equal((int)memoryStream.Position, Key.StructSize() - Key.ACQ_BYTES - Key.CID_BYTES);
        }

        [Fact]
        public void WriteRequestHeader_DataShouldBeWrittenCorrectly()
        {
            // Arrange
            RequestHeader providedValue = new(Command.GET, 99);
            int rawBufferSize = RequestHeader.StructSize();
            using var memoryStream = new MemoryStream(new byte[rawBufferSize + 1], index: 0, count: rawBufferSize + 1);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act and assert
            networkBuffer.Write(providedValue);
            networkBuffer.Send();
            Assert.Equal((int)memoryStream.Position, RequestHeader.StructSize());

            memoryStream.Position = 0;
            ReadRequestHeader(networkBuffer, out RequestHeader resultValue);
            Assert.Equal(providedValue, resultValue);
        }

        [Fact]
        public void ReadResponseHeaderAcq_DataShouldBeReadCorrectly()
        {
            // Arrange
            ResponseHeaderAcq providedValue = new(ResponseStatus.OK, 99, 101);
            int rawBufferSize = ResponseHeaderAcq.StructSize();
            using var memoryStream = new MemoryStream(new byte[rawBufferSize + 1], index: 0, count: rawBufferSize + 1);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);
            WriteResponseHeaderAcq(networkBuffer, providedValue);
            networkBuffer.Send();

            // Act and assert
            Assert.Equal((int)memoryStream.Position, ResponseHeaderAcq.StructSize());

            memoryStream.Position = 0;
            int numberOfBytesRead = networkBuffer.Read(out ResponseHeaderAcq resultValue);
            Assert.Equal(ResponseHeaderAcq.StructSize(), numberOfBytesRead);
            Assert.Equal(providedValue, resultValue);
        }

        [Fact]
        public void ReadResponseHeader_DataShouldBeReadCorrectly()
        {
            // Arrange
            ResponseHeader providedValue = new(ResponseStatus.OK, 102);
            int rawBufferSize = ResponseHeader.StructSize();
            using var memoryStream = new MemoryStream(new byte[rawBufferSize + 1], index: 0, count: rawBufferSize + 1);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);
            WriteResponseHeader(networkBuffer, providedValue);
            networkBuffer.Send();

            // Act and assert
            Assert.Equal((int)memoryStream.Position, ResponseHeader.StructSize());

            memoryStream.Position = 0;
            int numberOfBytesRead = networkBuffer.Read(out ResponseHeader resultValue);
            Assert.Equal(ResponseHeader.StructSize(), numberOfBytesRead);
            Assert.Equal(providedValue, resultValue);
        }

        [Theory]
        [InlineData(0)]
        [InlineData(1)]
        [InlineData(2)]
        [InlineData(3)]
        [InlineData(4)]
        public void WriteReadBytes_ExactSizeOfDataShouldBeReadFromBuffer(int numberOfBytesToRead)
        {
            // Arrange
            byte[] providedBytes = [0x12, 0x34, 0x56, 0x78];
            using var memoryStream = new MemoryStream(new byte[providedBytes.Length + 1], index: 0, count: providedBytes.Length);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act and assert
            networkBuffer.Write(providedBytes);
            networkBuffer.Send();

            memoryStream.Position = 0;
            int numberOfBytesRead = networkBuffer.Read(out byte[] resultBytes, numberOfBytesToRead);
            Assert.Equal(numberOfBytesToRead, numberOfBytesRead);
            Assert.Equal(providedBytes.Take(numberOfBytesToRead), resultBytes);
        }

        [Fact]
        public void WriteReadBytes_ReadingMoreThanExistsInStreamShouldThrowEndOfStreamException()
        {
            // Arrange
            byte[] providedBytes = [0x12, 0x34, 0x56, 0x78];
            using var memoryStream = new MemoryStream(new byte[providedBytes.Length + 1], index: 0, count: providedBytes.Length);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act
            networkBuffer.Write(providedBytes);
            networkBuffer.Send();

            // Assert
            memoryStream.Position = 0;
            Assert.Throws<EndOfStreamException>(() => networkBuffer.Read(out byte[] resultBytes, 999));
        }

        [Fact]
        public void WriteReadBytes_ProvidingNegativeCountToReadShouldThrowArgumentOutOfRangeException()
        {
            // Arrange
            byte[] providedBytes = [0x12, 0x34, 0x56, 0x78];
            using var memoryStream = new MemoryStream(new byte[providedBytes.Length + 1], index: 0, count: providedBytes.Length);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act
            networkBuffer.Write(providedBytes);
            networkBuffer.Send();

            // Assert
            memoryStream.Position = 0;
            Assert.Throws<ArgumentOutOfRangeException>(() => networkBuffer.Read(out byte[] resultBytes, -10));
        }

        [Fact]
        public void Send_DataShouldBeAvailableInStreamOnlyAfterSend()
        {
            // Arrange
            ulong providedValue = 101UL;
            int rawBufferSize = sizeof(ulong);
            using var memoryStream = new MemoryStream(new byte[rawBufferSize + 1], index: 0, count: rawBufferSize + 1);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act and assert
            networkBuffer.Write(providedValue);
            Assert.Equal(0, (int)memoryStream.Position);

            networkBuffer.Send();
            Assert.Equal(Marshal.SizeOf(providedValue), (int)memoryStream.Position);
        }

        [Fact]
        public void Reset_DataShouldNotBeAvailableInStreamAfterReset()
        {
            // Arrange
            ulong providedValue = 101UL;
            int rawBufferSize = sizeof(ulong);
            using var memoryStream = new MemoryStream(new byte[rawBufferSize + 1], index: 0, count: rawBufferSize + 1);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act
            networkBuffer.Write(providedValue);
            networkBuffer.Reset();
            networkBuffer.Send();

            // Assert
            Assert.Equal(0, memoryStream.Length);
        }

        [Fact]
        public void Read_ReadingFromSmallerThanReadDataStreamShouldThrowEndOfStreamException()
        {
            // Arrange
            int rawBufferSize = sizeof(int);
            using var memoryStream = new MemoryStream(new byte[rawBufferSize - 1], index: 0, count: rawBufferSize - 1);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act and assert
            Assert.Throws<EndOfStreamException>(() => networkBuffer.Read(out int res));
        }

        [Fact]
        public void Read_ReadingFromDisposedNetworkBufferShouldThrowObjectDisposedException()
        {
            // Arrange
            int providedValue = 99;
            using var memoryStream = new MemoryStream(BitConverter.GetBytes(providedValue));
            using var networkBuffer = new NetworkBuffer(memoryStream, memoryStream.Capacity);

            // Act and assert
            networkBuffer.Dispose();
            Assert.Throws<ObjectDisposedException>(() => networkBuffer.Read(out int res));
        }

        [Fact]
        public void Write_WritingDataThatSizeIsEqualToBufferSizeShouldBeFlushedAutomatically()
        {
            // Arrange
            ulong providedValue = 101UL;
            int rawBufferSize = sizeof(ulong);
            using var memoryStream = new MemoryStream(new byte[rawBufferSize], index: 0, count: rawBufferSize);
            using var networkBuffer = new NetworkBuffer(memoryStream, bufferSize: memoryStream.Capacity);

            // Act
            networkBuffer.Write(providedValue);

            // Assert
            Assert.Equal(Marshal.SizeOf(providedValue), (int)memoryStream.Position);
        }
    }
}