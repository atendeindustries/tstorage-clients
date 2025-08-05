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

class __proto_GETAcq {

    protected final __proto_RequestWriter<__proto_GETAcqRequest> writer;
    protected final __proto_ResponseReader<ResponseAcq> reader;

    __proto_GETAcq(
        OutputStream outputStream,
        InputStream inputStream,
        ByteOrder order
    ) throws IllegalArgumentException {
        Objects.requireNonNull(inputStream);
        Objects.requireNonNull(outputStream);
        Objects.requireNonNull(order);
        writer = new __proto_GETAcqWriterInPlace(
            new __proto_ProtocolOutputStream(
                new BufferedOutputStream(
                    outputStream,
                    __proto_GETAcqRequest.BYTES
                ),
                order)
        );
        reader = new __proto_GETAcqReaderInPlace(
            new __proto_ProtocolInputStream(
                new BufferedInputStream(
                    inputStream,
                    __proto_Header.BYTES + Key.BYTES_ACQ
                ),
                order)
        );
    }

    ResponseAcq run(Key min, Key max) {
        if (writer.write(new __proto_GETAcqRequest(min, max)) != ResponseStatus.OK) {
            return new ResponseGet<>(ResponseStatus.ERROR);
        }
        return reader.read();
    }

}
