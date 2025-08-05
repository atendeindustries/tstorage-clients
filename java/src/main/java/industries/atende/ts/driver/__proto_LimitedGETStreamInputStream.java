/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.io.IOException;
import java.io.InputStream;
import java.util.Objects;

class __proto_LimitedGETStreamInputStream extends __proto_LimitedInputStream {

    protected final long limit;
    protected final LimitReachedCallback callback;

    __proto_LimitedGETStreamInputStream(InputStream in, long limit, LimitReachedCallback callback) {
        super(in, limit);
        this.limit = remaining;
        this.callback = Objects.requireNonNull(callback);
    }

    @Override
    protected int readExhausted() throws IOException {
        callback.onLimitReached();
        remaining = limit;
        return read();
    }

    @Override
    protected int readExhausted(byte[] b, int off, int len) throws IOException {
        callback.onLimitReached();
        remaining = limit;
        return read(b, off, len);
    }

    interface LimitReachedCallback {
        void onLimitReached() throws __proto_LimitExceededException;
    }

}
