import struct
from typing import Any

import pytest

from tstorage_client._channel_common import (
    ReceiveBuffer,
    RecordsParsingStatus,
    RequestHeader,
    _ChannelMixin,
    _CommandType,
)
from tstorage_client.payload_type import StructPayloadType
from tstorage_client.record import Key, Record
from tstorage_client.records_set import RecordsSet


@pytest.mark.parametrize(
    "cmd,key_min,key_max,expected",
    [
        (
            _CommandType.GET,
            Key.min(),
            Key.max(),
            b"\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x80\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f",
        ),
        (
            _CommandType.GETACQ,
            Key.min(),
            Key.max(),
            b"\x07\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x80\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f",
        ),
    ],
)
def test_mixin_prepare_keyrange_request(cmd: _CommandType, key_min: Key, key_max: Key, expected: bytes) -> None:
    assert _ChannelMixin._prepare_keyrange_request(cmd, key_min, key_max) == expected


@pytest.mark.parametrize(
    "data,expected",
    [
        (
            b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
            Record(Key(0, 0, 0, 0, 0), 0),
        ),
        (
            b"\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x0a\x00\x00\x00",
            Record(Key(1, 0, 0, 0, 0), 10),
        ),
        (
            b"\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x03\x00\x00\x00",
            Record(Key(1, 1, 1, 1, 1), 3),
        ),
        (
            b"\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x01\x00\x00\x00",
            Record(Key(1, 1, 1, 1), 1),
        ),
        (
            b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00\x00\x00\x80\x0e\x00\x00\x00",
            Record(Key.min(), 14),
        ),
        (
            b"\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\xff\xff\xff\xff\xff\xff\xff\x7f\x00\x00\x00\x01",
            Record(Key.max(), 16777216),
        ),  #
        (
            b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff",
            None,
        ),
        (
            b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xff\xff\xff\xff\xff",
            None,
        ),
        (
            b"\x00",
            None,
        ),
        (
            b"",
            None,
        ),
        (
            b"qwertyuiop",
            None,
        ),
        (
            b"\xff\xff\xff\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00",
            None,
        ),
    ],
)
def test_mixin_parse_record(data: bytes, expected: Record[int] | None) -> None:
    payload_type = StructPayloadType[int]("<i")
    assert _ChannelMixin._parse_record(data, payload_type) == expected


def test_mixin_group_records_by_cid() -> None:
    records = [
        Record(Key(1, 1, 0, 1234), 0),
        Record(Key(0, 0, 2, 1234), 0),
        Record(Key(10, 0, 0, 10), 0),
        Record(Key(10, 1, 0, 11), 0),
        Record(Key(1, 0, 0, 1234), 0),
        Record(Key(0, 0, 0, 1234), 0),
        Record(Key(0, 0, 1, 1234), 0),
    ]
    records_grouped = {
        0: [Record(Key(0, 0, 2, 1234), 0), Record(Key(0, 0, 0, 1234), 0), Record(Key(0, 0, 1, 1234), 0)],
        1: [Record(Key(1, 1, 0, 1234), 0), Record(Key(1, 0, 0, 1234), 0)],
        10: [Record(Key(10, 0, 0, 10), 0), Record(Key(10, 1, 0, 11), 0)],
    }
    assert _ChannelMixin._group_records_by_cid(records) == records_grouped


@pytest.mark.parametrize(
    "records,expected",
    [
        ([Record(Key(1, 1, 0, 1234), 0)], 1),
        ([Record(Key(1, 0, 0, 1234), 0), Record(Key(1, 1, 0, 1234), 0)], 1),
        ([Record(Key(1, 0, 0, 1234), 0), Record(Key(2, 1, 0, 1234), 0)], 2),
    ],
)
def test_mixin_serialize_records_iter(records: RecordsSet[int], expected: int) -> None:
    payload_type = StructPayloadType[int]("<i")
    raw = list(_ChannelMixin._serialize_records_batches_iter(records, True, payload_type))
    assert len(raw) == expected


