/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Random;

/**
 * This is more like a tool than a real builder as each method should ideally
 * return this instance.
 */
class GETResponseInputBuilder {

    private final ByteOrder order;
    private int firstResult = 0;
    private long firstSize = 0;
    private int secondResult = 0;
    private long secondSize = 0;
    private final ArrayList<Record<byte[]>> records;
    private int recordsTotalSize = 0;
    private long acq = -1;
    private boolean responseError = false;

    public GETResponseInputBuilder(ByteOrder order) {
        this.order = order;
        records = new ArrayList<>();
    }

    public void setFirstHeader(int result, long size) {
        firstResult = result;
        firstSize = size;
    }

    public void setSecondHeader(int result, long size) {
        secondResult = result;
        secondSize = size;
    }

    public long getAcq() {
        return acq;
    }

    public void setAcq(long acq) {
        this.acq = acq;
    }

    /**
     * Generates n records where keys are auto generated in a way that
     * each key piece is incremented by one in each dimension all starting from zero.
     * Each payload is an array of randomly generated n=payloadSize bytes.
     */
    public ArrayList<Record<byte[]>> generateRecords(int count, int payloadSize) {
        int cid = 0; long mid =0; int moid = 0; long cap = 0; long acq = 0;
        var random = new Random();
        while(count --> 0) {
            byte[] payload = new byte[payloadSize];
            random.nextBytes(payload);
            records.add(new Record<>(new Key(cid++, mid++, moid++, cap++, acq++), payload));
            recordsTotalSize += Key.BYTES + payloadSize;
        }
        return records;
    }

    public void useErrorSecondHeader() {
        secondResult = 1;
        secondSize = 0;
        responseError = true;
    }

    public void useSuccessFirstHeader() {
        firstResult = 0;
        firstSize = 0;
        responseError = false;
    }

    public void useSuccessSecondHeader() {
        secondResult = 0;
        secondSize = Key.BYTES_ACQ;
        responseError = false;
    }

    public byte[] build() {
        // After GET request we receive two responses thus two headers
        var firstHeaderSize = __proto_Header.BYTES;
        var recSizeFieldSize = Integer.BYTES;
        var keySize = Key.BYTES;
        var payloadSize = Integer.BYTES;
        var recordSize = keySize + payloadSize;
        var secondHeaderSize = __proto_Header.BYTES;
        var acqSize = responseError ? 0 : Long.BYTES;
        var bb = ByteBuffer.allocate(firstHeaderSize + recSizeFieldSize * records.size() + recordsTotalSize + recSizeFieldSize
                + secondHeaderSize + acqSize).order(order);
        bb.putInt(firstResult);
        bb.putLong(firstSize);
        for(var rec : records) {
            bb.putInt(Key.BYTES + rec.value().length); // recSize = key size in bytes + payload size
            __proto_ProtocolOutputStream.putKey(bb, rec.key(), true);
            bb.put(rec.value()); // payload
        }
        bb.putInt(0); // terminating recSize=0
        bb.putInt(secondResult);
        bb.putLong(secondSize);
        if(!responseError) {
            bb.putLong(acq);
        }
        return bb.array();
    }

}
