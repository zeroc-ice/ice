#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import os, sys, traceback

for toplevel in [".", "..", "../..", "../../..", "../../../.."]:
    toplevel = os.path.normpath(toplevel)
    if os.path.exists(os.path.join(toplevel, "python", "Ice", "__init__.py")):
        break
else:
    raise RuntimeError("can't find toplevel directory!")

import Ice

Ice.loadSlice('Key.ice')
Ice.loadSlice('Clash.ice')

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

def run(args, communicator):
    communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -p 12010")
    adapter = communicator.createObjectAdapter("TestAdapter")
    adapter.add(execI(), Ice.stringToIdentity("test"))
    adapter.activate()

    sys.stdout.write("Testing operation name... ")
    sys.stdout.flush()
    p = _and.execPrx.uncheckedCast(
        adapter.createProxy(Ice.stringToIdentity("test")));
    p._finally();
    print("ok")

    testtypes()

    return True

try:
    initData = Ice.InitializationData()
    initData.properties = Ice.createProperties(sys.argv)
    #
    # Its possible to have batch oneway requests dispatched after the
    # adapter is deactivated due to thread scheduling so we supress
    # this warning.
    #
    initData.properties.setProperty("Ice.Warn.Dispatch", "0");
    with Ice.initialize(sys.argv, initData) as communicator:
        status = run(sys.argv, communicator)
except:
    traceback.print_exc()
    status = False

sys.exit(not status)
