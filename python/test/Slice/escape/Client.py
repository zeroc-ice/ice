#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import sys

from TestHelper import TestHelper

if "--load-slice" in sys.argv:
    TestHelper.loadSlice("Key.ice Clash.ice")

from generated.test.Slice.escape.Test import escaped_and
from generated.test.Slice.escape.Test import Sequence, SequencePrx
from collections.abc import Sequence as SequenceABC

import Ice


class delI(escaped_and._del):
    def _elif(self, _else, current):
        pass


class execI(escaped_and._exec):
    def _finally(self, current):
        assert current.operation == "finally"


class ifI(escaped_and._if):
    def _elif(self, _else, current):
        pass

    def _finally(self, current):
        pass

    def _raise(self, _else, _return, _while, _yield, _or, _global):
        pass

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
    def run(self, args):
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
