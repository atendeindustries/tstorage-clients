/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Objects;

/**
 * <p>
 * <b>Design note:</b>
 * We read as many bytes as possible on single read call. Prefer readKey1 over readKey2 style
 * when you add more functionality. In essence - don't reuse existing read methods for
 * more complex objects.
 * </p>
 * <code>
 *     Key readKey1() throws IOException {
 *        var bb = wrap(Key.Bytes);
 *        return new Key(bb.getInt(), bb.getLong(), bb.getInt(), bb.getLong(), bb.getLong());
 *     }
 *     Key readKey2() throws IOException {
 *        return new Key(readInt(), readLong(), readInt(), readLong(), readLong());
 *     }
 *  </code>
 */
class __proto_ProtocolInputStream extends _OrderedDataInputStream {

    private final Impl impl;

    __proto_ProtocolInputStream(InputStream in, ByteOrder order) {
        super(in, order);
        impl = __proto_Protocol.inputStreamPolicy == __proto_Protocol.InputStreamPolicy.InPlace
                ? new InPlace()
                : new Ahead();
    }

    long readAcq() throws IOException {
        return consume(Key.BYTES_ACQ).getLong();
    }

    __proto_Header readHeader() throws IOException {
        return impl.readHeader();
    }

    Key readKey() throws IOException {
        return impl.readKey();
    }

    Key readKeyNoAcq() throws IOException {
        return impl.readKeyNoAcq();
    }

    int readRecSize() throws IOException {
        return consume(Integer.BYTES).getInt(); // Only here hence using Integer.BYTES
    }

    /**
     * Read payload's raw data.
     * @param payloadSize How many bytes to read.
     * @return Raw payload data.
     * @throws IOException If IO operation fails.
     */
    byte[] readPayload(int payloadSize) throws IOException {
        assert payloadSize >= 0;
        return readExactly(payloadSize);
    }

    /**
     * Helper deserialization methods (one for now) complementing ByteBuffer api.
     * Extending ByteBuffer is not possible hence the "free" getKey method.
     * @param bb ByteBuffer instance to read data from.
     * @return Key instance.
     */
    static Key getKey(ByteBuffer bb, boolean withAcq) {
        Objects.requireNonNull(bb);
        return new Key(bb.getInt(), bb.getLong(), bb.getInt(), bb.getLong(), withAcq ? bb.getLong() : ResponseAcq.INVALID_ACQ);
    }

    private interface Impl {
        __proto_Header readHeader() throws IOException;
        Key readKey() throws IOException;
        Key readKeyNoAcq() throws IOException;
    }

    /**
     * Reads currently needed bytes only.
     * Reuses previous read* methods.
     * It doesn't matter much (as in our case) if the input stream was wrapped in buffered one.
     */
    private class InPlace implements Impl {
        @Override
        public __proto_Header readHeader() throws IOException {
            return new __proto_Header(readInt(), readLong());
        }

        @Override
        public Key readKey() throws IOException {
            return new Key(readInt(), readLong(), readInt(), readLong(), readLong());
        }

        @Override
        public Key readKeyNoAcq() throws IOException {
            return new Key(readInt(), readLong(), readInt(), readLong(), ResponseAcq.INVALID_ACQ);
        }
    }

    /**
     * Calculates needed bytes first then reads.
     * Each read* method tries to be independent meaning
     * it doesn't use lesser methods to create complex object.
     */
    private class Ahead implements Impl {
        @Override
        public __proto_Header readHeader() throws IOException {
            ByteBuffer bb = consume(__proto_Header.BYTES);
            return new __proto_Header(bb.getInt(), bb.getLong());
        }

        @Override
        public Key readKey() throws IOException {
            return getKey(consume(Key.BYTES), true);
        }

        @Override
        public Key readKeyNoAcq() throws IOException {
            return getKey(consume(Key.BYTES - Key.BYTES_ACQ), false);
        }
    }

}
