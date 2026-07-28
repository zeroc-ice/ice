# Copyright (c) ZeroC, Inc.

import NetworkProxy
from Util import Client, ClientServerTestCase, Driver, Process, Props, TestSuite


class NetworkProxyTestSuite(TestSuite):
    # The test port offset, bumped by each test case as it tears down.
    portNum: int

    def setup(self, current: Driver.Current) -> None:
        self.portNum = 30


class NetworkProxyTestCase(ClientServerTestCase):
    def __init__(self, proxyName: str, proxyType: type[NetworkProxy.BaseProxy]):
        def clientProps(process: Process, current: Driver.Current) -> Props:
            testsuite = current.testsuite
            assert isinstance(testsuite, NetworkProxyTestSuite)
            return {
                "Ice.{0}ProxyHost".format(proxyName): "localhost",
                "Ice.{0}ProxyPort".format(proxyName): "{0}".format(current.driver.getTestPort(testsuite.portNum)),
            }

        ClientServerTestCase.__init__(
            self,
            proxyName + " client/server",
            client=Client(props=clientProps),
        )
        self.proxyName = proxyName
        self.proxyType = proxyType
        self.proxy: NetworkProxy.BaseProxy | None = None

    def canRun(self, current: Driver.Current) -> bool:
        if current.config.buildPlatform in ["iphonesimulator", "iphoneos"]:
            return False
        return True

    def setupClientSide(self, current: Driver.Current) -> None:
        current.write("starting {0} proxy... ".format(self.proxyName))
        testsuite = current.testsuite
        assert isinstance(testsuite, NetworkProxyTestSuite)
        self.proxy = self.proxyType(current.driver.getTestPort(testsuite.portNum))
        current.writeln("ok")

    def teardownClientSide(self, current: Driver.Current, success: bool) -> None:
        current.write("terminating {0} proxy... ".format(self.proxyName))
        assert self.proxy is not None
        self.proxy.terminate()
        self.proxy = None
        testsuite = current.testsuite
        assert isinstance(testsuite, NetworkProxyTestSuite)
        testsuite.portNum += 1
        current.writeln("ok")


NetworkProxyTestSuite(
    __name__,
    [
        NetworkProxyTestCase("SOCKS", NetworkProxy.SocksProxy),
        NetworkProxyTestCase("HTTP", NetworkProxy.HttpProxy),
    ],
    options={"ipv6": [False]},
)
