#!/bin/env python3

import socket
import struct
from time import sleep
from typing import Callable, Dict

from ..tests import standardTest
from ..utils import info, warn, err, success, testname, testdesc


@standardTest("test_socket_close")
def socketTest_connectClose(conn: socket.socket, phase: int) -> bool:
    if phase == 0:
        testdesc("The client should establish connection and close it immediately.")
    info("Detected connection attempt.")
    return True


@standardTest("test_socket_send")
def socketTest_send(conn: socket.socket) -> bool:
    testdesc("Receiving message 'Hello world!' sent from the client...")
    msg = conn.recv(socket.MSG_WAITALL).decode("utf8")
    info(f"'{msg}'")
    if msg != "Hello world!":
        err("[ERROR] Server received unexpected message.")
        return False
    return True


@standardTest("test_socket_recv")
def socketTest_recv(conn: socket.socket) -> bool:
    msg = "Hello from python!"
    testdesc(
        f"Sending '{msg}' in two parts, split at 'p_ython'. "
        f"The client should receive it in three calls to socket.recv(), "
        f"the last one should emit warning with code 523."
    )
    sent = 0
    firstBatch = len(msg) - 6
    while sent < firstBatch:
        sent += conn.send(msg[sent:firstBatch].encode("utf8"))
    info("Sent first part. Sleeping...")
    sleep(0.5)
    while sent < len(msg):
        sent += conn.send(msg[sent : len(msg)].encode("utf8"))
    info("Sent all. Awaiting client response...")
    return True


@standardTest("test_socket_recv_exactly")
def socketTest_recvExactly(conn: socket.socket) -> bool:
    msg = "Hello world! I miss summer though."
    testdesc(
        f"Sending '{msg}' in two parts, split at 'worl_d'. "
        f"Client should receive the message exactly up to the exclamation mark "
        f"and then it should sever the connection, dropping the rest of the message."
    )
    sent = 0
    firstBatch = 10
    while sent < firstBatch:
        sent += conn.send(msg[sent:firstBatch].encode("utf8"))
    info("Sent first part. Sleeping...")
    sleep(0.5)
    while sent < len(msg):
        sent += conn.send(msg[sent : len(msg)].encode("utf8"))
    info("Sent all. Awaiting client response...")
    return True


@standardTest("test_socket_recv_atleast")
def socketTest_recvAtLeast(conn: socket.socket) -> bool:
    msg = (
        "Hello world, server here. Just checking up on you. Let's hope my "
        "message reaches you, or at least a meaningful part of it...\0"
    )
    info(len(msg))
    testdesc(
        f"Sending '{msg}' in several parts, split at every 5th letter. "
        f"Client should receive at least the '{msg[:15]}' fragment, and the length of the "
        f"received message should be a multiple of 5."
    )
    sent = 0
    connAlive = True
    info("Sending message in 5B-chunks")
    while connAlive and sent < len(msg):
        try:
            sent += conn.send(msg[sent : sent + 5].encode("utf8"))
        except BrokenPipeError as err:
            warn("Connection closed")
            connAlive = False
        sleep(0)
    info("Sent all. Awaiting client response...")
    return True


@standardTest("test_socket_skip_exactly")
def socketTest_skipExactly(conn: socket.socket) -> bool:
    msg = "Hello, pitiful world!"
    testdesc(
        f"Sending '{msg}' in two parts, split at 'pit_iful'. "
        f"Client should skip the ', pitiful' part of the message, "
        f"leaving us with 'Hello world!' as a result."
    )
    sent = 0
    firstBatch = 10
    while sent < firstBatch:
        sent += conn.send(msg[sent:firstBatch].encode("utf8"))
    info("Sent first part. Sleeping...")
    sleep(0.5)
    while sent < len(msg):
        sent += conn.send(msg[sent : len(msg)].encode("utf8"))
    info("Sent all. Awaiting client response...")
    return True


