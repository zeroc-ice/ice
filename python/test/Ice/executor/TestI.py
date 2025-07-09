# Copyright (c) ZeroC, Inc.

import time

import Executor
import Test


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class TestIntfI(Test.TestIntf):
    def op(self, current):
        test(Executor.Executor.isExecutorThread())

    def sleep(self, ms, current):
        time.sleep(ms / 1000.0)

    def opWithPayload(self, bytes, current):
        test(Executor.Executor.isExecutorThread())

    def shutdown(self, current):
        test(Executor.Executor.isExecutorThread())
        current.adapter.getCommunicator().shutdown()


class TestIntfControllerI(Test.TestIntfController):
    def __init__(self, adapter):
        self._adapter = adapter

    def holdAdapter(self, current):
        test(Executor.Executor.isExecutorThread())
        self._adapter.hold()

    def resumeAdapter(self, current):
        test(Executor.Executor.isExecutorThread())
        self._adapter.activate()
