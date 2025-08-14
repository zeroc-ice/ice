# Copyright (c) ZeroC, Inc.

from typing import override

from generated.test.Ice.info import Test

import Ice


def getIPEndpointInfo(info: Ice.EndpointInfo) -> Ice.IPEndpointInfo:
    while info:
        if isinstance(info, Ice.IPEndpointInfo):
            return info
        assert info.underlying is not None
        info = info.underlying


def getIPConnectionInfo(info: Ice.ConnectionInfo) -> Ice.IPConnectionInfo:
    while info:
        if isinstance(info, Ice.IPConnectionInfo):
            return info

        assert info.underlying is not None
        info = info.underlying


class MyDerivedClassI(Test.TestIntf):
    def __init__(self):
        self.ctx = None

    @override
    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()

    @override
    def getEndpointInfoAsContext(self, current: Ice.Current) -> dict[str, str]:
        ctx = {}
        assert current.con is not None
        info = getIPEndpointInfo(current.con.getEndpoint().getInfo())
        if info.compress:
            ctx["compress"] = "true"
        else:
            ctx["compress"] = "false"
        if info.datagram():
            ctx["datagram"] = "true"
        else:
            ctx["datagram"] = "false"
        if info.secure():
            ctx["secure"] = "true"
        else:
            ctx["secure"] = "false"
        ctx["type"] = str(info.type())

        ctx["host"] = info.host
        ctx["port"] = str(info.port)

        return ctx

    @override
    def getConnectionInfoAsContext(self, current: Ice.Current) -> dict[str, str]:
        ctx = {}
        assert current.con is not None
        info = current.con.getInfo()
        ipinfo = getIPConnectionInfo(info)
        ctx["adapterName"] = info.adapterName
        if info.incoming:
            ctx["incoming"] = "true"
        else:
            ctx["incoming"] = "false"

        ctx["localAddress"] = ipinfo.localAddress
        ctx["localPort"] = str(ipinfo.localPort)
        ctx["remoteAddress"] = ipinfo.remoteAddress
        ctx["remotePort"] = str(ipinfo.remotePort)

        if isinstance(info, Ice.WSConnectionInfo):
            for key, value in info.headers.items():
                ctx["ws." + key] = value

        return ctx
