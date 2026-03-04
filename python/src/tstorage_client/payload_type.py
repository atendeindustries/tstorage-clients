"""Provides PayloadType interface and simple ready to use implementations"""

import struct
from abc import ABC, abstractmethod
from typing import Any, ClassVar, Generic, Literal, TypeVar

from tstorage_client.record import Key


try:
    HAS_NUMPY = True
    import numpy
except ImportError:
    HAS_NUMPY = False


__all__ = ["PayloadType", "StructPayloadType", "TuplePayloadType", "UnitPayloadType", "BytesPayloadType"]

if HAS_NUMPY:
    __all__.append("NumpyPayloadType")


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


class TuplePayloadType(PayloadType[tuple[Any, ...]]):
    """Class for simple multi-struct module based serialization."""

    def __init__(self, format: str) -> None:
        self._format = struct.Struct(format)

    def to_bytes(self, value: tuple[Any, ...]) -> bytes:
        return self._format.pack(*value)

    def from_bytes(self, buffer: bytes) -> tuple[Any, ...] | None:
        value: tuple[Any, ...]
        try:
            value = self._format.unpack(buffer)
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


class NumpyPayloadType(PayloadType[Any]):
    """Support for NumPy based data retrieval and processing.

    With NumpyPayloadType (Async)Channel.put(a) accepts:
     - NumPy's structured array or Recarray with '_size' field of any values and standard Record fields.
     - Dictionary of NumPy's arrays of standard Record fields.

    With NumpyPayloadType (Async)Channel.get functions family returns NumPy's structured array with standard Record fields.
    """

    _init_args: ClassVar = list[tuple[str, type]] | numpy.dtype | None if HAS_NUMPY else Any

    def __init__(self, dtype: _init_args) -> None:
        if not HAS_NUMPY:
            raise RuntimeError("NumPy is not installed!")
        size_dtype = [("_size", numpy.int32)]
        if dtype is None:
            dtype = []
        if isinstance(dtype, list):
            self.serializer_dtype_with_acq = numpy.dtype(size_dtype + Key._numpy_dtype_list[1:] + dtype)
            self.serializer_dtype_no_acq = numpy.dtype(size_dtype + Key._numpy_dtype_list[1:-1] + dtype)
            self.parsing_dtype = numpy.dtype(size_dtype + Key._numpy_dtype_list + dtype)
            self.record_dtype = numpy.dtype(Key._numpy_dtype_list + dtype)
        else:
            wrapped = numpy.dtype(dtype)
            if wrapped.fields is None:
                wrapped = numpy.dtype([("v0", wrapped)])
            self.serializer_dtype_with_acq = numpy.dtype(
                size_dtype + Key._numpy_dtype_list[1:] + [(name, desc[0]) for name, desc in wrapped.fields.items()]
            )
            self.serializer_dtype_no_acq = numpy.dtype(
                size_dtype + Key._numpy_dtype_list[1:-1] + [(name, desc[0]) for name, desc in wrapped.fields.items()]
            )
            self.parsing_dtype = numpy.dtype(
                size_dtype + Key._numpy_dtype_list + [(name, desc[0]) for name, desc in wrapped.fields.items()]
            )
            self.record_dtype = numpy.dtype(
                Key._numpy_dtype_list + [(name, desc[0]) for name, desc in wrapped.fields.items()]
            )

    def to_bytes(self, array: Any) -> bytes:
        return array.tobytes()  # type: ignore[no-any-return]

    def from_bytes(self, buffer: bytes) -> Any | None:
        try:
            return numpy.frombuffer(buffer)
        except ValueError:
            return None
