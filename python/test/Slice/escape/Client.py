#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from TestHelper import TestHelper

TestHelper.loadSlice("Key.ice Clash.ice")
import sys
import Ice
import escapedAnd


class delI(escapedAnd._del):
    def _elifAsync(self, _else, current):
        pass


class execI(escapedAnd._exec):
    def _finally(self, current):
        assert current.operation == "finally"


class ifI(escapedAnd._if):
    def _elifAsync(self, _else, current):
        pass

    def _finally(self, current):
        pass

    def _raise(self, _else, _return, _while, _yield, _or, _global):
        pass


def testtypes():
    sys.stdout.write("Testing generated type names... ")
    sys.stdout.flush()
    _a = escapedAnd._assert._break
    b = escapedAnd._continue
    b._def = 0
    _c = escapedAnd._delPrx.uncheckedCast(None)
    assert "_elif" in dir(escapedAnd._delPrx)
    _c1 = delI()
    _d = escapedAnd._execPrx.uncheckedCast(None)
    assert "_finally" in dir(escapedAnd._execPrx)
    _d1 = execI()

    _e1 = escapedAnd._for()
    _f = escapedAnd._ifPrx.uncheckedCast(None)

    assert "_finally" in dir(escapedAnd._ifPrx)
    assert "_elif" in dir(escapedAnd._ifPrx)
    _f1 = ifI()
    g = escapedAnd._is()
    g.bar = 0
    h = escapedAnd._not()
    h.bar = 0
    h._pass = 2
    _j = escapedAnd._lambda
    _en = escapedAnd.EnumNone._None
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
            p = escapedAnd._execPrx.uncheckedCast(
                adapter.createProxy(Ice.stringToIdentity("test"))
            )
            p._finally()
            print("ok")

            testtypes()
