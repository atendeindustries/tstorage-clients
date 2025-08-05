package industries.atende.ts.driver;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.MethodSource;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.stream.Stream;

import static industries.atende.ts.driver.Globals.order;
import static org.junit.jupiter.api.Assertions.*;

class PUTsSafeWriterInPlaceTests {

    @Test
    void testNullRequest() {
        var outputStream = new __proto_ProtocolOutputStream(new ByteArrayOutputStream(), order);
        var putWriter = new __proto_PUTsSafeWriterInPlace<>(outputStream, Globals.PAYLOAD_TYPE_BYTE_ARRAY, order);
        assertThrows(NullPointerException.class, () -> putWriter.write(null));
    }

    @Test
    void testRequestNullCMD() {
        var outputStream = new __proto_ProtocolOutputStream(new ByteArrayOutputStream(), order);
        var putWriter = new __proto_PUTsSafeWriterInPlace<>(outputStream, Globals.PAYLOAD_TYPE_BYTE_ARRAY, order);
        assertThrows(NullPointerException.class, () -> putWriter.write(new __proto_PUTsRequest<>(null, new _RecordsSetCollection<>(new ArrayList<>()))));
    }

    @Test
    void testRequestNullRecordsSet() {
        var outputStream = new __proto_ProtocolOutputStream(new ByteArrayOutputStream(), order);
        var putWriter = new __proto_PUTsSafeWriterInPlace<>(outputStream, Globals.PAYLOAD_TYPE_BYTE_ARRAY, order);
        assertThrows(NullPointerException.class, () -> putWriter.write(new __proto_PUTsRequest<>(__proto_PUTsRequest.RequestType.PUTSAFE, null)));
    }

    @Test
    void testReturnedValueIsResponseInstance() {
        var outputStream = new __proto_ProtocolOutputStream(new ByteArrayOutputStream(), order);
        var putWriter = new __proto_PUTsSafeWriterInPlace<>(outputStream, Globals.PAYLOAD_TYPE_BYTE_ARRAY, order);
        var responseStatus = putWriter.write(new __proto_PUTsRequest<>(__proto_PUTsRequest.RequestType.PUTASAFE, new _RecordsSetCollection<>(new ArrayList<>())));
        assertInstanceOf(ResponseStatus.class, responseStatus);
    }

    static Stream<__proto_PUTsRequest.RequestType> putsProvider() {
        return Stream.of(__proto_PUTsRequest.RequestType.PUTSAFE, __proto_PUTsRequest.RequestType.PUTASAFE);
    }

    @ParameterizedTest
    @MethodSource("putsProvider")
    void testZeroLengthRecs(__proto_PUTsRequest.RequestType requestType) {
        // start typing...
        var whereAllTheBytesAre = new ByteArrayOutputStream();
        var outputStream = new __proto_ProtocolOutputStream(whereAllTheBytesAre, order);
        var putWriter = new __proto_PUTsSafeWriterInPlace<>(outputStream, Globals.PAYLOAD_TYPE_BYTE_ARRAY, order);
        var responseStatus = putWriter.write(new __proto_PUTsRequest<>(requestType, new _RecordsSetCollection<>(new ArrayList<>())));
        assertSame(ResponseStatus.OK, responseStatus);
        var bytes = whereAllTheBytesAre.toByteArray();
        assertEquals(__proto_Header.BYTES + Integer.BYTES, bytes.length); // header + terminating cid
        var inputStream = new __proto_ProtocolInputStream(new ByteArrayInputStream(bytes), order);
        try {
            assertEquals(requestType.getValue(), inputStream.readInt());
            assertEquals(0L, inputStream.readLong());
            assertEquals(-1, inputStream.readInt());
        } catch (IOException e) {
            fail();
        }
    }

