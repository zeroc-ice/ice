#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import NetworkProxy

class NetworkProxyTestSuite(TestSuite):

    def setup(self, current):
        self.portNum = 30

class NetworkProxyTestCase(ClientServerTestCase):

    def __init__(self, proxyName, proxyType):
        ClientServerTestCase.__init__(self, proxyName + " client/server", client = Client(props = lambda p, c: {
            "Ice.{0}ProxyHost".format(proxyName): "127.0.0.1", # our proxy server only supports IPv4
            "Ice.{0}ProxyPort".format(proxyName): "{0}".format(c.driver.getTestPort(c.testsuite.portNum))
        }))
        self.proxyName = proxyName
        self.proxyType = proxyType
        self.proxy = None

    def canRun(self, current):
        if current.config.buildPlatform in ["iphonesimulator", "iphoneos"]:
            return False
        return True

    def setupClientSide(self, current):
        current.write("starting {0} proxy... ".format(self.proxyName))
        self.proxy = self.proxyType(current.driver.getTestPort(current.testsuite.portNum))
        current.writeln("ok")

    def teardownClientSide(self, current, success):
        current.write("terminating {0} proxy... ".format(self.proxyName))
        self.proxy.terminate()
        self.proxy = None
        current.testsuite.portNum += 1
        current.writeln("ok")

NetworkProxyTestSuite(__name__, [
    NetworkProxyTestCase("SOCKS", NetworkProxy.SocksProxy),
    NetworkProxyTestCase("HTTP", NetworkProxy.HttpProxy),
], options = { "ipv6" : [False] })
