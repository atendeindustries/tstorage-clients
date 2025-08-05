/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.util.Objects;

class __proto_PUTsRequest<T> {

    private final RecordsSet<T> recordsSet;

    protected final RequestType CMD;
    protected final long size;

    __proto_PUTsRequest(RequestType CMD, RecordsSet<T> recordsSet) {
        Objects.requireNonNull(CMD);
        Objects.requireNonNull(recordsSet);
        this.CMD = CMD;
        size = 0;
        this.recordsSet = recordsSet;
    }

    RecordsSet<T> getRecordsSet() {
        return recordsSet;
    }

    enum RequestType {
        PUTSAFE(5),
        PUTASAFE(6);

        private final int value;

        RequestType(int i) {
            value = i;
        }

        public int getValue() {
            return value;
        }
    }

}
