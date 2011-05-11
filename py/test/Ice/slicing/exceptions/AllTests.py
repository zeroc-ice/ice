#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, threading, sys

Ice.loadSlice('Test.ice')
import Test

def test(b):
    if not b:
        raise RuntimeError('test assertion failed')

class CallbackBase:
    def __init__(self):
        self._called = False
        self._cond = threading.Condition()

    def check(self):
        self._cond.acquire()
        try:
            while not self._called:
                self._cond.wait()
            self._called = False
        finally:
            self._cond.release()

    def called(self):
        self._cond.acquire()
        self._called = True
        self._cond.notify()
        self._cond.release()

class Callback(CallbackBase):
    def response(self):
        test(False)

    def exception_baseAsBase(self, exc):
        try:
            raise exc
        except Test.Base, b:
            test(b.b == "Base.b")
            test(b.ice_name() =="Test::Base")
        except:
            test(False)
        self.called()

    def exception_unknownDerivedAsBase(self, exc):
        try:
            raise exc
        except Test.Base, b:
            test(b.b == "UnknownDerived.b")
            test(b.ice_name() =="Test::Base")
        except:
            test(False)
        self.called()

    def exception_knownDerivedAsBase(self, exc):
        try:
            raise exc
        except Test.KnownDerived, k:
            test(k.b == "KnownDerived.b")
            test(k.kd == "KnownDerived.kd")
            test(k.ice_name() =="Test::KnownDerived")
        except:
            test(False)
        self.called()

    def exception_knownDerivedAsKnownDerived(self, exc):
        try:
            raise exc
        except Test.KnownDerived, k:
            test(k.b == "KnownDerived.b")
            test(k.kd == "KnownDerived.kd")
            test(k.ice_name() =="Test::KnownDerived")
        except:
            test(False)
        self.called()

    def exception_unknownIntermediateAsBase(self, exc):
        try:
            raise exc
        except Test.Base, b:
            test(b.b == "UnknownIntermediate.b")
            test(b.ice_name() =="Test::Base")
        except:
            test(False)
        self.called()

    def exception_knownIntermediateAsBase(self, exc):
        try:
            raise exc
        except Test.KnownIntermediate, ki:
            test(ki.b == "KnownIntermediate.b")
            test(ki.ki == "KnownIntermediate.ki")
            test(ki.ice_name() =="Test::KnownIntermediate")
        except:
            test(False)
        self.called()

    def exception_knownMostDerivedAsBase(self, exc):
        try:
            raise exc
        except Test.KnownMostDerived, kmd:
            test(kmd.b == "KnownMostDerived.b")
            test(kmd.ki == "KnownMostDerived.ki")
            test(kmd.kmd == "KnownMostDerived.kmd")
            test(kmd.ice_name() =="Test::KnownMostDerived")
        except:
            test(False)
        self.called()

    def exception_knownIntermediateAsKnownIntermediate(self, exc):
        try:
            raise exc
        except Test.KnownIntermediate, ki:
            test(ki.b == "KnownIntermediate.b")
            test(ki.ki == "KnownIntermediate.ki")
            test(ki.ice_name() =="Test::KnownIntermediate")
        except:
            test(False)
        self.called()

    def exception_knownMostDerivedAsKnownMostDerived(self, exc):
        try:
            raise exc
        except Test.KnownMostDerived, kmd:
            test(kmd.b == "KnownMostDerived.b")
            test(kmd.ki == "KnownMostDerived.ki")
            test(kmd.kmd == "KnownMostDerived.kmd")
            test(kmd.ice_name() =="Test::KnownMostDerived")
        except:
            test(False)
        self.called()

    def exception_knownMostDerivedAsKnownIntermediate(self, exc):
        try:
            raise exc
        except Test.KnownMostDerived, kmd:
            test(kmd.b == "KnownMostDerived.b")
            test(kmd.ki == "KnownMostDerived.ki")
            test(kmd.kmd == "KnownMostDerived.kmd")
            test(kmd.ice_name() =="Test::KnownMostDerived")
        except:
            test(False)
        self.called()

    def exception_unknownMostDerived1AsBase(self, exc):
        try:
            raise exc
        except Test.KnownIntermediate, ki:
            test(ki.b == "UnknownMostDerived1.b")
            test(ki.ki == "UnknownMostDerived1.ki")
            test(ki.ice_name() =="Test::KnownIntermediate")
        except:
            test(False)
        self.called()

    def exception_unknownMostDerived1AsKnownIntermediate(self, exc):
        try:
            raise exc
        except Test.KnownIntermediate, ki:
            test(ki.b == "UnknownMostDerived1.b")
            test(ki.ki == "UnknownMostDerived1.ki")
            test(ki.ice_name() =="Test::KnownIntermediate")
        except:
            test(False)
        self.called()

    def exception_unknownMostDerived2AsBase(self, exc):
        try:
            raise exc
        except Test.Base, b:
            test(b.b == "UnknownMostDerived2.b")
            test(b.ice_name() =="Test::Base")
        except:
            test(False)
        self.called()

