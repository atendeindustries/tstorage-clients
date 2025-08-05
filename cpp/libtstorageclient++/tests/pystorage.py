#!/bin/env python3

###
# pystorage.py
#
# Copyright 2025 Atende Industries
#

import argparse
from dataclasses import dataclass
from enum import Enum
import signal
import socket
import struct
import sys
import time
import traceback
from typing import ByteString, Dict, Iterable, IO, List, Tuple, Optional, Union

MINACQ = -9223372036854775808

MAXCID = 2147483647
MAXMID = 9223372036854775807
MAXMOID = 2147483647
MAXCAP = 9223372036854775807
MAXACQ = 9223372036854775807

MAXPAYLOADSIZE = 32 * 2**20
ACQFOLLOWTHRESHOLD = 10000000

TDIFF = 978307200000000000


def printu(*args, **kwargs):
    print(*args, **kwargs)
    sys.stdout.flush()


@dataclass(frozen=True)
class Key:
    cid: int
    mid: int
    moid: int
    cap: int
    acq: int = MINACQ

    def __eq__(self, other) -> bool:
        return (
            self.cid == other.cid
            and self.mid == other.mid
            and self.moid == other.moid
            and self.cap == other.cap
            and self.acq == other.acq
        )

    def __le__(self, other) -> bool:
        return (
            self.cid <= other.cid
            and self.mid <= other.mid
            and self.moid <= other.moid
            and self.cap <= other.cap
            and self.acq <= other.acq
        )

    def __lt__(self, other) -> bool:
        return (
            self.cid < other.cid
            and self.mid < other.mid
            and self.moid < other.moid
            and self.cap < other.cap
            and self.acq < other.acq
        )


cMaxKey = Key(MAXCID, MAXMID, MAXMOID, MAXCAP, MAXACQ)


class MsgType(Enum):
    NONE = -1
    GET = 1
    PUT = 5
    PUTA = 6
    GETACQ = 7


class ProtocolError(Exception):
    def __init__(self, *args):
        super().__init__(*args)


class ActiveConnection:
    def __init__(self, conn: socket.socket) -> None:
        self._conn: socket.socket = conn
        self._closed: bool = False

    def recvExactly(self, amt: int) -> ByteString:
        msgParts: List[ByteString] = []
        recvd: int = 0
        lastrecvd: int = -1
        while recvd < amt and lastrecvd != 0:
            newmsg: ByteString = self._conn.recv(amt - recvd)
            lastrecvd = len(newmsg)
            recvd += lastrecvd
            msgParts.append(newmsg)
        return bytes().join(msgParts)

    def send(self, data: ByteString) -> None:
        amtSent: int = 0
        while amtSent < len(data):
            amtSent += self._conn.send(data[amtSent:])

    def close(self) -> None:
        if not self._closed:
            self._conn.close()
            self._closed = True

    @property
    def closed(self):
        return self._closed

    def fetchHeader(self) -> MsgType:
        msg = self.recvExactly(12)
        if len(msg) == 0:
            return MsgType.NONE
        msgType, hSize = struct.unpack("<lQ", msg)
        msgType = MsgType(msgType)
        if msgType == MsgType.GET or msgType == MsgType.GETACQ:
            hSize -= 64
        if hSize < 0:
            raise ProtocolError("Invalid header size")
        self.recvExactly(hSize)
        return MsgType(msgType)

    def fetchKeyPair(self) -> Tuple[Key, Key]:
        keyMin: Key = Key(*struct.unpack("<lqlqq", self.recvExactly(32)))
        keyMax: Key = Key(*struct.unpack("<lqlqq", self.recvExactly(32)))
        return keyMin, keyMax

    def fetchRecords(self) -> Iterable[Tuple[Key, ByteString]]:
        cid: int = struct.unpack("<l", self.recvExactly(4))[0]
        while cid >= 0:
            batchSize: int = struct.unpack("<l", self.recvExactly(4))[0]
            while batchSize > 0:
                recordSize: int = struct.unpack("<l", self.recvExactly(4))[0]
                if recordSize < 28 or recordSize > MAXPAYLOADSIZE + 28:
                    raise ProtocolError(
                        f"Invalid record size encountered ({recordSize})"
                    )
                key: Key = Key(cid, *struct.unpack("<qlqq", self.recvExactly(28)))
                payload: ByteString = self.recvExactly(recordSize - 28)
                yield key, payload
                batchSize -= recordSize + 4
            cid = struct.unpack("<l", self.recvExactly(4))[0]

    def fetchRecordsWoAcq(self) -> Iterable[Tuple[Key, ByteString]]:
        cid: int = struct.unpack("<l", self.recvExactly(4))[0]
        while cid >= 0:
            batchSize: int = struct.unpack("<l", self.recvExactly(4))[0]
            while batchSize > 0:
                recordSize: int = struct.unpack("<l", self.recvExactly(4))[0]
                if recordSize < 20 or recordSize > MAXPAYLOADSIZE + 20:
                    raise ProtocolError(
                        f"Invalid record size encountered ({recordSize})"
                    )
                key: Key = Key(cid, *struct.unpack("<qlq", self.recvExactly(20)))
                payload: ByteString = self.recvExactly(recordSize - 20)
                yield key, payload
                batchSize -= recordSize + 4
            cid = struct.unpack("<l", self.recvExactly(4))[0]

    def sendResponse(self, status: int) -> None:
        self.send(struct.pack("<lQ", status, 0))

    def sendError(self) -> None:
        self.send(struct.pack("<lQ", -1, 0))

    def sendAcq(self, status: int, acq: int) -> None:
        self.send(struct.pack("<lQq", status, 8, acq))

    def sendTwoAcqs(self, status: int, acqMin: int, acqMax) -> None:
        self.send(struct.pack("<lQqq", status, 16, acqMin, acqMax))

    def sendRecords(self, records: Iterable[Tuple[Key, ByteString]]) -> None:
        for key, payload in records:
            msg = (
                struct.pack(
                    "<llqlqq",
                    len(payload) + 32,
                    key.cid,
                    key.mid,
                    key.moid,
                    key.cap,
                    key.acq,
                )
                + payload
            )
            self.send(msg)

    def sendTerm(self):
        self.send(struct.pack("<l", 0))


