#!/usr/bin/env python
# **********************************************************************
#
# Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
        with self._cond:
            while not self._called:
                self._cond.wait()
            self._called = False

    def called(self):
        with self._cond:
            self._called = True
            self._cond.notify()

class Callback(CallbackBase):
    def exception_baseAsBase(self, f):
        try:
            f.result()
            test(False)
        except Test.Base as b:
            test(b.b == "Base.b")
            test(b.ice_id() == "::Test::Base")
        except:
            test(False)
        self.called()

    def exception_unknownDerivedAsBase(self, f):
        try:
            f.result()
            test(False)
        except Test.Base as b:
            test(b.b == "UnknownDerived.b")
            test(b.ice_id() == "::Test::Base")
        except:
            test(False)
        self.called()

    def exception_knownDerivedAsBase(self, f):
        try:
            f.result()
            test(False)
        except Test.KnownDerived as k:
            test(k.b == "KnownDerived.b")
            test(k.kd == "KnownDerived.kd")
            test(k.ice_id() == "::Test::KnownDerived")
        except:
            test(False)
        self.called()

    def exception_knownDerivedAsKnownDerived(self, f):
        try:
            f.result()
            test(False)
        except Test.KnownDerived as k:
            test(k.b == "KnownDerived.b")
            test(k.kd == "KnownDerived.kd")
            test(k.ice_id() == "::Test::KnownDerived")
        except:
            test(False)
        self.called()

    def exception_unknownIntermediateAsBase(self, f):
        try:
            f.result()
            test(False)
        except Test.Base as b:
            test(b.b == "UnknownIntermediate.b")
            test(b.ice_id() == "::Test::Base")
        except:
            test(False)
        self.called()

    def exception_knownIntermediateAsBase(self, f):
        try:
            f.result()
            test(False)
        except Test.KnownIntermediate as ki:
            test(ki.b == "KnownIntermediate.b")
            test(ki.ki == "KnownIntermediate.ki")
            test(ki.ice_id() == "::Test::KnownIntermediate")
        except:
            test(False)
        self.called()

    def exception_knownMostDerivedAsBase(self, f):
        try:
            f.result()
            test(False)
        except Test.KnownMostDerived as kmd:
            test(kmd.b == "KnownMostDerived.b")
            test(kmd.ki == "KnownMostDerived.ki")
            test(kmd.kmd == "KnownMostDerived.kmd")
            test(kmd.ice_id() == "::Test::KnownMostDerived")
        except:
            test(False)
        self.called()

    def exception_knownIntermediateAsKnownIntermediate(self, f):
        try:
            f.result()
            test(False)
        except Test.KnownIntermediate as ki:
            test(ki.b == "KnownIntermediate.b")
            test(ki.ki == "KnownIntermediate.ki")
            test(ki.ice_id() == "::Test::KnownIntermediate")
        except:
            test(False)
        self.called()

    def exception_knownMostDerivedAsKnownMostDerived(self, f):
        try:
            f.result()
            test(False)
        except Test.KnownMostDerived as kmd:
            test(kmd.b == "KnownMostDerived.b")
            test(kmd.ki == "KnownMostDerived.ki")
            test(kmd.kmd == "KnownMostDerived.kmd")
            test(kmd.ice_id() == "::Test::KnownMostDerived")
        except:
            test(False)
        self.called()

    def exception_knownMostDerivedAsKnownIntermediate(self, f):
        try:
            f.result()
            test(False)
        except Test.KnownMostDerived as kmd:
            test(kmd.b == "KnownMostDerived.b")
            test(kmd.ki == "KnownMostDerived.ki")
            test(kmd.kmd == "KnownMostDerived.kmd")
            test(kmd.ice_id() == "::Test::KnownMostDerived")
        except:
            test(False)
        self.called()

    def exception_unknownMostDerived1AsBase(self, f):
        try:
            f.result()
            test(False)
        except Test.KnownIntermediate as ki:
            test(ki.b == "UnknownMostDerived1.b")
            test(ki.ki == "UnknownMostDerived1.ki")
            test(ki.ice_id() == "::Test::KnownIntermediate")
        except:
            test(False)
        self.called()

    def exception_unknownMostDerived1AsKnownIntermediate(self, f):
        try:
            f.result()
            test(False)
        except Test.KnownIntermediate as ki:
            test(ki.b == "UnknownMostDerived1.b")
            test(ki.ki == "UnknownMostDerived1.ki")
            test(ki.ice_id() == "::Test::KnownIntermediate")
        except:
            test(False)
        self.called()

    def exception_unknownMostDerived2AsBase(self, f):
        try:
            f.result()
            test(False)
        except Test.Base as b:
            test(b.b == "UnknownMostDerived2.b")
            test(b.ice_id() == "::Test::Base")
        except:
            test(False)
        self.called()

