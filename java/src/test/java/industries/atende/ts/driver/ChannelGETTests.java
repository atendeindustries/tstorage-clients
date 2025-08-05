/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import org.junit.jupiter.api.Test;

import java.io.*;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.function.Supplier;

import static org.junit.jupiter.api.Assertions.*;
import static org.junit.jupiter.api.Assumptions.assumeTrue;

public class ChannelGETTests {

    static ByteOrder order = Globals.order;

    public abstract static class SocketMock implements __net_Connection {

        public ByteArrayOutputStream out;
        public ByteArrayInputStream in;
        public ArrayList<Record<byte[]>> inRecords;

        public SocketMock() {
            out = new ByteArrayOutputStream();
            in = new ByteArrayInputStream(new byte[0]);
            prepareResponseBytes();
        }

        @Override
        public void close() throws IOException {
            out.close();
            in.close();
        }

        @Override
        public InputStream getInputStream() throws IOException {
            return in;
        }

        @Override
        public OutputStream getOutputStream() throws IOException {
            return out;
        }

        @Override
        public void open() throws IOException {
        }

        protected void prepareResponseBytes() {
        }

    }

    @Test
    void testChannelBasicSuccess() {
        var sm = new SocketMock(){
            protected void prepareResponseBytes() {
                var builder = new GETResponseInputBuilder(order);
                builder.useSuccessFirstHeader();
                inRecords = builder.generateRecords(1, 4);
                builder.useSuccessSecondHeader();
                final long acq = Key.max().acq();
                builder.setAcq(acq);
                in = new ByteArrayInputStream(builder.build());
            }
        };
        var channel = new Channel<>(sm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        assertEquals(ResponseStatus.OK, channel.connect().getStatus());
        var responseGet = channel.get(Key.min(), Key.max(), Globals.RESPONSE_BYTES_LIMIT);
        Globals.assertGETRequestAsBytes(sm.out.toByteArray(), Key.min(), Key.max());
        Globals.assertResponseGet(ResponseStatus.OK, sm.inRecords, responseGet, Key.max().acq());
        assertEquals(ResponseStatus.OK, channel.close().getStatus());
    }

    @Test
    void testChannelLimitBelowMinimum() {
        var channel = new Channel<>(new SocketMock() {}, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        assertEquals(ResponseStatus.OK, channel.connect().getStatus());
        assertThrows(IllegalArgumentException.class,
                () -> channel.get(Key.min(), Key.max(), __proto_Protocol.MIN_GET_RESPONSE_SIZE_ERROR -1));
        assertEquals(ResponseStatus.OK, channel.close().getStatus());
    }

    @Test
    void testChannelLimitEqualMinimumError() {
        var sm = new SocketMock(){
            protected void prepareResponseBytes() {
                var builder = new GETResponseInputBuilder(order);
                builder.useSuccessFirstHeader();
                inRecords = builder.generateRecords(0, 4);
                builder.useErrorSecondHeader();
                in = new ByteArrayInputStream(builder.build());
            }
        };
        var channel = new Channel<>(sm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        assertEquals(ResponseStatus.OK, channel.connect().getStatus());
        var responseGet = channel.get(Key.min(), Key.max(), __proto_Protocol.MIN_GET_RESPONSE_SIZE_ERROR);
        Globals.assertResponseGet(ResponseStatus.ERROR, sm.inRecords, responseGet, -1);
        assertEquals(ResponseStatus.OK, channel.close().getStatus());
    }

    @Test
    void testChannelLimitEqualMinimumSuccess() {
        var sm = new SocketMock(){
            protected void prepareResponseBytes() {
                var builder = new GETResponseInputBuilder(order);
                builder.useSuccessFirstHeader();
                inRecords = builder.generateRecords(0, 4);
                builder.useSuccessSecondHeader();
                builder.setAcq(12345);
                in = new ByteArrayInputStream(builder.build());
            }
        };
        var channel = new Channel<>(sm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        channel.connect();
        var responseGet = channel.get(Key.min(), Key.max(), __proto_Protocol.MIN_GET_RESPONSE_SIZE_SUCCESS);
        Globals.assertResponseGet(ResponseStatus.OK, sm.inRecords, responseGet, 12345);
        channel.close();
    }

    @Test
    void testChannelLimitRecordIncomplete() {
        var sm = new SocketMock(){
            protected void prepareResponseBytes() {
                var builder = new GETResponseInputBuilder(order);
                builder.useSuccessFirstHeader();
                inRecords = builder.generateRecords(1, 1); // recSize and Key bytes added later, so only 1 byte off
                builder.useSuccessSecondHeader();
                builder.setAcq(12345);
                in = new ByteArrayInputStream(builder.build());
            }
        };
        var channel = new Channel<>(sm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        channel.connect();
        var responseGet = channel.get(Key.min(), Key.max(), __proto_Protocol.MIN_GET_RESPONSE_SIZE_SUCCESS + Integer.BYTES + Key.BYTES);
        assertSame(ResponseStatus.LIMIT_TOO_LOW, responseGet.getStatus());
        assertEquals(ResponseAcq.INVALID_ACQ, responseGet.getAcq());
        var outRecords = responseGet.getData();
        assertEquals(1, outRecords.size()); // Added one record, but there should be not enough bytes to read acq.
        channel.close();
    }

    @Test
    void testChannelLimitLastRecordIncomplete() {
        Supplier<SocketMock> smf = () -> new SocketMock(){
            protected void prepareResponseBytes() {
                var builder = new GETResponseInputBuilder(order);
                builder.useSuccessFirstHeader();
                inRecords = builder.generateRecords(3, 1);
                builder.useSuccessSecondHeader();
                builder.setAcq(12345);
                in = new ByteArrayInputStream(builder.build());
            }
        };
        // Each payloadSize is 1 byte so each additional record
        // will occupy recSize(4) + key(32) + payload(1) = 37 bytes.
        // So total additional bytes to the minimum successful response is:
        // 37 * recs.length = 111 bytes;
        // To drop the third record we need to give value between +74 and +110,
        // but we must also subtract successful confirm block secondHeader(12) + acq(8) = 20 bytes
        // and recSize=0(4)
        int c = 74;
        for(; c <= 110; c++) {
            var channel = new Channel<>(smf.get(), Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
            channel.connect();
            var responseGet = channel.get(Key.min(), Key.max(), __proto_Protocol.MIN_GET_RESPONSE_SIZE_SUCCESS - 20 - Integer.BYTES + c);
            assertSame(ResponseStatus.LIMIT_TOO_LOW, responseGet.getStatus());
            assumeTrue(responseGet.getAcq() == ResponseAcq.INVALID_ACQ);
            var outRecords = responseGet.getData();
            assertEquals(2, outRecords.size()); // Should be able to read 2/3 records
            channel.close();
        }
        var sm = smf.get();
        var channel = new Channel<>(sm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        channel.connect();
        var responseGet = channel.get(Key.min(), Key.max(), __proto_Protocol.MIN_GET_RESPONSE_SIZE_SUCCESS + 87);
        Globals.assertResponseGet(ResponseStatus.LIMIT_TOO_LOW, sm.inRecords, responseGet, ResponseAcq.INVALID_ACQ);
        channel.close();
    }

    @Test
    void testChannelLimitDoubleTheInputBufferSize() {
        var recsCount = __proto_Protocol.INPUT_BUFFER_SIZE / 37 + 1;
        Supplier<SocketMock> smf = () -> new SocketMock(){
            protected void prepareResponseBytes() {
                var builder = new GETResponseInputBuilder(order);
                builder.useSuccessFirstHeader();
                inRecords = builder.generateRecords(recsCount, 1);
                builder.useSuccessSecondHeader();
                builder.setAcq(12345);
                in = new ByteArrayInputStream(builder.build());
            }
        };
        var sm = smf.get();
        var channel = new Channel<>(sm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        channel.connect();
        var responseGet = channel.get(Key.min(), Key.max(), __proto_Protocol.INPUT_BUFFER_SIZE*2);
        assertSame(ResponseStatus.OK, responseGet.getStatus());
        assumeTrue(responseGet.getAcq() == 12345);
        var outRecords = responseGet.getData();
        assertEquals(sm.inRecords.size(), outRecords.size()); // Should be able to read 2/3 records
        channel.close();
    }

}
