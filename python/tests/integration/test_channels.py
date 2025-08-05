"""Integration tests.

This module contains tests that work with real network connection.
They can connect to dev TStorage, dockered TStorage, in-memory mocked TStorage
and even production TStorage. Please be cautious running those tests.
To run this test module it is necessary to pass '-m integration' to pytest
as it is disabled by default in pyproject.toml. At module level guard fixture
is called. It checks if TStorage is empty before doing anything
(unfortunelly with client being tested).
"""

import asyncio
import dataclasses
import random
import time
from typing import Any, Generic, TypeVar

import pytest

from tstorage_client.channel import Channel
from tstorage_client.channel_async import AsyncChannel
from tstorage_client.payload_type import (
    BytesPayloadType,
    StructPayloadType,
    UnitPayloadType,
)
from tstorage_client.record import Key, Record
from tstorage_client.records_set import RecordsSet
from tstorage_client.response import ResponseAcq


pytestmark = pytest.mark.integration


TSTORAGE_HOST = "localhost"
TSTORAGE_PORT = 62025


T = TypeVar("T")


def get_stream_callback(records: RecordsSet[int]) -> None:
    pass


class Collector(Generic[T]):
    def __init__(self) -> None:
        self.records: list[Record[T]] = []
        self.called: int = 0

    def receive(self, records: RecordsSet[T]) -> None:
        self.records.extend(records)
        self.called += 1


@pytest.fixture(scope="module", autouse=True)
def check_tstorage_empty() -> None:
    """Guard against running integration tests with non empty TStorage."""
    try:
        with Channel(TSTORAGE_HOST, TSTORAGE_PORT, UnitPayloadType(), timeout=0.1) as client:
            for data in client.get_iter(Key.min(), Key.max()):
                match data:
                    case Record():
                        raise RuntimeError("TStorage is not empty!")
                    case _:
                        pass
    except OSError:
        raise RuntimeError("Can't connect to TStorage!")


@pytest.fixture
def channel() -> Channel[int]:
    payload_type = StructPayloadType[int]("<i")
    return Channel(TSTORAGE_HOST, TSTORAGE_PORT, payload_type, timeout=60)


@pytest.fixture
def async_channel() -> AsyncChannel[int]:
    payload_type = StructPayloadType[int]("<i")
    return AsyncChannel(TSTORAGE_HOST, TSTORAGE_PORT, payload_type)


def test_tstorage_integration(
    check_tstorage_empty: Any, channel: Channel[int], async_channel: AsyncChannel[int]
) -> None:
    """Integration test with real TStorage.

    It is not necessary to run with real TStorage as anything that behaves like one is sufficent.
    There is no control over TStorage from tests, no clear() or any convenience function.
    This test must work from begin to end testing everything possible.
    """
    asyncio.run(tstorage_integration_test_impl(channel, async_channel), debug=True)


async def tstorage_integration_test_impl(channel: Channel[int], async_channel: AsyncChannel[int]) -> None:
    """To check socket and asyncio based clients at once ensuring order."""
    print("Let the test begin")
    get_empty(channel)
    await get_empty_async(async_channel)
    put_nothing(channel)
    await put_nothing_async(async_channel)
    put_2_records(channel)
    await put_2_records_async(async_channel)
    puta_2_records(channel)
    await puta_2_records_async(async_channel)
    strange_puta(channel)
    await strange_put_async(async_channel)
    get_acq(channel)
    await get_acq_async(async_channel)
    get(channel)
    await get_async(async_channel)
    get_limited_memory(channel)
    await get_limited_memory_async(async_channel)
    get_bad_keys(channel)
    await get_bad_keys_async(async_channel)
    get_bad_payload_type(channel)
    await get_bad_payload_type_async(async_channel)
    get_stream(channel)
    await get_stream_async(async_channel)
    get_stream_limited_memory(channel)
    await get_stream_limited_memory_async(async_channel)
    get_stream_limited_memory_low(channel)
    await get_stream_limited_memory_low_async(async_channel)
    get_stream_bad_keys(channel)
    await get_stream_bad_keys_async(async_channel)
    get_stream_bad_payload_type(channel)
    await get_stream_bad_payload_type_async(async_channel)
    get_iter(channel)
    await get_iter_async(async_channel)
    get_iter_bad_keys(channel)
    await get_iter_bad_keys_async(async_channel)
    get_iter_bad_payload_type(channel)
    await get_iter_bad_payload_type_async(async_channel)
    work_with_many_records()
    await work_with_many_records_async()


