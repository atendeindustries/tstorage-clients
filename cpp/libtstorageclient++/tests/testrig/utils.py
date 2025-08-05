#!/bin/env python3

###
# testrig: utils.py
#
# Copyright 2025 Atende Industries
#

TEST_DESCRIPTIONS = True

ANSI_RESET = "\033[0m"
ANSI_BOLD = "\033[1m"
ANSI_RED = "\033[91m"
ANSI_GREEN = "\033[92m"
ANSI_CYAN = "\033[94m"
ANSI_GRAY = "\033[90m"
ANSI_YELLOW = "\033[93m"


def info(what, **kwargs):
    print(what, **kwargs)


def warn(what, **kwargs):
    print(ANSI_YELLOW + what + ANSI_RESET, **kwargs)


def err(what, **kwargs):
    print(ANSI_RED + ANSI_BOLD + what + ANSI_RESET, **kwargs)


def success(what, **kwargs):
    print(ANSI_GREEN + ANSI_BOLD + what + ANSI_RESET, **kwargs)


def testname(what, **kwargs):
    print(ANSI_CYAN + ANSI_BOLD + what + ANSI_RESET, **kwargs)


def testdesc(what, **kwargs):
    if TEST_DESCRIPTIONS:
        print(ANSI_GRAY + what + ANSI_RESET, **kwargs)
