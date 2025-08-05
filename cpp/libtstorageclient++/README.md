# libtstorageclient++

A C++14 library providing client functionality for integration with TStorage distributed database system.

### Quick overview

The library provides:
 - a self-contained `Channel<T>` class used for efficient exchange of typed records over TCP with the database,
 - a dead-simple `PayloadType<T>` serializer interface allowing users to marshall their own complex datatypes,
 - an implementation of the two standard TStorage operations `GET` and `PUT`, together with two additional maintenance commands `GETACQ` and `PUTA`,

all under an adjustable memory usage limit per `Channel<T>` instance.

To use the channel, follow these two steps.

1. Implement a serializer for the datatype of your choice.

    ```c++
    // FloatPayload.h
    #include <tstorageclient++/PayloadType.h>
    #include <cstring>

    using namespace tstorage;

    class FloatPayload : public PayloadType<float>
    {
	    std::size_t toBytes(const float& val, void* outputBuffer, std::size_t bufferSize)
	    {
		    if (bufferSize >= sizeof(float)) {
			    std::memcpy(outputBuffer, &val, sizeof(float));
		    }
		    return sizeof(float);
		    // do provide the expected size of the record data
		    // even when serialization fails
	    }

	    bool fromBytes(float& oVar, const void* payloadBuffer, std::size_t payloadSize)
	    {
		    if (payloadSize != sizeof(float)) {
			    // data is suspicious, we signal that we do not perform deserialization
			    return false;
		    }
		    std::memcpy(&oVar, payloadBuffer, sizeof(float));
		    return true; // all clear!
	    }
    };
    ```

2. Construct and use the `Channel<T>` to interact with the database by passing around `RecordsSet<T>` instances, where `T` is the expected record data type.

    ```c++
    // Main.cpp
    #include <cstdio>

    #include <tstorageclient++/Channel.h>
    #include "FloatPayload.h"

    RecordsSet<float> gatherOutboundRecords(...);

    using namespace tstorage;

    RecordsSet<float> gatherOutboundRecords(...) {
        RecordsSet<float> outboundRecs{};
        outboundRecs.append(Key(1,2,3,Timestamp::now()), -1e3F);
        ...
        return outboundRecs;
    }

    int main()
    {
	    Channel<float> channel(
		    "some.host.tstorage", // address
		    2025, // port
		    std::make_unique<FloatPayload>() // serializer
	    );

	    float sum = 0.0F;

	    if (!channel.connect()) {
		    printf("Unable to connect!");
		    return 1;
	    }

        RecordsSet<float> outboundRecs = gatherOutboundRecords(...);
	    Response resPut = channel.put(outboundRecs);
	    ResponseAcq resGet = channel.getStream(cKeyMin, cKeyMax, // filter the whole database...
	    [&sum](const RecordsSet<float>& records) { // ...through this function...
		    for (const Record<float>& record : records) {
                // ...in batches of a certian (controllable) size
			    sum += record.value;
		    }
	    });
	    channel.close();

	    if (resPut.error() || resGet.error()) {
		    printf("Error encountered!");
		    return 1;
	    }
	    printf("Sum = %f\n", sum);
	    return 0;
    }
    ```

### Building

Run

```sh
	git clone <REPO>
	cd libtstorageclient++
	make
```

to build a shared library. Then, install it using

```sh
	make install
```

This will copy the API headers and the library itself to the `include/tstorageclient++` and `lib/` directories under `$PREFIX`. It's `/usr/local/` by default if the `PREFIX` environment variable is not set. In this case, you'll probably have to run the command with `sudo`.

### Using the library

To include the API headers in your project, use

```c++
#include <tstorageclient++/Channel.h>
using namespace tstorage; // optional
```

directives. All other headers will be included automatically. During compilation and linking, use the `-ltstorageclient++` flag to make the class and function definitions available to your program.

### License

This library is licensed under _Apache License, Version 2.0_. See `LICENSE` for terms and conditions.
