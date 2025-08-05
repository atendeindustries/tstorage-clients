/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.IOException;
import java.net.UnknownHostException;

import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;
import static org.junit.jupiter.api.Assertions.assertThrows;

import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

public class ConnectionSocketNotConnectedTests {

    private __net_ConnectionSocket cs;

    @BeforeEach
    void createSampleConnectionSocket() {
        try {
            cs = new __net_ConnectionSocket(Globals.LOCALHOST, Globals.PORT, Globals.SOCKET_TIMEOUT);
        } catch (UnknownHostException | IllegalArgumentException e) {
            throw new RuntimeException(
                    "Couldn't create __net_ConnectionSocket instance in @BeforeEach method");
        }
    }

    @Test
    void ConnectionSocketClosingNoThrowTest() {
        assertDoesNotThrow(() -> cs.close());
    }

    @Test
    void ConnectionSocketGetInputStreamThrowsTest() {
        assertThrows(IOException.class, () -> cs.getInputStream());
    }

    @Test
    void ConnectionSocketGetOutputStreamThrowsTest() {
        assertThrows(IOException.class, () -> cs.getOutputStream());
    }

    @Test
    void ConnectionSocketOpenNoThrowTest() {
        Globals.LocalhostServerSocket server = new Globals.LocalhostServerSocket();
        try (__net_ConnectionSocket connectionSocket =
                     new __net_ConnectionSocket(Globals.LOCALHOST, server.getPort(), Globals.SOCKET_TIMEOUT)) {
            server.start();
            assertDoesNotThrow(() -> {
                Thread.sleep(200);
                connectionSocket.open();
            });
        } catch (RuntimeException | IOException e) {
            e.printStackTrace();
        }
        try {
            server.join();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }
}
