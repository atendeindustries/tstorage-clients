import pytest

from tstorage_client._channel_common import RequestHeader


@pytest.mark.parametrize(
    "header,expected",
    [
        (RequestHeader(0, 0), b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"),
        (RequestHeader(0, 4), b"\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00"),
        (RequestHeader(1, 0), b"\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"),
    ],
)
def test_request_header_to_bytes(header: RequestHeader, expected: bytes) -> None:
    assert header.to_bytes() == expected


@pytest.mark.parametrize(
    "data,expected",
    [
        (b"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", RequestHeader(0, 0)),
        (b"\x00\x00\x00\x00\x04\x00\x00\x00\x00\x00\x00\x00", RequestHeader(0, 4)),
        (b"\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", RequestHeader(1, 0)),
        (b"\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", RequestHeader(2, 0)),
    ],
)
def test_request_header_from_bytes(data: bytes, expected: RequestHeader) -> None:
    assert RequestHeader.from_bytes(data) == expected


@pytest.mark.parametrize(
    "header,expected",
    [
        (RequestHeader(0, 0), True),
        (RequestHeader(0, 4), True),
        (RequestHeader(1, 0), False),
        (RequestHeader(2, 4), False),
    ],
)
def test_request_header_is_ok(header: RequestHeader, expected: bool) -> None:
    assert header.is_ok() == expected
