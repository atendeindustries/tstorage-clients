import pytest

from tstorage_client.record import Key, Record


@pytest.mark.parametrize(
    "key,expected",
    [
        (
            Key(0, 0, 0, 0, 0),
            b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        ),
        (
            Key(1, 0, 0, 0, 0),
            b"\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
        ),
        (
            Key(1, 1, 1, 1, 1),
            b"\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00",
        ),
        (
            Key(1, 1, 1, 1),
            b"\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff",
        ),
        (
            Key.min(),
            b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x80",
        ),
        (
            Key.max(),
            b"\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f",
        ),
    ],
)
def test_key_to_bytes(key: Key, expected: bytes) -> None:
    assert key.to_bytes(with_cid=True, with_acq=True) == expected
    assert key.to_bytes(with_cid=True, with_acq=False) == expected[:24]
    assert key.to_bytes(with_cid=False, with_acq=True) == expected[4:]
    assert key.to_bytes(with_cid=False, with_acq=False) == expected[4:24]


@pytest.mark.parametrize(
    "data,expected",
    [
        (
            b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
            Key(0, 0, 0, 0, 0),
        ),
        (
            b"\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
            Key(1, 0, 0, 0, 0),
        ),
        (
            b"\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00",
            Key(1, 1, 1, 1, 1),
        ),
        (
            b"\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff",
            Key(1, 1, 1, 1),
        ),
        (
            b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x80",
            Key.min(),
        ),
        (
            b"\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f",
            Key.max(),
        ),
    ],
)
def test_key_from_bytes(data: bytes, expected: Key) -> None:
    assert Key.from_bytes(data) == expected


@pytest.mark.parametrize(
    "key,expected",
    [
        (Key(), False),
        (Key(-1, 0, 0, 0, 0), False),
        (Key(0, 0, 0, 0, 0), True),
        (Key(1, 2, 3, 4, 5), True),
        (Key(1), True),
        (Key.min(), True),
        (Key.max(), True),
    ],
)
def test_valid_key(key: Key, expected: bool) -> None:
    assert key.valid() == expected


def test_key_order() -> None:
    keys = [
        Key(0, 0, 0, 0, 0),
        Key(0, 0, 0, 0, 1),
        Key(0, 0, 0, 0, 4),
        Key(0, 0, 0, 1, 0),
        Key(0, 0, 0, 1, 1),
        Key(10, 0, 0, 0, 0),
    ]
    unordered = [keys[2], keys[1], keys[4], keys[0], keys[3], keys[5]]
    assert keys == sorted(unordered)


def test_record_order() -> None:
    records = [
        Record(Key(0, 0, 0, 0, 0), 1),
        Record(Key(0, 0, 0, 0, 0), 2),
        Record(Key(0, 0, 0, 0, 0), 3),
        Record(Key(0, 0, 4, 0, 0), 1),
        Record(Key(0, 0, 4, 1, 1), 0),
        Record(Key(1, 0, 0, 0, 0), 0),
        Record(Key(1, 0, 0, 0, 0), 4),
    ]
    unordered = [records[2], records[1], records[6], records[4], records[0], records[3], records[5]]
    assert records == sorted(unordered)
