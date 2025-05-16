# Copyright (c) ZeroC, Inc.


from Util import Client, ClientServerTestCase, Server, TestSuite

traceProps = {"Ice.Trace.Network": 2, "Ice.Trace.Retry": 1, "Ice.Trace.Protocol": 1}

class UdpTestCase(ClientServerTestCase):
    def setupServerSide(self, current):
        if current.config.android:
            self.servers = [Server(ready="McastTestAdapter", traceProps=traceProps)]
        else:
            self.servers = [
                Server(args=[i], props = {"Ice.ProgramName" : f"server-{i}"}, ready="McastTestAdapter", traceProps=traceProps) for i in range(0, 5)
            ]

    def setupClientSide(self, current):
        if current.config.android:
            self.clients = [Client(traceProps=traceProps)]
        else:
            self.clients = [Client(args=[5], traceProps=traceProps)]


TestSuite(__name__, [UdpTestCase()], multihost=False)
