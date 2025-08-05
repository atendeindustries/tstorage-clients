/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.util.Optional;

public class PayloadTypeByteArray implements PayloadType<byte[]> {

    public PayloadTypeByteArray() {
    }

    @Override
    public byte[] toBytes(byte[] value) {
        return value;
    }

    @Override
    public Optional<byte[]> fromBytes(byte[] buffer) {
        return Optional.ofNullable(buffer);
    }

}
