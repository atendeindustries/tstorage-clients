/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Objects;

class __proto_LimitedInputStream extends FilterInputStream {

    protected long remaining;

    __proto_LimitedInputStream(InputStream in, long limit) {
        super(in);
        Objects.requireNonNull(in);
        if(limit <= 0) { // limit is a jar user value
            throw new IllegalArgumentException(__proto_Protocol.STR_LIMIT_TOO_LOW);
        }
        remaining = limit;
    }

    @Override
    public int read() throws IOException {
        if (remaining <= 0) return readExhausted();
        int result = super.read();
        if (result != -1) remaining--;
        return result;
    }

    @Override
    public int read(byte[] b, int off, int len) throws IOException {
        if (remaining <= 0) return readExhausted(b, off, len);
        int bytesRead = super.read(b, off, (int) Math.min(len, remaining));
        if (bytesRead > 0) remaining -= bytesRead;
        return bytesRead;
    }

    protected int readExhausted() throws IOException {
        return -1;
    }

    protected int readExhausted(byte[] b, int off, int len) throws IOException {
        return -1;
    }

}
