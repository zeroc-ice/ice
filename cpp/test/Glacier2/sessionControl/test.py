# Copyright (c) ZeroC, Inc.

#
# Note: we limit the send buffer size with Ice.TCP.SndSize, the
# test relies on send() blocking
#
from __future__ import annotations

from Glacier2Util import Glacier2Router, Glacier2TestSuite
from Util import ClientServerTestCase, Driver, Process, Props, Server


def routerProps(process: Process, current: Driver.Current) -> Props:
    return {
        "Glacier2.SessionManager": "SessionManager:{0}".format(current.getTestEndpoint(0)),
        "Glacier2.PermissionsVerifier": "Glacier2/NullPermissionsVerifier",
    }


Glacier2TestSuite(
    __name__,
    testcases=[ClientServerTestCase(servers=[Glacier2Router(props=routerProps), Server()])],
)
