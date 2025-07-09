# Copyright (c) ZeroC, Inc.

from Util import Client, ClientTestCase, TestSuite

TestSuite(
    __name__,
    [ClientTestCase(client=Client(props={"Ice.Warn.Dispatch": 0}))],
    libDirs=["interceptortest"],
)
