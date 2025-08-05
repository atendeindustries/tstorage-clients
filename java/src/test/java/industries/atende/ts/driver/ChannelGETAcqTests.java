/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import org.junit.jupiter.api.Test;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;

import static org.junit.jupiter.api.Assertions.*;
import static org.junit.jupiter.api.Assumptions.assumeTrue;

public class ChannelGETAcqTests {

    static ByteOrder order = Globals.order;

    public abstract static class ConnectionSocketMock implements __net_Connection {

        public ByteArrayOutputStream out;
        public ByteArrayInputStream in;
        public ArrayList<Record<byte[]>> inRecords;

        public ConnectionSocketMock() {
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

        protected void error() {
            var bb = ByteBuffer.allocate(__proto_Header.BYTES).order(order);
            bb.putInt(1);
            bb.putLong(0);
            in = new ByteArrayInputStream(bb.array());
        }

        protected void success() {
            var bb = ByteBuffer.allocate(__proto_Header.BYTES + Key.BYTES_ACQ).order(order);
            bb.putInt(0);
            bb.putLong(Key.BYTES_ACQ);
            bb.putLong(12345);
            in = new ByteArrayInputStream(bb.array());
        }

        protected void prepareResponseBytes() {
            throw new NotImplementedException();
        }

    }

    @Test
    void testGETAcqRequestToBytes() {
        var socketOutputStreamMock = new ByteArrayOutputStream();
        var getOut = new __proto_ProtocolOutputStream(socketOutputStreamMock, order);
        var writer = new __proto_GETAcqWriterInPlace(getOut);
        writer.write(new __proto_GETAcqRequest(Key.min(), Key.max()));
        var bytes = socketOutputStreamMock.toByteArray();
        Globals.assertGETAcqRequestAsBytes(bytes, Key.min(), Key.max());
    }

    @Test
    void testReturnedValueNotNull() {
        var csm = new ConnectionSocketMock(){
            protected void prepareResponseBytes() {
            }
        };
        var channel = new Channel<>(csm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        channel.connect();
        assertNotNull(channel.getAcq(Key.min(), Key.max()));
        channel.close();
    }

    @Test
    void testReturnedValueIsResponseAcqInstance() {
        var csm = new ConnectionSocketMock(){
            protected void prepareResponseBytes() {
            }
        };
        var channel = new Channel<>(csm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        channel.connect();
        var responseAcq = channel.getAcq(Key.min(), Key.max());
        assertInstanceOf(ResponseAcq.class, responseAcq);
        channel.close();
    }

    @Test
    void testError() {
        var csm = new ConnectionSocketMock(){
            protected void prepareResponseBytes() {
                error();
            }
        };
        var channel = new Channel<>(csm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        channel.connect();
        var responseAcq = channel.getAcq(Key.min(), Key.max());
        assertSame(ResponseStatus.ERROR, responseAcq.getStatus());
        assertEquals(ResponseAcq.INVALID_ACQ, responseAcq.getAcq());
        channel.close();
    }

    @Test
    void testSuccess() {
        var csm = new ConnectionSocketMock(){
            protected void prepareResponseBytes() {
                success();
            }
        };
        var channel = new Channel<>(csm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        channel.connect();
        var responseAcq = channel.getAcq(Key.min(), Key.max());
        assertSame(ResponseStatus.OK, responseAcq.getStatus());
        assertEquals(12345, responseAcq.getAcq());
        channel.close();
    }

}
