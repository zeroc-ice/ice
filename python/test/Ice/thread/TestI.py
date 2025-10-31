# Copyright (c) ZeroC, Inc.

import threading
import time
from typing import override

from generated.test.Ice.thread import Test

import Ice


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
    @override
    def sleep(self, ms: int, current: Ice.Current):
        time.sleep(ms / 1000.0)


class RemoteCommunicatorI(Test.RemoteCommunicator):
    def __init__(self, communicator: Ice.Communicator, hook: ThreadHook):
        self.communicator = communicator
        self.hook = hook
        oa = communicator.createObjectAdapterWithEndpoints("", "default")
        self.obj = Test.TestIntfPrx.uncheckedCast(oa.addWithUUID(TestIntfI()))
        oa.activate()

    def getObject(self, current: Ice.Current):
        return self.obj

    def getThreadStartCount(self, current: Ice.Current):
        return self.hook.getThreadStartCount()

    def getThreadStopCount(self, current: Ice.Current):
        return self.hook.getThreadStopCount()

    def destroy(self, current: Ice.Current):
        self.communicator.destroy()


class RemoteCommunicatorFactoryI(Test.RemoteCommunicatorFactory):
    def createCommunicator(self, props: dict[str, str], current: Ice.Current):
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
        communicator = Ice.Communicator(initData=init)

        proxy = current.adapter.addWithUUID(RemoteCommunicatorI(communicator, threadHook))
        return Test.RemoteCommunicatorPrx.uncheckedCast(proxy)

    def shutdown(self, current: Ice.Current):
        current.adapter.getCommunicator().shutdown()
