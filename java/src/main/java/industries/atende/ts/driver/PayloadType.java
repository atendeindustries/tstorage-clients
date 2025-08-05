/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

import java.util.Optional;

/**
 * @interface PayloadType
 * @brief Defines serialization and deserialization logic for a payload of type T.
 *
 * This interface is used to convert objects of type T to and from byte arrays
 * for transmission over a network or storage. Implementations must ensure that
 * the conversion is reversible where applicable.
 *
 * @tparam T The type of the payload to be serialized/deserialized.
 */
public interface PayloadType<T> {

    /**
     * @brief Serializes a value of type T into a byte array.
     *
     * @param value The value to serialize.
     * @return A byte array representing the serialized form of the value.
     */
    byte[] toBytes(T value);

    /**
     * @brief Deserializes a byte array into a value of type T.
     *
     * @param buffer The byte array to deserialize.
     * @return An Optional containing the deserialized value if successful, or empty if the input is invalid.
     */
    Optional<T> fromBytes(byte[] buffer);
}

