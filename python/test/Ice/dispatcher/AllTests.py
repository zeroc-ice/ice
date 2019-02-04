#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

import Ice, Test, Dispatcher, sys, threading, random

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

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
        test(Dispatcher.Dispatcher.isDispatcherThread())
        self.called()

    def exception(self, f):
        test(isinstance(f.exception(), Ice.NoEndpointException))
        test(Dispatcher.Dispatcher.isDispatcherThread())
        self.called()

    def exceptionEx(self, f):
        test(isinstance(f.exception(), Ice.InvocationTimeoutException))
        test(Dispatcher.Dispatcher.isDispatcherThread())
        self.called()

    def payload(self, f):
        if f.exception():
            test(isinstance(f.exception(), Ice.CommunicatorDestroyedException))
        else:
            test(Dispatcher.Dispatcher.isDispatcherThread())

def allTests(helper, communicator):
    sref = "test:{0}".format(helper.getTestEndpoint())
    obj = communicator.stringToProxy(sref)
    test(obj)

    p = Test.TestIntfPrx.uncheckedCast(obj)

    sref = "testController:{0}".format(helper.getTestEndpoint(num=1))
    obj = communicator.stringToProxy(sref)
    test(obj)

    testController = Test.TestIntfControllerPrx.uncheckedCast(obj)

    sys.stdout.write("testing dispatcher... ")
    sys.stdout.flush()

    p.op()

    cb = Callback()

    p.opAsync().add_done_callback_async(cb.response)
    cb.check()

    #
    # Expect NoEndpointException.
    #
    i = p.ice_adapterId("dummy")
    i.opAsync().add_done_callback_async(cb.exception)
    cb.check()

    #
    # Expect InvocationTimeoutException.
    #
    to = p.ice_invocationTimeout(10);
    to.sleepAsync(500).add_done_callback_async(cb.exceptionEx)
    cb.check()

    #
    # Hold adapter to make sure invocations don't _complete_ synchronously
    #
    testController.holdAdapter()

    if sys.version_info[0] == 2:
        b = [chr(random.randint(0, 255)) for x in range(0, 1024)]
        seq = ''.join(b)
    else:
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
