/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.util.Objects;

class __proto_PUTsReaderInPlace implements __proto_ResponseReader<Response> {

    protected final __proto_ProtocolInputStream in;

    __proto_PUTsReaderInPlace(__proto_ProtocolInputStream in) {
        this.in = Objects.requireNonNull(in);
    }

    public Response read() {
        try {
            __proto_Header header = in.readHeader();
            expectSuccess(header.result() == 0 && header.size() == 2 * Key.BYTES_ACQ);
            in.readAcq();
            in.readAcq();
            // What do we do wit acqs?
            return new Response(ResponseStatus.OK);
        } catch (Exception ignore) {
            return new Response(ResponseStatus.ERROR);
        }

    }

    protected void expectSuccess(boolean success) throws Exception {
        if(!success) {
            throw new Exception();
        }
    }

}
