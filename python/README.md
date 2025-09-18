# TStorage client

A dedicated python client for communication with TStorage database.
Built to simplify integration of TStorage with python projects, enabling developers to communicate with the database efficiently and with minimal setup.


## Table of Contents

- [Description](#description)
- [Requirements](#requirements)
- [Installation](#installation)
- [Interface](#interface)
    - [Channel(_ChannelMixin[T])](#channel_channelmixint)
    - [PayloadType(ABC, Generic[T]))](#payloadtypeabc-generict)
    - [Key](#key)
    - [Record(Generic[T]) and RecordsSet(...)](#recordgenerict-and-recordsset)
    - [Responses](#responses)
    - [Timestamp helpers](#timestamp-helpers)
- [Usage](#usage)
- [Configuration](#configuration)
- [Example app](#example-app)
- [License](#license)


## Description

The main functionalities of the client are getting and adding new data from/to TStorage. Both socket and asyncio based connections are supported by Channel and AsyncChannel classes. Before sending any request, it is necessary to connect to a running TStorage instance. It can be done by using `connect()` and `close()` functions pairs or by context manager (`(async) with` statement).

The following kinds of requests are supported:
 - `put` - add new records to TStorage where acq value is being set by TStorage
 - `puta` - add new records to TStorage where acq value is being set by user
 - `get_acq` - get last acq timestamp
 - `get` - get records
 - `get_stream` - get records in batches. Should be used when records may require more memory than the system can provide
 - `get_iter` - get records yielding one by one. Usable as iterator and also when records would require more memory than the system can provide


## Requirements

- python >= 3.10


### Optional

- mypy >= 1.14
- black >= 24.10
- flake8 >= 7.1
- Flake8-pyproject >= 1.2
- isort >= 5.13
- ruff >= 0.9
- pytest >= 8.3
- pytest-asyncio >= 0.26
- build >= 1.2


## Installation

1. Clone the repository.
    ```bash
    git clone https://github.com/atendeindustries/tstorage-clients.git
    ```

2. Add python TStorage library to your project or install it in your environment.
    ```bash
    pip install tstorage-client/python
    ```

3. Import TStorage library into your code.
    ```python
    from tstorage_client.channel import Channel
    from tstorage_client.records_set import RecordsSet
    ...
    ```


## Interface

### Channel(_ChannelMixin[T])

Main entry point for communication. Provides:

- `connect()`, `close()`
- `get(...)`, `get_stream(...)`, `get_iter`
- `put(...)`, `puta(...)`
- `get_acq(...)`


### PayloadType(ABC, Generic[T])

Interface for converting data to and from bytes:

```python
    def to_bytes(self, value: T) -> bytes:
    def from_bytes(self, buffer: bytes) -> T | None:
```


### Key

Represents a record identifier:
- `cid` - client ID
- `mid` - meter ID
- `moid` - meter object ID
- `cap` - capture timestamp in nanoseconds
- `acq` - acquisition timestamp in nanoseconds, during normal operation, TStorage takes care of fulfilling this


### Record(Generic[T]) and RecordsSet(...)

- `Record(Generic[T])` - binds a `Key` to a payload
- `RecordsSet(...)` - a collection interface for records


### Responses

- `Response` - base class for results
- `ResponseGet(ResponseAcq, Generic[T])` - response containing result and `RecordsSet(...)`
- `ResponseAcq(Response)` - response containing result and acquisition timestamp


### Timestamp helpers

- `Tstoragedatetime` - stores nanosecond precise time in friendly manner, consists of python's dt.datetime and nanoseconds data
    - `from_tstorage(timestamp: int | float) -> Tstoragedatetime  /  from_tstorage_ns(timestamp: int) -> Tstoragedatetime` - creates Tstoragedatetime from TStorage seconds/nanoseconds
    - `from_unix(timestamp: int | float) -> Tstoragedatetime  /  from_unix_ns(timestamp: int) -> Tstoragedatetime` - creates Tstoragedatetime from unix seconds/nanoseconds
    - `to_tstorage() -> float  /  to_tstorage_ns() -> int` - converts Tstoragedatetime to TStorage seconds/nanoseconds
    - `to_unix() -> float / to_unix_ns() -> int` - convert Tstoragedatetime to unix seconds/nanoseconds
- `to_unix(timestamp: int | float) -> int | float  /  to_unix_ns(timestamp: int) -> int` - converts TStorage seconds/nanoseconds timestamp to unix seconds/nanoseconds timestamp
- `from_unix(timestamp: int | float) -> int | float  /  from_unix_ns(timestamp: int) -> int` - converts unix seconds/nanoseconds timestamp to TStorage seconds/nanoseconds timestamp


## Usage

Let's define some data to help you understand how to use the client correctly. Our main goal here is to learn basic commands for sending and getting data to/from TStorage.

1. First, let's assume the type of data we want to send/receive. Our records may look as follows:
    ```python
    [cid, mid, moid, cap, payload(int, float, bool)]
    [1, 4, 7, 1234, payload(10, 12.0, True)]
    [2, 5, 8, 4567, payload(11, 23.0, False)]
    [3, 6, 9, 7890, payload(12, 34.0, True)]
    ```

2. One important thing to remember is that TStorage stores time data (Cap, Acq) in its own format. Therefore, conversion from Unix time to TStorage time is necessary. To ensure easy and quick conversion, we provide timestamp.py module. It contains all the functions needed for correct time transformation. Below you can find an example of conversion from a string iso date format to TStorage nanoseconds format.

    ```python
    import datetime as dt
    from tstorage_client.timestamp import Tstoragedatetime
    date = dt.datetime.fromisoformat("2016-08-09 12:34:00")
    ts_date_ns = Tstoragedatetime(datetime=date, nanoseconds=0).to_tstorage_ns()
    ```

3. When we have defined our payload data structure and our timestamp data is converted, we can move to the next step, which is implementation of the PayloadType serializer. It is critically required so that the TStorage client can correctly work with the payload data.

    PayloadType interface, that we're going to implement:
    ```python
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
    ```

There are two path that we can follow:
- providing implementation for some custom type
    ```python
    class MyType:
        def __init__(self, x: int, y: float, z: bool):
            self.x = x
            self.y = y
            self.z = z

    class MyTypePayloadType(PayloadType[MyType]):
        _format = struct.Struct("<if?")
        def to_bytes(self, value: MyType) -> bytes:
            return self._format.pack(value.x, value.y, value.z)

        def from_bytes(self, buffer: bytes) -> MyType | None:
            return MyType(*self._format.unpack(buffer))
    ```

- using the built-in library TuplePayloadType or StructPayloadType classes, that can do a single/multi type serializaton base on provided format
    ```python
    payload_type_serializer = TuplePayloadType("if?") # "if?" corresponds to our payload(int, float, bool)
    ```

4. When we have the serializer, we can construct and use the Channel to interact with the database.

    Putting data into TStorage via `put` command. The first step is to establish a connection to TStorage. After that we can prepare our data and send it through the Channel. It is worth mentioning that the `now_ns()` function used below returns the current time in TStorage format. The result of the request can be considered as executed successfuly if the received `put_response.is_ok()` returns `True`.
    ```python
    PayloadType = tuple[int, float, bool]
    with Channel(host, port, payload_type_serializer) as channel:
            records: RecordsSet[PayloadType]  =
            [
                Record(Key(1, 4, 7, now_ns()), PayloadType((10, 24.0, True))),
                Record(Key(2, 5, 8, now_ns()), PayloadType((10, 34.0, False))),
                Record(Key(3, 6, 9, now_ns()), PayloadType((10, 44.0, True)))
            ]
            acq_before_put: int = now_ns()
            put_response: Response = channel.put(records)
            acq_after_put: int = now_ns()
            if not put_response.is_ok():
                raise RuntimeError("Failed to put data to TStorage!")
    ```

    After putting our data into TStorage, we can get them back via `get` command. Same as in the case of putting data, the first step is to establish a connection to TStorage. The next step is to define ranges of the request. To do achive this, we have to create two range keys, the min one, for the lower closed boundary and the max one, for the upper open boundary. It may seem a little problematic that we have to provide also ranges for Acq but it is required in order to retrieve the data from the correct acquisition time range. These values should be selected reasonably to narrow the search area, but it is also safe to set them to the maximum range, i.e. [ int64_t::min(), int64_t::max() ). In our case, to narrow Acq range, we captured a time twice, the first one before putting the data - `acq_before_put`, and the second one after the put - `acq_after_put`. The result of the request can be considered as executed successfuly if the received `get_response.is_ok()` returns `True`.
    ```python
    with Channel(host, port, payload_type_serializer) as channel:
            key_min = Key(cid=1, mid=4, moid=7, cap=0, acq=acq_before_put)
            key_max = Key(cid=4, mid=7, moid=10, cap=now_ns(), acq=acq_after_put)
            get_response: ResponseGet = channel.get(key_min, key_max)
            if not get_response.is_ok():
                raise RuntimeError("Failed to get data from TStorage!")
            for r in get_response.data:
                print(f"Cid: {r.key.cid}, Mid:{r.key.mid}, Moid: {r.key.moid}, Cap:{r.key.cap}, Acq: {r.key.acq}, PayloadType( int: {r.value[0]}, float: {r.value[1]}, bool: {r.value[2]}")
    ```

5. Check the basic example code in tstorage_clients/python/tests/integration/example_usage.py.

## Configuration

The channel provides several options to help you customize your connection with TStorage:
- `timeout` - the connection timeout
- `memory_limit` - maximum memory used for GET requests in bytes
- `max_batch_size` - used in `put` and `puta` commands, controls maximal serialization buffer size.


## Example app

In the library you can find also an example app that uses TStorage client. The app includes basic `put` and `get_iter` for iterative traversal of received records. To check how the application works, try the following command:

```bash
python -m tstorage_client
```

You can browse tests for more examples.


## Tests

To check correctness of TStorage library you can run tests. To do this follow:

```bash
python -m build
docker compose -f tests/docker_compose.yaml up
```


## License

[Apache License 2.0](LICENSE)