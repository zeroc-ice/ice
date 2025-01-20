# Copyright (c) ZeroC, Inc.

import Ice
import Test
import time
import threading


class ThreadHook:
    def __init__(self):
        self.threadStartCount = 0
        self.threadStopCount = 0
        self.cond = threading.Condition()

    def threadStart(self):
        with self.cond:
            self.threadStartCount += 1

    def threadStop(self):
        with self.cond:
            self.threadStopCount += 1

    def getThreadStartCount(self):
        with self.cond:
            return self.threadStartCount

    def getThreadStopCount(self):
        with self.cond:
            return self.threadStopCount


class TestIntfI(Test.TestIntf):
    def sleep(self, ms, current):
        time.sleep(ms / 1000.0)


class RemoteCommunicatorI(Test.RemoteCommunicator):
    def __init__(self, communicator, hook):
        self.communicator = communicator
        self.hook = hook
        oa = communicator.createObjectAdapterWithEndpoints("", "default")
        self.obj = Test.TestIntfPrx.uncheckedCast(oa.addWithUUID(TestIntfI()))
        oa.activate()

    def getObject(self, current):
        return self.obj

    def getThreadStartCount(self, current):
        return self.hook.getThreadStartCount()

    def getThreadStopCount(self, current):
        return self.hook.getThreadStopCount()

    def destroy(self, current):
        self.communicator.destroy()


class RemoteCommunicatorFactoryI(Test.RemoteCommunicatorFactory):
    def createCommunicator(self, props, current):
        #
        # Prepare the property set using the given properties.
        #
        init = Ice.InitializationData()
        init.properties = Ice.createProperties()
        for k, v in props.items():
            init.properties.setProperty(k, v)

        threadHook = ThreadHook()

        init.threadStart = threadHook.threadStart
        init.threadStop = threadHook.threadStop

        #
        # Initialize a new communicator.
        #
        communicator = Ice.initialize(init)

        proxy = current.adapter.addWithUUID(
            RemoteCommunicatorI(communicator, threadHook)
        )
        return Test.RemoteCommunicatorPrx.uncheckedCast(proxy)

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()