def get_empty(channel: Channel[int]) -> None:
    print("get_empty")
    with channel:
        # Good range
        response = channel.get(Key.min(), Key.max())
        assert response.is_ok()
        assert len(response.data) == 0
        # Bad range but with good acq
        response_strange_range = channel.get(dataclasses.replace(Key.max(), acq=0), Key.min())
        assert not response_strange_range.is_ok()
        # Bad gets disconnect by themselves so this will fail
        response_after_disconnect = channel.get(Key.min(), Key.max())
        assert not response_after_disconnect.is_ok()
    with channel:
        # Good range
        response_strange_range = channel.get(Key(0, -10, -10, -10, -10), Key(10, 10, 10, 10, 10))
        assert response_strange_range.is_ok()
        # Bad range due to negative cid
        assert not channel.get(Key(-1, -10, -10, -10, -10), Key(10, 10, 10, 10, 10)).is_ok()


async def get_empty_async(async_channel: AsyncChannel[int]) -> None:
    print("get_empty_async")
    async with async_channel:
        response = await async_channel.get(Key.min(), Key.max())
        assert response.is_ok()
        assert len(response.data) == 0
        response_strange_range = await async_channel.get(dataclasses.replace(Key.max(), acq=0), Key.min())
        assert not response_strange_range.is_ok()
        response_after_disconnect = await async_channel.get(Key.min(), Key.max())
        assert not response_after_disconnect.is_ok()
    async with async_channel:
        response_strange_range = await async_channel.get(Key(0, -10, -10, -10, -10), Key(10, 10, 10, 10, 10))
        assert response_strange_range.is_ok()
        assert not (await async_channel.get(Key(-1, -10, -10, -10, -10), Key(10, 10, 10, 10, 10))).is_ok()


def put_nothing(channel: Channel[int]) -> None:
    print("put_nothing")
    with channel:
        assert channel.put([]).is_ok()
        assert channel.puta([]).is_ok()


async def put_nothing_async(async_channel: AsyncChannel[int]) -> None:
    print("put_nothing_async")
    async with async_channel:
        assert (await async_channel.put([])).is_ok()
        assert (await async_channel.puta([])).is_ok()


def put_2_records(channel: Channel[int]) -> None:
    print("put_2_records")
    with channel:
        records = [Record(Key(0, 12, 0, 10), 314), Record(Key(1, 12, 1, 10), 314)]
        assert channel.put(records).is_ok()


async def put_2_records_async(async_channel: AsyncChannel[int]) -> None:
    print("put_2_records_async")
    async with async_channel:
        records = [Record(Key(0, 34, 0, 10), 314), Record(Key(1, 34, 1, 10), 314)]
        assert (await async_channel.put(records)).is_ok()


def puta_2_records(channel: Channel[int]) -> None:
    print("puta_2_records")
    with channel:
        records = [Record(Key(0, 12, 2, 10, 10), 314), Record(Key(1, 12, 3, 10, 11), 314)]
        assert channel.puta(records).is_ok()


async def puta_2_records_async(async_channel: AsyncChannel[int]) -> None:
    print("puta_2_records_async")
    async with async_channel:
        records = [Record(Key(0, 34, 2, 10, 12), 314), Record(Key(1, 34, 3, 10, 13), 314)]
        assert (await async_channel.puta(records)).is_ok()


def strange_puta(channel: Channel[int]) -> None:
    print("strange_puta")
    with channel:
        assert channel.puta([Record(Key.min(), 314)]).is_ok()
        # It will be ignored due to max value but responds with OK
        assert channel.puta([Record(Key.max(), 314)]).is_ok()
        # It will be ignored due to max value but responds with OK
        assert channel.puta([Record(Key(11, 11, 11, Key.max().cap, 0), 314)]).is_ok()


