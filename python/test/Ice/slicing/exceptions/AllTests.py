#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import threading
import sys
import Test


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


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
        except Exception:
            test(False)
        self.called()

    def exception_unknownDerivedAsBase(self, f):
        try:
            f.result()
            test(False)
        except Test.Base as b:
            test(b.b == "UnknownDerived.b")
            test(b.ice_id() == "::Test::Base")
        except Exception:
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
        except Exception:
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
        except Exception:
            test(False)
        self.called()

    def exception_unknownIntermediateAsBase(self, f):
        try:
            f.result()
            test(False)
        except Test.Base as b:
            test(b.b == "UnknownIntermediate.b")
            test(b.ice_id() == "::Test::Base")
        except Exception:
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
        except Exception:
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
        except Exception:
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
        except Exception:
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
        except Exception:
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
        except Exception:
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
        except Exception:
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
        except Exception:
            test(False)
        self.called()

    def exception_unknownMostDerived2AsBase(self, f):
        try:
            f.result()
            test(False)
        except Test.Base as b:
            test(b.b == "UnknownMostDerived2.b")
            test(b.ice_id() == "::Test::Base")
        except Exception:
            test(False)
        self.called()


def allTests(helper, communicator):
    t = Test.TestIntfPrx(communicator, f"Test:{helper.getTestEndpoint()}")

    sys.stdout.write("base... ")
    sys.stdout.flush()
    try:
        t.baseAsBase()
        test(False)
    except Test.Base as b:
        test(b.b == "Base.b")
        test(b.ice_id() == "::Test::Base")
    except Exception:
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
        test(False)
    except Test.Base as b:
        test(b.b == "UnknownDerived.b")
        test(b.ice_id() == "::Test::Base")
    except Exception:
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
        test(False)
    except Test.KnownDerived as k:
        test(k.b == "KnownDerived.b")
        test(k.kd == "KnownDerived.kd")
        test(k.ice_id() == "::Test::KnownDerived")
    except Exception:
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
        test(False)
    except Test.KnownDerived as k:
        test(k.b == "KnownDerived.b")
        test(k.kd == "KnownDerived.kd")
        test(k.ice_id() == "::Test::KnownDerived")
    except Exception:
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
        test(False)
    except Test.Base as b:
        test(b.b == "UnknownIntermediate.b")
        test(b.ice_id() == "::Test::Base")
    except Exception:
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
        test(False)
    except Test.KnownIntermediate as ki:
        test(ki.b == "KnownIntermediate.b")
        test(ki.ki == "KnownIntermediate.ki")
        test(ki.ice_id() == "::Test::KnownIntermediate")
    except Exception:
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
        test(False)
    except Test.KnownMostDerived as kmd:
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_id() == "::Test::KnownMostDerived")
    except Exception:
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
        test(False)
    except Test.KnownIntermediate as ki:
        test(ki.b == "KnownIntermediate.b")
        test(ki.ki == "KnownIntermediate.ki")
        test(ki.ice_id() == "::Test::KnownIntermediate")
    except Exception:
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
        test(False)
    except Test.KnownMostDerived as kmd:
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_id() == "::Test::KnownMostDerived")
    except Exception:
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
        test(False)
    except Test.KnownMostDerived as kmd:
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_id() == "::Test::KnownMostDerived")
    except Exception:
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
        test(False)
    except Test.KnownIntermediate as ki:
        test(ki.b == "UnknownMostDerived1.b")
        test(ki.ki == "UnknownMostDerived1.ki")
        test(ki.ice_id() == "::Test::KnownIntermediate")
    except Exception:
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
        test(False)
    except Test.KnownIntermediate as ki:
        test(ki.b == "UnknownMostDerived1.b")
        test(ki.ki == "UnknownMostDerived1.ki")
        test(ki.ice_id() == "::Test::KnownIntermediate")
    except Exception:
        test(False)
    print("ok")

    sys.stdout.write("slicing of unknown most derived, known intermediate as intermediate (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.unknownMostDerived1AsKnownIntermediateAsync().add_done_callback(
        cb.exception_unknownMostDerived1AsKnownIntermediate
    )
    cb.check()
    print("ok")

    sys.stdout.write("slicing of unknown most derived, unknown intermediate as base... ")
    sys.stdout.flush()
    try:
        t.unknownMostDerived2AsBase()
        test(False)
    except Test.Base as b:
        test(b.b == "UnknownMostDerived2.b")
        test(b.ice_id() == "::Test::Base")
    except Exception:
        test(False)
    print("ok")

    sys.stdout.write("slicing of unknown most derived, unknown intermediate as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    t.unknownMostDerived2AsBaseAsync().add_done_callback(cb.exception_unknownMostDerived2AsBase)
    cb.check()
    print("ok")

    return t
