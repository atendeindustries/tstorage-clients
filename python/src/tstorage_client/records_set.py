from abc import abstractmethod
from typing import Callable, Iterable, Protocol, Sized, TypeVar

from .record import Record


__all__ = "GetCallback", "RecordsSet"


T = TypeVar("T")


class RecordsSet(Iterable[Record[T]], Sized, Protocol):
    """RecordsSet[T] is iterable, sized and appendable abstract collection of Record[T]"""

    @abstractmethod
    def append(self, value: Record[T]) -> None: ...


GetCallback = Callable[[RecordsSet[T]], None]
"""GetCallback[T] is an generic alias to any callable taking RecordsSet[T] as argument and returning nothing."""
