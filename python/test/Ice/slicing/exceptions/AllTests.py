#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
#
# This copy of Ice is licensed to you under the terms described in the
# ICE_LICENSE file included in this distribution.
#
# **********************************************************************

import Ice, threading, sys

Ice.loadSlice('-I. --all ClientPrivate.ice')
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
        except Test.Base as b:
            test(b.b == "Base.b")
            test(b.ice_name() =="Test::Base")
        except:
            test(False)
        self.called()

    def exception_unknownDerivedAsBase(self, exc):
        try:
            raise exc
        except Test.Base as b:
            test(b.b == "UnknownDerived.b")
            test(b.ice_name() =="Test::Base")
        except:
            test(False)
        self.called()

    def exception_knownDerivedAsBase(self, exc):
        try:
            raise exc
        except Test.KnownDerived as k:
            test(k.b == "KnownDerived.b")
            test(k.kd == "KnownDerived.kd")
            test(k.ice_name() =="Test::KnownDerived")
        except:
            test(False)
        self.called()

    def exception_knownDerivedAsKnownDerived(self, exc):
        try:
            raise exc
        except Test.KnownDerived as k:
            test(k.b == "KnownDerived.b")
            test(k.kd == "KnownDerived.kd")
            test(k.ice_name() =="Test::KnownDerived")
        except:
            test(False)
        self.called()

    def exception_unknownIntermediateAsBase(self, exc):
        try:
            raise exc
        except Test.Base as b:
            test(b.b == "UnknownIntermediate.b")
            test(b.ice_name() =="Test::Base")
        except:
            test(False)
        self.called()

    def exception_knownIntermediateAsBase(self, exc):
        try:
            raise exc
        except Test.KnownIntermediate as ki:
            test(ki.b == "KnownIntermediate.b")
            test(ki.ki == "KnownIntermediate.ki")
            test(ki.ice_name() =="Test::KnownIntermediate")
        except:
            test(False)
        self.called()

    def exception_knownMostDerivedAsBase(self, exc):
        try:
            raise exc
        except Test.KnownMostDerived as kmd:
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
        except Test.KnownIntermediate as ki:
            test(ki.b == "KnownIntermediate.b")
            test(ki.ki == "KnownIntermediate.ki")
            test(ki.ice_name() =="Test::KnownIntermediate")
        except:
            test(False)
        self.called()

    def exception_knownMostDerivedAsKnownMostDerived(self, exc):
        try:
            raise exc
        except Test.KnownMostDerived as kmd:
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
        except Test.KnownMostDerived as kmd:
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
        except Test.KnownIntermediate as ki:
            test(ki.b == "UnknownMostDerived1.b")
            test(ki.ki == "UnknownMostDerived1.ki")
            test(ki.ice_name() =="Test::KnownIntermediate")
        except:
            test(False)
        self.called()

    def exception_unknownMostDerived1AsKnownIntermediate(self, exc):
        try:
            raise exc
        except Test.KnownIntermediate as ki:
            test(ki.b == "UnknownMostDerived1.b")
            test(ki.ki == "UnknownMostDerived1.ki")
            test(ki.ice_name() =="Test::KnownIntermediate")
        except:
            test(False)
        self.called()

    def exception_unknownMostDerived2AsBase(self, exc):
        try:
            raise exc
        except Test.Base as b:
            test(b.b == "UnknownMostDerived2.b")
            test(b.ice_name() =="Test::Base")
        except:
            test(False)
        self.called()

class RelayI(Test.Relay):
    def knownPreservedAsBase(self, current=None):
        ex = Test.KnownPreservedDerived()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        raise ex

    def knownPreservedAsKnownPreserved(self, current=None):
        ex = Test.KnownPreservedDerived()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        raise ex

    def unknownPreservedAsBase(self, current=None):
        ex = Test.Preserved2()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        ex.p1 = Test.PreservedClass("bc", "pc")
        ex.p2 = ex.p1
        raise ex

    def unknownPreservedAsKnownPreserved(self, current=None):
        ex = Test.Preserved2()
        ex.b = "base"
        ex.kp = "preserved"
        ex.kpd = "derived"
        ex.p1 = Test.PreservedClass("bc", "pc")
        ex.p2 = ex.p1
        raise ex

