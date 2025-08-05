/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.util.Objects;

class __proto_GETAcqReaderInPlace implements __proto_ResponseReader<ResponseAcq> {

    protected final __proto_ProtocolInputStream in;

    __proto_GETAcqReaderInPlace(__proto_ProtocolInputStream in) {
        this.in = Objects.requireNonNull(in);
    }

    public ResponseAcq read() {
        long acq = ResponseAcq.INVALID_ACQ;
        ResponseStatus status;
        try {
            __proto_Header header = in.readHeader();
            expectSuccess(header.result() == 0 && header.size() == Key.BYTES_ACQ);
            acq = in.readAcq();
            status = ResponseStatus.OK;
        } catch (Exception ignore) {
            status = ResponseStatus.ERROR;
        }
        return new ResponseAcq(status, acq);
    }

    protected void expectSuccess(boolean success) throws Exception {
        if(!success) {
            throw new Exception();
        }
    }

}
