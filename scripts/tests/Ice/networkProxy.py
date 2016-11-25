# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import NetworkProxy

class NetworkProxyTestCase(ClientServerTestCase):

    def __init__(self, proxyName, proxyType, proxyPortNum):
        ClientServerTestCase.__init__(self, proxyName + " client/server", client = Client(props = lambda p, c: {
            "Ice.{0}ProxyHost".format(proxyName): "localhost",
            "Ice.{0}ProxyPort".format(proxyName): "{0}".format(c.driver.getTestPort(proxyPortNum))
        }))
        self.proxyName = proxyName
        self.proxyType = proxyType
        self.proxyPortNum = proxyPortNum
        self.proxy = None

    def setupClientSide(self, current):
        current.write("starting {0} proxy... ".format(self.proxyName))
        self.proxy = self.proxyType(current.driver.getTestPort(self.proxyPortNum))
        current.writeln("ok")

    def teardownClientSide(self, current, success):
        current.write("terminating {0} proxy... ".format(self.proxyName))
        self.proxy.terminate()
        self.proxy = None
        current.writeln("ok")

TestSuite(__name__, [
    NetworkProxyTestCase("SOCKS", NetworkProxy.SocksProxy, 30),
    NetworkProxyTestCase("HTTP", NetworkProxy.HttpProxy, 31),
], options = { "ipv6" : [False] })