    @ParameterizedTest
    @MethodSource("putsProvider")
    void testUniqueCids(__proto_PUTsRequest.RequestType requestType) {
        var whereAllTheBytesAre = new ByteArrayOutputStream();
        byte[] counters = new byte[6];
        final int BATCH_SIZE = 0, CID = 1, CMD = 2, SIZE = 3, FLUSH = 4, BATCH = 5;
        int[] expectedCid = new int[5];
        var outputStream = new __proto_ProtocolOutputStream(whereAllTheBytesAre, order) {
            @Override
            void writeBatchSize(int value) throws IOException {
                counters[BATCH_SIZE]++;
                if (requestType == __proto_PUTsRequest.RequestType.PUTSAFE) {
                    assertEquals(28, value);
                }
                if (requestType == __proto_PUTsRequest.RequestType.PUTASAFE) {
                    assertEquals(36, value);
                }
                super.writeBatchSize(value);
            }

            @Override
            void writeCid(int value) throws IOException {
                expectedCid[counters[CID]] = value;
                counters[CID]++;
                super.writeCid(value);
            }

            @Override
            void writeCmd(int value) throws IOException {
                counters[CMD]++;
                if (requestType == __proto_PUTsRequest.RequestType.PUTSAFE) {
                    assertEquals(__proto_PUTsRequest.RequestType.PUTSAFE.getValue(), value);
                }
                if (requestType == __proto_PUTsRequest.RequestType.PUTASAFE) {
                    assertEquals(__proto_PUTsRequest.RequestType.PUTASAFE.getValue(), value);
                }
                super.writeCmd(value);
            }

            @Override
            void writeSize(long value) throws IOException {
                counters[SIZE]++;
                assertEquals(0, value);
                super.writeSize(value);
            }

            @Override
            public void flush() throws IOException {
                counters[FLUSH]++;
                super.flush();
            }

            @Override
            public void writeBatch(byte[] b) throws IOException {
                counters[BATCH]++;
                if (requestType == __proto_PUTsRequest.RequestType.PUTSAFE) {
                    assertEquals(28, b.length);
                }
                if (requestType == __proto_PUTsRequest.RequestType.PUTASAFE) {
                    assertEquals(36, b.length);
                }
                super.write(b);
            }
        };

        var inRecordsSetCollection = new _RecordsSetCollection<>(new GETResponseInputBuilder(order).generateRecords(4, 4));
        assertEquals(4, inRecordsSetCollection.size());

        var putWriter = new __proto_PUTsSafeWriterInPlace<>(outputStream, Globals.PAYLOAD_TYPE_BYTE_ARRAY, order);

        var responseStatus = putWriter.write(new __proto_PUTsRequest<>(requestType, inRecordsSetCollection));
        assertSame(ResponseStatus.OK, responseStatus);
        assertEquals(4, counters[BATCH_SIZE]);
        assertEquals(5, counters[CID]);
        assertEquals(1, counters[CMD]);
        assertEquals(1, counters[SIZE]);
        assertEquals(1, counters[FLUSH]);
        assertEquals(4, counters[BATCH]);

        var bytes = whereAllTheBytesAre.toByteArray();
        // recordSize = field recSize + key + payload
        final int recSizeBytes = Integer.BYTES;
        final int batchSizeBytes = Integer.BYTES;
        final int cidSizeBytes = Integer.BYTES;

        var recordSize = recSizeBytes + Key.BYTES_MID + Key.BYTES_MOID + Key.BYTES_CAP;
        if (requestType == __proto_PUTsRequest.RequestType.PUTASAFE) {
            recordSize += Key.BYTES_ACQ;
        }
        recordSize += 4; // + payload size
        final int outputBytes = __proto_Header.BYTES + 4 * (cidSizeBytes + batchSizeBytes + recordSize) + recSizeBytes;
        assertEquals(outputBytes, bytes.length);
        var inputStream = new __proto_ProtocolInputStream(new ByteArrayInputStream(bytes), order);
        try {
            assertEquals(requestType.getValue(), inputStream.readInt()); // cmd
            assertEquals(0L, inputStream.readLong()); // size
            var outRecordsSetCollection = new _RecordsSetCollection<>(new ArrayList<>());
            int cid;
            int cidCounter = 0;
            long acqCounter = 0;
            while ((cid = inputStream.readInt()) != -1) {
                assertEquals(expectedCid[cidCounter++], cid);
                int batchSize = inputStream.readInt();
                if (requestType == __proto_PUTsRequest.RequestType.PUTSAFE) {
                    assertEquals(28, batchSize);
                }
                if (requestType == __proto_PUTsRequest.RequestType.PUTASAFE) {
                    assertEquals(36, batchSize);
                }

                int recSize = inputStream.readRecSize();
                byte[] payload;
                Key k;
                switch (requestType) {
                    case PUTSAFE -> {
                        assertEquals(24, recSize);
                        k = new Key(
                            cid,
                            inputStream.readLong(),
                            inputStream.readInt(),
                            inputStream.readLong(),
                            acqCounter++
                        );
                        payload = inputStream.readPayload(4); // should be 4 bytes
                        outRecordsSetCollection.append(k, payload);
                    }
                    case PUTASAFE -> {
                        assertEquals(32, recSize);
                        k = new Key(
                            cid,
                            inputStream.readLong(),
                            inputStream.readInt(),
                            inputStream.readLong(),
                            inputStream.readLong()
                        );
                        payload = inputStream.readPayload(4);
                        outRecordsSetCollection.append(k, payload);
                    }
                }

            }
            assertEquals(outRecordsSetCollection.size(), inRecordsSetCollection.size());
            assertEquals(-1, cid);
            assertEquals(-1, inputStream.read());
        } catch (IOException e) {
            fail();
        }
    }

