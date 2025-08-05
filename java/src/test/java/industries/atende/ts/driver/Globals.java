/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Objects;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Consumer;

import static org.junit.jupiter.api.Assertions.*;

public class Globals {

    private static final AtomicInteger portCounter = new AtomicInteger(Globals.PORT);

    public static final int RESPONSE_BYTES_LIMIT = __proto_Protocol.INPUT_BUFFER_SIZE;
    public static final String LOCALHOST = "127.0.0.1";
    public static final ByteOrder order = ByteOrder.LITTLE_ENDIAN;
    public static final int PORT = 12345;
    public static final int SOCKET_TIMEOUT = __proto_Protocol.SOCKET_TIMEOUT; // ms
    public static final int WAIT_FOR_SERVER_TIMEOUT = 5; // ms
    public static final PayloadTypeInt PAYLOAD_TYPE_INT = new PayloadTypeInt(order);
    public static final PayloadTypeByteArray PAYLOAD_TYPE_BYTE_ARRAY = new PayloadTypeByteArray();
    public static final int MILLION = 1000000;

    public static void assertGETRequestAsBytes(byte[] bytes, Key min, Key max) {
        assertEquals(__proto_GETRequest.BYTES, bytes.length);
        var bb = ByteBuffer.wrap(bytes).order(order);
        assertEquals(__proto_GETRequest.CMD, bb.getInt()); // cmd
        assertEquals(_KeyRange.BYTES, bb.getLong()); // size
        assertEquals(min, __proto_ProtocolInputStream.getKey(bb, true));
        assertEquals(max, __proto_ProtocolInputStream.getKey(bb, true));
    }

    public static void assertGETAcqRequestAsBytes(byte[] bytes, Key min, Key max) {
        assertEquals(__proto_GETAcqRequest.BYTES, bytes.length);
        var bb = ByteBuffer.wrap(bytes).order(order);
        assertEquals(__proto_GETAcqRequest.CMD, bb.getInt()); // cmd
        assertEquals(_KeyRange.BYTES, bb.getLong()); // size
        assertEquals(min, __proto_ProtocolInputStream.getKey(bb, true));
        assertEquals(max, __proto_ProtocolInputStream.getKey(bb, true));
    }

    public static <T> void assertResponseGet(ResponseStatus status, ArrayList<Record<T>> inRecords, ResponseGet<T> responseGet, long acq) {
        assertSame(status, responseGet.getStatus());
        assertEquals(acq, responseGet.getAcq());
        var outRecords = responseGet.getData();
        assertEquals(inRecords.size(), outRecords.size());
        int i = 0;
        for (var outRec : outRecords.iterator()) {
            var inRec = inRecords.get(i++);
            assertEquals(inRec.key(), outRec.key());
            if (inRec.value().getClass().isArray()) {
                assertArrayEquals((byte[]) inRec.value(), (byte[]) outRec.value());
            } else {
                assertEquals(inRec.value(), outRec.value());
            }
        }
    }

    public static int getNextPort() throws RuntimeException {
        int nextPort = portCounter.incrementAndGet();
        if (nextPort > 65535) {
            throw new RuntimeException("Port numbers limit exceeded");
        }
        return nextPort;
    }

    public static class LocalhostServerSocket extends Thread {

        private Consumer<ServerSocket> onPreAccept = new OnPreAccept();
        private Consumer<Socket> onPostAccept = new SkipAllData();
        private Consumer<ServerSocket> onServerSocketCreated = new OnServerSocketCreated();
        private int acceptCount = 1;
        private int acceptTimeout = 1000;
        private int connectedSocketTimeout = 1000;
        private final int port;

        /**
         * Constructs localhost TCP server with given port.
         * Calls accept() once.
         * Both server and client sockets have their timeout set for 1 second.
         * Check setters methods for further server setup.
         * @param port Port number.
         */
        public LocalhostServerSocket(int port) {
            this.port = port;
        }

        /**
         * Constructs localhost TCP server with implementation assigned port.
         * Calls accept() once.
         * Both server and client sockets have their timeout set for 1 second.
         * Check setters methods for further server setup.
         * @throws RuntimeException When port limit (65535) has been exceeded.
         */
        public LocalhostServerSocket() throws RuntimeException {
            port = Globals.getNextPort();
        }

