/*
 * Copyright 2025 Atende Industries
 */

namespace TStorage.Interfaces
{
    public delegate void GetCallback<T>(RecordsSet<T> records);

    public interface IChannel<T>
    {
        Response Close();
        Response Connect();
        ResponseGet<T> Get(Key keyMin, Key keyMax);
        ResponseAcq GetAcq(Key keyMin, Key keyMax);
        ResponseAcq GetStream(Key keyMin, Key keyMax, GetCallback<T> callback);
        Response Put(RecordsSet<T> data);
        Response Puta(RecordsSet<T> data);
        int NetTimeout { get; set; }
        int MemoryLimit { get; set; }
    }
}