# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

import Ice, Test, threading

class ConnectionCallbackI():
    def __init__(self):
        self.m = threading.Condition()
        self.count = 0

    def heartbeat(self, con):
        with self.m:
            self.count += 1
            self.m.notifyAll()

    def waitForCount(self, count):
        with self.m:
            while self.count < count:
                self.m.wait()

class RemoteCommunicatorI(Test.RemoteCommunicator):
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

class RemoteObjectAdapterI(Test.RemoteObjectAdapter):
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

class TestIntfI(Test.TestIntf):
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

    def startHeartbeatCount(self, current=None):
        self.callback = ConnectionCallbackI()
        current.con.setHeartbeatCallback(lambda con: self.callback.heartbeat(con))

    def waitForHeartbeatCount(self, count, current=None):
        self.callback.waitForCount(2)
