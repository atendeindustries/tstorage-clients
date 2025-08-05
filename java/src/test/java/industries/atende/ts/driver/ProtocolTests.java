/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertTrue;

public class ProtocolTests {
    @Test
    void testInputSize() {
        assertTrue(__proto_Protocol.INPUT_BUFFER_SIZE >= 0);
    }

    @Test
    void testMinGETResponseSize() {
        assertTrue(__proto_Protocol.MIN_GET_RESPONSE_SIZE_ERROR > 0);
    }
}
