"""Socket module based TStorage communication channel."""

import socket
import ssl
import struct
from types import TracebackType
from typing import Iterator, TypeVar

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


__all__ = ("Channel",)


T = TypeVar("T")


class Channel(_ChannelMixin[T]):
    """Channel provides TStorage communication facilities using socket module.

    Channel can be used as context manager to open and close connection.
    All put and fetched data should be serializable by passed payload_type.
    """

    def __init__(
        self,
        host: str,
        port: int,
        payload_type: PayloadType[T],
        timeout: float | None = None,
        memory_limit: int | None = None,
        ssl_context: ssl.SSLContext | None = None,
    ) -> None:
        """Initialize new Channel instance.

        Args:
            host: TStorage hostname.
            port: TStorage port.
            payload_type: Data converter of this Channel.
            timeout: Socket's timeout.
            memory_limit: Max memory for GET requests in bytes.
            ssl_context: SSLContext instance if secure connection is required.
        """
        self._host: str = host
        self._port: int = port
        self._payload_type: PayloadType[T] = payload_type
        self._timeout: float | None = timeout
        self._memory_limit: int | None = memory_limit
        self._ssl_context: ssl.SSLContext | None = ssl_context
        self._socket: socket.socket | None = None

    @property
    def timeout(self) -> float | None:
        """Get or set socket timeout"""
        return self._timeout

    @timeout.setter
    def timeout(self, value: float | None) -> None:
        self._timeout = value
        if self._socket is not None:
            self._socket.settimeout(self._timeout)

    @property
    def memory_limit(self) -> int | None:
        """Get or set memory limit in bytes for GET requests."""
        return self._memory_limit

    @memory_limit.setter
    def memory_limit(self, value: int | None) -> None:
        self._memory_limit = value

    def connect(self) -> Response:
        """Connect to specified host at port with set timeout.

        Returns:
            OK status if connected else ERROR status.
        """
        try:
            self._socket = socket.create_connection((self._host, self._port), self._timeout)
            if self._ssl_context is not None:
                self._socket = self._ssl_context.wrap_socket(self._socket, server_hostname=self._host)
            return Response(ResponseStatus.OK)
        except OSError:
            return Response(ResponseStatus.ERROR)

    def close(self) -> Response:
        """Close underlying socket.

        Returns:
            OK status if closed socket else ERROR status.
        """
        if self._socket is not None:
            sock = self._socket
            self._socket = None
            try:
                sock.shutdown(socket.SHUT_RDWR)
            except OSError:
                pass
            sock.close()
            return Response(ResponseStatus.OK)
        else:
            return Response(ResponseStatus.ERROR)

    def __enter__(self) -> "Channel[T]":
        """Connect and close this Channel in context manager.

        Raises:
            socket.error: If connect fails.
        """
        if self.connect():
            return self
        else:
            raise OSError()

    def __exit__(
        self, exc_type: type[BaseException] | None, exc_value: BaseException | None, traceback: TracebackType | None
    ) -> None:
        self.close()

    def put(self, data: RecordsSet[T], max_batch_size: int = 2147483647, skip_invalid: bool = False) -> Response:
        """Put records to TStorage without acq value.

        Args:
            data: Records to put.
            max_batch_size: Controls maximal serialization buffer size. Must be lower than 2147483648 (2GiB).
                Despite this value always at least 1 record will be serialized.
            skip_invalid: Skip invalid records. Otherwise finishes put immediately at invalid record.

        Returns:
            OK status on success else status indicates error.
        """
        return self._put(data, cmd=_CommandType.PUTSAFE, max_batch_size=max_batch_size, skip_invalid=skip_invalid)

    def puta(self, data: RecordsSet[T], max_batch_size: int = 2147483647, skip_invalid: bool = False) -> Response:
        """Put records to TStorage with acq value.

        Args:
            data: Records to put.
            max_batch_size: Controls maximal serialization buffer size. Must be lower than 2147483648 (2GiB).
                Despite this value always at least 1 record will be serialized.
            skip_invalid: Skip invalid records. Otherwise finishes puta immediately at invalid record.

        Returns:
            OK status on success else status indicates error.
        """
        return self._put(data, cmd=_CommandType.PUTASAFE, max_batch_size=max_batch_size, skip_invalid=skip_invalid)

    def _put(
        self, data: RecordsSet[T], *, cmd: _CommandType, max_batch_size: int = 2147483647, skip_invalid: bool = False
    ) -> Response:
        if self._socket is None:
            return Response(ResponseStatus.DISCONNECTED)
        request: bytes = RequestHeader(cmd, HEADER_AUX_SIZE).to_bytes()
        self._send_data(request)
        try:
            for batch in self._serialize_records_batches_iter(
                data, cmd == _CommandType.PUTASAFE, self._payload_type, max_batch_size, skip_invalid
            ):
                self._send_data(batch)
            self._send_data(struct.pack("<i", PUT_END_GUARD))
        except ConnectionError:
            pass
        buffer: ReceiveBuffer = ReceiveBuffer(RESPONSE_HEADER_SIZE + ACQS_PAIR_SIZE + HEADER_AUX_SIZE)
        while self._feed_buffer(buffer):
            if response := self._handle_response(buffer, "<qq"):
                header, _ = response
                return Response(ResponseStatus.OK if header.is_ok() else ResponseStatus.ERROR)
        return Response(ResponseStatus.DISCONNECTED)

    def get_acq(self, key_min: Key, key_max: Key) -> ResponseAcq:
        """Get last acq value from TStorage.

        Args:
            key_min: Lower end of keyrange (inclusive).
            key_max: Upper end of keyrange (exclusive).

        Returns:
            OK status, acq on success else status indicates error and acq is invalid.
        """
        if self._socket is None:
            return ResponseAcq(ResponseStatus.DISCONNECTED)
        request: bytes = self._prepare_keyrange_request(
            _CommandType.GETACQ, key_min, key_max, 64
        )  # TODO: Remove 64 in new protocol
        self._send_data(request)
        buffer: ReceiveBuffer = ReceiveBuffer(RESPONSE_HEADER_SIZE + ACQ_SIZE + HEADER_AUX_SIZE)
        while self._feed_buffer(buffer):
            if response := self._handle_response(buffer, "<q"):
                header, response_data = response
                if header.is_ok():
                    return ResponseAcq(ResponseStatus.OK, response_data[0])
                return ResponseAcq(ResponseStatus.ERROR)
        return ResponseAcq(ResponseStatus.DISCONNECTED)

    def get(self, key_min: Key, key_max: Key, recv_buffer_size: int = 65536) -> ResponseGet[T]:
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
        if self._socket is None:
            return self._early_close(ResponseGet(ResponseStatus.DISCONNECTED))
        request: bytes = self._prepare_keyrange_request(
            _CommandType.GET, key_min, key_max, 64
        )  # TODO: Remove 64 in new protocol
        self._send_data(request)
        stage: GetRequestState = GetRequestState.INITIAL_HEADER
        total_bytes: int = 0
        buffer: ReceiveBuffer = ReceiveBuffer(
            recv_buffer_size if self._memory_limit is None else min(recv_buffer_size, self._memory_limit)
        )
        records: RecordsSet[T] = []
        while bytes_received := self._feed_buffer(buffer):
            total_bytes += bytes_received
            if self._is_over_memory_limit(total_bytes):
                return self._early_close(ResponseGet(ResponseStatus.NO_MEMORY, data=records))
            if stage == GetRequestState.INITIAL_HEADER:
                if response := self._handle_response(buffer):
                    header, _ = response
                    if not header.is_ok():
                        return self._early_close(ResponseGet(ResponseStatus.BAD_REQUEST, data=records))
                    stage = GetRequestState.RECORDS_PARSING
            if stage == GetRequestState.RECORDS_PARSING:
                match self._parse_records(buffer, records, self._payload_type, self._memory_limit):
                    case RecordsParsingStatus.NEEDS_MORE_BYTES:
                        continue
                    case RecordsParsingStatus.FINISHED:
                        stage = GetRequestState.FINAL_HEADER
                    case RecordsParsingStatus.UNPARSEABLE:
                        return self._early_close(ResponseGet(ResponseStatus.UNPARSEABLE_ENTITY, data=records))
                    case RecordsParsingStatus.RECORD_TOO_BIG:
                        return self._early_close(ResponseGet(ResponseStatus.NO_MEMORY, data=records))
            if stage == GetRequestState.FINAL_HEADER:
                if response := self._handle_response(buffer, "<q"):
                    header, response_data = response
                    if header.is_ok():
                        return ResponseGet(ResponseStatus.OK, response_data[0], records)
                    return self._early_close(ResponseGet(ResponseStatus.ERROR, data=records))
        return self._early_close(ResponseGet(ResponseStatus.DISCONNECTED, data=records))

    def get_stream(
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
        if self._socket is None:
            return self._early_close(ResponseAcq(ResponseStatus.DISCONNECTED))
        request: bytes = self._prepare_keyrange_request(
            _CommandType.GET, key_min, key_max, 64
        )  # TODO: Remove 64 in new protocol
        self._send_data(request)
        stage: GetRequestState = GetRequestState.INITIAL_HEADER
        total_bytes: int = 0
        buffer: ReceiveBuffer = ReceiveBuffer(
            recv_buffer_size if self._memory_limit is None else min(recv_buffer_size, self._memory_limit)
        )
        records: list[Record[T]] = []
        while bytes_received := self._feed_buffer(
            buffer, self._memory_limit - total_bytes if self._memory_limit else 0
        ):
            total_bytes += bytes_received
            if stage == GetRequestState.INITIAL_HEADER:
                if response := self._handle_response(buffer):
                    header, _ = response
                    if not header.is_ok():
                        return self._early_close(ResponseAcq(ResponseStatus.BAD_REQUEST))
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
                                return self._early_close(ResponseAcq(ResponseStatus.NO_MEMORY))
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
                        return self._early_close(ResponseAcq(ResponseStatus.UNPARSEABLE_ENTITY))
                    case RecordsParsingStatus.RECORD_TOO_BIG:
                        if records:
                            callback(records)
                            records.clear()
                            total_bytes = 0
                        return self._early_close(ResponseAcq(ResponseStatus.NO_MEMORY))
            if stage == GetRequestState.FINAL_HEADER:
                if response := self._handle_response(buffer, "<q"):
                    header, response_data = response
                    if header.is_ok():
                        return ResponseAcq(ResponseStatus.OK, response_data[0])
                    return self._early_close(ResponseAcq(ResponseStatus.ERROR))
        if records:
            callback(records)
        return self._early_close(ResponseAcq(ResponseStatus.DISCONNECTED))

    def get_iter(self, key_min: Key, key_max: Key, recv_buffer_size: int = 65536) -> Iterator[Record[T] | ResponseAcq]:
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
        if self._socket is None:
            yield self._early_close(ResponseAcq(ResponseStatus.DISCONNECTED))
            return
        request: bytes = self._prepare_keyrange_request(
            _CommandType.GET, key_min, key_max, 64
        )  # TODO: Remove 64 in new protocol
        self._send_data(request)
        stage: GetRequestState = GetRequestState.INITIAL_HEADER
        total_bytes: int = 0
        buffer: ReceiveBuffer = ReceiveBuffer(
            recv_buffer_size if self._memory_limit is None else min(recv_buffer_size, self._memory_limit)
        )
        records: list[Record[T]] = []
        while bytes_received := self._feed_buffer(buffer):
            total_bytes += bytes_received
            if self._is_over_memory_limit(total_bytes):
                yield from records
                yield self._early_close(ResponseAcq(ResponseStatus.NO_MEMORY))
                return
            if stage == GetRequestState.INITIAL_HEADER:
                if response := self._handle_response(buffer):
                    header, _ = response
                    if not header.is_ok():
                        yield self._early_close(ResponseAcq(ResponseStatus.BAD_REQUEST))
                        return
                    stage = GetRequestState.RECORDS_PARSING
            if stage == GetRequestState.RECORDS_PARSING:
                parsing_status: RecordsParsingStatus = self._parse_records(
                    buffer, records, self._payload_type, self._memory_limit
                )
                yield from records
                records.clear()
                match parsing_status:
                    case RecordsParsingStatus.NEEDS_MORE_BYTES:
                        continue
                    case RecordsParsingStatus.FINISHED:
                        stage = GetRequestState.FINAL_HEADER
                    case RecordsParsingStatus.UNPARSEABLE:
                        yield self._early_close(ResponseAcq(ResponseStatus.UNPARSEABLE_ENTITY))
                        return
                    case RecordsParsingStatus.RECORD_TOO_BIG:
                        yield self._early_close(ResponseAcq(ResponseStatus.NO_MEMORY))
                        return
            if stage == GetRequestState.FINAL_HEADER:
                if response := self._handle_response(buffer, "<q"):
                    header, response_data = response
                    if header.is_ok():
                        yield ResponseAcq(ResponseStatus.OK, response_data[0])
                        return
                    yield self._early_close(ResponseAcq(ResponseStatus.ERROR))
                    return
        yield from records
        yield self._early_close(ResponseAcq(ResponseStatus.DISCONNECTED))
        return

    def _send_data(self, data: bytes) -> None:
        assert self._socket is not None
        self._socket.sendall(data)

    def _is_over_memory_limit(self, size: int) -> bool:
        return self._memory_limit is not None and size > self._memory_limit

    def _is_at_memory_limit(self, size: int) -> bool:
        return self._memory_limit is not None and size >= self._memory_limit

    def _feed_buffer(self, buffer: ReceiveBuffer, limit: int = 0) -> int:
        assert self._socket is not None
        memory: memoryview = buffer.free_space()
        recv_size: int = self._socket.recv_into(memory, min(limit, len(memory)))
        buffer.increase_available(recv_size)
        return recv_size

    _R = TypeVar("_R", bound=Response)

    def _early_close(self, response: _R) -> _R:
        self.close()
        return response
