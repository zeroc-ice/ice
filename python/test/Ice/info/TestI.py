# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test
import time

class MyDerivedClassI(Test.TestIntf):
    def __init__(self):
        self.ctx = None

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

    def getEndpointInfoAsContext(self, current):
        ctx = {}
        info = current.con.getEndpoint().getInfo()
        ctx["timeout"] = str(info.timeout)
        if info.compress:
            ctx["compress"] = "true"
        else:
            ctx["compress"] ="false"
        if info.datagram():
            ctx["datagram"] = "true"
        else:
            ctx["datagram"] ="false"
        if info.secure():
            ctx["secure"] = "true"
        else:
            ctx["secure"] ="false"
        ctx["type"] = str(info.type())

        ctx["host"] = info.host
        ctx["port"] = str(info.port)

        if isinstance(info, Ice.UDPEndpointInfo):
            ctx["protocolMajor"] = str(info.protocolMajor)
            ctx["protocolMinor"] = str(info.protocolMinor)
            ctx["encodingMajor"] = str(info.encodingMajor)
            ctx["encodingMinor"] = str(info.encodingMinor)
            ctx["mcastInterface"] = info.mcastInterface
            ctx["mcastTtl"] = str(info.mcastTtl)

        return ctx

    def getConnectionInfoAsContext(self, current):
        ctx = {}
        info = current.con.getInfo()
        ctx["adapterName"] = info.adapterName
        if info.incoming:
            ctx["incoming"] = "true"
        else:
            ctx["incoming"] ="false"

        ctx["localAddress"] = info.localAddress
        ctx["localPort"] = str(info.localPort)
        ctx["remoteAddress"] = info.remoteAddress
        ctx["remotePort"] = str(info.remotePort)

        if isinstance(info, Ice.WSConnectionInfo) or isinstance(info, Ice.WSSConnectionInfo):
            for key, value in info.headers.items():
                ctx["ws." + key] = value

        return ctx
