# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, sys, threading, time

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class CallbackBase:
    def __init__(self):
        self._called = False
        self._cond = threading.Condition()

    def called(self):
        with self._cond:
            self._called = True
            self._cond.notify()

    def check(self):
        with self._cond:
            while not self._called:
                self._cond.wait()
            self._called = False
            return True

class Callback(CallbackBase):
    def response(self):
        self.called()

    def exception(self, ex):
        test(False)

    def responseEx(self):
        test(False)

    def exceptionEx(self, ex):
        test(isinstance(ex, Ice.TimeoutException))
        self.called()

def connect(prx):
    # Establish connection with the given proxy (which might have a timeout
    # set and might sporadically fail on connection establishment if it's
    # too slow). The loop ensures that the connection is established by retrying
    # in case we can a ConnectTimeoutException
    nRetry = 10
    while --nRetry > 0:
        try:
            prx.ice_getConnection();
            break
        except Ice.ConnectTimeoutException:
            # Can sporadically occur with slow machines
            pass
    return prx.ice_getConnection(); # Establish connection

def allTests(communicator):

    sref = "timeout:default -p 12010"
    obj = communicator.stringToProxy(sref)
    test(obj != None)

    timeout = Test.TimeoutPrx.checkedCast(obj)
    test(timeout != None)

    controller = Test.ControllerPrx.checkedCast(communicator.stringToProxy("controller:default -p 12011"))
    test(controller != None)

    sys.stdout.write("testing connect timeout... ")
    sys.stdout.flush()
    #
    # Expect ConnectTimeoutException.
    #
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(100))
    controller.holdAdapter(-1)
    try:
        to.op()
        test(False)
    except Ice.ConnectTimeoutException:
       pass # Expected.
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.

    #
    # Expect success.
    #
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(2000))
    controller.holdAdapter(100)
    try:
        to.op()
    except Ice.ConnectTimeoutException:
        test(False)
    print("ok")

    sys.stdout.write("testing connection timeout... ")
    sys.stdout.flush()
    #
    # Expect TimeoutException.
    #
    if sys.version_info[0] == 2:
        seq = []
        seq[0:10000000] = range(0, 10000000) # add 10,000,000 entries.
        seq = ['\x00' for x in seq] # set them all to \x00
        seq = ''.join(seq) # make into a byte array
    else:
        seq = bytes([0 for x in range(0, 10000000)])
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(250))
    connect(to)
    controller.holdAdapter(-1)
    try:
        to.sendData(seq)
        test(False)
    except Ice.TimeoutException:
       pass # Expected.
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.
    #
    # Expect success.
    #
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(2000))
    controller.holdAdapter(100)
    try:
        if sys.version_info[0] == 2:
            seq2 = []
            seq2[0:1000000] = range(0, 1000000) # add 1,000,000 entries.
            seq2 = ['\x00' for x in seq2] # set them all to \x00
            seq2 = ''.join(seq2) # make into a byte array
        else:
            seq2 = bytes([0 for x in range(0, 1000000)])
        to.sendData(seq2)
    except Ice.TimeoutException:
        test(False)
    print("ok")

    sys.stdout.write("testing invocation timeout... ")
    sys.stdout.flush()
    connection = obj.ice_getConnection()
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_invocationTimeout(100))
    test(connection == to.ice_getConnection())
    try:
        to.sleep(500)
        test(False)
    except Ice.InvocationTimeoutException:
        pass
    obj.ice_ping()
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_invocationTimeout(1000))
    test(connection == to.ice_getConnection())
    try:
        to.sleep(100)
    except Ice.InvocationTimeoutException:
        test(False)
    test(connection == to.ice_getConnection())

    # #
    # # Expect InvocationTimeoutException.
    # #
    # to = Test.TimeoutPrx.uncheckedCast(obj.ice_invocationTimeout(250))
    # cb = new Callback()
    # to.begin_sleep(500, newCallback_Timeout_sleep(cb, &Callback.responseEx, &Callback.exceptionEx))
    # cb.check()

    # #
    # # Expect success.
    # #
    # to = Test.TimeoutPrx.uncheckedCast(obj.ice_invocationTimeout(1000))
    # cb = new Callback()
    # to.begin_sleep(100, newCallback_Timeout_sleep(cb, &Callback.response, &Callback.exception))
    # cb.check()
    print("ok")

    sys.stdout.write("testing close timeout... ")
    sys.stdout.flush()
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(250))
    connection = connect(to)
    controller.holdAdapter(-1)
    connection.close(Ice.ConnectionClose.GracefullyWithWait)
    try:
        connection.getInfo(); # getInfo() doesn't throw in the closing state.
    except Ice.LocalException:
        test(False)
    while True:
        try:
            connection.getInfo()
            time.sleep(0.001)
        except Ice.ConnectionManuallyClosedException as ex:
            # Expected.
            test(ex.graceful)
            break
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.
    print("ok")

    sys.stdout.write("testing timeout overrides... ")
    sys.stdout.flush()

    #
    # Test Ice.Override.Timeout. This property overrides all
    # endpoint timeouts.
    #
    initData = Ice.InitializationData()
    initData.properties = communicator.getProperties().clone()
    initData.properties.setProperty("Ice.Override.ConnectTimeout", "250")
    initData.properties.setProperty("Ice.Override.Timeout", "100")
    comm = Ice.initialize(initData)
    to = Test.TimeoutPrx.uncheckedCast(comm.stringToProxy(sref))
    connect(to)
    controller.holdAdapter(-1)
    try:
        to.sendData(seq)
        test(False)
    except Ice.TimeoutException:
       pass # Expected.
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.

    #
    # Calling ice_timeout() should have no effect.
    #
    to = Test.TimeoutPrx.uncheckedCast(to.ice_timeout(1000))
    connect(to)
    controller.holdAdapter(-1)
    try:
        to.sendData(seq)
        test(False)
    except Ice.TimeoutException:
       pass # Expected.
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.
    comm.destroy()
    #
    # Test Ice.Override.ConnectTimeout.
    #
    initData = Ice.InitializationData()
    initData.properties = communicator.getProperties().clone()
    initData.properties.setProperty("Ice.Override.ConnectTimeout", "250")
    comm = Ice.initialize(initData)
    controller.holdAdapter(-1)
    to = Test.TimeoutPrx.uncheckedCast(comm.stringToProxy(sref))
    try:
        to.op()
        test(False)
    except Ice.ConnectTimeoutException:
       pass # Expected.
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.
    #
    # Calling ice_timeout() should have no effect on the connect timeout.
    #
    controller.holdAdapter(-1)
    to = Test.TimeoutPrx.uncheckedCast(to.ice_timeout(1000))
    try:
        to.op()
        test(False)
    except Ice.ConnectTimeoutException:
       pass # Expected.
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.
    #
    # Verify that timeout set via ice_timeout() is still used for requests.
    #
    to = Test.TimeoutPrx.uncheckedCast(to.ice_timeout(250))
    connect(to)
    controller.holdAdapter(-1)
    try:
        to.sendData(seq)
        test(False)
    except Ice.TimeoutException:
       pass # Expected.
    controller.resumeAdapter()
    timeout.op() # Ensure adapter is active.
    comm.destroy()

    #
    # Test Ice.Override.CloseTimeout.
    #
    initData = Ice.InitializationData()
    initData.properties = communicator.getProperties().clone()
    initData.properties.setProperty("Ice.Override.CloseTimeout", "100")
    comm = Ice.initialize(initData)
    connection = comm.stringToProxy(sref).ice_getConnection()
    controller.holdAdapter(-1)
    now = time.clock()
    comm.destroy()
    test((time.clock() - now) < 0.7)
    controller.resumeAdapter()

    print("ok")

    controller.shutdown()
