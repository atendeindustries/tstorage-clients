/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteOrder;
import java.util.Objects;
import java.util.function.Function;

// Both __proto_GET and __proto_GETStream are candidates for refactoring,
// but let's not complicate things atm.
class __proto_GETStream<T> {

    protected final __proto_RequestWriter<__proto_GETRequest> writer;
    protected final __proto_GETReaderInPlace<T> reader;
    protected final Function<RecordsSet<T>, Void> userCallback;

    __proto_GETStream(
        OutputStream outputStream,
        InputStream inputStream,
        PayloadType<T> payloadType,
        ByteOrder order,
        int responseBytesLimit,
        Function<RecordsSet<T>, Void> userCallback
    ) throws IllegalArgumentException {
        Objects.requireNonNull(inputStream);
        Objects.requireNonNull(outputStream);
        Objects.requireNonNull(payloadType);
        Objects.requireNonNull(order);
        this.userCallback = Objects.requireNonNull(userCallback);
        expectLimit(responseBytesLimit);
        writer = new __proto_GETWriterInPlace(
            new __proto_ProtocolOutputStream(
                new BufferedOutputStream(
                    outputStream,
                    __proto_GETRequest.BYTES
                ),
                order
            )
        );
        __proto_LimitedGETStreamInputStream limitedInputStream = new __proto_LimitedGETStreamInputStream(
            inputStream,
            responseBytesLimit,
            this::flushRecords
        );
        BufferedInputStream bufferedInputStream = new BufferedInputStream(limitedInputStream, calculateBufferedInputSize(responseBytesLimit));
        __proto_ProtocolInputStream GETInputStream = new __proto_ProtocolInputStream(bufferedInputStream, order);
        reader = new __proto_GETReaderInPlace<>(GETInputStream, payloadType);

    }

    ResponseGet<T> run(Key min, Key max) {
        if (writer.write(new __proto_GETRequest(min, max)) != ResponseStatus.OK) {
            return new ResponseGet<>(ResponseStatus.ERROR);
        }
        // There might be some records left before next limit hit
        // so we must do one more flush.
        var responseAcq = reader.read();
        flushRecords();
        return responseAcq;
    }

    protected int calculateBufferedInputSize(int limit) {
        return limit <= __proto_Protocol.INPUT_BUFFER_SIZE ? limit : (limit + 1) / 2;
    }

    protected void expectLimit(int limit) throws IllegalArgumentException {
        // At least one record maybe?
        // if(limit < __proto_Protocol.MIN_GET_RESPONSE_SIZE_SUCCESS + Integer.BYTES + Key.BYTES + 1) {
        if(limit <= 0) {
            throw new IllegalArgumentException(__proto_Protocol.STR_LIMIT_TOO_LOW);
        }
    }

    protected void flushRecords() {
        if(reader.getRecords().size() > 0) {
            userCallback.apply(reader.getRecords());
            reader.getRecords().clear();
        }
    }

}