async def strange_put_async(async_channel: AsyncChannel[int]) -> None:
    print("strange_put_async")
    async with async_channel:
        assert (await async_channel.put([Record(Key.min(), 314)])).is_ok()
        # It will be ignored due to max value but responds with OK
        assert (await async_channel.put([Record(Key.max(), 314)])).is_ok()
        # It will be ignored due to max value but responds with OK
        assert (await async_channel.put([Record(Key(11, 11, 11, Key.max().cap), 314)])).is_ok()


def get_acq(channel: Channel[int]) -> None:
    print("get_acq")
    with channel:
        response = channel.get_acq(Key.min(), Key.max())
        assert response.is_ok() and response.acq >= 0
        response = channel.get_acq(Key(0, 0, 0, 0, 0), dataclasses.replace(Key.max(), cid=0))
        assert response.is_ok() and response.acq >= 0
        response = channel.get_acq(Key(0, 0, 0, 0, 0), Key(2, 13, 4, 11, Key.max().acq))
        assert response.is_ok() and response.acq >= 0
        response = channel.get_acq(Key(0, 0, 0, 0, 0), Key(2, 13, 4, 11, 15))
        assert response.is_ok() and response.acq == 15
        response = channel.get_acq(Key(0, 12, 2, 10, 10), Key(1, 13, 3, 11, 100))
        assert response.is_ok() and response.acq == 100
        assert not channel.get_acq(Key(-1, 12, 2, 10, 10), Key(1, 13, 3, 11, 100)).is_ok()


async def get_acq_async(async_channel: AsyncChannel[int]) -> None:
    print("get_acq_async")
    async with async_channel:
        response = await async_channel.get_acq(Key.min(), Key.max())
        assert response.is_ok() and response.acq >= 0
        response = await async_channel.get_acq(Key(0, 0, 0, 0, 0), dataclasses.replace(Key.max(), cid=0))
        assert response.is_ok() and response.acq >= 0
        response = await async_channel.get_acq(Key(0, 0, 0, 0, 0), Key(2, 13, 4, 11, Key.max().acq))
        assert response.is_ok() and response.acq >= 0
        response = await async_channel.get_acq(Key(0, 0, 0, 0, 0), Key(2, 13, 4, 11, 15))
        assert response.is_ok() and response.acq == 15
        response = await async_channel.get_acq(Key(0, 12, 2, 10, 10), Key(1, 13, 3, 11, 100))
        assert response.is_ok() and response.acq == 100
        assert not (await async_channel.get_acq(Key(-1, 12, 2, 10, 10), Key(1, 13, 3, 11, 100))).is_ok()


def get(channel: Channel[int]) -> None:
    print("get")
    with channel:
        response = channel.get(Key.min(), Key.max())
        assert response.is_ok() and len(response.data) == 10
        response = channel.get(Key(0, 0, 0, 0, 0), Key(2, 35, 4, 11, Key.max().acq))
        assert response.is_ok() and len(response.data) == 8
        response = channel.get(Key(0, 0, 0, 0, 0), Key(1, 35, 4, 11, 15))
        assert response.is_ok() and len(response.data) == 2
        response = channel.get(Key(100, 100, 100, 100, 100), Key(111, 111, 111, 111, 111))
        assert response.is_ok() and len(response.data) == 0


async def get_async(async_channel: AsyncChannel[int]) -> None:
    print("get_async")
    async with async_channel:
        response = await async_channel.get(Key.min(), Key.max())
        assert response.is_ok() and len(response.data) == 10
        response = await async_channel.get(Key(0, 0, 0, 0, 0), Key(2, 35, 4, 11, Key.max().acq))
        assert response.is_ok() and len(response.data) == 8
        response = await async_channel.get(Key(1, 0, 0, 0, 0), Key(2, 35, 4, 11, 15))
        assert response.is_ok() and len(response.data) == 2
        response = await async_channel.get(Key(100, 100, 100, 100, 100), Key(111, 111, 111, 111, 111))
        assert response.is_ok() and len(response.data) == 0