    @ParameterizedTest
    @MethodSource("putsProvider")
    void testErrorBatchSize(__proto_PUTsRequest.RequestType requestType) {
        var whereAllTheBytesAre = new ByteArrayOutputStream();
        byte[] counters = new byte[6];
        final int BATCH_SIZE = 0, CID = 1, CMD = 2, SIZE = 3, FLUSH = 4, BATCH = 5;
        var outputStream = new __proto_ProtocolOutputStream(whereAllTheBytesAre, order) {
            @Override
            void writeBatchSize(int value) throws IOException {
//                counters[BATCH_SIZE]++;
//                super.writeBatchSize(value);
                throw new IOException();
            }

            @Override
            void writeCid(int value) throws IOException {
                counters[CID]++;
                super.writeCid(value);
            }

            @Override
            void writeCmd(int value) throws IOException {
                counters[CMD]++;
                super.writeCmd(value);
            }

            @Override
            void writeSize(long value) throws IOException {
                counters[SIZE]++;
                super.writeSize(value);
            }

            @Override
            public void flush() throws IOException {
                counters[FLUSH]++;
                super.flush();
            }

            @Override
            public void writeBatch(byte[] b) throws IOException {
                counters[BATCH]++;
                super.write(b);
            }
        };

        var inRecordsSetCollection = new _RecordsSetCollection<>(new GETResponseInputBuilder(order).generateRecords(4, 4));
        assertEquals(4, inRecordsSetCollection.size());

        var putWriter = new __proto_PUTsSafeWriterInPlace<>(outputStream, Globals.PAYLOAD_TYPE_BYTE_ARRAY, order);

        var responseStatus = putWriter.write(new __proto_PUTsRequest<>(requestType, inRecordsSetCollection));
        assertSame(ResponseStatus.ERROR, responseStatus);
        assertEquals(0, counters[BATCH_SIZE]);
        assertEquals(1, counters[CID]);
        assertEquals(1, counters[CMD]);
        assertEquals(1, counters[SIZE]);
        assertEquals(0, counters[FLUSH]);
        assertEquals(0, counters[BATCH]);
    }

    @ParameterizedTest
    @MethodSource("putsProvider")
    void testErrorCid(__proto_PUTsRequest.RequestType requestType) {
        var whereAllTheBytesAre = new ByteArrayOutputStream();
        byte[] counters = new byte[6];
        final int BATCH_SIZE = 0, CID = 1, CMD = 2, SIZE = 3, FLUSH = 4, BATCH = 5;
        var outputStream = new __proto_ProtocolOutputStream(whereAllTheBytesAre, order) {
            @Override
            void writeBatchSize(int value) throws IOException {
                counters[BATCH_SIZE]++;
                super.writeBatchSize(value);
            }

            @Override
            void writeCid(int value) throws IOException {
                throw new IOException();
//                counters[CID]++;
//                super.writeCid(value);
            }

            @Override
            void writeCmd(int value) throws IOException {
                counters[CMD]++;
                super.writeCmd(value);
            }

            @Override
            void writeSize(long value) throws IOException {
                counters[SIZE]++;
                super.writeSize(value);
            }

            @Override
            public void flush() throws IOException {
                counters[FLUSH]++;
                super.flush();
            }

            @Override
            public void writeBatch(byte[] b) throws IOException {
                counters[BATCH]++;
                super.write(b);
            }
        };

        var inRecordsSetCollection = new _RecordsSetCollection<>(new GETResponseInputBuilder(order).generateRecords(4, 4));
        assertEquals(4, inRecordsSetCollection.size());

        var putWriter = new __proto_PUTsSafeWriterInPlace<>(outputStream, Globals.PAYLOAD_TYPE_BYTE_ARRAY, order);

        var responseStatus = putWriter.write(new __proto_PUTsRequest<>(requestType, inRecordsSetCollection));
        assertSame(ResponseStatus.ERROR, responseStatus);
        assertEquals(0, counters[BATCH_SIZE]);
        assertEquals(0, counters[CID]);
        assertEquals(1, counters[CMD]);
        assertEquals(1, counters[SIZE]);
        assertEquals(0, counters[FLUSH]);
        assertEquals(0, counters[BATCH]);
    }

