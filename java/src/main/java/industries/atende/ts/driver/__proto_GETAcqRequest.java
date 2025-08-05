/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

record __proto_GETAcqRequest(Key min, Key max) {

    static final int BYTES = __proto_Header.BYTES + _KeyRange.BYTES;
    static final int CMD = 7;

}
