/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.IOException;
import java.util.Objects;

class __proto_GETWriterInPlace implements __proto_RequestWriter<__proto_GETRequest> {

    private final __proto_ProtocolOutputStream out;

    __proto_GETWriterInPlace(__proto_ProtocolOutputStream out) {
        this.out = Objects.requireNonNull(out);
    }

    /**
     * Sends GET request.
     * <p>
     * Comment on code design:
     * Why not put it into GETOutputStream class as void writeRequest(__proto_GETRequest request)?
     * Well... no... I wanted to separate what you can lay out from how.
     * </p>
     */
    @Override
    public ResponseStatus write(__proto_GETRequest request) {
        try {
            out.writeInt(__proto_GETRequest.CMD);
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
