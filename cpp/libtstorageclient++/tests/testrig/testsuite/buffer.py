#!/bin/env python3

from typing import Callable, Dict, Optional

from ..tests import standaloneTest
from ..utils import info, warn, err, success, testname, testdesc


tests: Dict[str, Callable[[], bool]] = {
    "buffer create test": standaloneTest("test_buffer_create"),
    "buffer heads test": standaloneTest("test_buffer_heads"),
    "buffer reserve test": standaloneTest("test_buffer_reserve"),
}


if __name__ == "__main__":
    from ..tests import TestBatch

    exit(0 if TestBatch("Buffer", tests).run() else 1)
