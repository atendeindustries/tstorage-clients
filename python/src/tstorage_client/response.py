"""Module containing network response class hierarchy"""

from dataclasses import dataclass, field
from enum import IntEnum
from typing import Generic, TypeVar

from .records_set import RecordsSet


__all__ = "ResponseStatus", "Response", "ResponseAcq", "ResponseGet"


T = TypeVar("T")


class ResponseStatus(IntEnum):
    """Enumeration of possible return codes from various functions."""

    OK = 0
    ERROR = -1

    DISCONNECTED = 128
    BAD_REQUEST = 129
    UNPARSEABLE_ENTITY = 130
    NO_MEMORY = 131

    def __bool__(self) -> bool:
        """Check if status does not represent error condition."""
        return self.value == ResponseStatus.OK

    def is_ok(self) -> bool:
        """Check if status does not represent error condition."""
        return bool(self)


@dataclass(frozen=True, slots=True)
class Response:
    """Base response from network calls.

    Attributes:
        status: Error status of function.
    """

    status: ResponseStatus

    def __bool__(self) -> bool:
        """Check if status does not represent error condition."""
        return bool(self.status)

    def is_ok(self) -> bool:
        """Check if status does not represent error condition."""
        return bool(self)


@dataclass(frozen=True, slots=True)
class ResponseAcq(Response):
    """Base response from GET call.

    Attributes:
        status: Error status of function.
        acq: Returned acq value from TStorage.
    """

    acq: int = -1


@dataclass(frozen=True, slots=True)
class ResponseGet(ResponseAcq, Generic[T]):
    """Response from GET call that carries records.

    Attributes:
        status: Error status of function.
        acq: Returned acq value from TStorage.
        data: Returned records of requested type from TStorage.
    """

    data: RecordsSet[T] = field(default_factory=list)
