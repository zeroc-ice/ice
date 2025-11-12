# Copyright (c) ZeroC, Inc.

from DataStormUtil import Node, NodeTestCase, Reader, Writer
from Util import ClientServerTestCase, TestSuite

traceProps = {
    "DataStorm.Trace.Topic": 1,
    "DataStorm.Trace.Session": 3,
    "DataStorm.Trace.Data": 2,
    "Ice.Trace.Protocol": 1,
    "Ice.Trace.Network": 3,
}

# A client connected to the default server
clientProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Enabled": 0,
    "DataStorm.Node.ConnectTo": "tcp -p {port1}",
}

# A client connected to the second server
client2Props = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Enabled": 0,
    "DataStorm.Node.ConnectTo": "tcp -p {port2}",
}

# The default server, not connected to any other server.
serverProps = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Endpoints": "tcp -p {port1}",
    "DataStorm.Node.ConnectTo": "",
}

# A second server connected to the first server
server2Props = {
    "DataStorm.Node.Multicast.Enabled": 0,
    "DataStorm.Node.Server.Endpoints": "tcp -p {port2}",
    "DataStorm.Node.ConnectTo": "tcp -p {port1}",
}

# A server without a fixed endpoint, connected to the first server.
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
    (
        "reader as client and writer as server with 2 nodes",
        clientProps,
        serverAnyProps,
        serverProps,
        server2Props,
        False,
    ),
    (
        "reader as server and writer as client with 2 nodes",
        serverAnyProps,
        clientProps,
        serverProps,
        server2Props,
        False,
    ),
    ("reader/writer as client with 2 nodes", clientProps, client2Props, serverProps, server2Props, True),
    (
        "reader as client and writer as server with 2 nodes",
        clientProps,
        serverAnyProps,
        serverProps,
        server2Props,
        True,
    ),
    (
        "reader as server and writer as client with 2 nodes",
        serverAnyProps,
        clientProps,
        serverProps,
        server2Props,
        True,
    ),
]

testcases = []
for name, clientProps, serverProps, nodeProps, node2Props, reversedStart in props:
    if reversedStart:
        name += " (reversed start order)"

        readerProps = dict(serverProps)
        writerProps = dict(clientProps)
    else:
        readerProps = dict(clientProps)
        writerProps = dict(serverProps)

    readerProps["DataStorm.Node.Name"] = "reader-app"
    writerProps["DataStorm.Node.Name"] = "writer-app"

    if nodeProps:
        nodeProps["DataStorm.Node.Name"] = "relay-node-1"

    if node2Props:
        node2Props["DataStorm.Node.Name"] = "relay-node-2"

    reader = Reader(props=readerProps)
    writer = Writer(props=writerProps)

    if reversedStart:
        client = writer
        server = reader
    else:
        client = reader
        server = writer

    if node2Props:
        nodes = [Node(desc="node1", props=nodeProps), Node(desc="node2", props=node2Props)]
        testcases.append(NodeTestCase(name=name, client=client, server=server, nodes=nodes, traceProps=traceProps))
    elif nodeProps:
        testcases.append(
            NodeTestCase(name=name, client=client, server=server, nodeProps=nodeProps, traceProps=traceProps)
        )
    else:
        testcases.append(ClientServerTestCase(name=name, client=client, server=server, traceProps=traceProps))

TestSuite(__file__, testcases)
