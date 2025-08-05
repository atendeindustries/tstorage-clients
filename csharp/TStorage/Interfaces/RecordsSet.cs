/*
 * Copyright 2025 Atende Industries
 */

using System.Collections;

namespace TStorage.Interfaces
{
    public class RecordEnumerator<T> : IEnumerator<Record<T>>
    {
        public RecordEnumerator(List<Record<T>> records)
        {
            _records = records;
        }

        public Record<T> Current
        {
            get
            {
                if (_position < 0 || _position >= _records.Count)
                {
                    throw new InvalidOperationException("Enumerator is not at a valid position.");
                }
                return _records[_position];
            }
        }

        object IEnumerator.Current => Current;

        public void Dispose() { }

        public bool MoveNext() => ++_position < _records.Count;

        public void Reset() => _position = -1;

        protected List<Record<T>> _records;
        private int _position = -1;
    }

    /// <summary>
    /// Represents a collection of data entries in TStorage.
    /// </summary>
    /// <typeparam name="T"> The type of the value stored in the record. </typeparam>
    public class RecordsSet<T> : IEnumerable<Record<T>>
    {
        /// <summary>
        /// Initializes a new instance of the RecordsSet with capacity = 0.
        /// </summary>
        public RecordsSet()
        {
            _records = [];
        }

        /// <summary>
        /// Initializes a new instance of the RecordsSet with the provided capacity.
        /// </summary>
        /// <param name="capacity"> Starting capacity. </param>
        public RecordsSet(int capacity)
        {
            _records = new(capacity);
        }

        /// <summary> Returns an IEnumerator over all records in the RecordsSet. </summary>
        public IEnumerator<Record<T>> GetEnumerator()
        {
            return new RecordEnumerator<T>(_records);
        }

        /// <summary> Appends a new record to the set. </summary>
        /// <param name="record"> A new record. </param>
        public void Append(Record<T> record)
        {
            _records.Add(record);
        }

        /// <summary> Clears all data. </summary>
        public void Clear()
        {
            _records.Clear();
        }

        /// <summary> Returns the number of records currently stored. </summary>
        public int Size => _records.Count;

        IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

        protected List<Record<T>> _records;
    }
}