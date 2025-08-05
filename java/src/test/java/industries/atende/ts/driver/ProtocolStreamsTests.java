/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.MethodSource;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteOrder;
import java.util.stream.Stream;

import static org.junit.jupiter.api.Assertions.*;

public class ProtocolStreamsTests {

    final static ByteOrder order = Globals.order;
    ByteArrayOutputStream byteArrayOutputStream;
    __proto_ProtocolOutputStream out;

    @BeforeEach
    void setUp() {
        byteArrayOutputStream = new ByteArrayOutputStream();
        out = new __proto_ProtocolOutputStream(byteArrayOutputStream, order);
    }

    static Stream<Key> keysProvider() {
        return Stream.of(Key.min(), Key.max());
    }

    @ParameterizedTest
    @MethodSource("keysProvider")
    void testKey(Key key) {
        try {
            out.writeKey(key);
            var byteArrayInputStream = new ByteArrayInputStream(byteArrayOutputStream.toByteArray());
            var in = new __proto_ProtocolInputStream(byteArrayInputStream, order);
            var actualKey = in.readKey();
            assertEquals(key, actualKey);
        } catch (IOException e) {
            fail();
        }
    }

    @Test
    void testHeader() {
        try {
            var header = new __proto_Header(123, 321);
            out.writeHeader(header);
            var byteArrayInputStream = new ByteArrayInputStream(byteArrayOutputStream.toByteArray());
            var in = new __proto_ProtocolInputStream(byteArrayInputStream, order);
            var actualHeader = in.readHeader();
            assertEquals(header, actualHeader);
        } catch (IOException e) {
            fail();
        }
    }

    @Test
    void testPayloadSizeZero() {
        try {
            out.writePayload(new byte[]{-1, 0, 1});
            var byteArrayInputStream = new ByteArrayInputStream(byteArrayOutputStream.toByteArray());
            var in = new __proto_ProtocolInputStream(byteArrayInputStream, order);
            var actualPayload = in.readPayload(0);
            assertEquals(0, actualPayload.length);
        } catch (IOException e) {
            fail();
        }
    }

    static Stream<byte[]> payloadsProvider() {
        return Stream.of(new byte[0], new byte[]{-1, 0, 1});
    }

    @ParameterizedTest
    @MethodSource("payloadsProvider")
    void testPayload(byte[] payload) {
        try {
            out.writePayload(payload);
            var byteArrayInputStream = new ByteArrayInputStream(byteArrayOutputStream.toByteArray());
            var in = new __proto_ProtocolInputStream(byteArrayInputStream, order);
            var actualPayload = in.readPayload(payload.length);
            assertEquals(payload.length, actualPayload.length);
            assertArrayEquals( payload, actualPayload);
        } catch (IOException e) {
            fail();
        }
    }

}
