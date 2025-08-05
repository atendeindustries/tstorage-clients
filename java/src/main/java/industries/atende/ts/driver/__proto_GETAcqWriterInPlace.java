/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.IOException;
import java.util.Objects;

class __proto_GETAcqWriterInPlace implements __proto_RequestWriter<__proto_GETAcqRequest> {

    private final __proto_ProtocolOutputStream out;

    __proto_GETAcqWriterInPlace(__proto_ProtocolOutputStream out) {
        this.out = Objects.requireNonNull(out);
    }

    @Override
    public ResponseStatus write(__proto_GETAcqRequest request) {
        try {
            out.writeInt(__proto_GETAcqRequest.CMD);
            out.writeLong(_KeyRange.BYTES);
            out.writeKey(request.min());
            out.writeKey(request.max());
            out.flush();
            return ResponseStatus.OK;
        } catch (IOException e) {
            return ResponseStatus.ERROR;
        }
    }

}
