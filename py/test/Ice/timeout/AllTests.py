#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, threading

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
            self._cond.wait(5.0)
        self._called = False
        return True

class AMISendData(CallbackBase):
    def ice_response(self):
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMISendDataEx(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        test(isinstance(ex, Ice.TimeoutException))
        self.called()

class AMISleep(CallbackBase):
    def ice_response(self):
        self.called()

    def ice_exception(self, ex):
        test(False)

class AMISleepEx(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, ex):
        test(isinstance(ex, Ice.TimeoutException))
        self.called()

def allTests(communicator, collocated):
    sref = "timeout:default -p 12010 -t 10000"
    obj = communicator.stringToProxy(sref)
    test(obj != None)

    timeout = Test.TimeoutPrx.checkedCast(obj)
    test(timeout != None)

    print "testing connect timeout... ",
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
    print "ok"

    print "testing read timeout... ",
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
    print "ok"

    print "testing write timeout... ",
    #
    # Expect TimeoutException.
    #
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(500))
    to.holdAdapter(750)
    seq = []
    seq[0:100000] = range(0, 100000) # add 100,000 entries.
    seq = ['\x00' for x in seq] # set them all to \x00
    seq = ''.join(seq) # make into a byte array
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
    print "ok"

    print "testing AMI read timeout... ",
    #
    # The resolution of AMI timeouts is limited by the connection monitor
    # thread. We set Ice.MonitorConnections=1 (one second) in main().
    #
    # Expect TimeoutException.
    #
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(500))
    cb = AMISleepEx()
    to.sleep_async(cb, 2000)
    test(cb.check())
    #
    # Expect success.
    #
    timeout.op() # Ensure adapter is active.
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(1000))
    cb = AMISleep()
    to.sleep_async(cb, 500)
    test(cb.check())
    print "ok"

    print "testing AMI write timeout... ",
    #
    # The resolution of AMI timeouts is limited by the connection monitor
    # thread. We set Ice.MonitorConnections=1 (one second) in main().
    #
    # Expect TimeoutException.
    #
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(500))
    to.holdAdapter(2000)
    cb = AMISendDataEx()
    to.sendData_async(cb, seq)
    test(cb.check())
    #
    # Expect success.
    #
    timeout.op() # Ensure adapter is active.
    to = Test.TimeoutPrx.uncheckedCast(obj.ice_timeout(1000))
    to.holdAdapter(500)
    cb = AMISendData()
    to.sendData_async(cb, seq)
    test(cb.check())
    print "ok"

    print "testing timeout overrides... ",
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
        to.sleep(1500)
        test(False)
    except Ice.TimeoutException:
       pass # Expected.
    comm.destroy()
    print "ok"

    return timeout
