# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test

class RemoteCommunicatorI(Test.RemoteCommunicator):

    def __init__(self):
        self._nextPort = 10001

    def createObjectAdapter(self, name, endpoints, current=None):
        self._nextPort += 1
        if endpoints.find("-p") < 0:
            endpoints += " -h \"{0}\" -p {1}".format(
                current.adapter.getCommunicator().getProperties().getPropertyWithDefault("Ice.Default.Host", "127.0.0.1"),
                self._nextPort)

        com = current.adapter.getCommunicator()
        com.getProperties().setProperty(name + ".ThreadPool.Size", "1")
        adapter = com.createObjectAdapterWithEndpoints(name, endpoints)
        return Test.RemoteObjectAdapterPrx.uncheckedCast(current.adapter.addWithUUID(RemoteObjectAdapterI(adapter)))

    def deactivateObjectAdapter(self, adapter, current=None):
        adapter.deactivate()

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

class RemoteObjectAdapterI(Test.RemoteObjectAdapter):
    def __init__(self, adapter):
        self._adapter = adapter
        self._testIntf = Test.TestIntfPrx.uncheckedCast(self._adapter.add(TestI(), Ice.stringToIdentity("test")))
        self._adapter.activate()

    def getTestIntf(self, current=None):
        return self._testIntf

    def deactivate(self, current=None):
        try:
            self._adapter.destroy()
        except Ice.ObjectAdapterDeactivatedException:
            pass

class TestI(Test.TestIntf):
    def getAdapterName(self, current=None):
        return current.adapter.getName()
