# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, time, threading

class ThreadHook(Ice.ThreadNotification):
    def __init__(self):
        self.threadHookStartCount = 0
        self.threadHookStopCount = 0
        self.threadStartCount = 0
        self.threadStopCount = 0
        self.cond = threading.Condition()

    def start(self):
        with self.cond:
            self.threadHookStartCount += 1

    def stop(self):
        with self.cond:
            self.threadHookStopCount += 1

    def threadStart(self):
        with self.cond:
            self.threadStartCount += 1

    def threadStop(self):
        with self.cond:
            self.threadStopCount += 1

    def getThreadHookStartCount(self):
        with self.cond:
            return self.threadHookStartCount

    def getThreadHookStopCount(self):
        with self.cond:
            return self.threadHookStopCount

    def getThreadStartCount(self):
        with self.cond:
            return self.threadStartCount

    def getThreadStopCount(self):
        with self.cond:
            return self.threadStopCount

class TestIntfI(Test.TestIntf):
    def sleep(self, ms, current = None):
        time.sleep(ms / 1000.0)

class RemoteCommunicatorI(Test.RemoteCommunicator):
    def __init__(self, communicator, hook):
        self.communicator = communicator
        self.hook = hook
        oa = communicator.createObjectAdapterWithEndpoints("", "default")
        self.obj = Test.TestIntfPrx.uncheckedCast(oa.addWithUUID(TestIntfI()))
        oa.activate()

    def getObject(self, current = None):
        return self.obj

    def getThreadHookStartCount(self, current = None):
        return self.hook.getThreadHookStartCount()

    def getThreadHookStopCount(self, current = None):
        return self.hook.getThreadHookStopCount()

    def getThreadStartCount(self, current = None):
        return self.hook.getThreadStartCount()

    def getThreadStopCount(self, current = None):
        return self.hook.getThreadStopCount()

    def destroy(self, current = None):
        self.communicator.destroy()

class RemoteCommunicatorFactoryI(Test.RemoteCommunicatorFactory):

    def createCommunicator(self, props, current = None):
        #
        # Prepare the property set using the given properties.
        #
        init = Ice.InitializationData()
        init.properties = Ice.createProperties()
        for k, v in props.items():
            init.properties.setProperty(k, v)

        init.threadHook = ThreadHook()
        init.threadStart = init.threadHook.threadStart
        init.threadStop = init.threadHook.threadStop

        #
        # Initialize a new communicator.
        #
        communicator = Ice.initialize(init)

        proxy = current.adapter.addWithUUID(RemoteCommunicatorI(communicator, init.threadHook))
        return Test.RemoteCommunicatorPrx.uncheckedCast(proxy)

    def shutdown(self, current = None):
        current.adapter.getCommunicator().shutdown()