def allTests(communicator):
    obj = communicator.stringToProxy("Test:default -p 12010")
    t = Test.TestIntfPrx.checkedCast(obj)

    print "base... ",
    try:
        t.baseAsBase()
        test(false)
    except Test.Base, b:
        test(b.b == "Base.b")
        test(b.ice_name() == "Test::Base")
    except:
        test(False)
    print "ok"

    print "base (AMI)... ",
    cb = Callback()
    t.begin_baseAsBase(cb.response, cb.exception_baseAsBase)
    cb.check()
    print "ok"

    print "slicing of unknown derived... ",
    try:
        t.unknownDerivedAsBase()
        test(false)
    except Test.Base, b:
        test(b.b == "UnknownDerived.b")
        test(b.ice_name() == "Test::Base")
    except:
        test(False)
    print "ok"

    print "slicing of unknown derived (AMI)... ",
    cb = Callback()
    t.begin_unknownDerivedAsBase(cb.response, cb.exception_unknownDerivedAsBase)
    cb.check()
    print "ok"

    print "non-slicing of known derived as base... ",
    try:
        t.knownDerivedAsBase()
        test(false)
    except Test.KnownDerived, k:
        test(k.b == "KnownDerived.b")
        test(k.kd == "KnownDerived.kd")
        test(k.ice_name() == "Test::KnownDerived")
    except:
        test(False)
    print "ok"

    print "non-slicing of known derived as base (AMI)... ",
    cb = Callback()
    t.begin_knownDerivedAsBase(cb.response, cb.exception_knownDerivedAsBase)
    cb.check()
    print "ok"

    print "non-slicing of known derived as derived... ",
    try:
        t.knownDerivedAsKnownDerived()
        test(false)
    except Test.KnownDerived, k:
        test(k.b == "KnownDerived.b")
        test(k.kd == "KnownDerived.kd")
        test(k.ice_name() == "Test::KnownDerived")
    except:
        test(False)
    print "ok"

    print "non-slicing of known derived as derived (AMI)... ",
    cb = Callback()
    t.begin_knownDerivedAsKnownDerived(cb.response, cb.exception_knownDerivedAsKnownDerived)
    cb.check()
    print "ok"

    print "slicing of unknown intermediate as base... ",
    try:
        t.unknownIntermediateAsBase()
        test(false)
    except Test.Base, b:
        test(b.b == "UnknownIntermediate.b")
        test(b.ice_name() == "Test::Base")
    except:
        test(False)
    print "ok"

    print "slicing of unknown intermediate as base (AMI)... ",
    cb = Callback()
    t.begin_unknownIntermediateAsBase(cb.response, cb.exception_unknownIntermediateAsBase)
    cb.check()
    print "ok"

    print "slicing of known intermediate as base... ",
    try:
        t.knownIntermediateAsBase()
        test(false)
    except Test.KnownIntermediate, ki:
        test(ki.b == "KnownIntermediate.b")
        test(ki.ki == "KnownIntermediate.ki")
        test(ki.ice_name() == "Test::KnownIntermediate")
    except:
        test(False)
    print "ok"

    print "slicing of known intermediate as base (AMI)... ",
    cb = Callback()
    t.begin_knownIntermediateAsBase(cb.response, cb.exception_knownIntermediateAsBase)
    cb.check()
    print "ok"

    print "slicing of known most derived as base... ",
    try:
        t.knownMostDerivedAsBase()
        test(false)
    except Test.KnownMostDerived, kmd:
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_name() == "Test::KnownMostDerived")
    except:
        test(False)
    print "ok"

    print "slicing of known most derived as base (AMI)... ",
    cb = Callback()
    t.begin_knownMostDerivedAsBase(cb.response, cb.exception_knownMostDerivedAsBase)
    cb.check()
    print "ok"

    print "non-slicing of known intermediate as intermediate... ",
    try:
        t.knownIntermediateAsKnownIntermediate()
        test(false)
    except Test.KnownIntermediate, ki:
        test(ki.b == "KnownIntermediate.b")
        test(ki.ki == "KnownIntermediate.ki")
        test(ki.ice_name() == "Test::KnownIntermediate")
    except:
        test(False)
    print "ok"

    print "non-slicing of known intermediate as intermediate (AMI)... ",
    cb = Callback()
    t.begin_knownIntermediateAsKnownIntermediate(cb.response, cb.exception_knownIntermediateAsKnownIntermediate)
    cb.check()
    print "ok"

    print "non-slicing of known most derived exception as intermediate... ",
    try:
        t.knownMostDerivedAsKnownIntermediate()
        test(false)
    except Test.KnownMostDerived, kmd:
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_name() == "Test::KnownMostDerived")
    except:
        test(False)
    print "ok"

    print "non-slicing of known most derived as intermediate (AMI)... ",
    cb = Callback()
    t.begin_knownMostDerivedAsKnownIntermediate(cb.response, cb.exception_knownMostDerivedAsKnownIntermediate)
    cb.check()
    print "ok"

    print "non-slicing of known most derived as most derived... ",
    try:
        t.knownMostDerivedAsKnownMostDerived()
        test(false)
    except Test.KnownMostDerived, kmd:
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_name() == "Test::KnownMostDerived")
    except:
        test(False)
    print "ok"

    print "non-slicing of known most derived as most derived (AMI)... ",
    cb = Callback()
    t.begin_knownMostDerivedAsKnownMostDerived(cb.response, cb.exception_knownMostDerivedAsKnownMostDerived)
    cb.check()
    print "ok"

    print "slicing of unknown most derived, known intermediate as base... ",
    try:
        t.unknownMostDerived1AsBase()
        test(false)
    except Test.KnownIntermediate, ki:
        test(ki.b == "UnknownMostDerived1.b")
        test(ki.ki == "UnknownMostDerived1.ki")
        test(ki.ice_name() == "Test::KnownIntermediate")
    except:
        test(False)
    print "ok"

    print "slicing of unknown most derived, known intermediate as base (AMI)... ",
    cb = Callback()
    t.begin_unknownMostDerived1AsBase(cb.response, cb.exception_unknownMostDerived1AsBase)
    cb.check()
    print "ok"

    print "slicing of unknown most derived, known intermediate as intermediate... ",
    try:
        t.unknownMostDerived1AsKnownIntermediate()
        test(false)
    except Test.KnownIntermediate, ki:
        test(ki.b == "UnknownMostDerived1.b")
        test(ki.ki == "UnknownMostDerived1.ki")
        test(ki.ice_name() == "Test::KnownIntermediate")
    except:
        test(False)
    print "ok"

    print "slicing of unknown most derived, known intermediate as intermediate (AMI)... ",
    cb = Callback()
    t.begin_unknownMostDerived1AsKnownIntermediate(cb.response, cb.exception_unknownMostDerived1AsKnownIntermediate)
    cb.check()
    print "ok"

    print "slicing of unknown most derived, unknown intermediate as base... ",
    try:
        t.unknownMostDerived2AsBase()
        test(false)
    except Test.Base, b:
        test(b.b == "UnknownMostDerived2.b")
        test(b.ice_name() == "Test::Base")
    except:
        test(False)
    print "ok"

    print "slicing of unknown most derived, unknown intermediate as base (AMI)... ",
    cb = Callback()
    t.begin_unknownMostDerived2AsBase(cb.response, cb.exception_unknownMostDerived2AsBase)
    cb.check()
    print "ok"

    return t