@standardTest("test_socket_shutdown_send")
def socketTest_shutdownSend(conn: socket.socket) -> bool:
    expectedMsg = "Hello?"
    recvdBytes = 0
    while recvdBytes < len(expectedMsg):
        recvdBytes += len(conn.recv(len(expectedMsg) - recvdBytes))

    sleep(0.2)
    info("Attempting to read bytes after shutdown")
    done = len(conn.recv(1)) == 0
    if not done:
        err("[ERROR] There are still unread bytes in the buffer")
        return False
    success("Connection on the recv end closed")

    info("Sending confirmation...")
    sent = 0
    msg = "OK".encode("utf8") + b"\0"
    while sent < len(msg):
        sent += conn.send(msg[sent:])
    info("Closing connection on the server end...")
    conn.close()
    return done


@standardTest("test_socket_shutdown_recv")
def socketTest_shutdownRecv(conn: socket.socket) -> bool:
    info("Sending msg...")
    sent = 0
    msg = "Hello?".encode("utf8") + b"\0"
    while sent < len(msg):
        sent += conn.send(msg[sent:])

    info("Receiving confirmation...")
    confirm = conn.recv(1024, socket.MSG_WAITALL).decode("utf8").split("\x00")[0]
    if confirm != "OK":
        err(f'[ERROR] Bad confirmation ("{confirm}", expected "OK")')
        return False

    info("Trying to send message after read shutdown...")
    sent = 0
    while sent < len(msg):
        sent += conn.send(msg[sent:])

    info("Closing connection on the server end...")
    conn.close()
    return True


@standardTest("test_socket_dialog")
def socketTest_dialog(conn: socket.socket) -> bool:
    info("Receiving msg1...")
    (msg1Len,) = struct.unpack("=L", conn.recv(4, socket.MSG_WAITALL))
    msg1 = conn.recv(msg1Len, socket.MSG_WAITALL).decode("utf8").split("\x00")[0]
    info(f"'{msg1}'")
    exp1 = "Hello. Are you world?"
    if msg1 != exp1:
        err(f"[ERROR] msg1 has wrong content (expected: '{exp1}')")

    info("Sending reply1...")
    sent = 0
    reply1 = "No, you must have been mistaken. I'm just a dumb server.\0"
    while sent < 4 + len(reply1):
        sent += conn.send(struct.pack("=L", len(reply1)) + reply1.encode("utf8"))

    info("Receiving msg2...")
    (msg2Len,) = struct.unpack("=L", conn.recv(4, socket.MSG_WAITALL))
    msg2 = conn.recv(msg2Len, socket.MSG_WAITALL).decode("utf8").split("\x00")[0]
    info(f"'{msg2}'")
    exp2 = "Oh, sorry then. Goodbye!"
    if msg2 != exp2:
        err(f"[ERROR] msg2 has wrong content (expected: '{exp2}')")

    info("Sending reply2...")
    sent = 0
    reply2 = "Hey, wait up!\0"
    while sent < 4 + len(reply2):
        sent += conn.send(struct.pack("=L", len(reply2)) + reply2.encode("utf8"))

    info("Closing connection on the server end...")
    conn.close()
    return True


@standardTest("test_socket_send_timeout")
def socketTest_send_timeout(conn: socket.socket) -> bool:
    conn.settimeout(20.0)
    info("Withholding contact...")
    sleep(0.7)
    conn.close()
    return True


@standardTest("test_socket_recv_timeout")
def socketTest_recv_timeout(conn: socket.socket) -> bool:
    conn.settimeout(20.0)
    info("Withholding contact...")
    sleep(0.5)
    conn.close()
    return True


tests: Dict[str, Callable[[], bool]] = {
    "connect and close test": socketTest_connectClose,
    "send test": socketTest_send,
    "recv test": socketTest_recv,
    "recvExactly test": socketTest_recvExactly,
    "recvAtLeast test": socketTest_recvAtLeast,
    "skipExactly test": socketTest_skipExactly,
    "send after shutdown test": socketTest_shutdownSend,
    "recv after shutdown test": socketTest_shutdownRecv,
    "dialog test": socketTest_dialog,
    "send timeout test": socketTest_send_timeout,
    "recv timeout test": socketTest_recv_timeout,
}

if __name__ == "__main__":
    from ..tests import TestBatch

    exit(0 if TestBatch("Socket", tests).run() else 1)
