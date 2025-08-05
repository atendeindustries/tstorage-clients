"""Asyncio module based TStorage communication channel."""

import asyncio
import ssl
import struct
from types import TracebackType
from typing import AsyncIterator, TypeVar

from ._channel_common import (
    ACQ_SIZE,
    ACQS_PAIR_SIZE,
    HEADER_AUX_SIZE,
    PUT_END_GUARD,
    RESPONSE_HEADER_SIZE,
    GetRequestState,
    ReceiveBuffer,
    RecordsParsingStatus,
    RequestHeader,
    _ChannelMixin,
    _CommandType,
)
from .payload_type import PayloadType
from .record import Key, Record
from .records_set import GetCallback, RecordsSet
from .response import Response, ResponseAcq, ResponseGet, ResponseStatus


__all__ = ("AsyncChannel",)


T = TypeVar("T")


class AsyncChannel(_ChannelMixin[T]):
    """AsyncChannel provides TStorage communication facilities using asyncio module.

    AsyncChannel can be used as async context manager to open and close connection.
    All put and fetched data should be serializable by passed payload_type.
    AsyncChannel doesn't provide timeout. Please use functions from https://docs.python.org/3/library/asyncio-task.html#timeouts.
    """

    def __init__(
        self,
        host: str,
        port: int,
        payload_type: PayloadType[T],
        memory_limit: int | None = None,
        ssl_context: ssl.SSLContext | None = None,
    ) -> None:
        """Initialize new AsyncChannel instance.

        Args:
            host: TStorage hostname.
            port: TStorage port.
            payload_type: Data converter of this AsyncChannel.
            memory_limit: Max memory for GET requests in bytes.
            ssl_context: SSLContext instance if secure connection is required.
        """
        self._host: str = host
        self._port: int = port
        self._payload_type: PayloadType[T] = payload_type
        self._memory_limit: int | None = memory_limit
        self._ssl_context: ssl.SSLContext | None = ssl_context
        self._writer: asyncio.StreamWriter | None = None
        self._reader: asyncio.StreamReader | None = None

    @property
    def memory_limit(self) -> int | None:
        """Get or set memory limit in bytes for GET requests."""
        return self._memory_limit

    @memory_limit.setter
    def memory_limit(self, value: int | None) -> None:
        self._memory_limit = value

    async def connect(self) -> Response:
        """Connect to specified host at port.

        Returns:
            OK status if connected else ERROR status.
        """
        try:
            reader, writer = await asyncio.open_connection(self._host, self._port, ssl=self._ssl_context)
            self._writer = writer
            self._reader = reader
            return Response(ResponseStatus.OK)
        except ConnectionError:
            return Response(ResponseStatus.ERROR)

    async def close(self) -> Response:
        """Close underlying connection.

        Returns:
            OK status if closed connection else ERROR status.
        """
        if self._writer is not None:
            writer = self._writer
            self._writer = None
            self._reader = None
            if writer.can_write_eof():
                writer.write_eof()
            writer.close()
            try:
                await writer.wait_closed()
            except ConnectionError:
                pass
            return Response(ResponseStatus.OK)
        else:
            return Response(ResponseStatus.ERROR)

    async def __aenter__(self) -> "AsyncChannel[T]":
        """Connect and close this AsyncChannel in async context manager.

        Raises:
            ConnectionError: If connect fails.
        """
        if await self.connect():
            return self
        else:
            raise ConnectionError()

    async def __aexit__(
        self, exc_type: type[BaseException] | None, exc_value: BaseException | None, traceback: TracebackType | None
    ) -> None:
        await self.close()

    async def put(self, data: RecordsSet[T], max_batch_size: int = 2147483647, skip_invalid: bool = False) -> Response:
        """Put records to TStorage without acq value.

        Args:
            data: Records to put.
            max_batch_size: Controls maximal serialization buffer size. Must be lower than 2147483648 (2GiB).
                Despite this value always at least 1 record will be serialized.
            skip_invalid: Skip invalid records. Otherwise finishes put immediately at invalid record.

        Returns:
            OK status on success else status indicates error.
        """
        return await self._put(data, cmd=_CommandType.PUTSAFE, max_batch_size=max_batch_size, skip_invalid=skip_invalid)

    async def puta(self, data: RecordsSet[T], max_batch_size: int = 2147483647, skip_invalid: bool = False) -> Response:
        """Put records to TStorage with acq value.

        Args:
            data: Records to put.
            max_batch_size: Controls maximal serialization buffer size. Must be lower than 2147483648 (2GiB).
                Despite this value always at least 1 record will be serialized.
            skip_invalid: Skip invalid records. Otherwise finishes puta immediately at invalid record.

        Returns:
            OK status on success else status indicates error.
        """
        return await self._put(
            data, cmd=_CommandType.PUTASAFE, max_batch_size=max_batch_size, skip_invalid=skip_invalid
        )

    async def _put(
        self, data: RecordsSet[T], *, cmd: _CommandType, max_batch_size: int = 2147483647, skip_invalid: bool = False
    ) -> Response:
        if self._writer is None or self._reader is None:
            return Response(ResponseStatus.DISCONNECTED)
        request: bytes = RequestHeader(cmd, HEADER_AUX_SIZE).to_bytes()
        await self._send_data(request)
        try:
            for batch in self._serialize_records_batches_iter(
                data, cmd == _CommandType.PUTASAFE, self._payload_type, max_batch_size, skip_invalid
            ):
                await self._send_data(batch)
            await self._send_data(struct.pack("<i", PUT_END_GUARD))
        except ConnectionError:
            self._reader.set_exception(None)  # type: ignore[arg-type] # Could not find better solution
        buffer: ReceiveBuffer = ReceiveBuffer(RESPONSE_HEADER_SIZE + ACQS_PAIR_SIZE + HEADER_AUX_SIZE)
        while await self._feed_buffer(buffer):
            if response := self._handle_response(buffer, "<qq"):
                header, _ = response
                return Response(ResponseStatus.OK if header.is_ok() else ResponseStatus.ERROR)
        return Response(ResponseStatus.DISCONNECTED)

    async def get_acq(self, key_min: Key, key_max: Key) -> ResponseAcq:
        """Get last acq value from TStorage.

        Args:
            key_min: Lower end of keyrange (inclusive).
            key_max: Upper end of keyrange (exclusive).

        Returns:
            OK status, acq on success else status indicates error and acq is invalid.
        """
        if self._writer is None or self._reader is None:
            return ResponseAcq(ResponseStatus.DISCONNECTED)
        request: bytes = self._prepare_keyrange_request(
            _CommandType.GETACQ, key_min, key_max, 64
        )  # TODO: Remove 64 in new protocol
        await self._send_data(request)
        buffer: ReceiveBuffer = ReceiveBuffer(RESPONSE_HEADER_SIZE + ACQ_SIZE + HEADER_AUX_SIZE)
        while await self._feed_buffer(buffer):
            if response := self._handle_response(buffer, "<q"):
                header, response_data = response
                if header.is_ok():
                    return ResponseAcq(ResponseStatus.OK, response_data[0])
                return ResponseAcq(ResponseStatus.ERROR)
        return ResponseAcq(ResponseStatus.DISCONNECTED)

    async def get(self, key_min: Key, key_max: Key, recv_buffer_size: int = 65536) -> ResponseGet[T]:
        """Get records from TStorage.

        Records are get up to memory_limit and request fails if there is more data.
        If request fails at some point it automatically closes connection.

        Args:
            key_min: Lower end of keyrange (inclusive).
            key_max: Upper end of keyrange (exclusive).
            recv_buffer_size: Initial buffer size for receiving data from network capped by self.memory_limit.

        Returns:
            OK status, acq, data on success else status indicates error, data stores partial result and acq is invalid.
        """
        if self._writer is None or self._reader is None:
            return await self._early_close(ResponseGet(ResponseStatus.DISCONNECTED))
        request: bytes = self._prepare_keyrange_request(
            _CommandType.GET, key_min, key_max, 64
        )  # TODO: Remove 64 in new protocol
        await self._send_data(request)
        stage: GetRequestState = GetRequestState.INITIAL_HEADER
        total_bytes: int = 0
        buffer: ReceiveBuffer = ReceiveBuffer(
            recv_buffer_size if self._memory_limit is None else min(recv_buffer_size, self._memory_limit)
        )
        records: RecordsSet[T] = []
        while bytes_received := await self._feed_buffer(buffer):
            total_bytes += bytes_received
            if self._is_over_memory_limit(total_bytes):
                return await self._early_close(ResponseGet(ResponseStatus.NO_MEMORY, data=records))
            if stage == GetRequestState.INITIAL_HEADER:
                if response := self._handle_response(buffer):
                    header, _ = response
                    if not header.is_ok():
                        return await self._early_close(ResponseGet(ResponseStatus.BAD_REQUEST, data=records))
                    stage = GetRequestState.RECORDS_PARSING
            if stage == GetRequestState.RECORDS_PARSING:
                match self._parse_records(buffer, records, self._payload_type, self._memory_limit):
                    case RecordsParsingStatus.NEEDS_MORE_BYTES:
                        continue
                    case RecordsParsingStatus.FINISHED:
                        stage = GetRequestState.FINAL_HEADER
                    case RecordsParsingStatus.UNPARSEABLE:
                        return await self._early_close(ResponseGet(ResponseStatus.UNPARSEABLE_ENTITY, data=records))
                    case RecordsParsingStatus.RECORD_TOO_BIG:
                        return await self._early_close(ResponseGet(ResponseStatus.NO_MEMORY, data=records))
            if stage == GetRequestState.FINAL_HEADER:
                if response := self._handle_response(buffer, "<q"):
                    header, response_data = response
                    if header.is_ok():
                        return ResponseGet(ResponseStatus.OK, response_data[0], records)
                    return await self._early_close(ResponseGet(ResponseStatus.ERROR, data=records))
        return await self._early_close(ResponseGet(ResponseStatus.DISCONNECTED, data=records))

    async def get_stream(
        self, key_min: Key, key_max: Key, callback: GetCallback[T], recv_buffer_size: int = 65536
    ) -> ResponseAcq:
        """Get records from TStorage.

        Records are get in memory_limit sized batches.
        If request fails at some point it automatically closes connection.

        Args:
            key_min: Lower end of keyrange (inclusive).
            key_max: Upper end of keyrange (exclusive).
            callback: Callable to call on each batch.
            recv_buffer_size: Initial buffer size for receiving data from network capped by self.memory_limit.

        Returns:
            OK status, acq on success else status indicates error and acq is invalid.
        """
        if self._writer is None or self._reader is None:
            return await self._early_close(ResponseAcq(ResponseStatus.DISCONNECTED))
        request: bytes = self._prepare_keyrange_request(
            _CommandType.GET, key_min, key_max, 64
        )  # TODO: Remove 64 in new protocol
        await self._send_data(request)
        stage: GetRequestState = GetRequestState.INITIAL_HEADER
        total_bytes: int = 0
        buffer: ReceiveBuffer = ReceiveBuffer(
            recv_buffer_size if self._memory_limit is None else min(recv_buffer_size, self._memory_limit)
        )
        records: list[Record[T]] = []
        while bytes_received := await self._feed_buffer(
            buffer, self._memory_limit - total_bytes if self._memory_limit else 0
        ):
            total_bytes += bytes_received
            if stage == GetRequestState.INITIAL_HEADER:
                if response := self._handle_response(buffer):
                    header, _ = response
                    if not header.is_ok():
                        return await self._early_close(ResponseAcq(ResponseStatus.BAD_REQUEST))
                    stage = GetRequestState.RECORDS_PARSING
            if stage == GetRequestState.RECORDS_PARSING:
                match self._parse_records(buffer, records, self._payload_type, self._memory_limit):
                    case RecordsParsingStatus.NEEDS_MORE_BYTES:
                        if self._is_at_memory_limit(total_bytes):
                            if records:
                                callback(records)
                                records.clear()
                                total_bytes = 0
                                continue
                            else:
                                return await self._early_close(ResponseAcq(ResponseStatus.NO_MEMORY))
                    case RecordsParsingStatus.FINISHED:
                        if records:
                            callback(records)
                            records.clear()
                            total_bytes = 0
                        stage = GetRequestState.FINAL_HEADER
                    case RecordsParsingStatus.UNPARSEABLE:
                        if records:
                            callback(records)
                            records.clear()
                            total_bytes = 0
                        return await self._early_close(ResponseAcq(ResponseStatus.UNPARSEABLE_ENTITY))
                    case RecordsParsingStatus.RECORD_TOO_BIG:
                        if records:
                            callback(records)
                            records.clear()
                            total_bytes = 0
                        return await self._early_close(ResponseAcq(ResponseStatus.NO_MEMORY))
            if stage == GetRequestState.FINAL_HEADER:
                if response := self._handle_response(buffer, "<q"):
                    header, response_data = response
                    if header.is_ok():
                        return ResponseAcq(ResponseStatus.OK, response_data[0])
                    return await self._early_close(ResponseAcq(ResponseStatus.ERROR))
        if records:
            callback(records)
        return await self._early_close(ResponseAcq(ResponseStatus.DISCONNECTED))

    async def get_iter(
        self, key_min: Key, key_max: Key, recv_buffer_size: int = 65536
    ) -> AsyncIterator[Record[T] | ResponseAcq]:
        """Get records from TStorage as iterator.

        If request fails at some point it automatically closes connection.
        Records are yielded after whole batch parsing has been done.
        Remember to check for final response with OK status.

        Args:
            key_min: Lower end of keyrange (inclusive).
            key_max: Upper end of keyrange (exclusive).
            recv_buffer_size: Initial buffer size for receiving data from network capped by self.memory_limit.

        Yields:
            Record or response control data indicating error or successfully finished request.
        """
        if self._writer is None or self._reader is None:
            yield await self._early_close(ResponseAcq(ResponseStatus.DISCONNECTED))
            return
        request: bytes = self._prepare_keyrange_request(
            _CommandType.GET, key_min, key_max, 64
        )  # TODO: Remove 64 in new protocol
        await self._send_data(request)
        stage: GetRequestState = GetRequestState.INITIAL_HEADER
        total_bytes: int = 0
        buffer: ReceiveBuffer = ReceiveBuffer(
            recv_buffer_size if self._memory_limit is None else min(recv_buffer_size, self._memory_limit)
        )
        records: list[Record[T]] = []
        while bytes_received := await self._feed_buffer(buffer):
            total_bytes += bytes_received
            if self._is_over_memory_limit(total_bytes):
                for r in records:
                    yield r
                yield await self._early_close(ResponseAcq(ResponseStatus.NO_MEMORY))
                return
            if stage == GetRequestState.INITIAL_HEADER:
                if response := self._handle_response(buffer):
                    header, _ = response
                    if not header.is_ok():
                        yield await self._early_close(ResponseAcq(ResponseStatus.BAD_REQUEST))
                        return
                    stage = GetRequestState.RECORDS_PARSING
            if stage == GetRequestState.RECORDS_PARSING:
                parsing_status = self._parse_records(buffer, records, self._payload_type, self._memory_limit)
                for r in records:
                    yield r
                records.clear()
                match parsing_status:
                    case RecordsParsingStatus.NEEDS_MORE_BYTES:
                        continue
                    case RecordsParsingStatus.FINISHED:
                        stage = GetRequestState.FINAL_HEADER
                    case RecordsParsingStatus.UNPARSEABLE:
                        yield await self._early_close(ResponseAcq(ResponseStatus.UNPARSEABLE_ENTITY))
                        return
                    case RecordsParsingStatus.RECORD_TOO_BIG:
                        yield await self._early_close(ResponseAcq(ResponseStatus.NO_MEMORY))
                        return
            if stage == GetRequestState.FINAL_HEADER:
                if response := self._handle_response(buffer, "<q"):
                    header, response_data = response
                    if header.is_ok():
                        yield ResponseAcq(ResponseStatus.OK, response_data[0])
                        return
                    yield await self._early_close(ResponseAcq(ResponseStatus.ERROR))
                    return
        for r in records:
            yield r
        yield await self._early_close(ResponseAcq(ResponseStatus.DISCONNECTED))
        return

    async def _send_data(self, data: bytes) -> None:
        assert self._writer is not None and self._reader is not None
        self._writer.write(data)
        await self._writer.drain()

    def _is_over_memory_limit(self, size: int) -> bool:
        return self._memory_limit is not None and size > self._memory_limit

    def _is_at_memory_limit(self, size: int) -> bool:
        return self._memory_limit is not None and size >= self._memory_limit

    async def _feed_buffer(self, buffer: ReceiveBuffer, limit: int = 0) -> int:
        # To make it more performant it is possible to implement custom Asyncio BufferedProtocol that avoids copying
        assert self._writer is not None and self._reader is not None
        amount: int = buffer.free_len() if limit == 0 or limit > buffer.free_len() else limit
        data: bytes = await self._reader.read(amount)
        buffer.feed(data)
        return len(data)

    _R = TypeVar("_R", bound=Response)

    async def _early_close(self, response: _R) -> _R:
        await self.close()
        return response
