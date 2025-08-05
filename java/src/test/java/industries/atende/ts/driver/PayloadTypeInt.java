/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Objects;
import java.util.Optional;

public class PayloadTypeInt implements PayloadType<Integer> {

    private final ByteOrder order;

    public PayloadTypeInt(ByteOrder order) {
        this.order = Objects.requireNonNull(order, "Order must not be null");
    }

    @Override
    public byte[] toBytes(Integer value) {
        return ByteBuffer.allocate(Integer.BYTES).order(order).putInt(value).array();
    }

    @Override
    public Optional<Integer> fromBytes(byte[] buffer) {
        return Optional.of(ByteBuffer.wrap(buffer).order(order).getInt());
    }

}
