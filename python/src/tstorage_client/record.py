import struct
from dataclasses import dataclass
from typing import ClassVar, Generic, TypeVar


__all__ = "Key", "Record"


T = TypeVar("T")


@dataclass(order=True, frozen=True, slots=True)
class Key:
    """TStorage Key.

    Keys must be unique amongst whole TStorage.

    Attributes:
        cid: Integer in range 0 to 2^31 - 1.
        mid: Integer in range -2^63 to 2^63 - 1.
        moid: Integer in range -2^31 to 2^31 - 1.
        cap: Integer in range -2^63 to 2^63 - 1.
        acq: Integer in range -2^63 to 2^63 - 1.
    """

    cid: int = -1
    mid: int = -1
    moid: int = -1
    cap: int = -1
    acq: int = -1

    _to_bytes_format: ClassVar[struct.Struct] = struct.Struct("<iqiqq")

    @classmethod
    def min(cls) -> "Key":
        return cls(0, -(2**63), -(2**31), -(2**63), -(2**63))

    @classmethod
    def max(cls) -> "Key":
        return cls(2**31 - 1, 2**63 - 1, 2**31 - 1, 2**63 - 1, 2**63 - 1)

    def to_bytes(self, *, with_cid: bool = True, with_acq: bool = True) -> bytes:
        match with_cid, with_acq:
            case True, True:
                return struct.pack("<iqiqq", self.cid, self.mid, self.moid, self.cap, self.acq)
            case True, False:
                return struct.pack("<iqiq", self.cid, self.mid, self.moid, self.cap)
            case False, True:
                return struct.pack("<qiqq", self.mid, self.moid, self.cap, self.acq)
            case False, False:
                return struct.pack("<qiq", self.mid, self.moid, self.cap)
            case _:
                raise TypeError()

    @classmethod
    def from_bytes(cls, data: bytes) -> "Key | None":
        try:
            key = cls(*cls._to_bytes_format.unpack(data))
            return key if key.valid() else None
        except struct.error:
            return None

    def valid(self) -> bool:
        return self.cid >= 0

    @classmethod
    def bytes_count(cls) -> int:
        return cls._to_bytes_format.size


@dataclass(order=True, frozen=True, slots=True)
class Record(Generic[T]):
    """Record as stored by TStorage with type generic value.

    Attributes:
        key: Key idnetyfying this Record.
        value: Payload as stored by TStorage.
    """

    key: Key
    value: T
