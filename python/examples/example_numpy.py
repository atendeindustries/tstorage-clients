import numpy as np

from tstorage_client.channel import Channel
from tstorage_client.payload_type import NumpyPayloadType
from tstorage_client.record import Key
from tstorage_client.timestamp import now_ns


def main() -> None:
    host = "127.0.0.1"  # TStorage host
    port = 2025  # TStorage port

    # Data setup part
    rng = np.random.Generator(np.random.Philox(4))  # Arbitrarily chosen source of data
    count: int = 1024 * 1024  # Just count of records
    internal_size = np.zeros(count, dtype=np.int32)  # Preallocate _size buffer
    cid = rng.integers(0, 1, count, dtype=np.int32)  # Part of the key
    mid = rng.integers(0, 1024, count, dtype=np.int64)  # Part of the key
    moid = rng.integers(0, 1024, count, dtype=np.int32)  # Part of the key
    cap = rng.integers(2000, 2200, count, dtype=np.int64)  # Part of the key
    f0 = rng.integers(0, 1024, count, dtype=np.int32)  # First payload column
    f1 = rng.integers(0, 1024, count, dtype=np.int32)  # Second payload column
    f2 = rng.uniform(0, 1024, count)  # Third payload column
    columns = [internal_size, cid, mid, moid, cap, f0, f1, f2]
    names = "_size, cid, mid, moid, cap, f0, f1, f2"  # In this example we rely on implicit payload column names for simplicity
    data = np.rec.fromarrays(columns, names=names)

    # Collect only cap and f2 from records while doing get discarding all other data
    received_caps = []
    received_f2 = []

    def records_filter(data: list[np.ndarray]) -> None:
        for d in data:
            received_caps.append(d["cap"])
            received_f2.append(d["f2"])

    # Using Numpy is as simple as using NumpyPayloadType as payload type
    # Here we only define types with implicit column names by simple string
    # You can also use list of name -> type tuples, eg. [("my_int", np.int32)]
    payload_type_serialzier = NumpyPayloadType("i,i,d")
    with Channel(host, port, payload_type_serialzier, memory_limit=1024**2) as channel:
        acq_before_put: int = now_ns()
        put_response = channel.put(data)
        acq_after_put: int = now_ns()
        if not put_response.is_ok():  # Remember to always check for errors
            raise RuntimeError("Failed to put data to TStorage!")
        key_min = Key(0, 0, 0, 0, acq_before_put)
        key_max = Key(1024, 1024, 1024, now_ns(), acq_after_put)
        get_response = channel.get_stream(key_min, key_max, records_filter)
        if not get_response.is_ok():  # Remember to always check for errors
            raise RuntimeError("Failed to get data from TStorage!")

    caps_arr = np.concatenate(received_caps, dtype=np.int64)
    f2_arr = np.concatenate(received_f2, dtype=np.float64)
    print(caps_arr.shape, caps_arr)
    print(f2_arr.shape, f2_arr)


if __name__ == "__main__":
    main()