    @ParameterizedTest
    @MethodSource("putsProvider")
    void testErrorCmd(__proto_PUTsRequest.RequestType requestType) {
        var whereAllTheBytesAre = new ByteArrayOutputStream();
        byte[] counters = new byte[6];
        final int BATCH_SIZE = 0, CID = 1, CMD = 2, SIZE = 3, FLUSH = 4, BATCH = 5;
        var outputStream = new __proto_ProtocolOutputStream(whereAllTheBytesAre, order) {
            @Override
            void writeBatchSize(int value) throws IOException {
                counters[BATCH_SIZE]++;
                super.writeBatchSize(value);
            }

            @Override
            void writeCid(int value) throws IOException {
                counters[CID]++;
                super.writeCid(value);
            }

            @Override
            void writeCmd(int value) throws IOException {
                throw new IOException();
//                counters[CMD]++;
//                super.writeCmd(value);
            }

            @Override
            void writeSize(long value) throws IOException {
                counters[SIZE]++;
                super.writeSize(value);
            }

            @Override
            public void flush() throws IOException {
                counters[FLUSH]++;
                super.flush();
            }

            @Override
            public void writeBatch(byte[] b) throws IOException {
                counters[BATCH]++;
                super.write(b);
            }
        };

        var inRecordsSetCollection = new _RecordsSetCollection<>(new GETResponseInputBuilder(order).generateRecords(4, 4));
        assertEquals(4, inRecordsSetCollection.size());

        var putWriter = new __proto_PUTsSafeWriterInPlace<>(outputStream, Globals.PAYLOAD_TYPE_BYTE_ARRAY, order);

        var responseStatus = putWriter.write(new __proto_PUTsRequest<>(requestType, inRecordsSetCollection));
        assertSame(ResponseStatus.ERROR, responseStatus);
        assertEquals(0, counters[BATCH_SIZE]);
        assertEquals(0, counters[CID]);
        assertEquals(0, counters[CMD]);
        assertEquals(0, counters[SIZE]);
        assertEquals(0, counters[FLUSH]);
        assertEquals(0, counters[BATCH]);
    }

    @ParameterizedTest
    @MethodSource("putsProvider")
    void testErrorSize(__proto_PUTsRequest.RequestType requestType) {
        var whereAllTheBytesAre = new ByteArrayOutputStream();
        byte[] counters = new byte[6];
        final int BATCH_SIZE = 0, CID = 1, CMD = 2, SIZE = 3, FLUSH = 4, BATCH = 5;
        var outputStream = new __proto_ProtocolOutputStream(whereAllTheBytesAre, order) {
            @Override
            void writeBatchSize(int value) throws IOException {
                counters[BATCH_SIZE]++;
                super.writeBatchSize(value);
            }

            @Override
            void writeCid(int value) throws IOException {
                counters[CID]++;
                super.writeCid(value);
            }

            @Override
            void writeCmd(int value) throws IOException {
                counters[CMD]++;
                super.writeCmd(value);
            }

            @Override
            void writeSize(long value) throws IOException {
                throw new IOException();
//                counters[SIZE]++;
//                super.writeSize(value);
            }

            @Override
            public void flush() throws IOException {
                counters[FLUSH]++;
                super.flush();
            }

            @Override
            public void writeBatch(byte[] b) throws IOException {
                counters[BATCH]++;
                super.write(b);
            }
        };

        var inRecordsSetCollection = new _RecordsSetCollection<>(new GETResponseInputBuilder(order).generateRecords(4, 4));
        assertEquals(4, inRecordsSetCollection.size());

        var putWriter = new __proto_PUTsSafeWriterInPlace<>(outputStream, Globals.PAYLOAD_TYPE_BYTE_ARRAY, order);

        var responseStatus = putWriter.write(new __proto_PUTsRequest<>(requestType, inRecordsSetCollection));
        assertSame(ResponseStatus.ERROR, responseStatus);
        assertEquals(0, counters[BATCH_SIZE]);
        assertEquals(0, counters[CID]);
        assertEquals(1, counters[CMD]);
        assertEquals(0, counters[SIZE]);
        assertEquals(0, counters[FLUSH]);
        assertEquals(0, counters[BATCH]);
    }

