/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.util.Objects;

/**
 * @class Record
 * @brief Represents a single data entry in TStorage, consisting of a key and a value.
 *
 * Each Record contains:
 * - A {@link Key} used for identifying and ordering the record.
 * - A value of type T, which holds the actual payload.
 *
 * Both key and value must be non-null.
 *
 * @tparam T The type of the value stored in the record.
 */
public record Record<T>(Key key, T value) {

    /**
     * @brief Constructs a new Record with the given key and value.
     *
     * @param key   The {@link Key} identifying the record. Must not be null.
     * @param value The value of type T associated with the key. Must not be null.
     *
     * @throws NullPointerException if key or value is null.
     */
    public Record {
        Objects.requireNonNull(key);
        Objects.requireNonNull(value);
    }
}

