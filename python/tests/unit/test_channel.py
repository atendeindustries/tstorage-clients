import functools
import struct
from typing import Generic, TypeVar

import pytest

from tstorage_client._channel_common import ReceiveBuffer, RequestHeader
from tstorage_client.channel import Channel
from tstorage_client.payload_type import StructPayloadType
from tstorage_client.record import Key, Record
from tstorage_client.records_set import RecordsSet
from tstorage_client.response import Response, ResponseAcq, ResponseGet, ResponseStatus


T = TypeVar("T")


TSTORAGE_HOST = "localhost"
TSTORAGE_PORT = 62025


def early_close_mock(response: ResponseAcq) -> ResponseAcq:
    return response


class Collector(Generic[T]):
    def __init__(self) -> None:
        self.records: list[Record[T]] = []
        self.called: int = 0
        self._back_buffer = bytearray()

    def receive(self, records: RecordsSet[T]) -> None:
        self.records.extend(records)
        self.called += 1


class BufferFeeder:
    """Simulate TCP buffer."""

    def __init__(self) -> None:
        self._back_buffer = bytearray()

    def feed(self, buffer: ReceiveBuffer, limit: int = 0, data: bytes = b"") -> int:
        amount: int = buffer.free_len() if limit == 0 or limit > buffer.free_len() else limit
        self._back_buffer += data
        limited_data = self._back_buffer[:amount]
        del self._back_buffer[:amount]
        buffer.feed(limited_data)
        return len(limited_data)


@pytest.fixture
def records() -> RecordsSet[int]:
    return [
        Record(Key(0, 0, 0, 1234, 1235), 1),
        Record(Key(0, 0, 1, 1234, 1235), 11),
        Record(Key(0, 0, 2, 1234, 1235), 12),
        Record(Key(0, 0, 3, 1234, 1235), 22),
        Record(Key(0, 1, 0, 1234, 1235), 255),
        Record(Key(1, 0, 0, 1234, 1235), 1024),
    ]


@pytest.fixture
def channel() -> Channel[int]:
    payload_type = StructPayloadType[int]("<i")
    return Channel(TSTORAGE_HOST, TSTORAGE_PORT, payload_type)


def test_channel_connect_fail(channel: Channel[int]) -> None:
    assert not channel.connect().is_ok()


def test_channel_with_fail(channel: Channel[int]) -> None:
    with pytest.raises(OSError):
        with channel:
            pass


def test_channel_no_connect(channel: Channel[int]) -> None:
    assert not channel.get(Key.min(), Key.max()).is_ok()
    assert not channel.get_acq(Key.min(), Key.max()).is_ok()
    assert not channel.get_stream(Key.min(), Key.max(), lambda _: None).is_ok()
    iter = channel.get_iter(Key.min(), Key.max())
    match e := next(iter):
        case ResponseAcq():
            assert not e.is_ok()
        case Record():
            raise RuntimeError("Impossible condition")
    assert not channel.put([]).is_ok()
    assert not channel.puta([]).is_ok()


