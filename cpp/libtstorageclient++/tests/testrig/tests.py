#!/bin/env python3

###
# testrig: tests.py
#
# Copyright 2025 Atende Industries
#

import socket
import threading
from typing import Any, Callable, Dict, Tuple, Optional

from .server import Process, PyStorageProcess, Server
from .utils import info, warn, err, success, testname


class TestBatch:
    def __init__(self, name: str, tests: Dict[str, Callable[[], bool]]) -> None:
        self._suiteName: str = name
        self._tests: Dict[str, Callable[[], bool]] = tests
        self._total: int = len(self._tests)
        self._passed: int = 0

    def name(self) -> str:
        return self._suiteName

    def run(self) -> bool:
        ctr: int = 1
        if self._total == 0:
            return True
        testname(f"[{self._suiteName}]")
        for name, test in self._tests.items():
            warn(f"[{ctr}/{self._total}] ", end="")
            success(f"{self._suiteName}: {name}")
            done: bool = test()
            if done:
                success("[ ✓ ] PASSED")
                self._passed += 1
            else:
                err("[ ✗ ] FAILED")
            ctr += 1
            if ctr <= self._total:
                print()
        print()
        return self._passed == self._total

    def summary(self) -> bool:
        if self._passed != self._total:
            testname(f"[{self._suiteName}] ", end="")
            err("failed! ", end="")
            warn(f"{self._passed}/{self._total}", end="")
            err(" passed")
            return False
        else:
            testname(f"[{self._suiteName}] ", end="")
            success("done! ", end="")
            warn(f"{self._passed}/{self._total}", end="")
            success(" passed")
            return True


def standaloneTest(testName: str):
    def test() -> bool:
        pr = Process("test")
        pr.run(testName)
        rv = pr.wait()
        return rv == 0

    return test


def standardTest(testName: str):
    def standardTestDecor(callback: Callable[..., bool]) -> Callable[[], bool]:
        def test() -> bool:
            sv = Server(callback)
            sv.run()
            pr = Process("test")
            pr.run(testName)
            rv = pr.wait()
            if rv != 0:
                sv.done()
                sv.wait()
                return False
            sv.done()
            return sv.wait()

        return test

    return standardTestDecor


def functionalTest(
    testName: str, verbose: bool = True, host: Optional[str] = None
) -> Callable[[], bool]:
    if host is not None:
        return tstorageLiveTest(testName, host)
    else:
        return pystorageTest(testName, verbose)


def tstorageLiveTest(testName: str, host: str) -> Callable[[], bool]:
    addr, port = host.split(":")

    def test() -> bool:
        pr = Process("test")
        pr.run(testName, addr, port)
        rv = pr.wait()
        return rv == 0

    return test


def pystorageTest(testName: str, verbose: bool) -> Callable[[], bool]:

    def test() -> bool:
        flags = "-v" if verbose else ""
        sv = PyStorageProcess(indent=0)
        if len(flags) == 0:
            sv.run()
        else:
            sv.run(flags)
        sv.waitForLine("Listen")
        pr = Process("test")

        def subproc():
            pr.run(testName)
            rvsub = pr.wait()
            sv.term()

        thread = threading.Thread(target=subproc)
        thread.start()
        rv = sv.wait()
        thread.join()
        rvsub = pr.wait()
        return rv == 0 and rvsub == 0

    return test
