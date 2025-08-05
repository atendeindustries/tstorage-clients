#!/bin/env python3

import argparse
from pathlib import Path
import sys
from typing import List, Optional

from testrig import server
from testrig.tests import TestBatch
from testrig.utils import warn

from testrig.testsuite.socket import tests as tests_socket
from testrig.testsuite.buffer import tests as tests_buffer
from testrig.testsuite.serializer import tests as tests_serializer
from testrig.testsuite.channel import tests as tests_channel

server.BINDIR = Path(__file__).parent.resolve() / "bin"


def runSingleBatch(batch: TestBatch) -> bool:
    result: bool = batch.run()
    batch.summary()
    return result


def runAllTests(testSuites: List[TestBatch]) -> bool:
    result: bool = True
    for batch in testSuites:
        result = batch.run() and result
    warn("Summary:")
    for batch in testSuites:
        batch.summary()
    return result


def runTests(suiteName: Optional[str] = None, host: Optional[str] = None) -> bool:
    result: bool = True
    testSuites: List[TestBatch] = [
        TestBatch("Socket", tests_socket),
        TestBatch("Buffer", tests_buffer),
        TestBatch("Serializer", tests_serializer),
        TestBatch("Channel", tests_channel(host)),
    ]
    if suiteName is not None:
        for batch in testSuites:
            if batch.name().lower() == suiteName.lower():
                return runSingleBatch(batch)
        warn(f"Test suite {suiteName} not found")
        return False
    else:
        return runAllTests(testSuites)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="run-tests.py", description="Runs the tests of tstorageclient++"
    )
    parser.add_argument(
        "-H",
        "--host",
        type=str,
        action="store",
        nargs=1,
        metavar="<HOST>",
        help="Performs client tests using an external tstorage instance",
    )
    parser.add_argument(
        "test",
        type=str,
        action="store",
        nargs="?",
        metavar="<NAME>",
        help="Performs only the test suite named <NAME>",
    )
    opts = parser.parse_args(sys.argv[1:])
    host: Optional[str] = opts.host[0] if opts.host is not None else None
    name: Optional[str] = opts.test
    exit(0 if runTests(name, host) else 1)
