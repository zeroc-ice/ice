# Copyright (c) ZeroC, Inc.

#
# Note: we limit the send buffer size with Ice.TCP.SndSize, the
# test relies on send() blocking
#
from Glacier2Util import Glacier2Router, Glacier2TestSuite
from Util import ClientServerTestCase, Server


def routerProps(process, current):
    return {
        "Glacier2.SessionManager": "SessionManager:{0}".format(
            current.getTestEndpoint(0)
        ),
        "Glacier2.PermissionsVerifier": "Glacier2/NullPermissionsVerifier",
        "Ice.Default.Locator": "locator:{0}".format(current.getTestEndpoint(1)),
    }


Glacier2TestSuite(
    __name__,
    testcases=[
        ClientServerTestCase(
            servers=[
                Glacier2Router(props=routerProps, passwords=None),
                Server(readyCount=3),
            ]
        )
    ],
)
