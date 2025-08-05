/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Objects;

class __proto_ProtocolOutputStream extends _OrderedDataOutputStream {

    __proto_ProtocolOutputStream(OutputStream out, ByteOrder order) {
        super(out, order);
    }

    // Wrappers like this were added to simplify unit testing
    void writeAcq(long value) throws IOException {
        writeLong(value);
    }

    void writeBatch(byte[] b) throws IOException {
        write(b);
    }

    void writeBatchSize(int value) throws IOException {
        writeInt(value);
    }

    void writeCap(long value) throws IOException {
        writeLong(value);
    }

    void writeCid(int value) throws IOException {
        writeInt(value);
    }

    void writeCmd(int value) throws IOException {
        writeInt(value);
    }

    void writeHeader(__proto_Header header) throws IOException {
        Objects.requireNonNull(header);
        var bb = allocate(__proto_Header.BYTES);
        write(putHeader(bb, header).array());
    }

    void writeKey(Key key) throws IOException {
        Objects.requireNonNull(key);
        var bb = allocate(Key.BYTES);
        write(putKey(bb, key, true).array());
    }

    void writeKeyNoAcq(Key key) throws IOException {
        Objects.requireNonNull(key);
        var bb = allocate(Key.BYTES - Key.BYTES_ACQ);
        write(putKey(bb, key, false).array());
    }

    void writeMid(long value) throws IOException {
        writeLong(value);
    }

    void writeMoid(int value) throws IOException {
        writeInt(value);
    }

    void writePayload(byte[] payload) throws IOException {
        Objects.requireNonNull(payload);
        write(payload);
    }

    void writeRecSize(int value) throws IOException {
        writeInt(value);
    }

    // header.size
    void writeSize(long value) throws IOException {
        writeLong(value);
    }

    /**
     * Helper serialization methods complementing ByteBuffer api.
     * Extending ByteBuffer is not possible. Doing it the other way around.
     * Why not define:
     * <code>
     *     void writeInt(ByteBuffer byteBuffer, int value) {
     *         byteBuffer.putInt(value);
     *     }
     *     void writeLong(ByteBuffer byteBuffer, long value) {
     *         byteBuffer.putLong(value);
     *     }
     *     void writeKey(ByteBuffer byteBuffer, Key key) {
     *         writeInt(byteBuffer, key.getCid());
     *         writeLong(byteBuffer, key.getMid());
     *         writeInt(byteBuffer, key.getMoid());
     *         writeLong(byteBuffer, key.getCap());
     *         writeLong(byteBuffer, key.getAcq());
     *     }
     * </code>
     * ?
     * Well, basically it would work and even lay out well,
     * but the "write" verb suggests you are going to call an IO operation,
     * and it was misleading, so I was forced to add comments what each write
     * method actually does.
     */
    static ByteBuffer putHeader(ByteBuffer bb, __proto_Header header) {
        Objects.requireNonNull(bb);
        Objects.requireNonNull(header);
        bb.putInt(header.result());
        bb.putLong(header.size());
        return bb;
    }

    /**
     * <b>Byte order is not changed for both in and out buffers.</b>
     */
    static ByteBuffer putKey(ByteBuffer bb, Key key, boolean withAcq) {
        Objects.requireNonNull(bb);
        Objects.requireNonNull(key);
        bb.putInt(key.cid());
        bb.putLong(key.mid());
        bb.putInt(key.moid());
        bb.putLong(key.cap());
        if(withAcq) {
            bb.putLong(key.acq());
        }
        return bb;
    }

}
