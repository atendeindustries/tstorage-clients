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

class __proto_PUTs<T> {

    protected __proto_RequestWriter<__proto_PUTsRequest<T>> writer;
    protected final __proto_ResponseReader<Response> reader;
    protected final __proto_PUTsRequest.RequestType requestType;
    protected final PayloadType<T> payloadType;
    protected OutputStream outputStream;
    protected InputStream inputStream;
    protected ByteOrder order;

    __proto_PUTs(
        __proto_PUTsRequest.RequestType requestType,
        OutputStream outputStream,
        InputStream inputStream,
        PayloadType<T> payloadType,
        ByteOrder order
    ) {
        this.requestType = Objects.requireNonNull(requestType);
        this.outputStream = Objects.requireNonNull(outputStream);
        this.inputStream = Objects.requireNonNull(inputStream);
        this.payloadType = Objects.requireNonNull(payloadType);
        this.order = Objects.requireNonNull(order);
        writer = null;
        reader = new __proto_PUTsReaderInPlace(
            new __proto_ProtocolInputStream(
                new BufferedInputStream(
                    this.inputStream,
                    __proto_Header.BYTES + 2 * Key.BYTES_ACQ
                ),
                this.order
            )
        );
    }

    __proto_PUTs(
        __proto_PUTsRequest.RequestType requestType,
        __proto_RequestWriter<__proto_PUTsRequest<T>> writer,
        __proto_ResponseReader<Response> reader,
        PayloadType<T> payloadType
    ) {
        this.requestType = Objects.requireNonNull(requestType);
        this.writer = Objects.requireNonNull(writer);
        this.reader = Objects.requireNonNull(reader);
        this.payloadType = Objects.requireNonNull(payloadType);
    }

    Response run(RecordsSet<T> data) {
        Objects.requireNonNull(data);
        if (writer == null) {
            writer = new __proto_PUTsSafeWriterInPlace<>(
                new __proto_ProtocolOutputStream(
                    new BufferedOutputStream(
                        outputStream,
                        __proto_Protocol.OUTPUT_BUFFER_SIZE
                    ),
                    order),
                payloadType,
                order
            );
        }
        if (writer.write(new __proto_PUTsRequest<>(requestType, data)) != ResponseStatus.OK) {
            return new Response(ResponseStatus.ERROR);
        }
        return reader.read();
    }

}
