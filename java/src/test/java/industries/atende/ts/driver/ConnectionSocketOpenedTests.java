/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import org.junit.jupiter.api.AfterAll;
import org.junit.jupiter.api.BeforeAll;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

public class ConnectionSocketOpenedTests {

    private static Thread server;
    private static __net_ConnectionSocket connection;

    @BeforeAll
    static void startServer() {
        final int port = Globals.getNextPort();
        server = new Thread(() -> {
            var message = "ServerSocket:" + port + ", Th[" + Thread.currentThread().getName() + "]";
            try (var ignored = new Globals.BeginEndPrinter(message);
                 ServerSocket ss = new ServerSocket(port);
                 Socket s = ss.accept()) {
                while (s.getInputStream().read() != -1) ;
            } catch (Exception e) {
                e.printStackTrace();
            }
        });
        server.start();
        int sleptFor = 0;
        while (true) { // The Javatrix
            try {
                if (connection == null) {
                    connection = new __net_ConnectionSocket(Globals.LOCALHOST, port, Globals.SOCKET_TIMEOUT);
                }
                Thread.sleep(Globals.WAIT_FOR_SERVER_TIMEOUT);
                connection.open();
                break;
            } catch (Exception e) {
                ++sleptFor;
            }
        }
        System.out.println("slept for " + sleptFor * Globals.WAIT_FOR_SERVER_TIMEOUT + "ms");
    }

    @AfterAll
    static void stopServer() {
        try {
            connection.close();
            server.join();
        } catch (IOException | InterruptedException e) {
            fail();
        }
    }

    @Test
    void ConnectionSocketGetInputStreamNoThrowTest() {
        assertDoesNotThrow(() -> {
            connection.getInputStream();
        });
    }

    @Test
    void ConnectionSocketGetOutputStreamNoThrowTest() {
        assertDoesNotThrow(() -> {
            connection.getOutputStream();
        });
    }

    @Test
    void ConnectionSocketSecondOpenNoThrowTest() {
        assertDoesNotThrow(() -> connection.open());
    }

}
