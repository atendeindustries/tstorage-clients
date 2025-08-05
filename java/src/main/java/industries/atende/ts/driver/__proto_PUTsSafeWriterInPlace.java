/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteOrder;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

class __proto_PUTsSafeWriterInPlace<T> implements __proto_RequestWriter<__proto_PUTsRequest<T>> {

    protected final __proto_ProtocolOutputStream out;
    protected final PayloadType<T> payloadType;
    protected final ByteOrder order;

    __proto_PUTsSafeWriterInPlace(__proto_ProtocolOutputStream out, PayloadType<T> payloadType, ByteOrder order) {
        this.out = Objects.requireNonNull(out);
        this.payloadType = Objects.requireNonNull(payloadType);
        this.order = Objects.requireNonNull(order);
    }

    public ResponseStatus write(__proto_PUTsRequest<T> request) {
        Objects.requireNonNull(request);
        try {
            out.writeCmd(request.CMD.getValue());
            out.writeSize(request.size);
            for(var entry : preWrite(request).entrySet()) {
                out.writeCid(entry.getKey()); // cid
                byte[] batchBytes = entry.getValue().inner.toByteArray();
                out.writeBatchSize(batchBytes.length);
                out.writeBatch(batchBytes);
            }
            out.writeCid(-1); // stream terminating cid
            out.flush();
            return ResponseStatus.OK;
        } catch (Exception ignore) {
            return ResponseStatus.ERROR;
        }
    }

    protected Map<Integer, OutStreams> preWrite(__proto_PUTsRequest<T> request) throws IOException {
        final Map<Integer, OutStreams> map = new HashMap<>();
        for (var record : request.getRecordsSet().iterator()) {
            OutStreams out = map.computeIfAbsent(
                record.key().cid(),
                k -> new OutStreams(order)
            );
            byte[] payload = payloadType.toBytes(record.value());
            int recSize = switch (request.CMD) {
                case PUTSAFE -> Key.BYTES - Key.BYTES_CID - Key.BYTES_ACQ + payload.length;
                case PUTASAFE -> Key.BYTES - Key.BYTES_CID + payload.length;
            };
            out.outer.writeRecSize(recSize);
            out.outer.writeMid(record.key().mid());
            out.outer.writeMoid(record.key().moid());
            out.outer.writeCap(record.key().cap());
            if(request.CMD == __proto_PUTsRequest.RequestType.PUTASAFE) {
                out.outer.writeAcq(record.key().acq());
            }
            out.outer.writePayload(payload);
        }
        return map;
    }

    final static class OutStreams {
        public final __proto_ProtocolOutputStream outer;
        public final ByteArrayOutputStream inner;

        OutStreams(ByteOrder order) {
            inner = new ByteArrayOutputStream();
            outer = new __proto_ProtocolOutputStream(inner, order);
        }
    }

}