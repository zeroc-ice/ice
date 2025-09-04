#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys
from typing import override

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice(["Key.ice", "Clash.ice"])

from generated.test.Slice.escape.Test import Sequence, SequencePrx, escaped_and

import Ice


class delI(escaped_and._del):
    @override
    def _elif(self, _else: int, current: Ice.Current) -> int:
        return 0


class execI(escaped_and._exec):
    @override
    def _finally(self, current: Ice.Current):
        assert current.operation == "finally"


class ifI(escaped_and._if):
    @override
    def _elif(self, _else: int, current: Ice.Current) -> int:
        return 0

    @override
    def _finally(self, current: Ice.Current):
        pass

    @override
    def _raise(
        self,
        _else: escaped_and._continue,
        _return: escaped_and._for | None,
        _while: escaped_and._delPrx | None,
        _yield: escaped_and._execPrx | None,
        _or: escaped_and._ifPrx | None,
        _global: int,
        current: Ice.Current,
    ) -> escaped_and._assert:
        return escaped_and._assert._break


class SequenceI(Sequence):
    def sendIntSeq(self, seq: list[int], current: Ice.Current):
        pass

    def abstractmethod(self, current: Ice.Current):
        pass


def testtypes():
    sys.stdout.write("Testing generated type names... ")
    sys.stdout.flush()
    _a = escaped_and._assert._break
    b = escaped_and._continue
    b._def = 0
    _c = escaped_and._delPrx.uncheckedCast(None)
    assert "_elif" in dir(escaped_and._delPrx)
    _c1 = delI()
    _d = escaped_and._execPrx.uncheckedCast(None)
    assert "_finally" in dir(escaped_and._execPrx)
    _d1 = execI()

    _e1 = escaped_and._for()
    _f = escaped_and._ifPrx.uncheckedCast(None)

    assert "_finally" in dir(escaped_and._ifPrx)
    assert "_elif" in dir(escaped_and._ifPrx)
    _f1 = ifI()
    g = escaped_and._is()
    g.bar = 0
    h = escaped_and._not()
    h.bar = 0
    h._pass = 2
    _j = escaped_and._lambda
    _en = escaped_and.EnumNone._None
    print("ok")


class Client(TestHelper):
    def run(self, args: list[str]):
        properties = self.createTestProperties(args)
        #
        # Its possible to have batch oneway requests dispatched after the
        # adapter is deactivated due to thread scheduling so we suppress
        # this warning.
        #
        properties.setProperty("Ice.Warn.Dispatch", "0")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty("TestAdapter.Endpoints", self.getTestEndpoint())
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(execI(), Ice.stringToIdentity("test"))
            adapter.add(SequenceI(), Ice.stringToIdentity("sequence"))
            adapter.activate()

            sys.stdout.write("Testing operation name... ")
            sys.stdout.flush()
            p = escaped_and._execPrx.uncheckedCast(adapter.createProxy(Ice.stringToIdentity("test")))
            p._finally()

            p = SequencePrx.uncheckedCast(adapter.createProxy(Ice.stringToIdentity("sequence")))
            p.abstractmethod()
            print("ok")

            testtypes()
