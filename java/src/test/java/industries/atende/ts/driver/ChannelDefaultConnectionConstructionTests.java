/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.net.UnknownHostException;

import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.ValueSource;

public class ChannelDefaultConnectionConstructionTests {

    @Test
    void ChannelConstructorValidArgsNoThrowTest() {
        assertDoesNotThrow(() -> {
            new Channel<>(Globals.LOCALHOST, Globals.PORT, Globals.SOCKET_TIMEOUT, Globals.PAYLOAD_TYPE_INT, Globals.order);
        });
    }

    @ParameterizedTest
    @ValueSource(
            strings = {"", "0", "0.0", "0.0.0", "localhost", "127.0.0.1", "::1", "0:0:0:0:0:0:0:1"})
    void ChannelConstructorValidFirstArgNoThrowTest(String host) {
        assertDoesNotThrow(() -> {
            // Empty string should resolve to localhost
            new Channel<>(host, Globals.PORT, Globals.SOCKET_TIMEOUT, Globals.PAYLOAD_TYPE_INT, Globals.order);
        });
    }

    @Test
    void ChannelConstructorNullFirstArgThrowsTest() {
        assertThrows(NullPointerException.class, () -> new Channel<>(null, Globals.PORT, Globals.SOCKET_TIMEOUT, Globals.PAYLOAD_TYPE_INT, Globals.order));
    }

    @ParameterizedTest
    @ValueSource(strings = {" ", "....", "0.0.0.", "0.0.0.0.0", "127.0.0.0/8", "::1/128"})
    void ChannelConstructorInvalidFirstArgThrowsTest(String host) {
        assertThrows(UnknownHostException.class, () -> new Channel<>(host, Globals.PORT, Globals.SOCKET_TIMEOUT, Globals.PAYLOAD_TYPE_INT, Globals.order));
    }

    @Test
    void ChannelConstructorInvalidMinSecondArgThrowsTest() {
        assertThrows(IllegalArgumentException.class, () -> new Channel<>(Globals.LOCALHOST, -1, Globals.SOCKET_TIMEOUT, Globals.PAYLOAD_TYPE_INT, Globals.order));
    }

    @Test
    void ChannelConstructorInvalidMaxSecondArgThrowsTest() {
        assertThrows(IllegalArgumentException.class, () -> new Channel<>(Globals.LOCALHOST, 256 * 256, Globals.SOCKET_TIMEOUT, Globals.PAYLOAD_TYPE_INT, Globals.order));
    }

    @Test
    void ChannelConstructorNullThirdArgThrowsTest() {
        assertThrows(NullPointerException.class, () -> new Channel<>(Globals.LOCALHOST, Globals.PORT, Globals.SOCKET_TIMEOUT, null, Globals.order));
    }

}