@pytest.mark.parametrize(
    "response,expected",
    [
        (b"", Response(ResponseStatus.DISCONNECTED)),
        (RequestHeader(0, 16).to_bytes() + b"\x00" * 16, Response(ResponseStatus.OK)),
        (RequestHeader(0, 20).to_bytes() + b"\x00" * 20, Response(ResponseStatus.OK)),
        (RequestHeader(0, 16).to_bytes() + b"\x00" * 20, Response(ResponseStatus.OK)),
        (RequestHeader(1, 0).to_bytes(), Response(ResponseStatus.ERROR)),
        (RequestHeader(1, 8).to_bytes() + b"\x00" * 8, Response(ResponseStatus.ERROR)),
        (RequestHeader(1, 8).to_bytes() + b"\x00" * 16, Response(ResponseStatus.ERROR)),
        (RequestHeader(123, 0).to_bytes(), Response(ResponseStatus.ERROR)),
    ],
)
def test_channel_puts(
    channel: Channel[int],
    records: RecordsSet[int],
    response: bytes,
    expected: Response,
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    feeder = BufferFeeder()
    monkeypatch.setattr(channel, "_socket", ())
    monkeypatch.setattr(channel, "_send_data", lambda _: None)
    monkeypatch.setattr(channel, "_feed_buffer", functools.partial(feeder.feed, data=response))
    assert channel.put(records) == expected
    assert channel.puta(records) == expected


@pytest.mark.parametrize(
    "response,expected",
    [
        (b"", ResponseAcq(ResponseStatus.DISCONNECTED)),
        (RequestHeader(0, 8).to_bytes() + b"\x00" * 8, ResponseAcq(ResponseStatus.OK, 0)),
        (RequestHeader(0, 20).to_bytes() + b"\x00" * 20, ResponseAcq(ResponseStatus.OK, 0)),
        (RequestHeader(0, 16).to_bytes() + b"\x00" * 20, ResponseAcq(ResponseStatus.OK, 0)),
        (RequestHeader(0, 8).to_bytes() + b"\x01" + b"\x00" * 7, ResponseAcq(ResponseStatus.OK, 1)),
        (RequestHeader(0, 8).to_bytes() + b"\xff" + b"\x00" * 7, ResponseAcq(ResponseStatus.OK, 255)),
        (RequestHeader(1, 0).to_bytes(), ResponseAcq(ResponseStatus.ERROR)),
        (RequestHeader(1, 8).to_bytes() + b"\x00" * 8, ResponseAcq(ResponseStatus.ERROR)),
        (RequestHeader(1, 8).to_bytes() + b"\x00" * 16, ResponseAcq(ResponseStatus.ERROR)),
        (RequestHeader(123, 0).to_bytes(), ResponseAcq(ResponseStatus.ERROR)),
    ],
)
def test_channel_get_acq(
    channel: Channel[int], response: bytes, expected: ResponseAcq, monkeypatch: pytest.MonkeyPatch
) -> None:
    feeder = BufferFeeder()
    monkeypatch.setattr(channel, "_socket", ())
    monkeypatch.setattr(channel, "_send_data", lambda _: None)
    monkeypatch.setattr(channel, "_feed_buffer", functools.partial(feeder.feed, data=response))
    assert channel.get_acq(Key.min(), Key.max()) == expected


@pytest.mark.parametrize(
    "response,expected,memory",
    [
        (
            b"",
            ResponseGet(ResponseStatus.DISCONNECTED),
            None,
        ),
        (
            RequestHeader(0, 0).to_bytes() + b"\x00" * 4 + RequestHeader(0, 8).to_bytes() + b"\x00" * 8,
            ResponseGet(ResponseStatus.OK, 0, []),  # type: ignore[arg-type]
            None,
        ),
        (
            RequestHeader(0, 0).to_bytes() + b"\x00" * 4 + RequestHeader(1, 0).to_bytes(),
            ResponseGet(ResponseStatus.ERROR),
            None,
        ),
        (
            RequestHeader(0, 0).to_bytes() + b"\x00" * 4 + RequestHeader(2, 0).to_bytes(),
            ResponseGet(ResponseStatus.ERROR),
            None,
        ),
        (
            RequestHeader(0, 0).to_bytes() + b"\x00" * 4 + RequestHeader(1, 0).to_bytes() + b"\x00" * 8,
            ResponseGet(ResponseStatus.ERROR),
            None,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 36)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseGet(ResponseStatus.OK, 16, [Record(Key(1, 2, 3, 4, 5), 4)]),
            None,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 35)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseGet(ResponseStatus.UNPARSEABLE_ENTITY),
            None,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 37)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseGet(ResponseStatus.UNPARSEABLE_ENTITY),
            None,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 36)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseGet(ResponseStatus.NO_MEMORY, data=[Record(Key(1, 2, 3, 4, 5), 4)]),
            75,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 36)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 36)
            + Key(2, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 8)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseGet(ResponseStatus.OK, 16, [Record(Key(1, 2, 3, 4, 5), 4), Record(Key(2, 2, 3, 4, 5), 8)]),
            200,
        ),
    ],
)
def test_channel_get(
    channel: Channel[int],
    response: bytes,
    expected: ResponseGet[int],
    memory: int | None,
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    feeder = BufferFeeder()
    monkeypatch.setattr(channel, "_socket", ())
    monkeypatch.setattr(channel, "_send_data", lambda _: None)
    monkeypatch.setattr(channel, "_feed_buffer", functools.partial(feeder.feed, data=response))
    monkeypatch.setattr(channel, "_early_close", early_close_mock)
    channel.memory_limit = memory
    assert channel.get(Key.min(), Key.max()) == expected


@pytest.mark.parametrize(
    "response,expected,memory,called,recs",
    [
        (
            b"",
            ResponseAcq(ResponseStatus.DISCONNECTED),
            None,
            0,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes() + b"\x00" * 4 + RequestHeader(0, 8).to_bytes() + b"\x00" * 8,
            ResponseAcq(ResponseStatus.OK, 0),
            None,
            0,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes() + b"\x00" * 4 + RequestHeader(1, 0).to_bytes(),
            ResponseAcq(ResponseStatus.ERROR),
            None,
            0,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes() + b"\x00" * 4 + RequestHeader(2, 0).to_bytes(),
            ResponseAcq(ResponseStatus.ERROR),
            None,
            0,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes() + b"\x00" * 4 + RequestHeader(1, 0).to_bytes() + b"\x00" * 8,
            ResponseAcq(ResponseStatus.ERROR),
            None,
            0,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 36)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseAcq(ResponseStatus.OK, 16),
            None,
            1,
            1,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 35)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseAcq(ResponseStatus.UNPARSEABLE_ENTITY),
            None,
            0,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 37)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseAcq(ResponseStatus.UNPARSEABLE_ENTITY),
            None,
            0,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 36)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseAcq(ResponseStatus.OK, 16),
            76,
            1,
            1,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 36)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 36)
            + Key(2, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 8)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseAcq(ResponseStatus.OK, 16),
            200,
            1,
            2,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 36)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 36)
            + Key(2, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 8)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseAcq(ResponseStatus.OK, 16),
            52,
            2,
            2,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 36)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseAcq(ResponseStatus.NO_MEMORY),
            51,
            0,
            0,
        ),
    ],
)
def test_channel_get_stream(
    channel: Channel[int],
    response: bytes,
    expected: ResponseAcq,
    memory: int | None,
    called: int,
    recs: int,
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    feeder = BufferFeeder()
    monkeypatch.setattr(channel, "_socket", ())
    monkeypatch.setattr(channel, "_send_data", lambda _: None)
    monkeypatch.setattr(channel, "_feed_buffer", functools.partial(feeder.feed, data=response))
    monkeypatch.setattr(channel, "_early_close", early_close_mock)
    channel.memory_limit = memory
    collector = Collector[int]()
    assert channel.get_stream(Key.min(), Key.max(), collector.receive) == expected
    assert collector.called == called
    assert len(collector.records) == recs


@pytest.mark.parametrize(
    "response,expected,memory,recs",
    [
        (
            b"",
            ResponseAcq(ResponseStatus.DISCONNECTED),
            None,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes() + b"\x00" * 4 + RequestHeader(0, 8).to_bytes() + b"\x00" * 8,
            ResponseAcq(ResponseStatus.OK, 0),
            None,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes() + b"\x00" * 4 + RequestHeader(1, 0).to_bytes(),
            ResponseAcq(ResponseStatus.ERROR),
            None,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes() + b"\x00" * 4 + RequestHeader(2, 0).to_bytes(),
            ResponseAcq(ResponseStatus.ERROR),
            None,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes() + b"\x00" * 4 + RequestHeader(1, 0).to_bytes() + b"\x00" * 8,
            ResponseAcq(ResponseStatus.ERROR),
            None,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 36)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseAcq(ResponseStatus.OK, 16),
            None,
            1,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 35)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseAcq(ResponseStatus.UNPARSEABLE_ENTITY),
            None,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 37)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseAcq(ResponseStatus.UNPARSEABLE_ENTITY),
            None,
            0,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 36)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseAcq(ResponseStatus.NO_MEMORY),
            75,
            1,
        ),
        (
            RequestHeader(0, 0).to_bytes()
            + struct.pack("<i", 36)
            + Key(1, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 4)
            + struct.pack("<i", 36)
            + Key(2, 2, 3, 4, 5).to_bytes()
            + struct.pack("<i", 8)
            + struct.pack("<i", 0)
            + RequestHeader(0, 8).to_bytes()
            + struct.pack("<q", 16),
            ResponseAcq(ResponseStatus.OK, 16),
            200,
            2,
        ),
    ],
)
def test_channel_get_iter(
    channel: Channel[int],
    response: bytes,
    expected: ResponseAcq,
    memory: int | None,
    recs: int,
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    feeder = BufferFeeder()
    monkeypatch.setattr(channel, "_socket", ())
    monkeypatch.setattr(channel, "_send_data", lambda _: None)
    monkeypatch.setattr(channel, "_feed_buffer", functools.partial(feeder.feed, data=response))
    monkeypatch.setattr(channel, "_early_close", early_close_mock)
    channel.memory_limit = memory
    results = list(channel.get_iter(Key.min(), Key.max()))
    records = [r for r in results if isinstance(r, Record)]
    status = [s for s in results if isinstance(s, ResponseAcq)][0]
    assert status == expected
    assert len(records) == recs
