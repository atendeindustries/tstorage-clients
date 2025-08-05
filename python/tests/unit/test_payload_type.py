from typing import Any, TypeVar

import pytest

from tstorage_client.payload_type import (
    BytesPayloadType,
    PayloadType,
    StructPayloadType,
    UnitPayloadType,
)


T = TypeVar("T")


@pytest.mark.parametrize(
    "payload_type,value,expected",
    [
        (UnitPayloadType(), (), b""),
        (UnitPayloadType(), 17, b""),
        (BytesPayloadType(), b"", b""),
        (BytesPayloadType(), b"\x00\x14", b"\x00\x14"),
        (BytesPayloadType(), bytearray(b""), b""),
        (BytesPayloadType(), bytearray(b""), bytearray(b"")),
        (BytesPayloadType(), bytearray(b"\x00\x14"), b"\x00\x14"),
        (BytesPayloadType(), bytearray([0, 20]), b"\x00\x14"),
        (BytesPayloadType(), memoryview(b""), b""),
        (BytesPayloadType(), memoryview(b"\x00\x14"), b"\x00\x14"),
        (StructPayloadType[int]("<i"), 0, b"\x00\x00\x00\x00"),
        (StructPayloadType[int]("<i"), 1, b"\x01\x00\x00\x00"),
        (StructPayloadType[int]("<i"), 17, b"\x11\x00\x00\x00"),
        (StructPayloadType[int](">i"), 0, b"\x00\x00\x00\x00"),
        (StructPayloadType[int](">i"), 1, b"\x00\x00\x00\x01"),
        (StructPayloadType[int](">i"), 17, b"\x00\x00\x00\x11"),
        (StructPayloadType[int]("<q"), 0, b"\x00\x00\x00\x00\x00\x00\x00\x00"),
        (StructPayloadType[int]("<q"), 1, b"\x01\x00\x00\x00\x00\x00\x00\x00"),
        (StructPayloadType[int]("<q"), 17, b"\x11\x00\x00\x00\x00\x00\x00\x00"),
        (StructPayloadType[float]("<f"), 3.14, b"\xc3\xf5\x48\x40"),
        (StructPayloadType[float]("<d"), 3.14, b"\x1f\x85\xeb\x51\xb8\x1e\x09\x40"),
    ],
)
def test_struct_payload_type_to_bytes(payload_type: PayloadType[T], value: Any, expected: bytes) -> None:
    assert payload_type.to_bytes(value) == expected


@pytest.mark.parametrize(
    "payload_type,value,expected",
    [
        (UnitPayloadType(), b"", ()),
        (UnitPayloadType(), b"\x00\x00\x00\x00", ()),
        (BytesPayloadType(), b"", b""),
        (BytesPayloadType(), b"\x00\x14", b"\x00\x14"),
        (BytesPayloadType(), b"", bytearray(b"")),
        (BytesPayloadType(), b"\x00\x14", bytearray(b"\x00\x14")),
        (BytesPayloadType(), b"\x00\x14", bytearray([0, 20])),
        (BytesPayloadType(), b"s", memoryview(b"s")),
        (BytesPayloadType(), b"\x00\x14", memoryview(b"\x00\x14")),
        (StructPayloadType[int]("<i"), b"\x00\x00\x00\x00", 0),
        (StructPayloadType[float]("<f"), b"\xc3\xf5\x48\x40", 3.14),
        (StructPayloadType[float]("<d"), b"\x1f\x85\xeb\x51\xb8\x1e\x09\x40", 3.14),
    ],
)
def test_struct_payload_type_from_bytes(payload_type: PayloadType[T], value: bytes, expected: Any) -> None:
    assert payload_type.from_bytes(value) == (pytest.approx(expected) if isinstance(expected, float) else expected)


@pytest.mark.parametrize(
    "payload_type,value",
    [
        (UnitPayloadType(), ()),
        (BytesPayloadType(), b""),
        (BytesPayloadType(), b"\x00\x14"),
        (StructPayloadType[int]("<i"), 0),
        (StructPayloadType[int](">i"), 1234),
        (StructPayloadType[int]("<q"), 1234),
        (StructPayloadType[float]("<f"), 3.14),
        (StructPayloadType[float]("<d"), 3.14),
    ],
)
def test_struct_payload_type_idnetity(payload_type: PayloadType[T], value: Any) -> None:
    assert payload_type.from_bytes(payload_type.to_bytes(value)) == (
        pytest.approx(value) if isinstance(value, float) else value
    )
