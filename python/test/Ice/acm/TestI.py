# **********************************************************************
#
# Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, threading

class RemoteCommunicatorI(Test._RemoteCommunicatorDisp):
    def createObjectAdapter(self, timeout, close, heartbeat, current=None):
        com = current.adapter.getCommunicator()
        properties = com.getProperties()
        protocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");

        name = Ice.generateUUID()
        if timeout >= 0:
            properties.setProperty(name + ".ACM.Timeout", str(timeout))
        if close >= 0:
            properties.setProperty(name + ".ACM.Close", str(close))
        if heartbeat >= 0:
            properties.setProperty(name + ".ACM.Heartbeat", str(heartbeat))
        properties.setProperty(name + ".ThreadPool.Size", "2")
        adapter = com.createObjectAdapterWithEndpoints(name, protocol + " -h 127.0.0.1")
        return Test.RemoteObjectAdapterPrx.uncheckedCast(current.adapter.addWithUUID(RemoteObjectAdapterI(adapter)))

    def shutdown(self, current=None):
        current.adapter.getCommunicator().shutdown()

class RemoteObjectAdapterI(Test._RemoteObjectAdapterDisp):
    def __init__(self, adapter):
        self._adapter = adapter
        self._testIntf = Test.TestIntfPrx.uncheckedCast(adapter.add(TestIntfI(),
                                                        Ice.stringToIdentity("test")))
        adapter.activate()

    def getTestIntf(self, current=None):
        return self._testIntf

    def activate(self, current=None):
        self._adapter.activate()

    def hold(self, current=None):
        self._adapter.hold()

    def deactivate(self, current=None):
        try:
            self._adapter.destroy()
        except Ice.ObjectAdapterDeactivatedException:
            pass

class TestIntfI(Test._TestIntfDisp):
    def __init__(self):
        self.m = threading.Condition()

    def sleep(self, delay, current=None):
        with self.m:
            self.m.wait(delay)

    def sleepAndHold(self, delay, current=None):
        with self.m:
            current.adapter.hold()
            self.m.wait(delay)

    def interruptSleep(self, delay, current=None):
        with self.m:
            self.m.notifyAll()

    def waitForHeartbeat(self, count, current=None):

        class ConnectionCallbackI():

            def __init__(self):
                self.m = threading.Condition()
                self.count = 0

            def heartbeat(self, con):
                with self.m:
                    self.count -= 1
                    self.m.notifyAll()

            def waitForCount(self, count):
                with self.m:
                    self.count = count
                    while self.count > 0:
                        self.m.wait()

        callback = ConnectionCallbackI()
        current.con.setHeartbeatCallback(lambda con: callback.heartbeat(con))
        callback.waitForCount(2)

