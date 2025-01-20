# Copyright (c) ZeroC, Inc.

import Ice
import Test


class RemoteCommunicatorI(Test.RemoteCommunicator):
    def __init__(self):
        self._nextPort = 10001

    def createObjectAdapter(self, name, endpoints, current):
        self._nextPort += 1
        if endpoints.find("-p") < 0:
            endpoints += ' -h "{0}" -p {1}'.format(
                current.adapter.getCommunicator()
                .getProperties()
                .getPropertyWithDefault("Ice.Default.Host", "127.0.0.1"),
                self._nextPort,
            )

        com = current.adapter.getCommunicator()
        com.getProperties().setProperty(name + ".ThreadPool.Size", "1")
        adapter = com.createObjectAdapterWithEndpoints(name, endpoints)
        return Test.RemoteObjectAdapterPrx.uncheckedCast(
            current.adapter.addWithUUID(RemoteObjectAdapterI(adapter))
        )

    def deactivateObjectAdapter(self, adapter, current):
        adapter.deactivate()

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()


class RemoteObjectAdapterI(Test.RemoteObjectAdapter):
    def __init__(self, adapter):
        self._adapter = adapter
        self._testIntf = Test.TestIntfPrx.uncheckedCast(
            self._adapter.add(TestI(), Ice.stringToIdentity("test"))
        )
        self._adapter.activate()

    def getTestIntf(self, current):
        return self._testIntf

    def deactivate(self, current):
        self._adapter.destroy()


class TestI(Test.TestIntf):
    def getAdapterName(self, current):
        return current.adapter.getName()
