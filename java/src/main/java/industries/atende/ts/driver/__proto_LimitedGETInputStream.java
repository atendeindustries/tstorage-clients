/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.IOException;
import java.io.InputStream;

class __proto_LimitedGETInputStream extends __proto_LimitedInputStream {

    __proto_LimitedGETInputStream(InputStream in, long limit) {
        super(in, limit);
    }

    @Override
    protected int readExhausted() throws IOException {
        throw new __proto_LimitExceededException();
    }

    @Override
    protected int readExhausted(byte[] b, int off, int len) throws IOException {
        throw new __proto_LimitExceededException();
    }

}
