import sys
from tstorage_client.channel import Channel
from tstorage_client.payload_type import TuplePayloadType
from tstorage_client.record import Key, Record
from tstorage_client.records_set import RecordsSet
from tstorage_client.response import Response, ResponseGet
from tstorage_client.timestamp import now_ns

# Data to send
# [   cid,   mid,   moid,     cap,   payload(int, float, bool)]
# [     1,     4,      7,   now ts,  payload(10, 12.0, True)]
# [     2,     5,      8,   now ts,  payload(11, 23.0, False)]
# [     3,     6,      9,   now ts,  payload(12, 34.0, True)]
PayloadType = tuple[int, float, bool]
def generateData() -> RecordsSet[PayloadType]:
    return [
        Record(Key(1, 4, 7, now_ns()), PayloadType((10, 24.0, True))),
        Record(Key(2, 5, 8, now_ns()), PayloadType((10, 34.0, False))),
        Record(Key(3, 6, 9, now_ns()), PayloadType((10, 44.0, True)))]

def main() -> int:

    # Generate records data
    records: RecordsSet[PayloadType] = generateData()
    acq_before_put: int = now_ns()

    with Channel(host="127.0.01", port=2025, payload_type=TuplePayloadType("if?")) as channel: # "if?" corresponds to our payload(int, float, bool)
        # Putting data into TStorage
        put_response: Response = channel.put(records)
        if not put_response.is_ok():
            raise RuntimeError(f"Error: {put_response.status}, failed to put data to TStorage!")
        else:
            print("Data successfully sent to TStorage!")

        # Getting data from TStorages
        # Specify Key ranges that will cover sent records
        # Key range is right open [key_min, key_max)
        acq_after_put: int = now_ns()
        key_min = Key(cid=1, mid=4, moid=7, cap=0, acq=acq_before_put)
        key_max = Key(cid=4, mid=7, moid=10, cap=now_ns(), acq=acq_after_put)
        get_response: ResponseGet = channel.get(key_min, key_max)
        if not get_response.is_ok():
            raise RuntimeError("Failed to get data from TStorage!")
        else:
            print("Data successfully get from TStorage!")

        print("Records:")
        for r in get_response.data:
            print(f"Cid: {r.key.cid}, Mid: {r.key.mid}, Moid: {r.key.moid}, Cap: {r.key.cap}, Acq: {r.key.acq}, PayloadType( int: {r.value[0]}, float: {r.value[1]}, bool: {r.value[2]}")
    return 0


if __name__ == "__main__":
    sys.exit(main())