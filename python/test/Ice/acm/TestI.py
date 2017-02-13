# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, threading

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
                                                        adapter.getCommunicator().stringToIdentity("test")))
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
        self.m.acquire()
        try:
            self.m.wait(delay)
        finally:
            self.m.release()

    def sleepAndHold(self, delay, current=None):
        self.m.acquire()
        try:
            current.adapter.hold()
            self.m.wait(delay)
        finally:
            self.m.release()

    def interruptSleep(self, delay, current=None):
        self.m.acquire()
        try:
            self.m.notifyAll()
        finally:
            self.m.release()

    def waitForHeartbeat(self, count, current=None):

        class ConnectionCallbackI(Ice.ConnectionCallback):

            def __init__(self):
                self.m = threading.Condition()
                self.count = 0

            def heartbeat(self, con):
                self.m.acquire()
                try:
                    self.count -= 1
                    self.m.notifyAll()
                finally:
                    self.m.release()

            def closed(self, con):
                pass

            def waitForCount(self, count):
                self.m.acquire()
                self.count = count
                try:
                    while self.count > 0:
                        self.m.wait()
                finally:
                    self.m.release()

        callback = ConnectionCallbackI()
        current.con.setCallback(callback)
        callback.waitForCount(2)

