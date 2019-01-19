#!/usr/bin/env python
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from TestHelper import TestHelper
TestHelper.loadSlice("Key.ice Clash.ice")
import sys
import Ice
import _and


class delI(_and._del):
    def _elifAsync(self, _else, current=None):
        pass


class execI(_and._exec):
    def _finally(self, current=None):
        assert current.operation == "finally"


class ifI(_and._if):
    def _elifAsync(self, _else, current=None):
        pass

    def _finally(self, current=None):
        pass

    def foo(self, _from, current=None):
        pass


class printI(_and._print):
    def _raise(self, _else, _return, _try, _while, _yield, _lambda, _or, _global):
        pass


def testtypes():
    sys.stdout.write("Testing generated type names... ")
    sys.stdout.flush()
    a = _and._assert._break
    b = _and._continue
    b._def = 0
    c = _and.delPrx.uncheckedCast(None)
    assert "_elif" in dir(_and.delPrx)
    c1 = delI()
    d = _and.execPrx.uncheckedCast(None)
    assert "_finally" in dir(_and.execPrx)
    d1 = execI()

    e1 = _and._for()
    f = _and.ifPrx.uncheckedCast(None)

    assert "_finally" in dir(_and.ifPrx)
    assert "_elif" in dir(_and.ifPrx)
    f1 = ifI()
    g =  _and._is()
    g._lamba = 0
    h = _and._not()
    h._lamba = 0
    h._or = 1
    h._pass = 2
    i = printI()
    j = _and._lambda;
    en = _and.EnumNone._None
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
            p = _and.execPrx.uncheckedCast(adapter.createProxy(Ice.stringToIdentity("test")))
            p._finally()
            print("ok")

            testtypes()
