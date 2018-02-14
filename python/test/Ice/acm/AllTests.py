# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, sys, threading, time, traceback

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class LoggerI(Ice.Logger):
    def __init__(self):
        self._started = False
        self._messages = []
        self.m = threading.Lock()

    def start(self):
        self.m.acquire()
        try:
            self._started = True
            self.dump()
        finally:
            self.m.release()

    def _print(self, msg):
        self.m.acquire()
        try:
            self._messages.append(msg)
            if self._started:
                self.dump()
        finally:
            self.m.release()

    def trace(self, category, msg):
        self.m.acquire()
        try:
            self._messages.append("[" + category + "] " + msg)
            if self._started:
                self.dump()
        finally:
            self.m.release()

    def warning(self, msg):
        self.m.acquire()
        try:
            self._messages.append("warning: " + msg)
            if self._started:
                self.dump()
        finally:
            self.m.release()

    def error(self, msg):
        self.m.acquire()
        try:
            self._messages.append("error: " + msg)
            if self._started:
                self.dump()
        finally:
            self.m.release()

    def getPrefix(self):
        return ""

    def cloneWithPrefix(self, prefix):
        return self

    def dump(self):
        for p in self._messages:
            print(p)
        self._messages = []

class TestCase(threading.Thread, Ice.ConnectionCallback):
    def __init__(self, name, com):
        threading.Thread.__init__(self)
        self._name = name
        self._com = com
        self._logger = LoggerI()
        self._clientACMTimeout = -1
        self._clientACMClose = -1
        self._clientACMHeartbeat = -1
        self._serverACMTimeout = -1
        self._serverACMClose = -1
        self._serverACMHeartbeat = -1
        self._heartbeat = 0
        self._closed = False
        self._msg = ""
        self.m = threading.Lock()

    def init(self):
        self._adapter = \
            self._com.createObjectAdapter(self._serverACMTimeout, self._serverACMClose, self._serverACMHeartbeat)

        initData = Ice.InitializationData()
        initData.properties = self._com.ice_getCommunicator().getProperties().clone()
        initData.logger = self._logger
        initData.properties.setProperty("Ice.ACM.Timeout", "1")
        if self._clientACMTimeout >= 0:
            initData.properties.setProperty("Ice.ACM.Client.Timeout", str(self._clientACMTimeout))
        if self._clientACMClose >= 0:
            initData.properties.setProperty("Ice.ACM.Client.Close", str(self._clientACMClose))
        if self._clientACMHeartbeat >= 0:
            initData.properties.setProperty("Ice.ACM.Client.Heartbeat", str(self._clientACMHeartbeat))
        #initData.properties.setProperty("Ice.Trace.Protocol", "2")
        #initData.properties.setProperty("Ice.Trace.Network", "2")
        self._communicator = Ice.initialize(initData)

    def destroy(self):
        self._adapter.deactivate()
        self._communicator.destroy()

    def joinWithThread(self):
        sys.stdout.write("testing " + self._name + "... ")
        sys.stdout.flush()
        self._logger.start()
        self.join()
        if len(self._msg) == 0:
            print("ok")
        else:
            print("failed!\n" + self._msg)
            test(False)

    def run(self):
        proxy = Test.TestIntfPrx.uncheckedCast(self._communicator.stringToProxy(
                    self._adapter.getTestIntf().ice_toString()))
        try:
            proxy.ice_getConnection().setCallback(self)
            self.runTestCase(self._adapter, proxy)
        except Exception as ex:
            self._msg = "unexpected exception:\n" + traceback.format_exc()

    def heartbeat(self, con):
        self.m.acquire()
        try:
            self._heartbeat = self._heartbeat + 1
        finally:
            self.m.release()

    def closed(self, con):
        self.m.acquire()
        try:
            self._closed = True
        finally:
            self.m.release()

    def runTestCase(self, adapter, proxy):
        test(False)

    def setClientACM(self, timeout, close, heartbeat):
        self._clientACMTimeout = timeout
        self._clientACMClose = close
        self._clientACMHeartbeat = heartbeat

    def setServerACM(self, timeout, close, heartbeat):
        self._serverACMTimeout = timeout
        self._serverACMClose = close
        self._serverACMHeartbeat = heartbeat

