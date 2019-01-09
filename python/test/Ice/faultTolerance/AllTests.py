# **********************************************************************
#
# Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
#
# **********************************************************************

import Ice, sys, threading

Ice.loadSlice('Test.ice')
import Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class CallbackBase:
    def __init__(self):
        self._called = False
        self._cond = threading.Condition()

    def check(self):
        with self._cond:
            while not self._called:
                self._cond.wait()
            self._called = False

    def called(self):
        with self._cond:
            self._called = True
            self._cond.notify()

class Callback(CallbackBase):
    def opPidI(self, f):
        try:
            self._pid = f.result()
            self.called()
        except:
            test(False)

    def opShutdownI(self, f):
        test(f.exception() is None)
        self.called()

    def exceptAbortI(self, f):
        test(f.exception() is not None)
        try:
            f.result()
        except Ice.ConnectionLostException:
            pass
        except Ice.ConnectFailedException:
            pass
        except Ice.Exception as ex:
            print(ex)
            test(False)
        self.called()

    def pid(self):
        return self._pid

def allTests(helper, communicator, ports):
    sys.stdout.write("testing stringToProxy... ")
    sys.stdout.flush()
    ref = "test"
    for i in range(len(ports)):
        ref += ":{0}".format(helper.getTestEndpoint(num=i))
    base = communicator.stringToProxy(ref)
    test(base)
    print("ok")

    sys.stdout.write("testing checked cast... ")
    sys.stdout.flush()
    obj = Test.TestIntfPrx.checkedCast(base)
    test(obj)
    test(obj == base)
    print("ok")

    oldPid = 0
    ami = False
    i = 1
    j = 0
    while i <= len(ports):
        if j > 3:
            j = 0
            ami = not ami

        if not ami:
            sys.stdout.write("testing server #%d... " % i)
            sys.stdout.flush()
            pid = obj.pid()
            test(pid != oldPid)
            print("ok")
            oldPid = pid
        else:
            sys.stdout.write("testing server #%d with AMI... " % i)
            sys.stdout.flush()
            cb = Callback()
            obj.pidAsync().add_done_callback(cb.opPidI)
            cb.check()
            pid = cb.pid()
            test(pid != oldPid)
            print("ok")
            oldPid = pid

        if j == 0:
            if not ami:
                sys.stdout.write("shutting down server #%d... " % i)
                sys.stdout.flush()
                obj.shutdown()
                print("ok")
            else:
                sys.stdout.write("shutting down server #%d with AMI... " % i)
                sys.stdout.flush()
                cb = Callback()
                obj.shutdownAsync().add_done_callback(cb.opShutdownI)
                cb.check()
                print("ok")
        elif j == 1 or i + 1 > len(ports):
            if not ami:
                sys.stdout.write("aborting server #%d... " % i)
                sys.stdout.flush()
                try:
                    obj.abort()
                    test(False)
                except Ice.ConnectionLostException:
                    print("ok")
                except Ice.ConnectFailedException:
                    print("ok")
            else:
                sys.stdout.write("aborting server #%d with AMI... " % i)
                sys.stdout.flush()
                cb = Callback()
                obj.abortAsync().add_done_callback(cb.exceptAbortI)
                cb.check()
                print("ok")
        elif j == 2 or j == 3:
            if not ami:
                sys.stdout.write("aborting server #%d and #%d with idempotent call... " % (i, i + 1))
                sys.stdout.flush()
                try:
                    obj.idempotentAbort()
                    test(False)
                except Ice.ConnectionLostException:
                    print("ok")
                except Ice.ConnectFailedException:
                    print("ok")
            else:
                sys.stdout.write("aborting server #%d and #%d with idempotent AMI call... " % (i, i + 1))
                sys.stdout.flush()
                cb = Callback()
                obj.idempotentAbortAsync().add_done_callback(cb.exceptAbortI)
                cb.check()
                print("ok")

            i = i + 1
        else:
            assert(False)

        i = i + 1
        j = j + 1

    sys.stdout.write("testing whether all servers are gone... ")
    sys.stdout.flush()
    try:
        obj.ice_ping()
        test(False)
    except Ice.LocalException:
        print("ok")