@pytest.mark.parametrize(
    "data,format,expected",
    [
        (b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", "", (RequestHeader(0, 0), ())),
        (b"\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", "", (RequestHeader(1, 0), ())),
        (b"\xff\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", "", (RequestHeader(255, 0), ())),
        (b"\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", "<i", (RequestHeader(0, 4), (0,))),
        (b"\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00", "<i", (RequestHeader(0, 4), (8,))),
        (
            b"\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00",
            "<q",
            (RequestHeader(0, 4), (8,)),
        ),
        (
            b"\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00",
            "<q",
            (RequestHeader(0, 8), (8,)),
        ),
        (
            b"\x00\x00\x00\x00\x08\x00\x00\x00\x00\x00\x00\x00\x08\x00\x00\x00\x01\x00\x00\x00",
            "<ii",
            (RequestHeader(0, 8), (8, 1)),
        ),
        (b"\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04", "<b", (RequestHeader(0, 1), (4,))),
        (b"\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x04", "<c", (RequestHeader(0, 1), (b"\x04",))),
        (b"", "", None),
        (b"\x00\x00\x00\x00", "", None),
        (b"\x00\x00\x00\x00", "i", None),
        (b"\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00", "", None),
        (b"\x00\x00\x00\x00\x06\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", "<i", None),
    ],
)
def test_mixin_handle_response(data: bytes, format: str, expected: tuple[RequestHeader, tuple[Any, ...]]) -> None:
    buffer = ReceiveBuffer()
    buffer.feed(data)
    assert _ChannelMixin._handle_response(buffer, format) == expected


@pytest.mark.parametrize(
    "size,key,value,expected,add",
    [
        (36, Key(0, 0, 0, 0, 0), 0, RecordsParsingStatus.FINISHED, b"\x00\x00\x00\x00"),
        (32, Key(0, 0, 0, 0, 0), b"", RecordsParsingStatus.UNPARSEABLE, b"\x00\x00\x00\x00"),
        (40, Key(0, 0, 0, 0, 0), 0, RecordsParsingStatus.UNPARSEABLE, b"\x00\x00\x00\x00"),
        (34, Key(0, 0, 0, 0, 0), 0, RecordsParsingStatus.UNPARSEABLE, b"\x00\x00\x00\x00"),
        (40, Key(0, 0, 0, 0, 0), b"\x00" * 8, RecordsParsingStatus.UNPARSEABLE, b"\x00\x00\x00\x00"),
        (36, Key(-1, 0, 0, 0, 0), 0, RecordsParsingStatus.UNPARSEABLE, b"\x00\x00\x00\x00"),
        (200, Key(0, 0, 0, 0, 0), b"", RecordsParsingStatus.NEEDS_MORE_BYTES, b""),
        (36, Key(0, 0, 0, 0, 0), 0, RecordsParsingStatus.NEEDS_MORE_BYTES, b"\x36\x00\x00\x00"),
        (36, Key(0, 0, 0, 0, 0), 0, RecordsParsingStatus.NEEDS_MORE_BYTES, b"\x36\x00\x00\x00\xff\xff\xff\xff"),
        (36, Key(0, 0, 0, 0, 0), 0, RecordsParsingStatus.NEEDS_MORE_BYTES, b"\x02\x00\x00\x00"),
        (40, Key(0, 0, 0, 0, 0), 0, RecordsParsingStatus.NEEDS_MORE_BYTES, b""),
    ],
)
def test_mixin_parse_records(
    size: int, key: Key, value: int | bytes, expected: RecordsParsingStatus, add: bytes
) -> None:
    parser = struct.Struct("<i")
    payload_type = StructPayloadType[int]("<i")
    records: list[Record[int]] = []
    data = value if isinstance(value, bytes) else parser.pack(value)
    data = b"".join((parser.pack(size), key.to_bytes(), data, add))
    buffer = ReceiveBuffer()
    buffer.feed(data)
    assert _ChannelMixin._parse_records(buffer, records, payload_type, None) == expected
