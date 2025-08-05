/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Test;

public class KeyTests {

    @Test
    void gettersReturnCorrectValuesSetViaConstructor() {
        Key k = new Key(1, 2, 3, 4, 5);
        assertEquals(1, k.cid());
        assertEquals(2L, k.mid());
        assertEquals(3, k.moid());
        assertEquals(4L, k.cap());
        assertEquals(5L, k.acq());
    }

    @Test
    void keyStaticMin() {
        Key k = Key.min();
        assertEquals(Key.CID_MIN_VALUE, k.cid());
        assertEquals(-9223372036854775808L, k.mid());
        assertEquals(-2147483648, k.moid());
        assertEquals(-9223372036854775808L, k.cap());
        assertEquals(-9223372036854775808L, k.acq());
    }

    @Test
    void keyStaticMax() {
        Key k = Key.max();
        assertEquals(2147483647, k.cid());
        assertEquals(9223372036854775807L, k.mid());
        assertEquals(2147483647, k.moid());
        assertEquals(9223372036854775807L, k.cap());
        assertEquals(9223372036854775807L, k.acq());
    }

    @Test
    void cidLessThanZeroConstructorThrows() {
        IllegalArgumentException e = assertThrows(IllegalArgumentException.class, () -> {
            @SuppressWarnings("unused")
            Key k = new Key(-1, 0, 0, 0, 0);
        });
        assertEquals("Cid value must not be less than 0", e.getMessage());
    }

    @Test
    void keyMinValuesViaConstructor() {
        Key k = new Key(Key.CID_MIN_VALUE, Long.MIN_VALUE, Integer.MIN_VALUE, Long.MIN_VALUE,
                Long.MIN_VALUE);
        assertEquals(Key.CID_MIN_VALUE, k.cid());
        assertEquals(-9223372036854775808L, k.mid());
        assertEquals(-2147483648, k.moid());
        assertEquals(-9223372036854775808L, k.cap());
        assertEquals(-9223372036854775808L, k.acq());
    }

    @Test
    void keyMaxValuesViaConstructor() {
        Key k = new Key(Integer.MAX_VALUE, Long.MAX_VALUE, Integer.MAX_VALUE, Long.MAX_VALUE,
                Long.MAX_VALUE);
        assertEquals(2147483647, k.cid());
        assertEquals(9223372036854775807L, k.mid());
        assertEquals(2147483647, k.moid());
        assertEquals(9223372036854775807L, k.cap());
        assertEquals(9223372036854775807L, k.acq());
    }
}