    @ParameterizedTest
    @MethodSource("putsProvider")
    void testErrorFlush(__proto_PUTsRequest.RequestType requestType) {
        var whereAllTheBytesAre = new ByteArrayOutputStream();
        byte[] counters = new byte[6];
        final int BATCH_SIZE = 0, CID = 1, CMD = 2, SIZE = 3, FLUSH = 4, BATCH = 5;
        var outputStream = new __proto_ProtocolOutputStream(whereAllTheBytesAre, order) {
            @Override
            void writeBatchSize(int value) throws IOException {
                counters[BATCH_SIZE]++;
                super.writeBatchSize(value);
            }

            @Override
            void writeCid(int value) throws IOException {
                counters[CID]++;
                super.writeCid(value);
            }

            @Override
            void writeCmd(int value) throws IOException {
                counters[CMD]++;
                super.writeCmd(value);
            }

            @Override
            void writeSize(long value) throws IOException {
                counters[SIZE]++;
                super.writeSize(value);
            }

            @Override
            public void flush() throws IOException {
                throw new IOException();
//                counters[FLUSH]++;
//                super.flush();
            }

            @Override
            public void writeBatch(byte[] b) throws IOException {
                counters[BATCH]++;
                super.write(b);
            }
        };

        var inRecordsSetCollection = new _RecordsSetCollection<>(new GETResponseInputBuilder(order).generateRecords(4, 4));
        assertEquals(4, inRecordsSetCollection.size());

        var putWriter = new __proto_PUTsSafeWriterInPlace<>(outputStream, Globals.PAYLOAD_TYPE_BYTE_ARRAY, order);

        var responseStatus = putWriter.write(new __proto_PUTsRequest<>(requestType, inRecordsSetCollection));
        assertSame(ResponseStatus.ERROR, responseStatus);
        assertEquals(4, counters[BATCH_SIZE]);
        assertEquals(5, counters[CID]);
        assertEquals(1, counters[CMD]);
        assertEquals(1, counters[SIZE]);
        assertEquals(0, counters[FLUSH]);
        assertEquals(4, counters[BATCH]);
    }


    @ParameterizedTest
    @MethodSource("putsProvider")
    void testErrorBatch(__proto_PUTsRequest.RequestType requestType) {
        var whereAllTheBytesAre = new ByteArrayOutputStream();
        byte[] counters = new byte[6];
        final int BATCH_SIZE = 0, CID = 1, CMD = 2, SIZE = 3, FLUSH = 4, BATCH = 5;
        var outputStream = new __proto_ProtocolOutputStream(whereAllTheBytesAre, order) {
            @Override
            void writeBatchSize(int value) throws IOException {
                counters[BATCH_SIZE]++;
                super.writeBatchSize(value);
            }

            @Override
            void writeCid(int value) throws IOException {
                counters[CID]++;
                super.writeCid(value);
            }

            @Override
            void writeCmd(int value) throws IOException {
                counters[CMD]++;
                super.writeCmd(value);
            }

            @Override
            void writeSize(long value) throws IOException {
                counters[SIZE]++;
                super.writeSize(value);
            }

            @Override
            public void flush() throws IOException {
                counters[FLUSH]++;
                super.flush();
            }

            @Override
            public void writeBatch(byte[] b) throws IOException {
                throw new IOException();
//                counters[BATCH]++;
//                super.write(b);
            }
        };

        var inRecordsSetCollection = new _RecordsSetCollection<>(new GETResponseInputBuilder(order).generateRecords(4, 4));
        assertEquals(4, inRecordsSetCollection.size());

        var putWriter = new __proto_PUTsSafeWriterInPlace<>(outputStream, Globals.PAYLOAD_TYPE_BYTE_ARRAY, order);

        var responseStatus = putWriter.write(new __proto_PUTsRequest<>(requestType, inRecordsSetCollection));
        assertSame(ResponseStatus.ERROR, responseStatus);
        assertEquals(1, counters[BATCH_SIZE]);
        assertEquals(1, counters[CID]);
        assertEquals(1, counters[CMD]);
        assertEquals(1, counters[SIZE]);
        assertEquals(0, counters[FLUSH]);
        assertEquals(0, counters[BATCH]);
    }

}