# Copyright (c) ZeroC, Inc.

import re

from Util import (
    Client,
    ClientServerTestCase,
    Mapping,
    Process,
    ProcessFromBinDir,
    Server,
)

# Regex pattern to match placeholders like {port1}, {port2}, ..., {portXX}
port_pattern = re.compile(r"{port(\d+)}")


class DataStormProcess(Process):
    def getEffectiveProps(self, current, props):
        props = Process.getEffectiveProps(self, current, props)
        for key, value in props.items():
            if key.startswith("DataStorm.Node.") and type(value) is str:
                props[key] = port_pattern.sub(
                    lambda match: str(current.driver.getTestPort(10 + int(match.group(1)))), value
                )

        return props


class Writer(Client, DataStormProcess):
    processType = "writer"

    def __init__(self, instanceName=None, instance=None, *args, **kargs):
        Client.__init__(self, *args, **kargs)

    def getEffectiveProps(self, current, props):
        props = DataStormProcess.getEffectiveProps(self, current, props)
        if ("DataStorm.Node.Multicast.Enabled", 1) in props.items():
            port = current.driver.getTestPort(20)
            props["DataStorm.Node.Multicast.Endpoints"] = f"udp -h 239.255.0.1 -p {port}"
            props["DataStorm.Node.Multicast.Proxy"] = (
                f"DataStorm/Lookup -d:udp -h 239.255.0.1 --sourceAddress 127.0.0.1 -p {port}"
            )
        elif not any(key.startswith("DataStorm.Node.") for key in props):
            # Default properties for tests that don't specify any DataStorm.Node.* properties
            props.update(
                {
                    "DataStorm.Node.Multicast.Enabled": 0,
                    "DataStorm.Node.Server.Enabled": 0,
                    "DataStorm.Node.ConnectTo": f"tcp -p {current.driver.getTestPort(10)}",
                }
            )
        return props


class Reader(Server, DataStormProcess):
    processType = "reader"

    def __init__(self, instanceName=None, instance=None, *args, **kargs):
        # Set readyCount to 0 to skip waiting for adapter activation, as a DataStorm reader may not activate any adapters
        # when both DataStorm.Node.Server.Enabled and DataStorm.Node.Multicast.Enabled are set to 0.
        Server.__init__(self, readyCount=0, *args, **kargs)

    def getEffectiveProps(self, current, props):
        props = DataStormProcess.getEffectiveProps(self, current, props)
        if ("DataStorm.Node.Multicast.Enabled", 1) in props.items():
            port = current.driver.getTestPort(20)
            props["DataStorm.Node.Multicast.Endpoints"] = f"udp -h 239.255.0.1 -p {port}"
            props["DataStorm.Node.Multicast.Proxy"] = (
                f"DataStorm/Lookup -d:udp -h 239.255.0.1 --sourceAddress 127.0.0.1 -p {port}"
            )
        elif not any(key.startswith("DataStorm.Node.") for key in props):
            # Default properties for tests that don't specify any DataStorm.Node.* properties
            props.update(
                {
                    "DataStorm.Node.Multicast.Enabled": 0,
                    "DataStorm.Node.Server.Endpoints": f"tcp -p {current.driver.getTestPort(10)}",
                }
            )
        return props


class Node(ProcessFromBinDir, Server, DataStormProcess):
    def __init__(self, desc=None, *args, **kargs):
        Server.__init__(self, "dsnode", mapping=Mapping.getByName("cpp"), desc=desc or "DataStorm node", *args, **kargs)

    def shutdown(self, current):
        if self in current.processes:
            current.processes[self].terminate()

    def getProps(self, current):
        props = Server.getProps(self, current)
        props["Ice.ProgramName"] = self.desc
        return props

    def getEffectiveProps(self, current, props):
        return DataStormProcess.getEffectiveProps(self, current, props)


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
