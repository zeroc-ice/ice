#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from DataStormUtil import Reader, Writer, Node, NodeTestCase
from Util import ClientServerTestCase, TestSuite

traceProps = {
    "DataStorm.Trace.Topic" : 1,
    "DataStorm.Trace.Session" : 3,
    "DataStorm.Trace.Data" : 2,
}

clientProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Enabled": 0,
    "DataStorm.Node.ConnectTo": "tcp -p {port1}"
}

client2Props = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Enabled": 0,
    "DataStorm.Node.ConnectTo": "tcp -p {port2}"
}

serverProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Enabled": 1,
    "DataStorm.Node.Server.Endpoints": "tcp -p {port1}",
    "DataStorm.Node.ConnectTo": ""
}

server2Props = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Enabled": 1,
    "DataStorm.Node.Server.Endpoints": "tcp -p {port2}",
    "DataStorm.Node.ConnectTo": "tcp -p {port1}"
}

serverAnyProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Endpoints": "tcp",
    "DataStorm.Node.ConnectTo": "tcp -p {port1}",
}

props = [
    ("reader as client with writer as server", clientProps, serverProps, None, None, False),
    ("reader as server with writer as client", serverProps, clientProps, None, None, False),
    ("reader as client with writer as server", clientProps, serverProps, None, None, True),
    ("reader as server with writer as client", serverProps, clientProps, None, None, True),
    ("reader/writer as client with node", clientProps, clientProps, serverProps, None, False),
    ("reader as client and writer as server with node", clientProps, serverAnyProps, serverProps, None, False),
    ("reader as server and writer as client with node", serverAnyProps, clientProps, serverProps, None, False),
    ("reader/writer as client with node", clientProps, clientProps, serverProps, None, True),
    ("reader as client and writer as server with node", clientProps, serverAnyProps, serverProps, None, True),
    ("reader as server and writer as client with node", serverAnyProps, clientProps, serverProps, None, True),
    ("reader/writer as client with 2 nodes", clientProps, client2Props, serverProps, server2Props, False),
    ("reader as client and writer as server with 2 nodes", clientProps, serverAnyProps, serverProps, server2Props, False),
    ("reader as server and writer as client with 2 nodes", serverAnyProps, clientProps, serverProps, server2Props, False),
    ("reader/writer as client with 2 nodes", clientProps, client2Props, serverProps, server2Props, True),
    ("reader as client and writer as server with 2 nodes", clientProps, serverAnyProps, serverProps, server2Props, True),
    ("reader as server and writer as client with 2 nodes", serverAnyProps, clientProps, serverProps, server2Props, True),
]

testcases = []
for (name, readerProps, writerProps, nodeProps, node2Props, reversedStart) in props:
    if reversedStart:
        name += " (reversed start order)"
    c = Writer(props=writerProps) if not reversedStart else Reader(props=readerProps)
    s = Reader(props=readerProps) if not reversedStart else Writer(props=writerProps)
    if node2Props:
        nodes = [Node(desc="node1", props=nodeProps), Node(desc="node2", props=node2Props)]
        testcases.append(NodeTestCase(name=name, client=c, server=s, nodes=nodes, traceProps=traceProps))
    elif nodeProps:
        testcases.append(NodeTestCase(name=name, client=c, server=s, nodeProps=nodeProps, traceProps=traceProps))
    else:
        testcases.append(ClientServerTestCase(name=name, client=c, server=s, traceProps=traceProps))

TestSuite(__file__, testcases)
