/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Objects;

/**
 * Design note:
 * Didn't extend DataOutputStream as it doesn't support byte order.
 * Implementing necessary types only. Extending on demand.
 * Same for DataInputStream.
 */
class _OrderedDataOutputStream extends FilterOutputStream {

    private final ByteOrder order;

    _OrderedDataOutputStream(OutputStream out, ByteOrder order) {
        super(out);
        Objects.requireNonNull(out);
        this.order = Objects.requireNonNull(order);
    }

    ByteOrder order() {
        return order;
    }

    void writeInt(int value) throws IOException {
        write(allocate(Integer.BYTES).putInt(value).array());
    }

    void writeLong(long value) throws IOException {
        write(allocate(Long.BYTES).putLong(value).array());
    }

    /**
     * A convenience method for common code of all write* methods.
     * @param size How much memory to allocate in bytes.
     * @return ByteBuffer instance with allocated byte array and effective byte order.
     * @throws IllegalArgumentException - If the capacity is a negative integer
     */
    protected ByteBuffer allocate(int size) throws IllegalArgumentException {
        return ByteBuffer.allocate(size).order(order());
    }

}
