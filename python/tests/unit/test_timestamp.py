import datetime as dt

import pytest

from tstorage_client.timestamp import (
    Tstoragedatetime,
    from_unix,
    from_unix_ns,
    to_unix,
    to_unix_ns,
)


@pytest.mark.parametrize(
    "value,expected",
    [
        (0, 978307200),
        (978307200, 1956614400),
        (978307200.14, 1956614400.14),
    ],
)
def test_to_unix(value: int | float, expected: int | float) -> None:
    assert to_unix(value) == (pytest.approx(expected) if isinstance(expected, float) else expected)


@pytest.mark.parametrize(
    "value,expected",
    [
        (0, 978307200000000000),
        (1000, 978307200000001000),
    ],
)
def test_to_unix_ns(value: int, expected: int) -> None:
    assert to_unix_ns(value) == expected


@pytest.mark.parametrize(
    "value,expected",
    [
        (978307200, 0),
        (978307412.321, 212.321),
    ],
)
def test_from_unix(value: int | float, expected: int | float) -> None:
    assert from_unix(value) == (pytest.approx(expected) if isinstance(expected, float) else expected)


@pytest.mark.parametrize(
    "value,expected",
    [
        (978307200000000000, 0),
        (978307200000004321, 4321),
        (978307432100000000, 232100000000),
    ],
)
def test_from_unix_ns(value: int, expected: int) -> None:
    assert from_unix_ns(value) == expected


@pytest.mark.parametrize(
    "value,expected",
    [
        (Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 0), 0.0),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 123), 0.000000123),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, 0, 0, 1, tzinfo=dt.timezone.utc), 123), 1.000000123),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, 0, 0, 1, tzinfo=dt.timezone.utc), 999), 1.000000999),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, 1, 0, 1, tzinfo=dt.timezone.utc), 0), 3601.0),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, 1, 0, 1, tzinfo=dt.timezone.utc), 1), 3601.000000001),
    ],
)
def test_tstoragedatetime_to_unix(value: Tstoragedatetime, expected: float) -> None:
    assert value.to_unix() == pytest.approx(expected)


@pytest.mark.parametrize(
    "value,expected",
    [
        (Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 0), 0),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 123), 123),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 999), 999),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, 0, 0, 1, tzinfo=dt.timezone.utc), 123), 1000000123),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, 1, 0, 1, tzinfo=dt.timezone.utc), 0), 3601000000000),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, 1, 0, 1, tzinfo=dt.timezone.utc), 1), 3601000000001),
    ],
)
def test_tstoragedatetime_to_unix_ns(value: Tstoragedatetime, expected: int) -> None:
    assert value.to_unix_ns() == expected


@pytest.mark.parametrize(
    "value,expected",
    [
        (0.0, Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 0)),
        (0.000000004, Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 4)),
        (0.000000999, Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 999)),
        (0.000001000, Tstoragedatetime(dt.datetime(1970, 1, 1, microsecond=1, tzinfo=dt.timezone.utc), 0)),
        (123.000000004, Tstoragedatetime(dt.datetime(1970, 1, 1, 0, 2, 3, tzinfo=dt.timezone.utc), 4)),
    ],
)
def test_tstoragedatetime_from_unix(value: float, expected: Tstoragedatetime) -> None:
    assert Tstoragedatetime.from_unix(value) == expected


@pytest.mark.parametrize(
    "value,expected",
    [
        (0, Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 0)),
        (4, Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 4)),
        (123000000004, Tstoragedatetime(dt.datetime(1970, 1, 1, 0, 2, 3, tzinfo=dt.timezone.utc), 4)),
        (123000000999, Tstoragedatetime(dt.datetime(1970, 1, 1, 0, 2, 3, tzinfo=dt.timezone.utc), 999)),
    ],
)
def test_tstoragedatetime_from_unix_ns(value: int, expected: Tstoragedatetime) -> None:
    assert Tstoragedatetime.from_unix_ns(value) == expected


@pytest.mark.parametrize(
    "value",
    [
        (0),
        (1234),
        (1234.123456789),
        (1234.000001),
        (1234.000000001),
        (1234.000000002),
        (1234.000000010),
        (1234.000000999),
        (1234.000001000),
        (978307200),
        (1742239340),
        (2742239340),
    ],
)
def test_tstoragedatetime_from_unix_to_unix(value: int) -> None:
    assert Tstoragedatetime.from_unix(value).to_unix() == value


@pytest.mark.parametrize(
    "value",
    [
        (Tstoragedatetime(dt.datetime(1, 1, 1, 1, tzinfo=dt.timezone.utc), 0)),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 0)),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 1)),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 99)),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 999)),
    ],
)
def test_tstoragedatetime_to_unix_from_unix(value: Tstoragedatetime) -> None:
    assert Tstoragedatetime.from_unix(value.to_unix()) == value


@pytest.mark.parametrize(
    "value",
    [
        (0),
        (1234),
        (978307200000000000),
        (1742239340),
        (2742239340),
        (1742240105369765237),
        (1742240120737486956),
        (1742295372988643991),
        (763988172988643991),
        (3601000000000),
        (3601000000001),
        (3601000000999),
    ],
)
def test_tstoragedatetime_from_unix_ns_to_unix_ns(value: int) -> None:
    assert Tstoragedatetime.from_unix_ns(value).to_unix_ns() == value


@pytest.mark.parametrize(
    "value",
    [
        (Tstoragedatetime(dt.datetime(1, 1, 1, 1, tzinfo=dt.timezone.utc), 0)),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 0)),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 1)),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 99)),
        (Tstoragedatetime(dt.datetime(1970, 1, 1, tzinfo=dt.timezone.utc), 999)),
    ],
)
def test_tstoragedatetime_to_unix_ns_from_unix_ns(value: Tstoragedatetime) -> None:
    assert Tstoragedatetime.from_unix_ns(value.to_unix_ns()) == value