class TCPLoggerConnection(ActiveConnection):
    def __init__(self, conn: socket.socket, file: IO, indent: int = 2) -> None:
        super().__init__(conn)
        self._file: IO = file
        self._indent: int = indent
        self._depth: int = 0

        self._file.write(
            "@ " + time.strftime("%Y-%m-%d %H:%M.%S", time.localtime()) + "\n"
        )
        self._enter("Connect")

    def _makeIndent(self) -> None:
        self._file.write(self._depth * self._indent * " ")

    def _log(self, *msgs: Union[str, bytes]) -> None:
        self._makeIndent()
        for msg in msgs:
            if isinstance(msg, str):
                self._file.write(msg)
            elif isinstance(msg, bytes):
                self._file.write(msg.hex(" "))
        self._file.write("\n")

    def _enter(self, sec: str) -> None:
        self._makeIndent()
        self._file.write(sec + ":\n")
        self._depth += 1

    def _exit(self) -> None:
        self._depth -= 1 if self._depth > 0 else 0

    class _Section:
        def __init__(self, conn: "TCPLoggerConnection", sec: str) -> None:
            self._conn: "TCPLoggerConnection" = conn
            self._sec: str = sec

        def __enter__(self):
            self._conn._enter(self._sec)

        def __exit__(self, type, value, traceback):
            self._conn._exit()

    def _section(self, sec: str):
        return self._Section(self, sec)

    def recvExactly(self, amt: int) -> ByteString:
        response = super().recvExactly(amt)
        lenResp = len(response)
        for i in range(0, lenResp, 8):
            head = "recv: " if i == 0 else " " * len("recv: ")
            self._log(head, response[i: min(i+8, lenResp)])
        return response

    def send(self, data: ByteString) -> None:
        self._log("send: ", data)
        super().send(data)

    def close(self) -> None:
        super().close()
        self._exit()
        self._log("Close.\n")

    def fetchHeader(self) -> MsgType:
        with self._section("recv header"):
            result = super().fetchHeader()
            self._log(f"MsgType: {result}")
        return result

    def fetchKeyPair(self) -> Tuple[Key, Key]:
        with self._section("recv key pair"):
            result = super().fetchKeyPair()
            self._log(f"keyMin: {result[0]}, keyMax: {result[1]}")
        return result

    def fetchRecords(self) -> Iterable[Tuple[Key, ByteString]]:
        with self._section("recv records"):
            for key, payload in super().fetchRecords():
                self._log(f"key: {key}")
                yield key, payload

    def fetchRecordsWoAcq(self) -> Iterable[Tuple[Key, ByteString]]:
        with self._section("recv records w/o ACQ"):
            for key, payload in super().fetchRecordsWoAcq():
                self._log(f"key: {key}, payload: ", payload)
                yield key, payload

    def sendResponse(self, status: int) -> None:
        with self._section("send response"):
            self._log(f"status: {status}")
            super().sendResponse(status)

    def sendError(self) -> None:
        with self._section("send error"):
            super().sendError()

    def sendAcq(self, status: int, acq: int) -> None:
        with self._section("send acq"):
            self._log(f"status: {status}, acq: {acq}")
            super().sendAcq(status, acq)

    def sendTwoAcqs(self, status: int, acqMin: int, acqMax) -> None:
        with self._section("send two acqs"):
            self._log(f"status: {status}, acqMin: {acqMin}, acqMax: {acqMax}")
            super().sendTwoAcqs(status, acqMin, acqMax)

    def sendRecords(self, records: Iterable[Tuple[Key, ByteString]]) -> None:
        with self._section("send records"):
            super().sendRecords(records)

    def sendTerm(self):
        with self._section("send term"):
            super().sendTerm()


