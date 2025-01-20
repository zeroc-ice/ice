# Copyright (c) ZeroC, Inc.

from Glacier2Util import Glacier2Router, Glacier2TestSuite
from Util import Client, ClientServerTestCase, Server


passwords = {
    "userid": "abc123",
    "userid-0": "abc123",
    "userid-1": "abc123",
    "userid-2": "abc123",
    "userid-3": "abc123",
    "userid-4": "abc123",
}

routerProps = {
    "Ice.Warn.Dispatch": "0",
    "Ice.Warn.Connections": "0",
    "Glacier2.Filter.Category.Accept": "c1 c2",
    "Glacier2.Filter.Category.AcceptUser": "2",
    "Glacier2.Client.Connection.IdleTimeout": "30",
}

traceProps = {
    "Ice.Trace.Protocol": 1,
    "Ice.Trace.Network": 2,
    "Ice.Trace.Retry": 1,
}

Glacier2TestSuite(
    __name__,
    routerProps,
    [
        ClientServerTestCase(
            name="client/server with router",
            servers=[
                Glacier2Router(passwords=passwords),
                Server(),
            ],
            clients=[Client(), Client(args=["--shutdown"])],
            traceProps=traceProps,
        ),
    ],
)