class RelayI(Test._RelayDisp):
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
        test(b.ice_id() == "::Test::Base")
    except:
        test(False)
    print("ok")

    sys.stdout.write("base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.baseAsBaseAsync().add_done_callback(cb.exception_baseAsBase)
    cb.check()
    print("ok")

    sys.stdout.write("slicing of unknown derived... ")
    sys.stdout.flush()
    try:
        t.unknownDerivedAsBase()
        test(false)
    except Test.Base as b:
        test(b.b == "UnknownDerived.b")
        test(b.ice_id() == "::Test::Base")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of unknown derived (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.unknownDerivedAsBaseAsync().add_done_callback(cb.exception_unknownDerivedAsBase)
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
        test(k.ice_id() == "::Test::KnownDerived")
    except:
        test(False)
    print("ok")

    sys.stdout.write("non-slicing of known derived as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.knownDerivedAsBaseAsync().add_done_callback(cb.exception_knownDerivedAsBase)
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
        test(k.ice_id() == "::Test::KnownDerived")
    except:
        test(False)
    print("ok")

    sys.stdout.write("non-slicing of known derived as derived (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.knownDerivedAsKnownDerivedAsync().add_done_callback(cb.exception_knownDerivedAsKnownDerived)
    cb.check()
    print("ok")

    sys.stdout.write("slicing of unknown intermediate as base... ")
    sys.stdout.flush()
    try:
        t.unknownIntermediateAsBase()
        test(false)
    except Test.Base as b:
        test(b.b == "UnknownIntermediate.b")
        test(b.ice_id() == "::Test::Base")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of unknown intermediate as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.unknownIntermediateAsBaseAsync().add_done_callback(cb.exception_unknownIntermediateAsBase)
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
        test(ki.ice_id() == "::Test::KnownIntermediate")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of known intermediate as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.knownIntermediateAsBaseAsync().add_done_callback(cb.exception_knownIntermediateAsBase)
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
        test(kmd.ice_id() == "::Test::KnownMostDerived")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of known most derived as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.knownMostDerivedAsBaseAsync().add_done_callback(cb.exception_knownMostDerivedAsBase)
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
        test(ki.ice_id() == "::Test::KnownIntermediate")
    except:
        test(False)
    print("ok")

    sys.stdout.write("non-slicing of known intermediate as intermediate (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.knownIntermediateAsKnownIntermediateAsync().add_done_callback(cb.exception_knownIntermediateAsKnownIntermediate)
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
        test(kmd.ice_id() == "::Test::KnownMostDerived")
    except:
        test(False)
    print("ok")

    sys.stdout.write("non-slicing of known most derived as intermediate (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.knownMostDerivedAsKnownIntermediateAsync().add_done_callback(cb.exception_knownMostDerivedAsKnownIntermediate)
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
        test(kmd.ice_id() == "::Test::KnownMostDerived")
    except:
        test(False)
    print("ok")

    sys.stdout.write("non-slicing of known most derived as most derived (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.knownMostDerivedAsKnownMostDerivedAsync().add_done_callback(cb.exception_knownMostDerivedAsKnownMostDerived)
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
        test(ki.ice_id() == "::Test::KnownIntermediate")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of unknown most derived, known intermediate as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.unknownMostDerived1AsBaseAsync().add_done_callback(cb.exception_unknownMostDerived1AsBase)
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
        test(ki.ice_id() == "::Test::KnownIntermediate")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of unknown most derived, known intermediate as intermediate (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.unknownMostDerived1AsKnownIntermediateAsync().add_done_callback(
        cb.exception_unknownMostDerived1AsKnownIntermediate)
    cb.check()
    print("ok")

    sys.stdout.write("slicing of unknown most derived, unknown intermediate as base... ")
    sys.stdout.flush()
    try:
        t.unknownMostDerived2AsBase()
        test(false)
    except Test.Base as b:
        test(b.b == "UnknownMostDerived2.b")
        test(b.ice_id() == "::Test::Base")
    except:
        test(False)
    print("ok")

    sys.stdout.write("slicing of unknown most derived, unknown intermediate as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.unknownMostDerived2AsBaseAsync().add_done_callback(cb.exception_unknownMostDerived2AsBase)
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
