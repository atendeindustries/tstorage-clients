/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

interface __net_Connection extends AutoCloseable {

    /**
     * Closes the connection.
     * Closing not connected socket is not an error.
     */
    @Override
    void close() throws IOException;

    InputStream getInputStream() throws IOException;

    OutputStream getOutputStream() throws IOException;

    void open() throws IOException;

}