class ServerMock:
    def __init__(
        self, timeout=20.0, verbose: int = 0, log: Optional[str] = None
    ) -> None:
        self._db: Dict[Tuple[int, Key], ByteString] = dict()
        self._done: bool = False
        self._lastAcq: int = MINACQ
        self._listener: socket.socket = socket.socket()
        self._listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._log: Optional[str] = log
        self._timeout: float = timeout
        self._uid: int = 0
        self._verbose: int = verbose

    def listen(self) -> None:
        self._listener.bind(("127.0.0.1", 2090))
        self._listener.listen()
        printu("Listening...")
        self._done = False

        file: Optional[IO] = None
        if self._log is not None:
            file = open(self._log, "a")

        aconn: ActiveConnection
        while not self._done:
            try:
                conn, addr = self._listener.accept()
                if self._verbose > 0:
                    printu(f"Incoming connection from {addr}...")
                conn.settimeout(self._timeout)
            except OSError:
                done = True
                break
            if file is not None:
                aconn = TCPLoggerConnection(conn, file)
            else:
                aconn = ActiveConnection(conn)
            try:
                while not aconn.closed:
                    self.processRequests(aconn)
            except OSError as ose:
                printu("[ERROR] OSError: " + str(ose))
                aconn.close()

        if file is not None:
            file.close

    def done(self) -> None:
        self._done = True
        self._listener.shutdown(socket.SHUT_RDWR)
        self._listener.close()
        printu("Server closed")

    def processRequests(self, conn: ActiveConnection) -> None:
        msgType: MsgType = conn.fetchHeader()
        if self._verbose > 0:
            printu(
                f"Requested {msgType}"
                if msgType != MsgType.NONE
                else "Session terminated"
            )
        try:
            if msgType == MsgType.PUT:
                acqMin, acqMax = self.store(conn.fetchRecordsWoAcq(), hasAcq=False)
                if self._verbose > 0:
                    printu(
                        f"Store complete, ACQ range: [{acqMin, acqMax}]. Sending status response..."
                    )
                conn.sendTwoAcqs(0, acqMin, acqMax)
            elif msgType == MsgType.PUTA:
                self.store(conn.fetchRecords(), hasAcq=True)
                if self._verbose > 0:
                    printu(f"Store complete. Sending status response...")
                conn.sendTwoAcqs(0, -1, -1)
            elif msgType == MsgType.GET:
                keyMin, keyMax = conn.fetchKeyPair()
                conn.sendResponse(0)
                if self._verbose > 0:
                    printu(f"Acknowledged. Sending records")
                try:
                    conn.sendRecords(self.retrieve(keyMin, keyMax))
                    if self._verbose > 0:
                        printu(f"Records sent, signalling end-of-response...")
                    conn.sendTerm()
                except ConnectionResetError as ose:
                    printu("[ERROR] OSError: " + str(ose))
                    conn.close()
                except BrokenPipeError as ose:
                    printu("[ERROR] OSError: " + str(ose))
                    conn.close()
                except OSError as ose:
                    printu("[ERROR] OSError: " + str(ose))
                    if not conn.closed:
                        conn.sendTerm()
                        conn.sendError()
                        conn.close()
                if not conn.closed:
                    conn.sendAcq(0, self.getAcq(keyMin, keyMax))
                    if self._verbose > 0:
                        printu("ACQ sent")
            elif msgType == MsgType.GETACQ:
                keyMin, keyMax = conn.fetchKeyPair()
                conn.sendAcq(0, self.getAcq(keyMin, keyMax))
            else:
                conn.close()
        except ProtocolError as pe:
            printu("[ERROR] " + str(pe))
            if msgType == MsgType.GET:
                conn.sendTerm()
            conn.sendError()
            conn.close()

    def store(
        self, records: Iterable[Tuple[Key, ByteString]], hasAcq: bool
    ) -> Tuple[int, int]:
        acqResponse = self.now()
        for key, payload in records:
            if not hasAcq:
                key = Key(key.cid, key.mid, key.moid, key.cap, self.now())
            self.validateKeyForStore(key)
            if self._verbose > 1:
                printu(f"Storing {key} : 0x{payload.hex().upper()}")
            self._db[self._uid, key] = payload
            self._lastAcq = self.now()
            self._uid += 1
        return acqResponse, acqResponse

    def retrieve(self, keyMin: Key, keyMax: Key) -> Iterable[Tuple[Key, ByteString]]:
        self.validateKeyRange(keyMin, keyMax)
        if self._verbose > 0:
            printu(f"Got key range: {keyMin}, {keyMax}")
        for uidkey, payload in self._db.items():
            uid, key = uidkey
            if keyMin <= key and key < keyMax:
                if self._verbose > 1:
                    printu(f"Retrieving {key} : 0x{payload.hex().upper()}")
                yield (key, payload)
            elif self._verbose > 1:
                printu(f"Ignoring {key} : 0x{payload.hex().upper()}")

    def getAcq(self, keyMin: Key, keyMax: Key) -> int:
        self.validateKeyRange(keyMin, keyMax)
        if keyMax.acq > self._lastAcq + ACQFOLLOWTHRESHOLD:
            self._lastAcq = self.now()
        self._lastAcq = min(keyMax.acq, self._lastAcq)
        if self._verbose > 0:
            printu(f"Sending ACQ {self._lastAcq}")
        return self._lastAcq

    def validateKeyForStore(self, key: Key) -> None:
        self.validateKey(key)
        if not key < cMaxKey:
            raise ProtocolError(
                f"Invalid key encountered: one of its fields reaches a maximal value (in {key})"
            )

    def validateKey(self, key: Key) -> None:
        if key.cid < 0:
            raise ProtocolError(f"Invalid cid encountered (in {key})")

    def validateKeyRange(self, keyMin: Key, keyMax: Key) -> None:
        self.validateKey(keyMin)
        self.validateKey(keyMax)
        if keyMin.acq > self._lastAcq:
            raise ProtocolError(
                f'Key range "from the future" encountered: {keyMin}, {keyMax}'
            )
        if not keyMin < keyMax:
            raise ProtocolError("Empty key range encountered: {keyMin}, {keyMax}")

    @staticmethod
    def now() -> int:
        return time.time_ns() - TDIFF


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        prog="pystorage.py",
        description="A simple, non-persistent TStorage mock server for testing purposes,"
        " mimicking the behaviour of a suboptimally configured single-node, single-sack"
        " TStorage system. Intended for users without direct access to a TStorage instance"
        " who wish to experiment with the client code.",
    )
    parser.add_argument(
        "-v",
        "--verbose",
        action="count",
        help="output server's internal event info to standard output",
    )
    parser.add_argument(
        "-l",
        "--log",
        action="store",
        help="log TCP traffic to a file located in PATH",
        metavar="PATH",
        nargs=1,
        type=str,
    )
    opts = parser.parse_args(sys.argv[1:])
    verbose = 0 if opts.verbose is None else opts.verbose
    log = opts.log[0] if opts.log is not None else None

    storage = ServerMock(verbose=verbose, log=log)

    def sigHandler(signum, frame):
        storage.done()

    signal.signal(signal.SIGTERM, sigHandler)
    signal.signal(signal.SIGINT, sigHandler)

    storage.listen()
