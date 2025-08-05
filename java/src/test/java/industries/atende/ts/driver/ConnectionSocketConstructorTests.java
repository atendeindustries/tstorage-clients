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

public class ConnectionSocketConstructorTests {

    @Test
    void ConnectionSocketValidArgsNoThrowTest() {
        assertDoesNotThrow(() -> {
            try (__net_ConnectionSocket ignore = new __net_ConnectionSocket(Globals.LOCALHOST, Globals.PORT, Globals.SOCKET_TIMEOUT)) {
            }
        });
    }

    @ParameterizedTest
    @ValueSource(
            strings = {"", "0", "0.0", "0.0.0", "localhost", "127.0.0.1", "::1", "0:0:0:0:0:0:0:1"})
    void ConnectionSocketValidFirstArgNoThrowTest(String host) {
        assertDoesNotThrow(() -> {
            try (__net_ConnectionSocket ignore = new __net_ConnectionSocket(host, Globals.PORT, Globals.SOCKET_TIMEOUT)) {
            }
        });
    }

    @Test
    void ConnectionSocketNullFirstArgThrowsTest() {
        assertThrows(NullPointerException.class, () -> {
            try (__net_ConnectionSocket ignore = new __net_ConnectionSocket(null, Globals.PORT, Globals.SOCKET_TIMEOUT)) {
            }
        });
    }

    @ParameterizedTest
    @ValueSource(strings = {" ", "....", "0.0.0.", "0.0.0.0.0", "127.0.0.0/8", "::1/128"})
    void ConnectionSocketInvalidFirstArgThrowsTest(String host) {
        assertThrows(UnknownHostException.class, () -> {
            try (__net_ConnectionSocket ignore = new __net_ConnectionSocket(host, Globals.PORT, Globals.SOCKET_TIMEOUT)) {
            }
        });
    }

    @Test
    void ConnectionSocketInvalidMinSecondArgThrowsTest() {
        assertThrows(IllegalArgumentException.class, () -> {
            try (__net_ConnectionSocket ignore = new __net_ConnectionSocket(Globals.LOCALHOST, -1, Globals.SOCKET_TIMEOUT)) {
            }
        });
    }

    @Test
    void ConnectionSocketInvalidMaxSecondArgThrowsTest() {
        assertThrows(IllegalArgumentException.class, () -> {
            try (__net_ConnectionSocket ignore = new __net_ConnectionSocket(Globals.LOCALHOST, 256 * 256, Globals.SOCKET_TIMEOUT)) {
            }
        });
    }

}
