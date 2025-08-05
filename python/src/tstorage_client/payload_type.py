"""Provides PayloadType interface and simple ready to use implementations"""

import struct
from abc import ABC, abstractmethod
from typing import Any, Generic, Literal, TypeVar


__all__ = "PayloadType", "StructPayloadType", "UnitPayloadType", "BytesPayloadType"


T = TypeVar("T")


class PayloadType(ABC, Generic[T]):
    """Interface for payload serializers."""

    @abstractmethod
    def to_bytes(self, value: T) -> bytes:
        """Converts provided value to bytes to be send to TStorage."""
        ...

    @abstractmethod
    def from_bytes(self, buffer: bytes) -> T | None:
        """Converts bytes to value or None in case of failure."""
        ...


class StructPayloadType(PayloadType[T]):
    """Class for simple struct module based serialization.

    Only 1 value formats are supported.
    """

    def __init__(self, format: str) -> None:
        self._format = struct.Struct(format)

    def to_bytes(self, value: T) -> bytes:
        return self._format.pack(value)

    def from_bytes(self, buffer: bytes) -> T | None:
        value: T
        try:
            value = self._format.unpack(buffer)[0]
            return value
        except struct.error:
            return None


class UnitPayloadType(PayloadType[tuple[()]]):
    """Serialization of empty payloads."""

    def to_bytes(self, _: Any) -> Literal[b""]:
        """Converts provided value to empty bytes object."""
        return b""

    def from_bytes(self, _: bytes) -> tuple[()]:
        """Returns empty tuple. It never fails so it never returns None."""
        return ()


class BytesPayloadType(PayloadType[bytes]):
    """Serialization of raw bytes-like payloads.

    Builtin bytearray and memoryview are considered by mypy to be bytes compatible
    so other type than bytes can be returned.
    (https://mypy.readthedocs.io/en/latest/duck_type_compatibility.html).
    """

    def to_bytes(self, value: bytes) -> bytes:
        """Returns value as is."""
        return value

    def from_bytes(self, buffer: bytes) -> bytes:
        """Returns buffer as is."""
        return bytes(buffer)
