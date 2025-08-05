# TStorage client

TStorage client allows communication with TStorage.

## Usage

Client is capable of getting data from TStorage and adding new data to TStorage. Both socket and asyncio based connections are supported by Channel and AsyncChannel classes. Before issuing any request it is necessary to connect to running TStorage instance. It can be done by using `connect()` and `close()` functions pairs or by context manager (`(async) with` statement).

Following kinds of requests are supported:
 - `put` - add new records to TStorage where acq value is being set by TStorage.
 - `puta` - add new records to TStorage where acq value is being set by user.
 - `get_acq` - get last acq timestamp.
 - `get` - get records.
 - `get_stream` - get records in batches. Usable when records would use more memory than available on the system.
 - `get_iter` - get records yielding one by one. Usable as iterator and also when records would use more memory than available on the system.

For more details please refer to documentation and provided examples.

## Examples

TStorage client contains example application that uses basic commands. It can be run after installing with:

```
python -m tstorage_client
```

You can browse tests for more examples.

## Running tests in docker

```sh
python -m build
docker compose -f tests/docker_compose.yaml up
```

## License

[Apache License 2.0](LICENSE)