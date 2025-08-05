/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.Objects;

/**
 * When compared to java.net.Socket a constructor with host and port parameters
 * does not initiate the connection like java.net.Socket does. Call open() to connect.
 * In order to re-connect one must first call close() and then open().
 * Also when host is null it doesn't use the loopback address but throws instead.
 */
final class __net_ConnectionSocket implements __net_Connection {

    private final String host;
    private final int port;
    private __net_Connection stateImpl;
    private Socket socket;
    private final int timeout;

    __net_ConnectionSocket(String host, int port, int timeout)
            throws UnknownHostException, IllegalArgumentException {
        Objects.requireNonNull(host, "Host must not be null");
        InetSocketAddress isa = new InetSocketAddress(host, port);
        if (isa.isUnresolved()) {
            throw new UnknownHostException();
        }
        this.host = host;
        this.port = port;
        this.timeout = timeout;
        stateImpl = new Created();
    }

    /**
     * Created ----> Opened <----+
     *         \                  \
     *          +----> Closed <----+
     */
    @Override
    public void close() throws IOException {
        stateImpl.close();
        stateImpl = new Closed();
    }

    @Override
    public InputStream getInputStream() throws IOException {
        return stateImpl.getInputStream();
    }

    @Override
    public OutputStream getOutputStream() throws IOException {
        return stateImpl.getOutputStream();
    }

    @Override
    public void open() throws IOException {
        stateImpl.open();
        stateImpl = new Opened();
    }

    private final class Closed implements __net_Connection {

        @Override
        public void close() {
        }

        @Override
        public InputStream getInputStream() throws IOException {
            // throws SocketException with "Socket is closed"
            return socket.getInputStream();
        }

        @Override
        public OutputStream getOutputStream() throws IOException {
            // throws SocketException with "Socket is closed"
            return socket.getOutputStream();
        }

        @Override
        public void open() throws IOException {
            socket = new Socket(host, port);
            socket.setSoTimeout(timeout);
        }

    }

    private final class Created implements __net_Connection {

        public Created() {
            socket = new Socket();
        }

        @Override
        public void close() throws IOException {
            socket.close();
        }

        @Override
        public InputStream getInputStream() throws IOException {
            // throws SocketException with "Socket is not connected"
            return socket.getInputStream();
        }

        @Override
        public OutputStream getOutputStream() throws IOException {
            // throws SocketException with "Socket is not connected"
            return socket.getOutputStream();
        }

        @Override
        public void open() throws IOException {
            socket = new Socket(host, port);
            socket.setSoTimeout(timeout);
        }

    }

    private final class Opened implements __net_Connection {

        @Override
        public void close() throws IOException {
            socket.close();
        }

        @Override
        public InputStream getInputStream() throws IOException {
            return socket.getInputStream();
        }

        @Override
        public OutputStream getOutputStream() throws IOException {
            return socket.getOutputStream();
        }

        @Override
        public void open() {
        }

    }

}
