package industries.atende.ts.driver;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.ValueSource;

import java.io.ByteArrayInputStream;
import java.io.IOException;

import static org.junit.jupiter.api.Assertions.*;

public class LimitedGETStreamInputStreamTests {

    @Test
    void testSimpleCounter() {
        var inputStream = new ByteArrayInputStream(new byte[24]);
        byte[] counter = new byte[1];
        var limited = new __proto_LimitedGETStreamInputStream(inputStream, 1, () -> {
            counter[0]++;
        });
        try {
            var bytes = limited.readAllBytes();
            assertEquals(24, bytes.length);
            assertEquals(24, counter[0]);
        } catch (IOException e) {
            fail();
        }
    }

    @Test
    void testEmptyInput() {
        var inputStream = new ByteArrayInputStream(new byte[0]);
        final int[] counter = {0};
        var limited = new __proto_LimitedGETStreamInputStream(inputStream, 1,
                () -> counter[0]++
        );
        try {
            int out = 0;
            for (int i = 0; i < 4; i++) {
                out = limited.read();
                assertEquals(-1 , out);
            }
            assertEquals(0, counter[0]);
        } catch (IOException e) {
            fail();
        }
    }

    @ParameterizedTest
    @ValueSource(ints = {-1, 0})
    void testZeroOrLessThrows(int value) {
        var inputStream = new ByteArrayInputStream(new byte[4]);
        assertThrows(IllegalArgumentException.class, () -> new __proto_LimitedGETStreamInputStream(inputStream, value, () -> {}));
    }

    @ParameterizedTest
    @ValueSource(ints = {1, 2, 4, 8})
    void testVariousLimits(int value) {
        var inputStream = new ByteArrayInputStream(new byte[]{1,2,3,4});
        final int[] counter = {0};
        var limited = new __proto_LimitedGETStreamInputStream(inputStream, value,
            () -> counter[0]++
        );
        try {
            int out = 0;
            for (int i = 0; i < 4; i++) {
                out = limited.read();
                assertEquals(i+1, out); // testing returned byte, not how many bytes was read
            }
            limited.read();
            assertEquals(4/value, counter[0]);
            for (int i = 0; i < 4; i++) {
                out = limited.read();
                assertEquals(-1, out);
            }
            assertEquals(4/value, counter[0]);

        } catch (IOException e) {
            fail();
        }
    }

    // array

    @Test
    void testArrayEmptyInput() {
        var inputStream = new ByteArrayInputStream(new byte[0]);
        final int[] counter = {0};
        var limited = new __proto_LimitedGETStreamInputStream(inputStream, 1,
                () -> counter[0]++
        );
        try {

            for (int i = 0; i < 2; i++) {
                byte[] out = new byte[i];
                int count = limited.read(out, 0, i);
                assertEquals(-1, count);
            }
            assertEquals(0, counter[0]);
        } catch (IOException e) {
            fail();
        }
    }

    @ParameterizedTest
    @ValueSource(ints = {1})
    void testArrayLimit1(int value) {
        var inputStream = new ByteArrayInputStream(new byte[]{1,2,3,4});
        final int[] counter = {0};
        var limited = new __proto_LimitedGETStreamInputStream(inputStream, value,
                () -> counter[0]++
        );
        try {
            for (int i = 0; i < 4; i++) {
                byte[] out = new byte[i];
                int count = limited.read(out, 0, i);
                assertEquals( i > 0 ? 1 : 0, count);
            }
            assertEquals(2, counter[0]);
            byte[] out = new byte[4];
            int count = limited.read(out, 0, 4);
            assertEquals(1, count);
            assertEquals(4, out[0]);
            assertEquals(0, out[1]);
            assertEquals(3, counter[0]);
            for (int i = 0; i < 4; i++) {
                count = limited.read(out, 0, 4);
                assertEquals(-1, count);
                assertEquals(4, counter[0]);
            }
        } catch (IOException e) {
            fail();
        }
    }

    @Test
    void testArrayLimit2() {
        var inputStream = new ByteArrayInputStream(new byte[]{1,2,3,4,5,6});
        final int[] counter = {0};
        var limited = new __proto_LimitedGETStreamInputStream(inputStream, 2,
                () -> counter[0]++
        );
        try {
            // Requests 0 bytes, reads 0, 2 left
            // Requests 1 byte,  reads 1, 1 left
            // Requests 2 bytes, reads 1, 0 left
            // Requests 3 bytes, callback -> counter == 1, remaining=2, reads 2, 0 left
            byte[] expected = new byte[]{0,1,1,2};
            for (int i = 0; i < 4; i++) {
                byte[] out = new byte[i];
                int count = limited.read(out, 0, i);
                assertEquals(expected[i], count);
            }
            assertEquals(1, counter[0]);
            byte[] out = new byte[4];
            // Requests 4 bytes, callback -> counter == 2, remaining=2, reads 2, 0 left
            int count = limited.read(out, 0, 4);
            assertEquals(2, count);
            assertEquals(5, out[0]);
            assertEquals(6, out[1]);
            assertEquals(2, counter[0]);
        } catch (IOException e) {
            fail();
        }
    }

}
