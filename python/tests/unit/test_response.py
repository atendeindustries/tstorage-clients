import pytest

from tstorage_client.response import Response, ResponseStatus


@pytest.mark.parametrize(
    "response,expected",
    [
        (ResponseStatus.OK, True),
        (ResponseStatus.ERROR, False),
    ],
)
def test_response_status_bool(response: ResponseStatus, expected: bool) -> None:
    assert bool(response) == expected


@pytest.mark.parametrize(
    "response,expected",
    [
        (ResponseStatus.OK, True),
        (ResponseStatus.ERROR, False),
    ],
)
def test_response_status_is_ok(response: ResponseStatus, expected: bool) -> None:
    assert response.is_ok() == expected


@pytest.mark.parametrize(
    "response,expected",
    [
        (Response(ResponseStatus.OK), True),
        (Response(ResponseStatus.ERROR), False),
    ],
)
def test_response_bool(response: ResponseStatus, expected: bool) -> None:
    assert bool(response) == expected


@pytest.mark.parametrize(
    "response,expected",
    [
        (Response(ResponseStatus.OK), True),
        (Response(ResponseStatus.ERROR), False),
    ],
)
def test_response_is_ok(response: ResponseStatus, expected: bool) -> None:
    assert response.is_ok() == expected
