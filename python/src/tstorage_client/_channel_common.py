import struct
from collections import defaultdict
from dataclasses import dataclass
from enum import Enum, IntEnum
from typing import Any, ClassVar, Generic, Iterable, Literal, TypeVar

from .payload_type import PayloadType
from .record import Key, Record
from .records_set import RecordsSet


T = TypeVar("T")


class _CommandType(IntEnum):
    NULL = 0
    GET = 1
    PUTSAFE = 5
    PUTASAFE = 6
    GETACQ = 7


ACQS_PAIR_SIZE: Literal[16] = 16
ACQ_SIZE: Literal[8] = 8
FULL_KEY_SIZE: Literal[32] = 32
HEADER_AUX_SIZE: Literal[0] = 0
PUT_END_GUARD: Literal[-1] = -1
RESPONSE_HEADER_SIZE: Literal[12] = 12


@dataclass(frozen=True, slots=True)
class RequestHeader:
    status: int
    size: int

    _format: ClassVar[struct.Struct] = struct.Struct("<iQ")

    def to_bytes(self) -> bytes:
        return self._format.pack(self.status, self.size)

    @classmethod
    def from_bytes(cls, data: bytes) -> "RequestHeader":
        return cls(*cls._format.unpack(data))

    @classmethod
    def bytes_count(cls) -> int:
        return cls._format.size

    def has_expected_size(self, size: int) -> bool:
        return self.size >= size

    def is_ok(self) -> bool:
        return self.status == 0


class GetRequestState(Enum):
    INITIAL_HEADER = 0
    RECORDS_PARSING = 1
    FINAL_HEADER = 2


class RecordsParsingStatus(Enum):
    FINISHED = 0
    UNPARSEABLE = 1
    NEEDS_MORE_BYTES = 2
    RECORD_TOO_BIG = 3


class ReceiveBuffer:
    """Buffer for managing data from connection."""

    def __init__(self, initial_capacity: int = 65536) -> None:
        self._buffer: bytearray = bytearray(max(initial_capacity, 32))
        self._memory: memoryview = memoryview(self._buffer)
        self._current: int = 0
        self._available: int = 0

    def __len__(self) -> int:
        return self._available - self._current

    def free_len(self) -> int:
        return len(self._memory) - self._available

    def free_space(self) -> memoryview:
        assert self._available < len(self._memory)
        return self._memory[self._available :]

    def feed(self, data: bytes) -> None:
        size: int = len(data)
        new_available: int = self._available + size
        assert new_available <= len(self._memory)
        self._memory[self._available : new_available] = data
        self._available = new_available

    def peek(self, size: int, offset: int = 0) -> memoryview:
        position: int = self._current + offset
        end: int = min(position + size, self._available)
        return self._memory[position:end]

    def increase(self, size: int) -> None:
        self._current += size

    def rewind(self, size: int) -> None:
        self._current -= size

    def increase_available(self, size: int) -> None:
        self._available += size

    def fits(self, size: int) -> bool:
        return self._current + size <= self._available

    def fits_eventually(self, size: int) -> bool:
        return self._current + size <= len(self._memory)

    def truncate(self) -> None:
        if self._current != 0:
            distance: int = self._available - self._current
            self._memory[:distance] = self._memory[self._current : self._available]
            self._current = 0
            self._available = distance

    def grow_buffer(self, size: int) -> None:
        length: int = len(self._memory)
        self._memory.release()
        self._buffer += b"\0" * (size - length)
        self._memory = memoryview(self._buffer)


class _ChannelMixin(Generic[T]):
    """Class that provides commons for channels"""

    @staticmethod
    def _prepare_keyrange_request(cmd: _CommandType, key_min: Key, key_max: Key, aux_size: int = 0) -> bytes:
        return b"".join(e.to_bytes() for e in (RequestHeader(cmd, aux_size), key_min, key_max))

    @staticmethod
    def _parse_record(buffer: bytes, payload_type: PayloadType[T]) -> Record[T] | None:
        key: Key | None = Key.from_bytes(buffer[:FULL_KEY_SIZE])
        if key is None:
            return None
        value: T | None = payload_type.from_bytes(buffer[FULL_KEY_SIZE:])
        if value is None:
            return None
        return Record(key, value)

    @staticmethod
    def _group_records_by_cid(data: RecordsSet[T]) -> dict[int, list[Record[T]]]:
        result: defaultdict[int, list[Record[T]]] = defaultdict(list)
        for record in data:
            result[record.key.cid].append(record)
        return result

    @staticmethod
    def _serialize_records_batches_iter(
        data: RecordsSet[T],
        with_acq: bool,
        payload_type: PayloadType[T],
        max_batch_size: int = 2147483647,
        skip_invalid: bool = False,
    ) -> Iterable[bytes]:
        raw_records: bytearray = bytearray()
        for cid, records in _ChannelMixin._group_records_by_cid(data).items():
            for record in records:
                if not record.key.valid():
                    if skip_invalid:
                        continue
                    else:
                        yield b"".join((struct.pack("<ii", cid, len(raw_records)), raw_records))
                        return
                raw_key: bytes = record.key.to_bytes(with_cid=False, with_acq=with_acq)
                raw_payload: bytes = payload_type.to_bytes(record.value)
                size: int = len(raw_key) + len(raw_payload)
                if 0 < len(raw_records) > (max_batch_size - size):
                    yield b"".join((struct.pack("<ii", cid, len(raw_records)), raw_records))
                    raw_records.clear()
                raw_records += struct.pack("<i", size)
                raw_records += raw_key
                raw_records += raw_payload
            yield b"".join((struct.pack("<ii", cid, len(raw_records)), raw_records))
            raw_records.clear()

    @staticmethod
    def _handle_response(buffer: ReceiveBuffer, format: str = "") -> tuple[RequestHeader, tuple[Any, ...]] | None:
        header_size: int = RequestHeader.bytes_count()
        if buffer.fits(header_size):
            response: RequestHeader = RequestHeader.from_bytes(buffer.peek(header_size))
            if buffer.fits(header_size + response.size):
                parser = struct.Struct(format)
                data = parser.unpack(buffer.peek(parser.size, header_size)) if response.is_ok() else ()
                buffer.increase(header_size + response.size)
                return response, data
        return None

    @staticmethod
    def _parse_records(
        buffer: ReceiveBuffer, records: RecordsSet[T], payload_type: PayloadType[T], max_size: int | None
    ) -> RecordsParsingStatus:
        int_parser = struct.Struct("<i")
        while buffer.fits(int_parser.size):
            record_size: int = int_parser.unpack(buffer.peek(int_parser.size))[0]
            if record_size == 0:
                buffer.increase(int_parser.size)
                buffer.truncate()  # Truncate so confirmation header fits
                return RecordsParsingStatus.FINISHED
            elif buffer.fits(int_parser.size + record_size):
                record: Record[T] | None = _ChannelMixin._parse_record(
                    buffer.peek(record_size, int_parser.size), payload_type
                )
                buffer.increase(int_parser.size + record_size)
                if record is None:
                    return RecordsParsingStatus.UNPARSEABLE
                records.append(record)
            elif max_size is not None and int_parser.size + record_size > max_size:
                return RecordsParsingStatus.RECORD_TOO_BIG
            else:
                buffer.truncate()  # Truncate so record will fit
                if not buffer.fits_eventually(int_parser.size + record_size):
                    buffer.grow_buffer(int_parser.size + record_size)
                return RecordsParsingStatus.NEEDS_MORE_BYTES
        if not buffer.fits_eventually(int_parser.size):
            buffer.truncate()
        return RecordsParsingStatus.NEEDS_MORE_BYTES
