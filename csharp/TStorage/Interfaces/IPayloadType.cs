/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Interfaces
{
    /// <summary>
    /// Defines serialization and deserialization interface for a payload of type T.
    /// </summary>
    /// <typeparam name="T"> The type of the payload to be serialized/deserialized. </typeparam>
    public interface IPayloadType<T>
    {
        /// <summary>
        /// Deserializes a byte array into a value of type T.
        /// </summary>
        /// <param name="bytes"> The byte array to deserialize. </param>
        /// <returns> Data of type T containing the deserialized value,
        /// if successful, or null if the input is invalid
        /// </returns>
        public T? FromBytes(byte[] bytes);

        /// <summary>
        /// Serializes a value of type T into a byte array.
        /// </summary>
        /// <param name="value"> The value to serialize. </param>
        /// <returns> A byte array representing the serialized form of the value. </returns>
        public byte[] ToBytes(T value);
    }
}