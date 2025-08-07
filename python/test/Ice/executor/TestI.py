# Copyright (c) ZeroC, Inc.

import time
from typing import override

import Executor
from generated.test.Ice.executor import Test
from TestHelper import test

import Ice


class TestIntfI(Test.TestIntf):
    @override
    def op(self, current: Ice.Current):
        test(Executor.Executor.isExecutorThread())

    @override
    def sleep(self, to: int, current: Ice.Current):
        time.sleep(to / 1000.0)

    @override
    def opWithPayload(self, seq: bytes, current: Ice.Current):
        test(Executor.Executor.isExecutorThread())

    @override
    def shutdown(self, current: Ice.Current):
        test(Executor.Executor.isExecutorThread())
        current.adapter.getCommunicator().shutdown()


class TestIntfControllerI(Test.TestIntfController):
    def __init__(self, adapter: Ice.ObjectAdapter):
        self._adapter = adapter

    def holdAdapter(self, current: Ice.Current):
        test(Executor.Executor.isExecutorThread())
        self._adapter.hold()

    def resumeAdapter(self, current: Ice.Current):
        test(Executor.Executor.isExecutorThread())
        self._adapter.activate()