        @Override
        public void run() {
            var threadName = Thread.currentThread().getName();
            var whoami = "LocalhostServerSocket:" + port + ", thread: [" + threadName + "]";
            System.out.println(whoami + " BEGIN");
            var settings = "acceptCount=" + acceptCount + ", acceptTimeout=" + acceptTimeout
                    + ", connectedSocketTimeout=" + connectedSocketTimeout;
            System.out.println("Server settings: " + settings);
            try {
                ServerSocket ss = new ServerSocket(port);
                consume(onServerSocketCreated, ss);
                while (acceptCount != 0 && (acceptCount < 0 || acceptCount-- > 0)) {
                    System.out.println("accept counter=" + acceptCount);
                    consume(onPreAccept, ss);
                    try (Socket s = ss.accept()) {
                        consume(onPostAccept, s);
                    }
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
            System.out.println(whoami + " END");
        }

        public int getAcceptCount() {
            return acceptCount;
        }

        public int getAcceptTimeout() {
            return acceptTimeout;
        }

        public int getConnectedSocketTimeout() {
            return connectedSocketTimeout;
        }

        public int getPort() {
            return port;
        }

        /**
         * Sets accept() calls counter.
         * @param acceptCount How many connections to accept. Value < 0 for infinite loop, 0 for none, > 0 for n times
         */
        public void setAcceptCount(int acceptCount) {
            this.acceptCount = acceptCount;
        }

        public void setAcceptTimeout(int acceptTimeout) {
            this.acceptTimeout = acceptTimeout;
        }

        public void setConnectedSocketTimeout(int connectedSocketTimeout) {
            this.connectedSocketTimeout = connectedSocketTimeout;
        }

        /**
         * Execute code that's ran right after server socket had accepted the connection.
         * @param postAccept Consumer for connected Socket object.
         */
        public void setOnPostAccept(Consumer<Socket> postAccept) throws IllegalArgumentException {
            this.onPostAccept = Objects.requireNonNull(postAccept, "postAccept cannot be null");
        }

        /**
         * Execute code that's ran just before server socket calls accept().
         * @param preAccept Consumer for connected ServerSocket object.
         */
        public void setOnPreAccept(Consumer<ServerSocket> preAccept) {
            this.onPreAccept = Objects.requireNonNull(preAccept, "preAccept cannot be null");
        }

        /**
         * Execute code that's ran right after server socket had been created.
         * @param serverSocketCreated Consumer for connected ServerSocket object.
         */
        public void setOnServerSocketCreated(Consumer<ServerSocket> serverSocketCreated) {
            this.onServerSocketCreated = Objects.requireNonNull(serverSocketCreated, "serverSocketCreated cannot be null");
        }

        private <T> void consume(Consumer<T> c, T arg) {
            c.accept(arg);
        }

        public class OnPostAccept implements Consumer<Socket> {
            @Override
            public void accept(Socket socket) {
                try {
                    socket.setSoTimeout(connectedSocketTimeout);
                } catch (SocketException e) {
                    e.printStackTrace();
                }
            }
        }

        public class OnPreAccept implements Consumer<ServerSocket> {
            @Override
            public void accept(ServerSocket ss) {}
        }

        public class OnServerSocketCreated implements Consumer<ServerSocket> {
            @Override
            public void accept(ServerSocket ss) {
                try {
                    ss.setSoTimeout(acceptTimeout);
                } catch (SocketException e) {
                    e.printStackTrace();
                }
            }
        }

        public class SkipAllData extends OnPostAccept {
            @Override
            public void accept(Socket socket) {
                try {
                    super.accept(socket);
                    socket.setSoTimeout(0);
                    while (socket.getInputStream().read() != -1);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public static class BeginEndPrinter implements AutoCloseable {
        private final String message;

        public BeginEndPrinter(String message) {
            this.message = message;
            System.out.println("BEGIN: " + message);
        }

        @Override
        public void close() {
            System.out.println("END:   " + message);
        }
    }
}
