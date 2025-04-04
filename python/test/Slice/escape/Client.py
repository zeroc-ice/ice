#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from TestHelper import TestHelper

TestHelper.loadSlice("Key.ice Clash.ice")
import sys
import Ice
import escaped_and


class delI(escaped_and._del):
    def _elifAsync(self, _else, current):
        pass


class execI(escaped_and._exec):
    def _finally(self, current):
        assert current.operation == "finally"


class ifI(escaped_and._if):
    def _elifAsync(self, _else, current):
        pass

    def _finally(self, current):
        pass

    def _raise(self, _else, _return, _while, _yield, _or, _global):
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
            communicator.getProperties().setProperty("TestAdapter.Endpoints", "default")
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(execI(), Ice.stringToIdentity("test"))
            adapter.activate()

            sys.stdout.write("Testing operation name... ")
            sys.stdout.flush()
            p = escaped_and._execPrx.uncheckedCast(
                adapter.createProxy(Ice.stringToIdentity("test"))
            )
            p._finally()
            print("ok")

            testtypes()
