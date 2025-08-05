/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.*;
import java.nio.ByteOrder;
import java.util.Objects;

/**
 * The class name GET should be understood as a noun meaning
 * it describes an operation of sending the request, receiving the response
 * and returning the result to the class' object user.
 */
class __proto_GET<T> {

    protected final __proto_RequestWriter<__proto_GETRequest> writer;
    protected final __proto_ResponseReader<ResponseGet<T>> reader;

    __proto_GET(
        OutputStream outputStream,
        InputStream inputStream,
        PayloadType<T> payloadType,
        ByteOrder order,
        int responseBytesLimit
    ) throws IllegalArgumentException {
        Objects.requireNonNull(inputStream);
        Objects.requireNonNull(outputStream);
        Objects.requireNonNull(payloadType);
        Objects.requireNonNull(order);
        expectLimit(responseBytesLimit);
        writer = new __proto_GETWriterInPlace(
            new __proto_ProtocolOutputStream(
                new BufferedOutputStream(
                    outputStream,
                    __proto_GETRequest.BYTES
                ),
                order)
        );
        reader = new __proto_GETReaderInPlace<>(
            new __proto_ProtocolInputStream(
                new BufferedInputStream(
                    new __proto_LimitedGETInputStream(
                        inputStream,
                        responseBytesLimit
                    ),
                    calculateBufferedInputSize(responseBytesLimit)
                ),
                order),
            payloadType
        );
    }

    ResponseGet<T> run(Key min, Key max) {
        if (writer.write(new __proto_GETRequest(min, max)) != ResponseStatus.OK) {
            return new ResponseGet<>(ResponseStatus.ERROR);
        }
        return reader.read();
    }

    protected void expectLimit(int limit) throws IllegalArgumentException {
        /*
            Checking available heap memory is tricky with JVM.
            Much depends on -Xms and -Xmx JVM options.
            We could try to estimate, but I think it's rather user's responsibility
            to give the proper limit.
            The lib will check basic requirements.
         */
        if(limit < __proto_Protocol.MIN_GET_RESPONSE_SIZE_ERROR) {
            throw new IllegalArgumentException(__proto_Protocol.STR_LIMIT_TOO_LOW);
        }
    }

    // Must be called after expectLimit method
    protected int calculateBufferedInputSize(int limit) {
        return limit <= __proto_Protocol.INPUT_BUFFER_SIZE ? limit : (limit + 1) / 2;
    }
}