def get_limited_memory(channel: Channel[int]) -> None:
    print("get_limited_memory")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    with channel:
        # Records take 288 bytes
        channel.memory_limit = 360
        response = channel.get(k1, k2)
        assert response.is_ok() and len(response.data) == 8
        channel.memory_limit = 80
        response = channel.get(k1, k2)
        assert not response.is_ok()
    channel.memory_limit = None


async def get_limited_memory_async(async_channel: AsyncChannel[int]) -> None:
    print("get_limited_memory_async")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    async with async_channel:
        async_channel.memory_limit = 360
        response = await async_channel.get(k1, k2)
        assert response.is_ok() and len(response.data) == 8
        async_channel.memory_limit = 80
        response = await async_channel.get(k1, k2)
        assert not response.is_ok()
    async_channel.memory_limit = None


def get_bad_keys(channel: Channel[int]) -> None:
    print("get_bad_keys")
    with channel:
        assert not bool(channel.get(Key(0, 0, 0, 0, 0), dataclasses.replace(Key.max(), cid=0)))


async def get_bad_keys_async(async_channel: AsyncChannel[int]) -> None:
    print("get_bad_keys_async")
    async with async_channel:
        assert not bool(await async_channel.get(Key(0, 0, 0, 0, 0), dataclasses.replace(Key.max(), cid=0)))


def get_bad_payload_type(channel: Channel[int]) -> None:
    print("get_bad_payload_type")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    old = channel._payload_type
    channel._payload_type = StructPayloadType[int]("<q")
    with channel:
        response = channel.get(k1, k2)
        assert not bool(response)
    channel._payload_type = old


async def get_bad_payload_type_async(async_channel: AsyncChannel[int]) -> None:
    print("get_bad_payload_type_async")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    old = async_channel._payload_type
    async_channel._payload_type = StructPayloadType[int]("<q")
    async with async_channel:
        response = await async_channel.get(k1, k2)
        assert not bool(response)
    async_channel._payload_type = old


def get_stream(channel: Channel[int]) -> None:
    print("get_stream")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    collector = Collector[int]()
    with channel:
        assert bool(channel.get_stream(k1, k2, get_stream_callback))
        assert bool(channel.get_stream(k1, k2, collector.receive))
        assert len(collector.records) == 8
        assert collector.called == 1


async def get_stream_async(async_channel: AsyncChannel[int]) -> None:
    print("get_stream_async")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    collector = Collector[int]()
    async with async_channel:
        assert bool(await async_channel.get_stream(k1, k2, get_stream_callback))
        assert bool(await async_channel.get_stream(k1, k2, collector.receive))
        assert len(collector.records) == 8
        assert collector.called == 1


def get_stream_limited_memory(channel: Channel[int]) -> None:
    print("get_stream_limited_memory")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    collector_1 = Collector[int]()
    collector_2 = Collector[int]()
    with channel:
        channel.memory_limit = 90
        assert bool(channel.get_stream(k1, k2, collector_1.receive))
        assert len(collector_1.records) == 8
        assert collector_1.called == 4
        channel.memory_limit = 120
        assert bool(channel.get_stream(k1, k2, collector_2.receive))
        assert len(collector_2.records) == 8
        assert collector_2.called == 3
    channel.memory_limit = None


async def get_stream_limited_memory_async(async_channel: AsyncChannel[int]) -> None:
    print("get_stream_limited_memory_async")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    collector_1 = Collector[int]()
    collector_2 = Collector[int]()
    async with async_channel:
        async_channel.memory_limit = 90
        assert bool(await async_channel.get_stream(k1, k2, collector_1.receive))
        assert len(collector_1.records) == 8
        assert collector_1.called == 4
        async_channel.memory_limit = 120
        assert bool(await async_channel.get_stream(k1, k2, collector_2.receive))
        assert len(collector_2.records) == 8
        assert collector_2.called == 3
    async_channel.memory_limit = None


def get_stream_limited_memory_low(channel: Channel[int]) -> None:
    print("get_stream_limited_memory_low")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    collector = Collector[int]()
    with channel:
        channel.memory_limit = 4
        assert not bool(channel.get_stream(k1, k2, collector.receive))
        assert len(collector.records) == 0
        assert collector.called == 0
    channel.memory_limit = None


