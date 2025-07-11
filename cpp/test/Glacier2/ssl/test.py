# Copyright (c) ZeroC, Inc.

from Glacier2Util import Glacier2Router, Glacier2TestSuite
from Util import Client, ClientServerTestCase, Server


def routerProps(process, current):
    return {
        "Ice.Warn.Dispatch": "0",
        "Glacier2.AddConnectionContext": "1",
        "Glacier2.Client.Endpoints": "{0}:{1}".format(
            current.getTestEndpoint(0, "tcp"), current.getTestEndpoint(1, "ssl")
        ),
        "Ice.Admin.Endpoints": current.getTestEndpoint(2, "tcp"),
        "Glacier2.SessionManager": "sessionmanager:{0}".format(current.getTestEndpoint(3, "tcp")),
        "Glacier2.PermissionsVerifier": "verifier:{0}".format(current.getTestEndpoint(3, "tcp")),
        "Glacier2.SSLSessionManager": "sslsessionmanager:{0}".format(current.getTestEndpoint(3, "tcp")),
        "Glacier2.SSLPermissionsVerifier": "sslverifier:{0}".format(current.getTestEndpoint(3, "tcp")),
    }


#
# Always enable SSL for the Glacier2 router and client
#


def sslProps(process, current):
    return current.testcase.getMapping().getSSLProps(process, current)


Glacier2TestSuite(
    __name__,
    routerProps=routerProps,
    options={"ipv6": [False]},
    multihost=False,
    testcases=[
        ClientServerTestCase(
            servers=[Glacier2Router(props=sslProps), Server()],
            client=Client(props=sslProps),
        )
    ],
)
