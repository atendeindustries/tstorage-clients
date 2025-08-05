#!/bin/env python3

###
# testrig: tests.py
#
# Copyright 2025 Atende Industries
#

import signal
import socket
import subprocess
import threading

from pathlib import Path
from time import sleep
from typing import Any, Callable, Tuple, Optional, Union

from .utils import info, warn, err, testname

ROOTDIR = Path(__file__).resolve().parent.parent
BINDIR = ROOTDIR / "bin"


class Process:
    def __init__(self, name: str, indent: int = 2) -> None:
        self._indent: int = indent
        self._name: str = name
        self._proc: Optional[subprocess.Popen] = None
        self._rv: int = 0

    def run(self, *args) -> None:
        self._args = " ".join(args)
        self._binname = BINDIR / self._name
        self._proc = subprocess.Popen(
            [self._binname, *args],
            bufsize=0,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            encoding="utf8",
        )
        self.printBinName()
        info(f" executed")

    def printLine(self, line: str) -> None:
        if line.strip().startswith("[ERR"):
            err(" " * self._indent + line)
        elif line.strip().startswith("[WARN"):
            warn(" " * self._indent + line)
        else:
            info(" " * self._indent + line)

    def printBinName(self) -> None:
        name = self._binname.relative_to(ROOTDIR)
        info(f"./{str(name)}", end="")
        if self._args is not None and len(self._args) > 0:
            testname(f" {self._args}", end="")

    def waitForLine(self, text: str) -> None:
        if self._proc is None or self._proc.stdout is None:
            return
        for line in self._proc.stdout:
            if line.strip().startswith(text):
                self.printLine(line.rstrip())
                break
            self.printLine(line.rstrip())

    def wait(self) -> int:
        if self._proc is None or self._proc.stdout is None:
            return self._rv
        for line in self._proc.stdout:
            line = line.rstrip()
            self.printLine(line)
        self._rv = self._proc.wait()
        self._proc = None
        self.printBinName()
        info(f" exited with error code {self._rv}")
        return self._rv

    def term(self) -> None:
        if self._proc is not None:
            self._proc.send_signal(signal.SIGTERM)


class PyStorageProcess(Process):
    def __init__(self, *args, **kwargs):
        super().__init__(name="pystorage", *args, **kwargs)

    def printBinName(self):
        name = self._binname.relative_to(ROOTDIR)
        warn(f"./{str(name)}", end="")
        if self._args is not None and len(self._args) > 0:
            testname(f" {self._args}", end="")


class Server:
    def __init__(self, func: Callable[..., bool]):
        self._server: socket.socket = socket.socket(
            socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP
        )
        self._server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._func: Callable[..., bool] = func
        self._thread: Optional[threading.Thread] = None
        self._rv: bool = True
        self._done: bool = False

    def run(self, acceptDelay: Optional[float] = None) -> None:
        self._server.bind(("127.0.0.1", 2090))
        self._server.listen()
        self._server.settimeout(0.2)
        self._thread = self._serverThread()
        self._thread.start()

    def done(self) -> None:
        self._done = True

    def wait(self) -> bool:
        if self._thread is not None:
            self._thread.join()
        return self._rv

    def _serverThread(self) -> threading.Thread:
        def thread() -> None:
            phase: int = 0
            while not self._done:
                try:
                    conn, _ = self._server.accept()
                    singleArg = False
                    try:
                        self._rv = self._func(conn, phase)
                    except TypeError:
                        singleArg = True
                    if singleArg:
                        self._rv = self._func(conn)
                    conn.close()
                    phase += 1
                except socket.timeout:
                    continue

        return threading.Thread(target=thread)
