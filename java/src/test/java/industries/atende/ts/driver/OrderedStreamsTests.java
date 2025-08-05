/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.ValueSource;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteOrder;

import static org.junit.jupiter.api.Assertions.*;

public class OrderedStreamsTests {

    final static ByteOrder order = Globals.order;
    ByteArrayOutputStream byteArrayOutputStream;
    _OrderedDataOutputStream out;

    @BeforeEach
    void setUp() {
        byteArrayOutputStream = new ByteArrayOutputStream();
        out = new _OrderedDataOutputStream(byteArrayOutputStream, order);
    }

    @Test
    void testAllocateSizeNegative() {
        assertThrows(IllegalArgumentException.class, () -> out.allocate(-1));
    }

    @ParameterizedTest
    @ValueSource(ints = {1, 2, 3, Integer.BYTES, Long.BYTES, Key.BYTES, __proto_Header.BYTES, __proto_GETRequest.BYTES})
    void testAllocate(int size) {
        var bb = out.allocate(size);
        assertNotNull(bb);
        assertEquals(size, bb.capacity());
        assertEquals(size, bb.array().length);
    }

    @Test
    void testAllocateOrder() {
        assertEquals(order, out.allocate(4).order());
    }

    @ParameterizedTest
    @ValueSource(ints = {Integer.MIN_VALUE, 0, Integer.MAX_VALUE})
    void testInt(int value) {
        try {
            out.writeInt(value);
            var bytes = byteArrayOutputStream.toByteArray();
            var byteArrayInputStream = new ByteArrayInputStream(bytes);
            var in = new _OrderedDataInputStream(byteArrayInputStream, order);
            assertEquals(value, in.readInt());
        } catch (IOException e) {
            fail();
        }
    }

    @ParameterizedTest
    @ValueSource(longs = {Long.MIN_VALUE, 0L, Long.MAX_VALUE})
    void testLong(long value) {
        try {
            out.writeLong(value);
            var bytes = byteArrayOutputStream.toByteArray();
            var byteArrayInputStream = new ByteArrayInputStream(bytes);
            var in = new _OrderedDataInputStream(byteArrayInputStream, order);
            assertEquals(value, in.readLong());
        } catch (IOException e) {
            fail();
        }
    }



}
