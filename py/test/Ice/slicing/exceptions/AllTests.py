# **********************************************************************
#
# Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
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
                return True;
            else:
                return False
        finally:
            self._cond.release()

    def called(self):
        self._cond.acquire()
        _called = True
        self._cond.notify()
        self._cond.release()

def allTests(communicator):
    obj = communicator.stringToProxy("Test:default -p 12345")
    t = Test.TestIntfPrx.checkedCast(obj)

    print "base... ",
    gotException = False
    try:
        t.baseAsBase()
    except Test.Base, b:
        test(b.b == "Base.b")
        test(b.ice_id() == "::Test::Base")
        gotException = True
    except:
        test(False)
    test(gotException)
    print "ok"

    #print "base (AMI)... ",
    #cb = new AMI_Test_baseAsBaseI
    #t.baseAsBase_async(cb)
    #test(cb.check())
    #print "ok"

    print "slicing of unknown derived... ",
    gotException = False
    try:
        t.unknownDerivedAsBase()
    except Test.Base, b:
        test(b.b == "UnknownDerived.b")
        test(b.ice_id() == "::Test::Base")
        gotException = True
    except:
        test(False)
    test(gotException)
    print "ok"

    #print "slicing of unknown derived (AMI)... ",
    #cb = AMI_Test_unknownDerivedAsBaseI()
    #t.unknownDerivedAsBase_async(cb)
    #test(cb.check())
    #print "ok"

    print "non-slicing of known derived as base... ",
    gotException = False
    try:
        t.knownDerivedAsBase()
    except Test.KnownDerived, k:
        test(k.b == "KnownDerived.b")
        test(k.kd == "KnownDerived.kd")
        test(k.ice_id() == "::Test::KnownDerived")
        gotException = True
    except:
        test(False)
    test(gotException)
    print "ok"

    #print "non-slicing of known derived as base (AMI)... ",
    #cb = AMI_Test_knownDerivedAsBaseI()
    #t.knownDerivedAsBase_async(cb)
    #test(cb.check())
    #print "ok"

    print "non-slicing of known derived as derived... ",
    gotException = False
    try:
        t.knownDerivedAsKnownDerived()
    except Test.KnownDerived, k:
        test(k.b == "KnownDerived.b")
        test(k.kd == "KnownDerived.kd")
        test(k.ice_id() == "::Test::KnownDerived")
        gotException = True
    except:
        test(False)
    test(gotException)
    print "ok"

    #print "non-slicing of known derived as derived (AMI)... ",
    #cb = AMI_Test_knownDerivedAsKnownDerivedI()
    #t.knownDerivedAsKnownDerived_async(cb)
    #test(cb.check())
    #print "ok"

    print "slicing of unknown intermediate as base... ",
    gotException = False
    try:
        t.unknownIntermediateAsBase()
    except Test.Base, b:
        test(b.b == "UnknownIntermediate.b")
        test(b.ice_id() == "::Test::Base")
        gotException = True
    except:
        test(False)
    test(gotException)
    print "ok"

    #print "slicing of unknown intermediate as base (AMI)... ",
    #cb = AMI_Test_unknownIntermediateAsBaseI()
    #t.unknownIntermediateAsBase_async(cb)
    #test(cb.check())
    #print "ok"

    print "slicing of known intermediate as base... ",
    gotException = False
    try:
        t.knownIntermediateAsBase()
    except Test.KnownIntermediate, ki:
        test(ki.b == "KnownIntermediate.b")
        test(ki.ki == "KnownIntermediate.ki")
        test(ki.ice_id() == "::Test::KnownIntermediate")
        gotException = True
    except:
        test(False)
    test(gotException)
    print "ok"

    #print "slicing of known intermediate as base (AMI)... ",
    #cb = AMI_Test_knownIntermediateAsBaseI()
    #t.knownIntermediateAsBase_async(cb)
    #test(cb.check())
    #print "ok"

    print "slicing of known most derived as base... ",
    gotException = False
    try:
        t.knownMostDerivedAsBase()
    except Test.KnownMostDerived, kmd:
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_id() == "::Test::KnownMostDerived")
        gotException = True
    except:
        test(False)
    test(gotException)
    print "ok"

    #print "slicing of known most derived as base (AMI)... ",
    #cb = AMI_Test_knownMostDerivedAsBaseI()
    #t.knownMostDerivedAsBase_async(cb)
    #test(cb.check())
    #print "ok"

    print "non-slicing of known intermediate as intermediate... ",
    gotException = False
    try:
        t.knownIntermediateAsKnownIntermediate()
    except Test.KnownIntermediate, ki:
        test(ki.b == "KnownIntermediate.b")
        test(ki.ki == "KnownIntermediate.ki")
        test(ki.ice_id() == "::Test::KnownIntermediate")
        gotException = True
    except:
        test(False)
    test(gotException)
    print "ok"

    #print "non-slicing of known intermediate as intermediate (AMI)... ",
    #cb = AMI_Test_knownIntermediateAsKnownIntermediateI()
    #t.knownIntermediateAsKnownIntermediate_async(cb)
    #test(cb.check())
    #print "ok"

    print "non-slicing of known most derived exception as intermediate... ",
    gotException = False
    try:
        t.knownMostDerivedAsKnownIntermediate()
    except Test.KnownMostDerived, kmd:
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_id() == "::Test::KnownMostDerived")
        gotException = True
    except:
        test(False)
    test(gotException)
    print "ok"

    #print "non-slicing of known most derived as intermediate (AMI)... ",
    #cb = AMI_Test_knownMostDerivedAsKnownIntermediateI()
    #t.knownMostDerivedAsKnownIntermediate_async(cb)
    #test(cb.check())
    #print "ok"

    print "non-slicing of known most derived as most derived... ",
    gotException = False
    try:
        t.knownMostDerivedAsKnownMostDerived()
    except Test.KnownMostDerived, kmd:
        test(kmd.b == "KnownMostDerived.b")
        test(kmd.ki == "KnownMostDerived.ki")
        test(kmd.kmd == "KnownMostDerived.kmd")
        test(kmd.ice_id() == "::Test::KnownMostDerived")
        gotException = True
    except:
        test(False)
    test(gotException)
    print "ok"

    #print "non-slicing of known most derived as most derived (AMI)... ",
    #cb = AMI_Test_knownMostDerivedAsKnownMostDerivedI()
    #t.knownMostDerivedAsKnownMostDerived_async(cb)
    #test(cb.check())
    #print "ok"

    print "slicing of unknown most derived, known intermediate as base... ",
    gotException = False
    try:
        t.unknownMostDerived1AsBase()
    except Test.KnownIntermediate, ki:
        test(ki.b == "UnknownMostDerived1.b")
        test(ki.ki == "UnknownMostDerived1.ki")
        test(ki.ice_id() == "::Test::KnownIntermediate")
        gotException = True
    except:
        test(False)
    test(gotException)
    print "ok"

    #print "slicing of unknown most derived, known intermediate as base (AMI)... ",
    #cb = AMI_Test_unknownMostDerived1AsBaseI()
    #t.unknownMostDerived1AsBase_async(cb)
    #test(cb.check())
    #print "ok"

    print "slicing of unknown most derived, known intermediate as intermediate... ",
    gotException = False
    try:
        t.unknownMostDerived1AsKnownIntermediate()
    except Test.KnownIntermediate, ki:
        test(ki.b == "UnknownMostDerived1.b")
        test(ki.ki == "UnknownMostDerived1.ki")
        test(ki.ice_id() == "::Test::KnownIntermediate")
        gotException = True
    except:
        test(False)
    test(gotException)
    print "ok"

    #print "slicing of unknown most derived, known intermediate as intermediate (AMI)... ",
    #cb = AMI_Test_unknownMostDerived1AsKnownIntermediateI()
    #t.unknownMostDerived1AsKnownIntermediate_async(cb)
    #test(cb.check())
    #print "ok"

    print "slicing of unknown most derived, unknown intermediate as base... ",
    gotException = False
    try:
        t.unknownMostDerived2AsBase()
    except Test.Base, b:
        test(b.b == "UnknownMostDerived2.b")
        test(b.ice_id() == "::Test::Base")
        gotException = True
    except:
        test(False)
    test(gotException)
    print "ok"

    #print "slicing of unknown most derived, unknown intermediate as base (AMI)... ",
    #cb = AMI_Test_unknownMostDerived2AsBaseI()
    #t.unknownMostDerived2AsBase_async(cb)
    #test(cb.check())
    #print "ok"

    return t
