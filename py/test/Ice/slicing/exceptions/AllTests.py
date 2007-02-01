#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
                self._cond.wait(5.0)
            if self._called:
                self._called = False
                return True
            else:
                return False
        finally:
            self._cond.release()

    def called(self):
        self._cond.acquire()
        self._called = True
        self._cond.notify()
        self._cond.release()

class AMI_Test_baseAsBaseI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, exc):
        try:
            raise exc
        except Test.Base, b:
            test(b.b == "Base.b")
            test(b.ice_name() =="Test::Base")
        except:
            test(False)
        self.called()

class AMI_Test_unknownDerivedAsBaseI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, exc):
        try:
            raise exc
        except Test.Base, b:
            test(b.b == "UnknownDerived.b")
            test(b.ice_name() =="Test::Base")
        except:
            test(False)
        self.called()

class AMI_Test_knownDerivedAsBaseI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, exc):
        try:
            raise exc
        except Test.KnownDerived, k:
            test(k.b == "KnownDerived.b")
            test(k.kd == "KnownDerived.kd")
            test(k.ice_name() =="Test::KnownDerived")
        except:
            test(False)
        self.called()

class AMI_Test_knownDerivedAsKnownDerivedI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, exc):
        try:
            raise exc
        except Test.KnownDerived, k:
            test(k.b == "KnownDerived.b")
            test(k.kd == "KnownDerived.kd")
            test(k.ice_name() =="Test::KnownDerived")
        except:
            test(False)
        self.called()

class AMI_Test_unknownIntermediateAsBaseI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, exc):
        try:
            raise exc
        except Test.Base, b:
            test(b.b == "UnknownIntermediate.b")
            test(b.ice_name() =="Test::Base")
        except:
            test(False)
        self.called()

class AMI_Test_knownIntermediateAsBaseI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, exc):
        try:
            raise exc
        except Test.KnownIntermediate, ki:
            test(ki.b == "KnownIntermediate.b")
            test(ki.ki == "KnownIntermediate.ki")
            test(ki.ice_name() =="Test::KnownIntermediate")
        except:
            test(False)
        self.called()

class AMI_Test_knownMostDerivedAsBaseI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, exc):
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

class AMI_Test_knownIntermediateAsKnownIntermediateI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, exc):
        try:
            raise exc
        except Test.KnownIntermediate, ki:
            test(ki.b == "KnownIntermediate.b")
            test(ki.ki == "KnownIntermediate.ki")
            test(ki.ice_name() =="Test::KnownIntermediate")
        except:
            test(False)
        self.called()

class AMI_Test_knownMostDerivedAsKnownMostDerivedI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, exc):
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

class AMI_Test_knownMostDerivedAsKnownIntermediateI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, exc):
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

class AMI_Test_unknownMostDerived1AsBaseI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, exc):
        try:
            raise exc
        except Test.KnownIntermediate, ki:
            test(ki.b == "UnknownMostDerived1.b")
            test(ki.ki == "UnknownMostDerived1.ki")
            test(ki.ice_name() =="Test::KnownIntermediate")
        except:
            test(False)
        self.called()

class AMI_Test_unknownMostDerived1AsKnownIntermediateI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, exc):
        try:
            raise exc
        except Test.KnownIntermediate, ki:
            test(ki.b == "UnknownMostDerived1.b")
            test(ki.ki == "UnknownMostDerived1.ki")
            test(ki.ice_name() =="Test::KnownIntermediate")
        except:
            test(False)
        self.called()

class AMI_Test_unknownMostDerived2AsBaseI(CallbackBase):
    def ice_response(self):
        test(False)

    def ice_exception(self, exc):
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
    cb = AMI_Test_baseAsBaseI()
    t.baseAsBase_async(cb)
    test(cb.check())
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
    cb = AMI_Test_unknownDerivedAsBaseI()
    t.unknownDerivedAsBase_async(cb)
    test(cb.check())
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
    cb = AMI_Test_knownDerivedAsBaseI()
    t.knownDerivedAsBase_async(cb)
    test(cb.check())
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
    cb = AMI_Test_knownDerivedAsKnownDerivedI()
    t.knownDerivedAsKnownDerived_async(cb)
    test(cb.check())
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
    cb = AMI_Test_unknownIntermediateAsBaseI()
    t.unknownIntermediateAsBase_async(cb)
    test(cb.check())
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
    cb = AMI_Test_knownIntermediateAsBaseI()
    t.knownIntermediateAsBase_async(cb)
    test(cb.check())
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
    cb = AMI_Test_knownMostDerivedAsBaseI()
    t.knownMostDerivedAsBase_async(cb)
    test(cb.check())
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
    cb = AMI_Test_knownIntermediateAsKnownIntermediateI()
    t.knownIntermediateAsKnownIntermediate_async(cb)
    test(cb.check())
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
    cb = AMI_Test_knownMostDerivedAsKnownIntermediateI()
    t.knownMostDerivedAsKnownIntermediate_async(cb)
    test(cb.check())
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
    cb = AMI_Test_knownMostDerivedAsKnownMostDerivedI()
    t.knownMostDerivedAsKnownMostDerived_async(cb)
    test(cb.check())
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
    cb = AMI_Test_unknownMostDerived1AsBaseI()
    t.unknownMostDerived1AsBase_async(cb)
    test(cb.check())
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
    cb = AMI_Test_unknownMostDerived1AsKnownIntermediateI()
    t.unknownMostDerived1AsKnownIntermediate_async(cb)
    test(cb.check())
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
    cb = AMI_Test_unknownMostDerived2AsBaseI()
    t.unknownMostDerived2AsBase_async(cb)
    test(cb.check())
    print "ok"

    return t
