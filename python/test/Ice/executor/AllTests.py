# Copyright (c) ZeroC, Inc.

import Ice
import Test
import Executor
import sys
import threading
import random


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class Callback:
    def __init__(self):
        self._called = False
        self._cond = threading.Condition()
        self._mainThread = threading.current_thread()

    def check(self):
        with self._cond:
            while not self._called:
                self._cond.wait()
            self._called = False

    def called(self):
        with self._cond:
            self._called = True
            self._cond.notify()

    def response(self, f):
        test(f.exception() is None)
        test(Executor.Executor.isExecutorThread())
        self.called()

    def exception(self, f):
        test(isinstance(f.exception(), Ice.NoEndpointException))
        test(Executor.Executor.isExecutorThread())
        self.called()

    def exceptionEx(self, f):
        test(isinstance(f.exception(), Ice.InvocationTimeoutException))
        test(Executor.Executor.isExecutorThread())
        self.called()

    def payload(self, f):
        if f.exception():
            test(isinstance(f.exception(), Ice.CommunicatorDestroyedException))
        else:
            test(Executor.Executor.isExecutorThread())


def allTests(helper, communicator):
    p = Test.TestIntfPrx(communicator, f"test:{helper.getTestEndpoint()}")
    testController = Test.TestIntfControllerPrx(communicator, f"testController:{helper.getTestEndpoint(num=1)}")

    sys.stdout.write("testing executor... ")
    sys.stdout.flush()

    p.op()

    cb = Callback()

    # Hold adapter to make sure invocations don't _complete_ synchronously. If add_done_callback is called on
    # a completed future, the callback is called immediately in the calling thread.
    testController.holdAdapter()
    p.opAsync().add_done_callback(cb.response)
    testController.resumeAdapter()
    cb.check()

    #
    # Expect NoEndpointException.
    #
    i = p.ice_adapterId("dummy")
    i.opAsync().add_done_callback(cb.exception)
    cb.check()

    #
    # Expect InvocationTimeoutException.
    #
    to = p.ice_invocationTimeout(10)
    to.sleepAsync(500).add_done_callback(cb.exceptionEx)
    cb.check()

    #
    # Hold adapter to make sure invocations don't _complete_ synchronously
    #
    testController.holdAdapter()
    b = [random.randint(0, 255) for x in range(0, 1024)]
    seq = bytes(b)

    f = None
    while True:
        f = p.opWithPayloadAsync(seq)
        f.add_done_callback(cb.payload)
        if not f.is_sent_synchronously():
            break
    testController.resumeAdapter()
    f.result()

    print("ok")

    p.shutdown()