async def get_stream_limited_memory_low_async(async_channel: AsyncChannel[int]) -> None:
    print("get_stream_limited_memory_low_async")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    collector = Collector[int]()
    async with async_channel:
        async_channel.memory_limit = 4
        assert not bool(await async_channel.get_stream(k1, k2, collector.receive))
        assert len(collector.records) == 0
        assert collector.called == 0
    async_channel.memory_limit = None


def get_stream_bad_keys(channel: Channel[int]) -> None:
    print("get_stream_bad_keys")
    with channel:
        assert not bool(
            channel.get_stream(Key(0, 0, 0, 0, 0), dataclasses.replace(Key.max(), cid=0), get_stream_callback)
        )


async def get_stream_bad_keys_async(async_channel: AsyncChannel[int]) -> None:
    print("get_stream_bad_keys_async")
    async with async_channel:
        assert not bool(
            await async_channel.get_stream(
                Key(0, 0, 0, 0, 0), dataclasses.replace(Key.max(), cid=0), get_stream_callback
            )
        )


def get_stream_bad_payload_type(channel: Channel[int]) -> None:
    print("get_stream_bad_payload_type")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    old = channel._payload_type
    channel._payload_type = StructPayloadType[int]("<q")
    with channel:
        response = channel.get_stream(k1, k2, get_stream_callback)
        assert not bool(response)
    channel._payload_type = old


async def get_stream_bad_payload_type_async(async_channel: AsyncChannel[int]) -> None:
    print("get_stream_bad_payload_type_async")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    old = async_channel._payload_type
    async_channel._payload_type = StructPayloadType[int]("<q")
    async with async_channel:
        response = await async_channel.get_stream(k1, k2, get_stream_callback)
        assert not bool(response)
    async_channel._payload_type = old


def get_iter(channel: Channel[int]) -> None:
    print("get_iter")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    with channel:
        assert len(list(channel.get_iter(k1, k2))) == 9
        assert len([e for e in channel.get_iter(k1, k2) if isinstance(e, Record)]) == 8
        records: list[Record[int]] = []
        for r in channel.get_iter(k1, k2):
            match r:
                case Record():
                    records.append(r)
                case ResponseAcq():
                    assert r.is_ok()
                case _:
                    raise RuntimeError("Impossible")
        assert len(records) == 8


async def get_iter_async(async_channel: AsyncChannel[int]) -> None:
    print("get_iter_async")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    async with async_channel:
        assert len([e async for e in async_channel.get_iter(k1, k2)]) == 9
        assert len([e async for e in async_channel.get_iter(k1, k2) if isinstance(e, Record)]) == 8
        records: list[Record[int]] = []
        async for r in async_channel.get_iter(k1, k2):
            match r:
                case Record():
                    records.append(r)
                case ResponseAcq():
                    assert r.is_ok()
                case _:
                    raise RuntimeError("Impossible")
        assert len(records) == 8


def get_iter_bad_keys(channel: Channel[int]) -> None:
    print("get_iter_bad_keys")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(0, 35, 4, 11, Key.max().acq)
    with channel:
        records: list[Record[int]] = []
        for r in channel.get_iter(k1, k2):
            match r:
                case Record():
                    records.append(r)
                case ResponseAcq():
                    assert not r.is_ok()
                case _:
                    raise RuntimeError("Impossible")
        assert len(records) == 0


async def get_iter_bad_keys_async(async_channel: AsyncChannel[int]) -> None:
    print("get_iter_bad_keys_async")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(0, 35, 4, 11, Key.max().acq)
    async with async_channel:
        records: list[Record[int]] = []
        async for r in async_channel.get_iter(k1, k2):
            match r:
                case Record():
                    records.append(r)
                case ResponseAcq():
                    assert not r.is_ok()
                case _:
                    raise RuntimeError("Impossible")
        assert len(records) == 0


