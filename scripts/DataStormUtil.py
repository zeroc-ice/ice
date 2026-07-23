# Copyright (c) ZeroC, Inc.

from __future__ import annotations

import re
import time
from collections.abc import Callable
from typing import Any

from Util import (
    Client,
    ClientServerTestCase,
    Darwin,
    Driver,
    Mapping,
    Process,
    ProcessFromBinDir,
    Props,
    Server,
    TestSuite,
    platform,
)

# Regex pattern to match placeholders like {port1}, {port2}, ..., {portXX}
port_pattern = re.compile(r"{port(\d+)}")


def waitForLogMessage(path: str, pattern: str, timeout: float = 60) -> None:
    """Wait for a regular-expression match in a log file.

    Used by DataStorm tests to synchronize on trace messages written to an Ice.LogFile."""
    expr = re.compile(pattern)
    deadline = time.time() + timeout
    file = None
    try:
        while time.time() < deadline:
            if file is None:
                try:
                    file = open(path, "r", encoding="utf-8")
                except FileNotFoundError:
                    time.sleep(0.1)
                    continue
            line = file.readline()
            if not line:
                time.sleep(0.1)
                continue
            if expr.search(line):
                return
        raise RuntimeError(f"timed out after {timeout}s waiting for '{pattern}' in {path}")
    finally:
        if file is not None:
            file.close()


class DataStormProcess(Process):
    def getEffectiveProps(self, current: Driver.Current, props: Props) -> Props:
        props = Process.getEffectiveProps(self, current, props)
        for key, value in props.items():
            if key.startswith("DataStorm.Node.") and type(value) is str:
                props[key] = port_pattern.sub(
                    lambda match: str(current.driver.getTestPort(10 + int(match.group(1)))), value
                )

        return props


class Writer(Client, DataStormProcess):
    processType = "writer"

    def __init__(self, instanceName: str | None = None, instance: Any = None, *args: Any, **kargs: Any):
        Client.__init__(self, *args, **kargs)

    def getEffectiveProps(self, current: Driver.Current, props: Props) -> Props:
        props = DataStormProcess.getEffectiveProps(self, current, props)
        if ("DataStorm.Node.Multicast.Enabled", 1) in props.items():
            port = current.driver.getTestPort(20)
            props["DataStorm.Node.Multicast.Endpoints"] = f"udp -h 239.255.0.1 -p {port}"
            # Need to use --interface 127.0.0.1 for the macOS GitHub runners.
            if isinstance(platform, Darwin):
                props["DataStorm.Node.Multicast.Proxy"] = (
                    f"DataStorm/Lookup -d:udp -h 239.255.0.1 -p {port} --interface 127.0.0.1"
                )
            else:
                props["DataStorm.Node.Multicast.Proxy"] = f"DataStorm/Lookup -d:udp -h 239.255.0.1 -p {port}"
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

    def __init__(self, instanceName: str | None = None, instance: Any = None, *args: Any, **kargs: Any):
        # Set readyCount to 0 to skip waiting for adapter activation, as a DataStorm reader may not activate any adapters
        # when both DataStorm.Node.Server.Enabled and DataStorm.Node.Multicast.Enabled are set to 0.
        Server.__init__(self, readyCount=0, *args, **kargs)

    def getEffectiveProps(self, current: Driver.Current, props: Props) -> Props:
        props = DataStormProcess.getEffectiveProps(self, current, props)
        if ("DataStorm.Node.Multicast.Enabled", 1) in props.items():
            port = current.driver.getTestPort(20)
            props["DataStorm.Node.Multicast.Endpoints"] = f"udp -h 239.255.0.1 -p {port}"
            # Need to use --interface 127.0.0.1 for the macOS GitHub runners.
            if isinstance(platform, Darwin):
                props["DataStorm.Node.Multicast.Proxy"] = (
                    f"DataStorm/Lookup -d:udp -h 239.255.0.1 -p {port} --interface 127.0.0.1"
                )
            else:
                props["DataStorm.Node.Multicast.Proxy"] = f"DataStorm/Lookup -d:udp -h 239.255.0.1 -p {port}"
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
    def __init__(self, desc: str | None = None, *args: Any, **kargs: Any):
        Server.__init__(self, "dsnode", mapping=Mapping.getByName("cpp"), desc=desc or "DataStorm node", *args, **kargs)

    def shutdown(self, current: Driver.Current) -> None:
        if self in current.processes:
            current.processes[self].terminate()

    def getProps(self, current: Driver.Current) -> Props:
        props = Server.getProps(self, current)
        props["Ice.ProgramName"] = self.desc
        return props

    def getEffectiveProps(self, current: Driver.Current, props: Props) -> Props:
        return DataStormProcess.getEffectiveProps(self, current, props)


class NodeTestCase(ClientServerTestCase):
    def __init__(
        self,
        nodes: list[Node] | None = None,
        nodeProps: Props | Callable[[Process, Driver.Current], Props] | None = None,
        *args: Any,
        **kargs: Any,
    ):
        ClientServerTestCase.__init__(self, *args, **kargs)
        self.nodes: list[Node] | None
        if nodes:
            self.nodes = nodes
        elif nodeProps:
            self.nodes = [Node(props=nodeProps)]
        else:
            self.nodes = None

    def init(self, mapping: Mapping, testsuite: TestSuite) -> None:
        ClientServerTestCase.init(self, mapping, testsuite)
        if self.nodes:
            self.servers = self.nodes + self.getServers()

    def teardownClientSide(self, current: Driver.Current, success: bool) -> None:
        if self.nodes:
            for n in self.nodes:
                n.shutdown(current)
