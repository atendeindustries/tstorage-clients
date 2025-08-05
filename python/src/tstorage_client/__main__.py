"""TStorage example client usage"""

from argparse import ArgumentParser, Namespace
from pathlib import Path
from typing import Iterator

from tstorage_client.channel import Channel
from tstorage_client.payload_type import BytesPayloadType
from tstorage_client.record import Key, Record
from tstorage_client.records_set import RecordsSet


def load_records_from_csv(path: Path, separator: str | None = ",") -> Iterator[Record[bytes]]:
    with path.open() as file:
        for line in file:
            parts: list[str] = line.split(separator)
            key: Key = Key(*(int(p) for p in parts[:4]))
            yield Record(key, bytes.fromhex(parts[4]))


def _put_records(channel: Channel[bytes], records: RecordsSet[bytes]) -> None:
    result = channel.put(records)
    if not result:
        raise RuntimeError("Failed to put data to TStorage!")


def do_put(host: str, port: int, path: Path) -> None:
    record_size = 100
    batch_size = 100 * 1024**2 // record_size
    with Channel(host, port, BytesPayloadType()) as ch:
        records = []
        for record in load_records_from_csv(path):
            records.append(record)
            if len(records) == batch_size:
                _put_records(ch, records)
                records.clear()
        _put_records(ch, records)


def do_get(host: str, port: int, key_min: Key, key_max: Key) -> None:
    with Channel(host, port, BytesPayloadType()) as ch:
        for r in ch.get_iter(key_min, key_max):
            if isinstance(r, Record):
                k = r.key
                print(f"{k.cid},{k.mid},{k.moid},{k.cap},{k.acq},{r.value.hex()}")
            elif not r.is_ok():
                raise RuntimeError("Error during get!")


def main() -> None:
    """Entrypoint of example application"""
    parser: ArgumentParser = ArgumentParser()
    parser.add_argument("host", help="USConnector host", type=str)
    parser.add_argument("port", help="USConnector port", type=int)
    parser.add_argument("cid1", help="lower cid", type=int)
    parser.add_argument("mid1", help="lower mid", type=int)
    parser.add_argument("moid1", help="lower moid", type=int)
    parser.add_argument("cap1", help="lower cap", type=int)
    parser.add_argument("acq1", help="lower acq", type=int)
    parser.add_argument("cid2", help="upper cid", type=int)
    parser.add_argument("mid2", help="upper mid", type=int)
    parser.add_argument("moid2", help="upper moid", type=int)
    parser.add_argument("cap2", help="upper cap", type=int)
    parser.add_argument("acq2", help="upper acq", type=int)
    parser.add_argument("path", help="path to records file", type=Path)
    args: Namespace = parser.parse_args()

    do_put(args.host, args.port, args.path)

    key_min = Key(args.cid1, args.mid1, args.moid1, args.cap1, args.acq1)
    key_max = Key(args.cid2, args.mid2, args.moid2, args.cap2, args.acq2)
    do_get(args.host, args.port, key_min, key_max)


if __name__ == "__main__":
    main()
