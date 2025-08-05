# TStorage Database Java Driver

A lightweight and efficient Java driver for interacting with a TStorage NoSQL database, built around a generic and extensible `Channel<T>` API. Designed for binary data transfer with precise control over serialization, byte order, and acquisition timestamps.

## Features

- 🧩 Generic API using `Channel<T>` for typed communication
- 💾 Support for custom serialization via `PayloadType<T>`
- 🧵 Stream-based `getStream(...)` for efficient chunked data retrieval
- 🗃️ Records represented by `Key`, `Record<T>`, and `RecordsSet<T>`
- 📦 Robust response types: `Response`, `ResponseGet<T>`, `ResponseAcq`
- 🧭 Timestamp conversions between TStorage epoch and Unix time
- 🔧 Configurable byte order and connection timeout
- ❌ Minimal external dependencies (JDK-only, up to Java 21)

## Getting Started

### Requirements

- Java 21 or later (tested on Java 21)
- No external dependencies required

### Installation

Clone this repository and include it in your build (e.g., using Gradle or Maven).

```bash
git clone https://github.com/...
```

As a Maven dependency to your project:

```xml
    <dependency>
        <groupId>industries.atende.ts.driver</groupId>
        <artifactId>ts_driver_java</artifactId>
        <version>0.0.1-SNAPSHOT</version>
    </dependency>
```

### Example API Usage

Check out the example/tester_project_0/src/main/java/industries/atende/ts/Main.java file.

## Core Components

### `Channel<T>`

Main entry point for communication. Provides:

- `connect()`, `close()`
- `get(...)`, `getStream(...)`
- `put(...)`, `puta(...)`
- `getAcq(...)`

### `PayloadType<T>`

Interface for converting data to and from bytes:

```java
byte[] toBytes(T value);
Optional<T> fromBytes(byte[] buffer);
```

### `Key`

Represents a record identifier:

- `cid` (client ID), `mid` (meter ID), `moid` (meter object ID)
- `cap` (capture timestamp), `acq` (acquisition timestamp)
- Includes static factory methods: `Key.min()` and `Key.max()`

### `Record<T>` and `RecordsSet<T>`

- `Record<T>` binds a `Key` to a payload
- `RecordsSet<T>` is a collection interface for records with `append(...)`, `iterator()`, and `size()`

### `Response` types

- `Response`: base class for results
- `ResponseGet<T>`: includes `RecordsSet<T>`
- `ResponseAcq`: includes acquisition timestamp

### `Timestamp`

Utility class for nanosecond timestamps based on a TStorage 2001-01-01 epoch:

```java
Instant toUnix(long timestamp);
long fromUnix(Instant instant);
long now();
```

## Response Statuses

| Status              | Meaning                                 |
|---------------------|-----------------------------------------|
| `OK`                | Operation succeeded                     |
| `ERROR`             | General failure                         |
| `LIMIT_TOO_LOW`     | Configured byte limit was too low       |
| `CONVERSION_ERROR`  | Serialization or deserialization failed |

## Contributing

Contributions are welcome! Please open issues for bugs or submit pull requests with improvements.

## License

Apache License Version 2.0, January 2004 - see the [`LICENSE`](LICENSE) file for details.

---

> ⚠️ **Note:** This driver is designed for a specific TStorage protocol. Compatibility and stability across versions is not guaranteed.
