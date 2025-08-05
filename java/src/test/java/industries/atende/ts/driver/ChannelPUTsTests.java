/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.MethodSource;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.stream.Stream;

import static org.junit.jupiter.api.Assertions.*;

public class ChannelPUTsTests {

    static ByteOrder order = Globals.order;

    private byte[] successBytes() {
        var bb = ByteBuffer.allocate(__proto_Header.BYTES + 2 * Key.BYTES_ACQ).order(order);
        bb.putInt(0);
        bb.putLong(2 * Key.BYTES_ACQ);
        bb.putLong(12345);
        bb.putLong(54321);
        return bb.array();
    }

    private byte[] errorBytes() {
        var bb = ByteBuffer.allocate(__proto_Header.BYTES).order(order);
        bb.putInt(1);
        bb.putLong(0);
        return bb.array();
    }

    __proto_PUTsReaderInPlace successReader() {
        return new __proto_PUTsReaderInPlace(
            new __proto_ProtocolInputStream(
                new BufferedInputStream(
                    new ByteArrayInputStream(successBytes()),
                    __proto_Header.BYTES + 2 * Key.BYTES_ACQ
                ),
                order
            )
        );
    }

    __proto_PUTsReaderInPlace errorReader() {
        return new __proto_PUTsReaderInPlace(
            new __proto_ProtocolInputStream(
                new BufferedInputStream(
                    new ByteArrayInputStream(errorBytes()),
                    __proto_Header.BYTES
                ),
                order
            )
        );
    }

    public abstract static class ConnectionSocketMock implements __net_Connection {

        public ByteArrayOutputStream out;
        public ByteArrayInputStream in;
        public ArrayList<Record<byte[]>> inRecords;

        public ConnectionSocketMock() {
            prepareRequestBytes();
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
            throw new NotImplementedException();
        }

        protected void success() {
            throw new NotImplementedException();
        }

        protected void prepareRequestBytes() {
            out = new ByteArrayOutputStream();
        }

        protected void prepareResponseBytes() {
            in = new ByteArrayInputStream(new byte[0]);
        }

    }

    @Test
    void testNullData() {
        var csm = new ConnectionSocketMock() {
        };
        var channel = new Channel<>(csm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        channel.connect();
        assertThrows(NullPointerException.class, () -> channel.put(null));
        assertThrows(NullPointerException.class, () -> channel.puta(null));
        channel.close();
    }

    @Test
    void testReturnedObjectNotNull() {
        var csm = new ConnectionSocketMock() {
        };
        var channel = new Channel<>(csm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        channel.connect();
        assertNotNull(channel.put(new _RecordsSetCollection<>(new ArrayList<>())));
        assertNotNull(channel.puta(new _RecordsSetCollection<>(new ArrayList<>())));
        channel.close();
    }

    @Test
    void testReturnedValueIsResponseInstance() {
        var csm = new ConnectionSocketMock() {
        };
        var channel = new Channel<>(csm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        channel.connect();
        assertInstanceOf(Response.class, channel.put(new _RecordsSetCollection<>(new ArrayList<>())));
        assertInstanceOf(Response.class, channel.puta(new _RecordsSetCollection<>(new ArrayList<>())));
        channel.close();
    }

    static Stream<__proto_PUTsRequest.RequestType> putsProvider() {
        return Stream.of(__proto_PUTsRequest.RequestType.PUTSAFE, __proto_PUTsRequest.RequestType.PUTASAFE);
    }

    @ParameterizedTest
    @MethodSource("putsProvider")
    void testRequestErrorResponseOK(__proto_PUTsRequest.RequestType requestType) {
        var csm = new ConnectionSocketMock() {
        };
        var channel = new Channel<>(csm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        var writer = new __proto_RequestWriter<__proto_PUTsRequest<byte[]>>() {
            @Override
            public ResponseStatus write(__proto_PUTsRequest<byte[]> request) {
                return ResponseStatus.ERROR;
            }
        };

        var puts = new __proto_PUTs<>(requestType, writer, successReader(), Globals.PAYLOAD_TYPE_BYTE_ARRAY);
        channel.connect();
        if (requestType == __proto_PUTsRequest.RequestType.PUTSAFE) {
            assertSame(ResponseStatus.ERROR, channel.put(puts, new _RecordsSetCollection<>(new ArrayList<>())).getStatus());
        }
        if (requestType == __proto_PUTsRequest.RequestType.PUTASAFE) {
            assertSame(ResponseStatus.ERROR, channel.puta(puts, new _RecordsSetCollection<>(new ArrayList<>())).getStatus());
        }
        channel.close();
    }

    @ParameterizedTest
    @MethodSource("putsProvider")
    void testRequestOKResponseOK(__proto_PUTsRequest.RequestType requestType) {
        var csm = new ConnectionSocketMock() {
        };
        var channel = new Channel<>(csm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        var writer = new __proto_RequestWriter<__proto_PUTsRequest<byte[]>>() {
            @Override
            public ResponseStatus write(__proto_PUTsRequest<byte[]> request) {
                return ResponseStatus.OK;
            }
        };
        var puts = new __proto_PUTs<>(requestType, writer, successReader(), Globals.PAYLOAD_TYPE_BYTE_ARRAY);
        channel.connect();
        if (requestType == __proto_PUTsRequest.RequestType.PUTSAFE) {
            assertSame(ResponseStatus.OK, channel.put(puts, new _RecordsSetCollection<>(new ArrayList<>())).getStatus());
        }
        if (requestType == __proto_PUTsRequest.RequestType.PUTASAFE) {
            assertSame(ResponseStatus.OK, channel.puta(puts, new _RecordsSetCollection<>(new ArrayList<>())).getStatus());
        }
        channel.close();
    }

    @ParameterizedTest
    @MethodSource("putsProvider")
    void testRequestOKResponseError(__proto_PUTsRequest.RequestType requestType) {
        var csm = new ConnectionSocketMock() {
        };
        var channel = new Channel<>(csm, Globals.PAYLOAD_TYPE_BYTE_ARRAY, Globals.order);
        var writer = new __proto_RequestWriter<__proto_PUTsRequest<byte[]>>() {
            @Override
            public ResponseStatus write(__proto_PUTsRequest<byte[]> request) {
                return ResponseStatus.OK;
            }
        };
        var puts = new __proto_PUTs<>(requestType, writer, errorReader(), Globals.PAYLOAD_TYPE_BYTE_ARRAY);
        channel.connect();
        if (requestType == __proto_PUTsRequest.RequestType.PUTSAFE) {
            assertSame(ResponseStatus.ERROR, channel.put(puts, new _RecordsSetCollection<>(new ArrayList<>())).getStatus());
        }
        if (requestType == __proto_PUTsRequest.RequestType.PUTASAFE) {
            assertSame(ResponseStatus.ERROR, channel.puta(puts, new _RecordsSetCollection<>(new ArrayList<>())).getStatus());
        }
        channel.close();
    }

}