def allTests(communicator):
    ref = "communicator:default -p 12010"
    com = Test.RemoteCommunicatorPrx.uncheckedCast(communicator.stringToProxy(ref))

    tests = []

    class InvocationHeartbeatTest(TestCase):
        def __init__(self, com):
            TestCase.__init__(self, "invocation heartbeat", com)

        def runTestCase(self, adapter, proxy):
            proxy.sleep(2)

            self.m.acquire()
            try:
                test(self._heartbeat >= 2)
            finally:
                self.m.release()

    class InvocationHeartbeatOnHoldTest(TestCase):
        def __init__(self, com):
            TestCase.__init__(self, "invocation with heartbeat on hold", com)
            # Use default ACM configuration.

        def runTestCase(self, adapter, proxy):
            try:
                # When the OA is put on hold, connections shouldn't
                # send heartbeats, the invocation should therefore
                # fail.
                proxy.sleepAndHold(10)
                test(False)
            except Ice.ConnectionTimeoutException:
                adapter.activate()
                proxy.interruptSleep()

                self.m.acquire()
                try:
                    test(self._closed)
                finally:
                    self.m.release()

    class InvocationNoHeartbeatTest(TestCase):
        def __init__(self, com):
            TestCase.__init__(self, "invocation with no heartbeat", com)
            self.setServerACM(1, 2, 0) # Disable heartbeat on invocations

        def runTestCase(self, adapter, proxy):
            try:
                # Heartbeats are disabled on the server, the
                # invocation should fail since heartbeats are
                # expected.
                proxy.sleep(10)
                test(False)
            except Ice.ConnectionTimeoutException:
                proxy.interruptSleep()

                self.m.acquire()
                try:
                    test(self._heartbeat == 0)
                    test(self._closed)
                finally:
                    self.m.release()

    class InvocationHeartbeatCloseOnIdleTest(TestCase):
        def __init__(self, com):
            TestCase.__init__(self, "invocation with no heartbeat and close on idle", com)
            self.setClientACM(1, 1, 0) # Only close on idle.
            self.setServerACM(1, 2, 0) # Disable heartbeat on invocations

        def runTestCase(self, adapter, proxy):
            # No close on invocation, the call should succeed this time.
            proxy.sleep(2)

            self.m.acquire()
            try:
                test(self._heartbeat == 0)
                test(not self._closed)
            finally:
                self.m.release()

    class CloseOnIdleTest(TestCase):
        def __init__(self, com):
            TestCase.__init__(self, "close on idle", com)
            self.setClientACM(1, 1, 0) # Only close on idle.

        def runTestCase(self, adapter, proxy):
            time.sleep(1.6) # Idle for 1.6 seconds

            self.m.acquire()
            try:
                test(self._heartbeat == 0)
                test(self._closed)
            finally:
                self.m.release()

    class CloseOnInvocationTest(TestCase):
        def __init__(self, com):
            TestCase.__init__(self, "close on invocation", com)
            self.setClientACM(1, 2, 0) # Only close on invocation.

        def runTestCase(self, adapter, proxy):
            time.sleep(1.5) # Idle for 1.5 seconds

            self.m.acquire()
            try:
                test(self._heartbeat == 0)
                test(not self._closed)
            finally:
                self.m.release()

    class CloseOnIdleAndInvocationTest(TestCase):
        def __init__(self, com):
            TestCase.__init__(self, "close on idle and invocation", com)
            self.setClientACM(1, 3, 0) # Only close on idle and invocation.

        def runTestCase(self, adapter, proxy):
            #
            # Put the adapter on hold. The server will not respond to
            # the graceful close. This allows to test whether or not
            # the close is graceful or forceful.
            #
            adapter.hold()
            time.sleep(1.6) # Idle for 1.6 seconds

            self.m.acquire()
            try:
                test(self._heartbeat == 0)
                test(not self._closed) # Not closed yet because of graceful close.
            finally:
                self.m.release()

            adapter.activate()
            time.sleep(0.5)

            self.m.acquire()
            try:
                test(self._closed) # Connection should be closed this time.
            finally:
                self.m.release()

    class ForcefulCloseOnIdleAndInvocationTest(TestCase):
        def __init__(self, com):
            TestCase.__init__(self, "forceful close on idle and invocation", com)
            self.setClientACM(1, 4, 0) # Only close on idle and invocation.

        def runTestCase(self, adapter, proxy):
            adapter.hold()
            time.sleep(1.6) # Idle for 1.6 seconds

            self.m.acquire()
            try:
                test(self._heartbeat == 0)
                test(self._closed) # Connection closed forcefully by ACM.
            finally:
                self.m.release()

    class HeartbeatOnIdleTest(TestCase):
        def __init__(self, com):
            TestCase.__init__(self, "heartbeat on idle", com)
            self.setServerACM(1, -1, 2) # Enable server heartbeats.

        def runTestCase(self, adapter, proxy):
            time.sleep(2)

            self.m.acquire()
            try:
                test(self._heartbeat >= 3)
            finally:
                self.m.release()

    class HeartbeatAlwaysTest(TestCase):
        def __init__(self, com):
            TestCase.__init__(self, "heartbeat always", com)
            self.setServerACM(1, -1, 3) # Enable server heartbeats.

        def runTestCase(self, adapter, proxy):
            for i in range(0, 20):
                proxy.ice_ping()
                time.sleep(0.1)

            self.m.acquire()
            try:
                test(self._heartbeat >= 3)
            finally:
                self.m.release()

    class SetACMTest(TestCase):
        def __init__(self, com):
            TestCase.__init__(self, "setACM/getACM", com)
            self.setClientACM(15, 4, 0)

        def runTestCase(self, adapter, proxy):
            acm = proxy.ice_getCachedConnection().getACM()
            test(acm.timeout == 15)
            test(acm.close == Ice.ACMClose.CloseOnIdleForceful)
            test(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatOff)

            proxy.ice_getCachedConnection().setACM(Ice.Unset, Ice.Unset, Ice.Unset)
            acm = proxy.ice_getCachedConnection().getACM()
            test(acm.timeout == 15)
            test(acm.close == Ice.ACMClose.CloseOnIdleForceful)
            test(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatOff)

            proxy.ice_getCachedConnection().setACM(1, Ice.ACMClose.CloseOnInvocationAndIdle,
                                                   Ice.ACMHeartbeat.HeartbeatAlways)
            acm = proxy.ice_getCachedConnection().getACM()
            test(acm.timeout == 1)
            test(acm.close == Ice.ACMClose.CloseOnInvocationAndIdle)
            test(acm.heartbeat == Ice.ACMHeartbeat.HeartbeatAlways)

            proxy.waitForHeartbeat(2)

    tests.append(InvocationHeartbeatTest(com))
    tests.append(InvocationHeartbeatOnHoldTest(com))
    tests.append(InvocationNoHeartbeatTest(com))
    tests.append(InvocationHeartbeatCloseOnIdleTest(com))

    tests.append(CloseOnIdleTest(com))
    tests.append(CloseOnInvocationTest(com))
    tests.append(CloseOnIdleAndInvocationTest(com))
    tests.append(ForcefulCloseOnIdleAndInvocationTest(com))

    tests.append(HeartbeatOnIdleTest(com))
    tests.append(HeartbeatAlwaysTest(com))
    tests.append(SetACMTest(com))

    for p in tests:
        p.init()
    for p in tests:
        p.start()
    for p in tests:
        p.joinWithThread()
    for p in tests:
        p.destroy()

    sys.stdout.write("shutting down... ")
    sys.stdout.flush()
    com.shutdown()
    print("ok")
