/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.nio.ByteOrder;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;
import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assumptions.assumeTrue;

public class GETWriterReaderAndStreamsTests {

    static ByteOrder order = Globals.order;

    @Test
    void testReadResponseGetSimplestSuccess() {
        var builder = new GETResponseInputBuilder(order);
        builder.useSuccessFirstHeader();
        var inRecords = builder.generateRecords(1, 4);
        builder.useSuccessSecondHeader();
        final long acq = Key.max().acq();
        builder.setAcq(acq);
        var socket = new ByteArrayInputStream(builder.build());
        var in = new __proto_ProtocolInputStream(socket, order);
        var bulk = new __proto_GETReaderInPlace<>(in, Globals.PAYLOAD_TYPE_BYTE_ARRAY);
        var responseGet = bulk.read(); //in.readResponse(new PayloadTypeByteArray());
        assertSame(ResponseStatus.OK, responseGet.getStatus());
        assertEquals(acq, responseGet.getAcq());
        var outRecords = responseGet.getData();
        assertEquals(inRecords.size(), outRecords.size());
        int i = 0;
        for (var outRec : outRecords.iterator()) {
            var inRec = inRecords.get(i++);
            assertEquals(inRec.key(), outRec.key());
            assertArrayEquals(inRec.value(), outRec.value());
        }
    }

    @Test
    void testReadResponseGetZeroLengthInput() {
        var socket = new ByteArrayInputStream(new byte[0]);
        var in = new __proto_ProtocolInputStream(socket, order) {
            @Override
            __proto_Header readHeader() throws IOException {
                throw assertThrows(IOException.class, super::readHeader);
            }
        };
        var bulk = new __proto_GETReaderInPlace<>(in, Globals.PAYLOAD_TYPE_BYTE_ARRAY) {
            @Override
            protected void expectSuccess(boolean success) throws Exception {
                fail("This method must not be called");
                super.expectSuccess(success);
            }
        };
        var responseGet = bulk.read();
        assertSame(ResponseStatus.ERROR, responseGet.getStatus());
        assumeTrue(responseGet.getAcq() == -1);
        var outRecords = responseGet.getData();
        assertEquals(0, outRecords.size());
    }

    @Test
    void testReadResponseGetFirstHeaderBadResult() {
        var builder = new GETResponseInputBuilder(order);
        builder.setFirstHeader(-1, 0);
        builder.generateRecords(1, 4);
        builder.useSuccessSecondHeader();
        var socket = new ByteArrayInputStream(builder.build());
        var in = new __proto_ProtocolInputStream(socket, order) {
            @Override
            __proto_Header readHeader() throws IOException {
                var header = super.readHeader();
                assertEquals(-1, header.result());
                assertEquals(0, header.size());
                return header;
            }

            @Override
            int readRecSize() {
                fail("This method must not be called");
                return -1;
            }
        };
        var bulk = new __proto_GETReaderInPlace<>(in, Globals.PAYLOAD_TYPE_BYTE_ARRAY) {
            @Override
            protected void expectSuccess(boolean success) throws Exception {
                assertFalse(success);
                throw assertThrows(Exception.class, () -> super.expectSuccess(success));
            }
        };
        var responseGet = bulk.read();
        assertSame(ResponseStatus.ERROR, responseGet.getStatus());
        assumeTrue(responseGet.getAcq() == -1);
        var outRecords = responseGet.getData();
        assertEquals(0, outRecords.size());
    }

    @Test
    void testReadResponseGetFirstHeaderBadSize() {
        var builder = new GETResponseInputBuilder(order);
        builder.setFirstHeader(0, -1);
        builder.generateRecords(1, 4);
        builder.useSuccessSecondHeader();
        var socket = new ByteArrayInputStream(builder.build());
        var in = new __proto_ProtocolInputStream(socket, order) {
            @Override
            __proto_Header readHeader() throws IOException {
                var header = super.readHeader();
                assertEquals(0, header.result());
                assertEquals(-1, header.size());
                return header;
            }

            @Override
            int readRecSize() {
                fail("This method must not be called");
                return -1;
            }
        };
        var bulk = new __proto_GETReaderInPlace<>(in, Globals.PAYLOAD_TYPE_BYTE_ARRAY) {
            @Override
            protected void expectSuccess(boolean success) throws Exception {
                assertFalse(success);
                throw assertThrows(Exception.class, () -> super.expectSuccess(success));
            }
        };
        var responseGet = bulk.read();
        assertSame(ResponseStatus.ERROR, responseGet.getStatus());
        assumeTrue(responseGet.getAcq() == -1);
        var outRecords = responseGet.getData();
        assertEquals(0, outRecords.size());
    }

    @Test
    void testReadResponseGetOnlyTerminatingRecSize() {
        var builder = new GETResponseInputBuilder(order);
        builder.useSuccessFirstHeader();
        builder.generateRecords(0, 4);
        builder.useSuccessSecondHeader();
        builder.setAcq(12345);
        var socket = new ByteArrayInputStream(builder.build());
        var in = new __proto_ProtocolInputStream(socket, order) {
            int recSizeCounter = 0;
            int headerCounter = 0;

            @Override
            __proto_Header readHeader() throws IOException {
                __proto_Header header;
                if (headerCounter++ == 0) {
                    header = super.readHeader();
                    assertEquals(0, header.result());
                    assertEquals(0, header.size());
                } else if (headerCounter++ == 2) {
                    header = super.readHeader();
                    assertEquals(0, header.result());
                    assertEquals(Key.BYTES_ACQ, header.size());
                } else {
                    header = null;
                    fail();
                }
                return header;
            }

            @Override
            long readAcq() throws IOException {
                var acq = super.readAcq();
                assertEquals(12345, acq);
                return acq;
            }

            @Override
            Key readKey() {
                return fail("Must not be called");
            }

            @Override
            int readRecSize() throws IOException {
                if (recSizeCounter >= 1) {
                    fail("Must be called only once");
                }
                var recSize = super.readRecSize();
                assertEquals(0, recSize);
                recSizeCounter++;
                return recSize;
            }
        };
        var bulk = new __proto_GETReaderInPlace<>(in, Globals.PAYLOAD_TYPE_BYTE_ARRAY) {
            @Override
            protected void expectSuccess(boolean success) throws Exception {
                assertTrue(success);
                assertDoesNotThrow(() -> super.expectSuccess(success));
            }
        };
        var responseGet = bulk.read();
        assertSame(ResponseStatus.OK, responseGet.getStatus());
        assertEquals(12345, responseGet.getAcq());
        var outRecords = responseGet.getData();
        assertEquals(0, outRecords.size());
    }

}
