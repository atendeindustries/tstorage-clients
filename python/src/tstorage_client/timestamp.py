"""Utilities for storing and converting TStorage and unix timestamps"""

import datetime as dt
import time
from dataclasses import dataclass
from typing import overload


__all__ = "Tstoragedatetime", "to_unix", "to_unix_ns", "from_unix", "from_unix_ns", "now", "now_ns"


@dataclass(order=True)
class Tstoragedatetime:
    """Tstoragedatetime stores nanosecond precise time in friendly manner.

    Attributes:
        datetime: Python datetime part of nanosecond precise time.
        nanoseconds: Remaining nanoseconds in range 0-999.
    """

    datetime: dt.datetime
    nanoseconds: int

    def to_unix(self) -> float:
        """Convert to unix seconds with nanosecond precise fractions."""
        return self.to_unix_ns() / 10**9

    def to_unix_ns(self) -> int:
        """Convert to unix nanoseconds."""
        value: int = int(self.datetime.timestamp()) * 10**6 + self.datetime.microsecond
        return value * 1000 + self.nanoseconds

    @classmethod
    def from_unix(cls, timestamp: int | float, *, tzinfo: dt.tzinfo = dt.timezone.utc) -> "Tstoragedatetime":
        """Create Tstoragedatetime from unix seconds."""
        return cls.from_unix_ns(int(timestamp * 10**9), tzinfo=tzinfo)

    @classmethod
    def from_unix_ns(cls, timestamp: int, *, tzinfo: dt.tzinfo = dt.timezone.utc) -> "Tstoragedatetime":
        """Create Tstoragedatetime from unix nanoseconds."""
        nanos: int = timestamp % 1000
        micros: int = (timestamp // 1000) % 10**6
        seconds: int = timestamp // 10**9
        datetime: dt.datetime = dt.datetime.fromtimestamp(seconds, tzinfo)
        datetime = datetime.replace(microsecond=micros)
        return cls(datetime, nanos)

    def to_tstorage(self) -> float:
        """Convert to TStorage seconds with nanosecond precise fractions."""
        return from_unix(self.to_unix())

    def to_tstorage_ns(self) -> int:
        """Convert to TStorage nanoseconds."""
        return from_unix_ns(self.to_unix_ns())

    @classmethod
    def from_tstorage(cls, timestamp: int | float, *, tzinfo: dt.tzinfo = dt.timezone.utc) -> "Tstoragedatetime":
        """Create Tstoragedatetime from TStorage seconds."""
        return cls.from_unix(to_unix(timestamp), tzinfo=tzinfo)

    @classmethod
    def from_tstorage_ns(cls, timestamp: int, *, tzinfo: dt.tzinfo = dt.timezone.utc) -> "Tstoragedatetime":
        """Create Tstoragedatetime from TStorage nanoseconds.

        This function directly converts cap and acq values to user friendly format.
        """
        return cls.from_unix_ns(to_unix_ns(timestamp), tzinfo=tzinfo)


@overload
def to_unix(timestamp: int) -> int: ...
@overload
def to_unix(timestamp: float) -> float: ...
def to_unix(timestamp: int | float) -> int | float:
    """Converts TStorage second timestamp to unix second timestamp.

    Args:
        timestamp: TStorage timestamp in seconds.

    Returns:
        Unix timestamp in seconds.
    """
    DIFF_2001_1970_s = 978307200
    return timestamp + DIFF_2001_1970_s


def to_unix_ns(timestamp: int) -> int:
    """Converts TStorage nanosecond timestamp to unix nanosecond timestamp.

    Args:
        timestamp: TStorage timestamp in nanoseconds.

    Returns:
        Unix timestamp in nanoseconds.
    """
    DIFF_2001_1970_ns = 978307200000000000
    return timestamp + DIFF_2001_1970_ns


@overload
def from_unix(timestamp: int) -> int: ...
@overload
def from_unix(timestamp: float) -> float: ...
def from_unix(timestamp: int | float) -> int | float:
    """Converts unix second timestamp to TStorage second timestamp.

    Args:
        timestamp: Unix timestamp in seconds.

    Returns:
        TStorage timestamp in seconds.
    """
    DIFF_2001_1970_s = 978307200
    return timestamp - DIFF_2001_1970_s


def from_unix_ns(timestamp: int) -> int:
    """Converts unix nanosecond timestamp to TStorage nanosecond timestamp.

    Args:
        timestamp: Unix timestamp in nanoseconds.

    Returns:
        TStorage timestamp in nanoseconds.
    """
    DIFF_2001_1970_ns = 978307200000000000
    return timestamp - DIFF_2001_1970_ns


def now() -> float:
    """Get current TStorage timestamp in seconds"""
    return from_unix(time.time())


def now_ns() -> int:
    """Get current TStorage timestamp in nanoseconds"""
    return from_unix_ns(time.time_ns())
