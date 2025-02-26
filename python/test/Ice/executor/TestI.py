# Copyright (c) ZeroC, Inc.

import Test
import Dispatcher
import time


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class TestIntfI(Test.TestIntf):
    def op(self, current):
        test(Dispatcher.Dispatcher.isDispatcherThread())

    def sleep(self, ms, current):
        time.sleep(ms / 1000.0)

    def opWithPayload(self, bytes, current):
        test(Dispatcher.Dispatcher.isDispatcherThread())

    def shutdown(self, current):
        test(Dispatcher.Dispatcher.isDispatcherThread())
        current.adapter.getCommunicator().shutdown()


class TestIntfControllerI(Test.TestIntfController):
    def __init__(self, adapter):
        self._adapter = adapter

    def holdAdapter(self, current):
        test(Dispatcher.Dispatcher.isDispatcherThread())
        self._adapter.hold()

    def resumeAdapter(self, current):
        test(Dispatcher.Dispatcher.isDispatcherThread())
        self._adapter.activate()
