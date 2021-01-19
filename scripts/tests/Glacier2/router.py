# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

passwords = {
    "userid": "abc123",
    "userid-0": "abc123",
    "userid-1": "abc123",
    "userid-2": "abc123",
    "userid-3": "abc123",
    "userid-4": "abc123"
}

routerProps = {
   "Ice.Warn.Dispatch" : "0",
   "Ice.Warn.Connections" : "0",
   "Glacier2.Filter.Category.Accept" : "c1 c2",
   "Glacier2.Filter.Category.AcceptUser" : "2",
   "Glacier2.SessionTimeout" : "30",
}

traceProps = {
    "Ice.Trace.Protocol": 1,
    "Ice.Trace.Network": 2,
    "Ice.Trace.Retry": 1,
}

def buffered(enabled):
    return { "Glacier2.Client.Buffered": enabled, "Glacier2.Server.Buffered": enabled }

Glacier2TestSuite(__name__, routerProps, [
                  ClientServerTestCase(name="client/server with router in unbuffered mode",
                                       servers=[Glacier2Router(passwords=passwords, props=buffered(False)), Server()],
                                       client=Client(args=["--shutdown"]),
                                       traceProps=traceProps),
                  ClientServerTestCase(name="client/server with router in buffered mode",
                                       servers=[Glacier2Router(passwords=passwords, props=buffered(True)), Server()],
                                       clients=[Client(), Client(args=["--shutdown"])],
                                       traceProps=traceProps)])
