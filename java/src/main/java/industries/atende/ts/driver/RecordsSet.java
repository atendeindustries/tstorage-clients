/**
 * Copyright 2025 Atende Industries
 */
package industries.atende.ts.driver;

/**
 * @interface RecordsSet
 * @brief Represents a collection of records, each consisting of a {@link Key} and a value of type T.
 *
 * Provides methods for appending records, iterating over them, and querying the size of the collection.
 *
 * @tparam T The type of values stored in the records.
 */
public interface RecordsSet<T> {

     /**
      * @brief Appends a new record with the given key and value to the set.
      *
      * @param key   The {@link Key} identifying the record. Must not be null.
      * @param value The value associated with the key. Must not be null.
      *
      * @throws NullPointerException if key or value is null.
      */
     void append(Key key, T value);

     /**
      * @brief Returns an iterable over all records in the set.
      *
      * @return An {@link Iterable} of {@link Record} instances.
      */
     Iterable<Record<T>> iterator();

     /**
      * @brief Returns the number of records in the set.
      *
      * @return The size of the set.
      */
     int size();
}

