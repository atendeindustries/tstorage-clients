#!/bin/env python3

from typing import Callable, Dict, Optional

from ..tests import standaloneTest
from ..utils import info, warn, err, success, testname, testdesc


tests: Dict[str, Callable[[], bool]] = {
    "serializer put test": standaloneTest("test_serializer_put"),
    "serializer get test": standaloneTest("test_serializer_get"),
    "serializer buffers test": standaloneTest("test_serializer_buffer"),
}


if __name__ == "__main__":
    from ..tests import TestBatch

    exit(0 if TestBatch("Serializer", tests).run() else 1)
