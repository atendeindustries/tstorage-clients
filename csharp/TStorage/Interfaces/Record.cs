/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Interfaces
{
    /// <summary>
    /// Represents a single data entry in TStorage, consisting of a key and a value.
    /// </summary>
    /// <typeparam name="T"> The type of the value stored in the record. </typeparam>
    public readonly struct Record<T>
    {
        /// <summary>
        /// Initializes a new instance of the Record.
        /// </summary>
        /// <param name="key"> A key used for identifying and ordering the record. </param>
        /// <param name="value"> A value of type T, which holds the actual payload. </param>
        public Record(Key key, T value)
        {
            Key = key;
            Value = value;
        }

        /// <summary>
        /// A key used for identifying and ordering the record.
        /// </summary>
        public readonly Key Key;

        /// <summary>
        /// A value of type T, which holds the actual payload.
        /// </summary>
        public readonly T Value;
    }
}