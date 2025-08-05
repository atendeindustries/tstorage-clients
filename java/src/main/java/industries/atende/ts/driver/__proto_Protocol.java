/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

class __proto_Protocol {

    // To serialize protocol data use {GET|PUT...}OutputStream objects
    // To deserialize protocol data use {GET|PUT...}InputStream objects

    static final int OUTPUT_BUFFER_SIZE = 67108928;
    static final int INPUT_BUFFER_SIZE = 67108928; // TBD: Should devs be able to tweak it? - imho yes
    static final int DEFAULT_PAYLOAD_SIZE = 8;
    static final int MAX_PAYLOAD_SIZE = 32 * 1024 * 1024;
    static final int MAX_RECORD_SIZE = Key.BYTES + MAX_PAYLOAD_SIZE;
    static final int MIN_GET_RESPONSE_SIZE_ERROR = __proto_Header.BYTES + Integer.BYTES + __proto_Header.BYTES;
    static final int MIN_GET_RESPONSE_SIZE_SUCCESS = __proto_Header.BYTES + Integer.BYTES + __proto_Header.BYTES + Long.BYTES;
    static final int SOCKET_TIMEOUT = 10000;
    static final String STR_LIMIT_TOO_LOW = "Limit too low";

    enum InputStreamPolicy {
        InPlace,
        Ahead
    }

    static InputStreamPolicy inputStreamPolicy = InputStreamPolicy.Ahead;
}
