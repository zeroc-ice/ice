# **********************************************************************
#
# Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, sys, threading

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class CallbackBase:
    def __init__(self):
        self._called = False
        self._cond = threading.Condition()

    def called(self):
        self._cond.acquire()
        self._called = True
        self._cond.notify()
        self._cond.release()

    def check(self):
        self._cond.acquire()
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

def allTests(communicator, collocated):
    sref = "timeout:default -p 12010"
    obj = communicator.stringToProxy(sref)
    test(obj != None)

    timeout = Test.TimeoutPrx.checkedCast(obj)
    test(timeout != None)

    sys.stdout.write("testing connect timeout... ")
    sys.stdout.flush()
    #
    # Expect ConnectTimeoutException.
    #
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(500))
    to.holdAdapter(750)
    to.ice_getConnection().close(True) # Force a reconnect.
    try:
        to.op()
        test(False)
    except Ice.ConnectTimeoutException:
       pass # Expected.
    #
    # Expect success.
    #
    timeout.op() # Ensure adapter is active.
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(1000))
    to.holdAdapter(500)
    to.ice_getConnection().close(True) # Force a reconnect.
    try:
        to.op()
    except Ice.ConnectTimeoutException:
        test(False)
    print("ok")

    sys.stdout.write("testing read timeout... ")
    sys.stdout.flush()
    #
    # Expect TimeoutException.
    #
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(500))
    try:
        to.sleep(750)
        test(False)
    except Ice.TimeoutException:
       pass # Expected.
    #
    # Expect success.
    #
    timeout.op() # Ensure adapter is active.
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(1000))
    try:
        to.sleep(500)
    except Ice.TimeoutException:
        test(False)
    print("ok")

    sys.stdout.write("testing write timeout... ")
    sys.stdout.flush()
    #
    # Expect TimeoutException.
    #
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(500))
    to.holdAdapter(2000)
    if sys.version_info[0] == 2:
        seq = []
        seq[0:100000] = range(0, 100000) # add 100,000 entries.
        seq = ['\x00' for x in seq] # set them all to \x00
        seq = ''.join(seq) # make into a byte array
    else:
        seq = bytes([0 for x in range(0, 100000)])
    try:
        to.sendData(seq)
        test(False)
    except Ice.TimeoutException:
       pass # Expected.
    #
    # Expect success.
    #
    timeout.op() # Ensure adapter is active.
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(1000))
    to.holdAdapter(500)
    try:
        to.sendData(seq)
    except Ice.TimeoutException:
        test(False)
    print("ok")

    sys.stdout.write("testing AMI read timeout... ")
    sys.stdout.flush()
    #
    # Expect TimeoutException.
    #
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(500))
    cb = Callback()
    to.begin_sleep(2000, cb.responseEx, cb.exceptionEx)
    cb.check()
    #
    # Expect success.
    #
    timeout.op() # Ensure adapter is active.
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(1000))
    cb = Callback()
    to.begin_sleep(500, cb.response, cb.exception)
    cb.check()
    print("ok")

    sys.stdout.write("testing AMI write timeout... ")
    sys.stdout.flush()
    #
    # Expect TimeoutException.
    #
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(500))
    to.holdAdapter(2000)
    cb = Callback()
    to.begin_sendData(seq, cb.responseEx, cb.exceptionEx)
    cb.check()
    #
    # Expect success.
    #
    timeout.op() # Ensure adapter is active.
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(1000))
    to.holdAdapter(500)
    cb = Callback()
    to.begin_sendData(seq, cb.response, cb.exception)
    cb.check()
    print("ok")

    sys.stdout.write("testing timeout overrides... ")
    sys.stdout.flush()
    #
    # Test Ice.Override.Timeout. This property overrides all
    # endpoint timeouts.
    #
    initData = Ice.InitializationData()
    initData.properties = communicator.getProperties().clone()
    initData.properties.setProperty("Ice.Override.Timeout", "500")
    comm = Ice.initialize(initData)
    to = Test.TimeoutPrx.checkedCast(comm.stringToProxy(sref))
    try:
        to.sleep(750)
        test(False)
    except Ice.TimeoutException:
       pass # Expected.
    #
    # Calling ice_timeout() should have no effect.
    #
    timeout.op() # Ensure adapter is active.
    to = Test.TimeoutPrx.checkedCast(to.ice_timeout(1000))
    try:
        to.sleep(750)
        test(False)
    except Ice.TimeoutException:
       pass # Expected.
    comm.destroy()
    #
    # Test Ice.Override.ConnectTimeout.
    #
    initData = Ice.InitializationData()
    initData.properties = communicator.getProperties().clone()
    initData.properties.setProperty("Ice.Override.ConnectTimeout", "750")
    comm = Ice.initialize(initData)
    timeout.holdAdapter(1000)
    to = Test.TimeoutPrx.uncheckedCast(comm.stringToProxy(sref))
    try:
        to.op()
        test(False)
    except Ice.ConnectTimeoutException:
       pass # Expected.
    #
    # Calling ice_timeout() should have no effect on the connect timeout.
    #
    timeout.op() # Ensure adapter is active.
    timeout.holdAdapter(1000)
    to = Test.TimeoutPrx.uncheckedCast(to.ice_timeout(1250))
    try:
        to.op()
        test(False)
    except Ice.ConnectTimeoutException:
       pass # Expected.
    #
    # Verify that timeout set via ice_timeout() is still used for requests.
    #
    to.op() # Force connection.
    try:
        to.sleep(2000)
        test(False)
    except Ice.TimeoutException:
       pass # Expected.
    comm.destroy()
    print("ok")

    return timeout