def get_iter_bad_payload_type(channel: Channel[int]) -> None:
    print("get_iter_bad_payload_type")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    old = channel._payload_type
    channel._payload_type = StructPayloadType[int]("<q")
    with channel:
        records: list[Record[int]] = []
        for i, r in enumerate(channel.get_iter(k1, k2)):
            match r:
                case Record():
                    records.append(r)
                case ResponseAcq():
                    assert not r.is_ok()
                case _:
                    raise RuntimeError("Impossible")
        assert len(records) == 0
    channel._payload_type = old


async def get_iter_bad_payload_type_async(async_channel: AsyncChannel[int]) -> None:
    print("get_iter_bad_payload_type_async")
    k1 = Key(0, 0, 0, 0, 0)
    k2 = Key(2, 35, 4, 11, Key.max().acq)
    old = async_channel._payload_type
    async_channel._payload_type = StructPayloadType[int]("<q")
    async with async_channel:
        records: list[Record[int]] = []
        i: int = 0
        async for r in async_channel.get_iter(k1, k2):
            match r:
                case Record():
                    records.append(r)
                case ResponseAcq():
                    assert not r.is_ok()
                case _:
                    raise RuntimeError("Impossible")
            i += 1
        assert len(records) == 0
    async_channel._payload_type = old


def generate_records(cid_base: int, cid_range: int = 10) -> list[Record[bytes]]:
    rng = random.Random(2025)
    records: list[Record[bytes]] = []
    for i in range(cid_base, cid_base + cid_range):
        for j in range(0, 10):
            for k in range(0, 100):
                key = Key(i, j, k, 44444, 55555)
                value = rng.randbytes(rng.randrange(4, 1024 * 1024, 8))
                records.append(Record(key, value))
    rng.shuffle(records)
    return records[3000:4000]


def work_with_many_records() -> None:
    print("work_with_many_records")
    base_cid = 10000
    cids = 10
    records_list = generate_records(base_cid)
    payload_type = BytesPayloadType()
    with Channel(TSTORAGE_HOST, TSTORAGE_PORT, payload_type) as ch:
        assert bool(ch.puta(records_list))
        records = set(records_list)
        time.sleep(1)
        result = ch.get(Key(base_cid, 0, 0, 44444, 0), Key(base_cid + cids, 10, 100, 44445, Key.max().acq))
        assert bool(result)
        assert set(result.data) == records
        del result
        collector = Collector[bytes]()
        result_stream = ch.get_stream(
            Key(base_cid, 0, 0, 44444, 0), Key(base_cid + cids, 10, 100, 44445, Key.max().acq), collector.receive
        )
        assert bool(result_stream)
        assert set(collector.records) == records
        del collector
        result_iter: set[Record[bytes]] = set()
        for yielded in ch.get_iter(Key(base_cid, 0, 0, 44444, 0), Key(base_cid + cids, 10, 100, 44445, Key.max().acq)):
            match yielded:
                case Record():
                    result_iter.add(yielded)
                case ResponseAcq():
                    assert bool(yielded)
        assert result_iter == records


async def work_with_many_records_async() -> None:
    print("work_with_many_records_async")
    base_cid = 20000
    cids = 10
    records_list = generate_records(base_cid)
    payload_type = BytesPayloadType()
    async with AsyncChannel(TSTORAGE_HOST, TSTORAGE_PORT, payload_type) as ch:
        assert bool(await ch.puta(records_list))
        records = set(records_list)
        time.sleep(1)
        result = await ch.get(Key(base_cid, 0, 0, 44444, 0), Key(base_cid + cids, 10, 100, 44445, Key.max().acq))
        assert bool(result)
        assert set(result.data) == records
        del result
        collector = Collector[bytes]()
        result_stream = await ch.get_stream(
            Key(base_cid, 0, 0, 44444, 0), Key(base_cid + cids, 10, 100, 44445, Key.max().acq), collector.receive
        )
        assert bool(result_stream)
        assert set(collector.records) == records
        del collector
        result_iter: set[Record[bytes]] = set()
        async for yielded in ch.get_iter(
            Key(base_cid, 0, 0, 44444, 0), Key(base_cid + cids, 10, 100, 44445, Key.max().acq)
        ):
            match yielded:
                case Record():
                    result_iter.add(yielded)
                case ResponseAcq():
                    assert bool(yielded)
        assert result_iter == records
