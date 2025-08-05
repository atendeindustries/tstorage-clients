#!/bin/env python3

import socket
import struct
from typing import Callable, Dict, Optional

from ..tests import functionalTest
from ..utils import info, warn, err, success, testname, testdesc


def tests(host: Optional[str] = None) -> Dict[str, Callable[[], bool]]:
    return {
        "connect test": functionalTest("test_channel_connect", host=host),
        "getAcq test": functionalTest("test_channel_getacq", host=host),
        "get empty test": functionalTest("test_channel_get_empty", host=host),
        "put empty test": functionalTest("test_channel_put_empty", host=host),
        "put test": functionalTest("test_channel_put", host=host),
        "putA test": functionalTest("test_channel_puta", host=host),
        "variable payload size test": functionalTest(
            "test_channel_put_variable_payload_size", host=host
        ),
        "get test": functionalTest("test_channel_get", host=host),
        "many records test": functionalTest("test_channel_many_records", host=host),
        "large payloads test": functionalTest("test_channel_large_payloads", host=host),
        "mixed payloads test": functionalTest("test_channel_mixed_payloads", host=host),
        "get with memory limit test": functionalTest(
            "test_channel_get_with_memory_limit", host=host
        ),
        "get stream test": functionalTest("test_channel_get_stream", host=host),
        "invalid cid put test": functionalTest(
            "test_channel_put_bad_cid", verbose=False, host=host
        ),
        "maximal key put test": functionalTest(
            "test_channel_put_key_out_of_range", verbose=False, host=host
        ),
    }


if __name__ == "__main__":
    from ..tests import TestBatch

    exit(0 if TestBatch("Channel", tests()).run() else 1)
