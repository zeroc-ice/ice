# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, Test, Dispatcher, time

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class TestIntfI(Test._TestIntfDisp):
    def op(self, current=None):
        test(Dispatcher.Dispatcher.isDispatcherThread())

    def sleep(self, ms, current=None):
        time.sleep(ms / 1000.0)

    def opWithPayload(self, bytes, current=None):
        test(Dispatcher.Dispatcher.isDispatcherThread())

    def shutdown(self, current=None):
        test(Dispatcher.Dispatcher.isDispatcherThread())
        current.adapter.getCommunicator().shutdown()

class TestIntfControllerI(Test._TestIntfControllerDisp):
    def __init__(self, adapter):
        self._adapter = adapter

    def holdAdapter(self, current=None):
        test(Dispatcher.Dispatcher.isDispatcherThread())
        self._adapter.hold()

    def resumeAdapter(self, current=None):
        test(Dispatcher.Dispatcher.isDispatcherThread())
        self._adapter.activate()
