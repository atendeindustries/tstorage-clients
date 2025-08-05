/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.util.ArrayList;
import java.util.NoSuchElementException;
import java.util.Objects;

class __proto_GETReaderInPlace<T> implements __proto_ResponseReader<ResponseGet<T>> {

    private final __proto_ProtocolInputStream in;
    private final PayloadType<T> payloadType;
    private final _RecordsSetCollection<T> records;

    __proto_GETReaderInPlace(__proto_ProtocolInputStream in, PayloadType<T> payloadType) {
        this.in = Objects.requireNonNull(in);
        this.payloadType = Objects.requireNonNull(payloadType);
        records = createRecordsStorage();
    }

    public __proto_ProtocolInputStream getGETInputStream() {
        return in;
    }

    public PayloadType<T> getPayloadType() {
        return payloadType;
    }

    public _RecordsSetCollection<T> getRecords() {
        return records;
    }

    // It's a fail fast, get as much as you can algorithm.
    public ResponseGet<T> read() {
        long acq = ResponseAcq.INVALID_ACQ;
        ResponseStatus status;
        try {
            __proto_Header header = in.readHeader();
            expectSuccess(header.result() == 0 && header.size() == 0);
            int recSize;
            while ((recSize = in.readRecSize()) > 0) {
                // TBD: Possible max rec size check
                var key = in.readKey();
                var payloadSize = recSize - Key.BYTES;
                // TBD: Possible max payload size check
                var payloadBytes = in.readPayload(payloadSize);
                // TBD: Check Optional<T> obj is null or trust users? :) (we are still safe though)
                var payload = payloadType.fromBytes(payloadBytes); // User can use NPO to keep broken records
                getRecords().append(key, payload.orElseThrow());
            }
            header = in.readHeader();
            expectSuccess(header.result() == 0 && header.size() == Key.BYTES_ACQ);
            acq = in.readAcq();
            status = ResponseStatus.OK;
        } catch(__proto_LimitExceededException ignore) {
            status = ResponseStatus.LIMIT_TOO_LOW;
        } catch (NoSuchElementException ignore) {
            status = ResponseStatus.CONVERSION_ERROR;
        } catch (Exception ignore) {
            status = ResponseStatus.ERROR;
        }
        return new ResponseGet<>(status, acq, getRecords());
    }

    protected _RecordsSetCollection<T> createRecordsStorage() {
        return new _RecordsSetCollection<>(new ArrayList<>());
    }

    protected void expectSuccess(boolean success) throws Exception {
        if(!success) {
            throw new Exception();
        }
    }

}