def allTests(communicator):
    obj = communicator.stringToProxy("Test:default -p 12010")
    t = Test.TestIntfPrx.checkedCast(obj)

    sys.stdout.write("base... ")
    sys.stdout.flush()
    try:
        t.baseAsBase()
        test(false)
    except Test.Base as b:
        test(b.b == "Base.b")
        test(b.ice_name() == "Test::Base")
    except:
        test(False)
    print("ok")

    sys.stdout.write("base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.begin_baseAsBase(cb.response, cb.exception_baseAsBase)
    cb.check()
    print("ok")

    sys.stdout.write("slicing of unknown derived... ")
    sys.stdout.flush()
    try:
        t.unknownDerivedAsBase()
        test(false)
    except Test.Base as b:
        test(b.b == "UnknownDerived.b")
        test(b.ice_name() == "Test::Base")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of unknown derived (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.begin_unknownDerivedAsBase(cb.response, cb.exception_unknownDerivedAsBase)
    cb.check()
    print("ok")

    sys.stdout.write("non-slicing of known derived as base... ")
    sys.stdout.flush()
    try:
        t.knownDerivedAsBase()
        test(false)
    except Test.KnownDerived as k:
        test(k.b == "KnownDerived.b")
        test(k.kd == "KnownDerived.kd")
        test(k.ice_name() == "Test::KnownDerived")
    except:
        test(False)
    print("ok")

    sys.stdout.write("non-slicing of known derived as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.begin_knownDerivedAsBase(cb.response, cb.exception_knownDerivedAsBase)
    cb.check()
    print("ok")

    sys.stdout.write("non-slicing of known derived as derived... ")
    sys.stdout.flush()
    try:
        t.knownDerivedAsKnownDerived()
        test(false)
    except Test.KnownDerived as k:
        test(k.b == "KnownDerived.b")
        test(k.kd == "KnownDerived.kd")
        test(k.ice_name() == "Test::KnownDerived")
    except:
        test(False)
    print("ok")

    sys.stdout.write("non-slicing of known derived as derived (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.begin_knownDerivedAsKnownDerived(cb.response, cb.exception_knownDerivedAsKnownDerived)
    cb.check()
    print("ok")

    sys.stdout.write("slicing of unknown intermediate as base... ")
    sys.stdout.flush()
    try:
        t.unknownIntermediateAsBase()
        test(false)
    except Test.Base as b:
        test(b.b == "UnknownIntermediate.b")
        test(b.ice_name() == "Test::Base")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of unknown intermediate as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.begin_unknownIntermediateAsBase(cb.response, cb.exception_unknownIntermediateAsBase)
    cb.check()
    print("ok")

    sys.stdout.write("slicing of known intermediate as base... ")
    sys.stdout.flush()
    try:
        t.knownIntermediateAsBase()
        test(false)
    except Test.KnownIntermediate as ki:
        test(ki.b == "KnownIntermediate.b")
        test(ki.ki == "KnownIntermediate.ki")
        test(ki.ice_name() == "Test::KnownIntermediate")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of known intermediate as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.begin_knownIntermediateAsBase(cb.response, cb.exception_knownIntermediateAsBase)
    cb.check()
    print("ok")

    sys.stdout.write("slicing of known most derived as base... ")
    sys.stdout.flush()
    try:
        t.knownMostDerivedAsBase()
        test(false)
    except Test.KnownMostDerived as kmd:
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_name() == "Test::KnownMostDerived")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of known most derived as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.begin_knownMostDerivedAsBase(cb.response, cb.exception_knownMostDerivedAsBase)
    cb.check()
    print("ok")

    sys.stdout.write("non-slicing of known intermediate as intermediate... ")
    sys.stdout.flush()
    try:
        t.knownIntermediateAsKnownIntermediate()
        test(false)
    except Test.KnownIntermediate as ki:
        test(ki.b == "KnownIntermediate.b")
        test(ki.ki == "KnownIntermediate.ki")
        test(ki.ice_name() == "Test::KnownIntermediate")
    except:
        test(False)
    print("ok")

    sys.stdout.write("non-slicing of known intermediate as intermediate (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.begin_knownIntermediateAsKnownIntermediate(cb.response, cb.exception_knownIntermediateAsKnownIntermediate)
    cb.check()
    print("ok")

    sys.stdout.write("non-slicing of known most derived exception as intermediate... ")
    sys.stdout.flush()
    try:
        t.knownMostDerivedAsKnownIntermediate()
        test(false)
    except Test.KnownMostDerived as kmd:
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_name() == "Test::KnownMostDerived")
    except:
        test(False)
    print("ok")

    sys.stdout.write("non-slicing of known most derived as intermediate (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.begin_knownMostDerivedAsKnownIntermediate(cb.response, cb.exception_knownMostDerivedAsKnownIntermediate)
    cb.check()
    print("ok")

    sys.stdout.write("non-slicing of known most derived as most derived... ")
    sys.stdout.flush()
    try:
        t.knownMostDerivedAsKnownMostDerived()
        test(false)
    except Test.KnownMostDerived as kmd:
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_name() == "Test::KnownMostDerived")
    except:
        test(False)
    print("ok")

    sys.stdout.write("non-slicing of known most derived as most derived (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.begin_knownMostDerivedAsKnownMostDerived(cb.response, cb.exception_knownMostDerivedAsKnownMostDerived)
    cb.check()
    print("ok")

    sys.stdout.write("slicing of unknown most derived, known intermediate as base... ")
    sys.stdout.flush()
    try:
        t.unknownMostDerived1AsBase()
        test(false)
    except Test.KnownIntermediate as ki:
        test(ki.b == "UnknownMostDerived1.b")
        test(ki.ki == "UnknownMostDerived1.ki")
        test(ki.ice_name() == "Test::KnownIntermediate")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of unknown most derived, known intermediate as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.begin_unknownMostDerived1AsBase(cb.response, cb.exception_unknownMostDerived1AsBase)
    cb.check()
    print("ok")

    sys.stdout.write("slicing of unknown most derived, known intermediate as intermediate... ")
    sys.stdout.flush()
    try:
        t.unknownMostDerived1AsKnownIntermediate()
        test(false)
    except Test.KnownIntermediate as ki:
        test(ki.b == "UnknownMostDerived1.b")
        test(ki.ki == "UnknownMostDerived1.ki")
        test(ki.ice_name() == "Test::KnownIntermediate")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of unknown most derived, known intermediate as intermediate (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.begin_unknownMostDerived1AsKnownIntermediate(cb.response, cb.exception_unknownMostDerived1AsKnownIntermediate)
    cb.check()
    print("ok")

    sys.stdout.write("slicing of unknown most derived, unknown intermediate as base... ")
    sys.stdout.flush()
    try:
        t.unknownMostDerived2AsBase()
        test(false)
    except Test.Base as b:
        test(b.b == "UnknownMostDerived2.b")
        test(b.ice_name() == "Test::Base")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of unknown most derived, unknown intermediate as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.begin_unknownMostDerived2AsBase(cb.response, cb.exception_unknownMostDerived2AsBase)
    cb.check()
    print("ok")

    sys.stdout.write("unknown most derived in compact format... ")
    sys.stdout.flush()
    try:
        t.unknownMostDerived2AsBaseCompact()
        test(False)
    except Test.Base:
        #
        # For the 1.0 encoding, the unknown exception is sliced to Base.
        #
        test(t.ice_getEncodingVersion() == Ice.Encoding_1_0)
    except Ice.UnknownUserException:
        #
        # An UnkownUserException is raised for the compact format because the
        # most-derived type is unknown and the exception cannot be sliced.
        #
        test(t.ice_getEncodingVersion() != Ice.Encoding_1_0)
    except Ice.OperationNotExistException:
        pass
    except:
        test(False)
    print("ok")

    sys.stdout.write("preserved exceptions... ")
    sys.stdout.flush()
    adapter = communicator.createObjectAdapterWithEndpoints("Relay", "default")
    relay = Test.RelayPrx.uncheckedCast(adapter.addWithUUID(RelayI()))
    adapter.activate()

    try:
        t.relayKnownPreservedAsBase(relay)
        test(False)
    except Test.KnownPreservedDerived as ex:
        test(ex.b == "base")
        test(ex.kp == "preserved")
        test(ex.kpd == "derived")
    except Ice.OperationNotExistException:
        pass
    except:
        test(False)

    try:
        t.relayKnownPreservedAsKnownPreserved(relay)
        test(False)
    except Test.KnownPreservedDerived as ex:
        test(ex.b == "base")
        test(ex.kp == "preserved")
        test(ex.kpd == "derived")
    except Ice.OperationNotExistException:
        pass
    except:
        test(False)

    try:
        t.relayUnknownPreservedAsBase(relay)
        test(False)
    except Test.Preserved2 as ex:
        test(ex.b == "base")
        test(ex.kp == "preserved")
        test(ex.kpd == "derived")
        test(ex.p1.ice_id() == Test.PreservedClass.ice_staticId())
        pc = ex.p1
        test(isinstance(pc, Test.PreservedClass))
        test(pc.bc == "bc")
        test(pc.pc == "pc")
        test(ex.p2 == ex.p1)
    except Test.KnownPreservedDerived as ex:
        #
        # For the 1.0 encoding, the unknown exception is sliced to KnownPreserved.
        #
        test(t.ice_getEncodingVersion() == Ice.Encoding_1_0)
        test(ex.b == "base")
        test(ex.kp == "preserved")
        test(ex.kpd == "derived")
    except Ice.OperationNotExistException:
        pass
    except:
        test(False)

    try:
        t.relayUnknownPreservedAsKnownPreserved(relay)
        test(False)
    except Test.Preserved2 as ex:
        test(ex.b == "base")
        test(ex.kp == "preserved")
        test(ex.kpd == "derived")
        test(ex.p1.ice_id() == Test.PreservedClass.ice_staticId())
        pc = ex.p1
        test(isinstance(pc, Test.PreservedClass))
        test(pc.bc == "bc")
        test(pc.pc == "pc")
        test(ex.p2 == ex.p1)
    except Test.KnownPreservedDerived as ex:
        #
        # For the 1.0 encoding, the unknown exception is sliced to KnownPreserved.
        #
        test(t.ice_getEncodingVersion() == Ice.Encoding_1_0)
        test(ex.b == "base")
        test(ex.kp == "preserved")
        test(ex.kpd == "derived")
    except Ice.OperationNotExistException:
        pass
    except:
        test(False)

    adapter.destroy()
    print("ok")

    return t
