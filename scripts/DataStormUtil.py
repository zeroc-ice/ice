#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from Util import (
    Client,
    ClientServerTestCase,
    Mapping,
    ProcessFromBinDir,
    Server,
)

class Writer(Client):
    processType = "writer"

    def __init__(self, instanceName=None, instance=None, *args, **kargs):
        Client.__init__(self, *args, **kargs)
    
    def getProps(self, current):
        props = Client.getProps(self, current)

        # Default properties
        props.update(
            {
                "DataStorm.Node.Server.Enabled": 0,
                "DataStorm.Node.ConnectTo": "tcp -p 12345"
            }
        )
        return props

class Reader(Server):
    processType = "reader"

    def __init__(self, instanceName=None, instance=None, *args, **kargs):
        Server.__init__(self, *args, **kargs)

    def getProps(self, current):
        props = Server.getProps(self, current)

        # Default properties
        props.update(
            {
                "DataStorm.Node.Server.Endpoints": "tcp -p 12345"
            }
        )
        return props

class Node(ProcessFromBinDir, Server):
    def __init__(self, desc=None, *args, **kargs):
        Server.__init__(self, "dsnode", mapping=Mapping.getByName("cpp"), desc=desc or "DataStorm node", *args, **kargs)

    def shutdown(self, current):
        if self in current.processes:
            current.processes[self].terminate()

    def getProps(self, current):
        props = Server.getProps(self, current)
        props['Ice.ProgramName'] = self.desc
        return props

class NodeTestCase(ClientServerTestCase):

    def __init__(self, nodes=None, nodeProps=None, *args, **kargs):
        ClientServerTestCase.__init__(self, *args, **kargs)
        if nodes:
            self.nodes = nodes
        elif nodeProps:
           self.nodes = [Node(props=nodeProps)]
        else:
            self.nodes = None

    def init(self, mapping, testsuite):
        ClientServerTestCase.init(self, mapping, testsuite)
        if self.nodes:
            self.servers = self.nodes + self.servers

    def teardownClientSide(self, current, success):
        if self.nodes:
            for n in self.nodes:
                n.shutdown(current)
