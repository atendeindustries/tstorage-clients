package industries.atende.ts.driver;

import org.junit.jupiter.api.Test;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

import static industries.atende.ts.driver.Globals.order;
import static org.junit.jupiter.api.Assertions.*;

class PUTsReaderInPlaceTests {

    private byte[] successBytes() {
        var bb = ByteBuffer.allocate(__proto_Header.BYTES + 2 * Key.BYTES_ACQ).order(order);
        bb.putInt(0);
        bb.putLong(2*Key.BYTES_ACQ);
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

    @Test
    void testReturnedValueIsResponseInstance() {
        var putReader = new __proto_PUTsReaderInPlace(
            new __proto_ProtocolInputStream(
                new ByteArrayInputStream(successBytes()),
                order
            )
        );
        var response = putReader.read();
        assertInstanceOf(Response.class, response);
    }

    @Test
    void testResponseError() {
        var counter = new byte[1];
        var putReader = new __proto_PUTsReaderInPlace(
            new __proto_ProtocolInputStream(
                new ByteArrayInputStream(errorBytes()),
                order
            ) {
                @Override
                long readAcq() throws IOException {
                    fail();
                    return super.readAcq();
                }

                @Override
                __proto_Header readHeader() throws IOException {
                    counter[0]++;
                    return super.readHeader();
                }
            }
        );
        var response = putReader.read();
        assertSame(ResponseStatus.ERROR, response.getStatus());
        assertEquals(1, counter[0]);
    }

    @Test
    void testResponseSuccess() {
        var counter = new byte[2];
        var putReader = new __proto_PUTsReaderInPlace(
            new __proto_ProtocolInputStream(
                new ByteArrayInputStream(successBytes()),
                order
            ) {
                @Override
                long readAcq() throws IOException {
                    counter[1]++;
                    return super.readAcq();
                }

                @Override
                __proto_Header readHeader() throws IOException {
                    counter[0]++;
                    return super.readHeader();
                }
            }

        );
        var response = putReader.read();
        assertSame(ResponseStatus.OK, response.getStatus());
        assertEquals(1, counter[0]);
        assertEquals(2, counter[1]);
    }
}