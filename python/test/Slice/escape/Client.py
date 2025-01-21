#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

from TestHelper import TestHelper

TestHelper.loadSlice("Key.ice Clash.ice")
import sys
import Ice
import _and


class delI(_and._del):
    def _elifAsync(self, _else, current):
        pass


class execI(_and._exec):
    def _finally(self, current):
        assert current.operation == "finally"


class ifI(_and._if):
    def _elifAsync(self, _else, current):
        pass

    def _finally(self, current):
        pass

    def foo(self, _from, current):
        pass


class printI(_and._print):
    def _raise(self, _else, _return, _while, _yield, _or, _global):
        pass


def testtypes():
    sys.stdout.write("Testing generated type names... ")
    sys.stdout.flush()
    _a = _and._assert._break
    b = _and._continue
    b._def = 0
    _c = _and.delPrx.uncheckedCast(None)
    assert "_elif" in dir(_and.delPrx)
    _c1 = delI()
    _d = _and.execPrx.uncheckedCast(None)
    assert "_finally" in dir(_and.execPrx)
    _d1 = execI()

    _e1 = _and._for()
    _f = _and.ifPrx.uncheckedCast(None)

    assert "_finally" in dir(_and.ifPrx)
    assert "_elif" in dir(_and.ifPrx)
    _f1 = ifI()
    g = _and._is()
    g._lamba = 0
    h = _and._not()
    h._lamba = 0
    h._or = 1
    h._pass = 2
    _i = printI()
    _j = _and._lambda
    _en = _and.EnumNone._None
    print("ok")


class Client(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        #
        # Its possible to have batch oneway requests dispatched after the
        # adapter is deactivated due to thread scheduling so we supress
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
            p = _and.execPrx.uncheckedCast(
                adapter.createProxy(Ice.stringToIdentity("test"))
            )
            p._finally()
            print("ok")

            testtypes()
