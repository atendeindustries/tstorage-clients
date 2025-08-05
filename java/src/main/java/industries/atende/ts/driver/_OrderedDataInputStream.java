/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Objects;

/**
 * Design note:
 * Didn't extend DataInputStream as it doesn't support byte order.
 * Implementing necessary types only. Extending on demand.
 * Same for DataOutputStream.
 */
class _OrderedDataInputStream extends FilterInputStream {

    private final ByteOrder order;

    protected static final String STR_INCORRECT_BYTES_COUNT = "Incorrect bytes count to recreate object";

    _OrderedDataInputStream(InputStream in, ByteOrder order) {
        super(in);
        Objects.requireNonNull(in);
        this.order = Objects.requireNonNull(order);
    }

    ByteOrder order() {
        return order;
    }

    int readInt() throws IOException {
        return consume(Integer.BYTES).getInt();
    }

    long readLong() throws IOException {
        return consume(Long.BYTES).getLong();
    }

    protected byte[] readExactly(int count) throws IOException {
        byte[] bytes = readNBytes(count);
        if (count != bytes.length) {
            throw new IOException(STR_INCORRECT_BYTES_COUNT);
        }
        return bytes;
    }

    /**
     * A convenience method for common code of all read* methods.
     * @param size How many bytes to consume.
     * @return ByteBuffer instance wrapped around required byte array and with this class instance byte order.
     * @throws IOException if readExactly fails.
     */
    protected ByteBuffer consume(int size) throws IOException {
        return ByteBuffer.wrap(readExactly(size)).order(order());
    }

}
