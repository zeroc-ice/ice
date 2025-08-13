#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import gc
import sys
import threading
from typing import Optional, TypeVar, cast

from generated.test.Ice.slicing.objects import Test
from generated.test.Ice.slicing.objects.client_private import Test as ClientPrivateTest
from TestHelper import TestHelper, test

import Ice

_T = TypeVar("_T")


class CallbackBase:
    def __init__(self) -> None:
        self._called = False
        self._cond = threading.Condition()

    def check(self) -> None:
        with self._cond:
            while not self._called:
                self._cond.wait()
            self._called = False

    def called(self) -> None:
        with self._cond:
            self._called = True
            self._cond.notify()


class Callback[_T](CallbackBase):
    def __init__(self) -> None:
        super().__init__()
        self.r: _T | None = None
        self.bout: dict[int, Test.B] | None = None

    def response_SBaseAsObject(self, f: Ice.Future[Ice.Value]) -> None:
        o = f.result()
        assert o is not None
        test(o.ice_id() == "::Test::SBase")
        sb = o
        assert isinstance(sb, Test.SBase)
        test(sb.sb == "SBase.sb")
        self.called()

    def response_SBaseAsSBase(self, f: Ice.Future[Test.SBase]) -> None:
        sb = f.result()
        test(sb.sb == "SBase.sb")
        self.called()

    def response_SBSKnownDerivedAsSBase(self, f: Ice.Future[Test.SBSKnownDerived]) -> None:
        sbskd = f.result()
        test(isinstance(sbskd, Test.SBSKnownDerived))
        test(sbskd.sbskd == "SBSKnownDerived.sbskd")
        self.called()

    def response_SBSKnownDerivedAsSBSKnownDerived(self, f: Ice.Future[Test.SBSKnownDerived]) -> None:
        sbskd = f.result()
        test(sbskd.sbskd == "SBSKnownDerived.sbskd")
        self.called()

    def response_SBSUnknownDerivedAsSBase(self, f: Ice.Future[Test.SBase]) -> None:
        sb = f.result()
        test(sb.sb == "SBSUnknownDerived.sb")
        self.called()

    def exception_SBSUnknownDerivedAsSBaseCompact(self, f: Ice.Future[Test.SBase]) -> None:
        test(f.exception() is not None)
        test(isinstance(f.exception(), Ice.MarshalException))
        self.called()

    def exception_SUnknownAsObject10(self, f: Ice.Future[Ice.Value]) -> None:
        test(f.exception() is not None)
        test("the Slice loader did not find a class for type ID '::Test::SUnknown'" in str(f.exception()))
        self.called()

    def response_SUnknownAsObject11(self, f: Ice.Future[Ice.Value]) -> None:
        o = f.result()
        assert isinstance(o, Ice.UnknownSlicedValue)
        test(o.unknownTypeId == "::Test::SUnknown")
        self.called()

    def response_oneElementCycle(self, f: Ice.Future[Test.B]) -> None:
        b = f.result()
        test(b)
        test(b.ice_id() == "::Test::B")
        test(b.sb == "B1.sb")
        test(b.pb == b)
        self.called()

    def response_twoElementCycle(self, f: Ice.Future[Test.B]) -> None:
        b1 = f.result()
        assert b1 is not None
        test(b1.ice_id() == "::Test::B")
        test(b1.sb == "B1.sb")

        b2 = b1.pb
        assert b2 is not None
        test(b2.ice_id() == "::Test::B")
        test(b2.sb == "B2.sb")
        test(b2.pb == b1)
        self.called()

    def response_D1AsB(self, f: Ice.Future[Test.B]) -> None:
        b1 = f.result()
        assert b1 is not None
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb)
        test(b1.pb != b1)
        d1 = b1
        assert isinstance(d1, Test.D1)
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1)
        test(d1.pd1 != b1)
        test(b1.pb == d1.pd1)

        b2 = b1.pb
        assert b2 is not None
        test(b2.pb == b1)
        test(b2.sb == "D2.sb")
        test(b2.ice_id() == "::Test::B")
        self.called()

    def response_D1AsD1(self, f: Ice.Future[Test.D1]) -> None:
        d1 = f.result()
        test(d1)
        test(d1.ice_id() == "::Test::D1")
        test(d1.sb == "D1.sb")
        test(d1.pb)
        test(d1.pb != d1)

        b2 = d1.pb
        assert b2 is not None
        test(b2.ice_id() == "::Test::B")
        test(b2.sb == "D2.sb")
        test(b2.pb == d1)
        self.called()

    def response_D2AsB(self, f: Ice.Future[Test.B]) -> None:
        b2 = f.result()
        test(b2)
        test(b2.ice_id() == "::Test::B")
        test(b2.sb == "D2.sb")
        test(b2.pb)
        test(b2.pb != b2)

        b1 = b2.pb
        assert b1 is not None
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb == b2)
        d1 = b1
        assert isinstance(d1, Test.D1)
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1 == b2)
        self.called()

    def response_paramTest1(self, f: Ice.Future[tuple[Test.B, Test.B]]) -> None:
        (b1, b2) = f.result()
        test(b1)
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb == b2)
        d1 = b1
        assert isinstance(d1, Test.D1)
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1 == b2)

        test(b2)
        test(b2.ice_id() == "::Test::B")  # No factory, must be sliced
        test(b2.sb == "D2.sb")
        test(b2.pb == b1)
        self.called()

    def response_returnTest1(self, f: Ice.Future[tuple[Test.B, Test.B, Test.B]]) -> None:
        (r, p1, _) = f.result()
        test(r == p1)
        self.called()

    def response_returnTest2(self, f: Ice.Future[tuple[Test.B, Test.B, Test.B]]) -> None:
        (r, p1, _) = f.result()
        test(r == p1)
        self.called()

    def response_returnTest3(self, f: Ice.Future[_T]) -> None:
        b = f.result()
        assert b is not None
        self.r = b
        self.called()

    def response_paramTest3(self, f: Ice.Future[tuple[Test.B, Test.B, Test.B]]) -> None:
        (ret, p1, p2) = f.result()
        test(p1)
        test(p1.sb == "D2.sb (p1 1)")
        test(p1.pb is None)
        test(p1.ice_id() == "::Test::B")

        test(p2)
        test(p2.sb == "D2.sb (p2 1)")
        test(p2.pb is None)
        test(p2.ice_id() == "::Test::B")

        test(ret)
        test(ret.sb == "D1.sb (p2 2)")
        test(ret.pb is None)
        test(ret.ice_id() == "::Test::D1")
        self.called()

    def response_paramTest4(self, f: Ice.Future[tuple[Test.B, Test.B]]) -> None:
        (ret, b) = f.result()
        test(b)
        test(b.sb == "D4.sb (1)")
        test(b.pb is None)
        test(b.ice_id() == "::Test::B")

        test(ret)
        test(ret.sb == "B.sb (2)")
        test(ret.pb is None)
        test(ret.ice_id() == "::Test::B")
        self.called()

    def response_sequenceTest(self, f: Ice.Future[_T]) -> None:
        ss = f.result()
        self.r = ss
        self.called()

    def response_dictionaryTest(self, f: Ice.Future[tuple[_T, dict[int, Test.B]]]) -> None:
        (r, bout) = f.result()
        self.r = r
        self.bout = bout
        self.called()

    def exception_throwBaseAsBase(self, f: Ice.Future[None]) -> None:
        ex = f.exception()

        assert isinstance(ex, Test.BaseException)
        test(ex.ice_id() == "::Test::BaseException")
        e = ex
        test(e.sbe == "sbe")
        assert e.pb
        test(e.pb.sb == "sb")
        test(e.pb.pb == e.pb)
        self.called()

    def exception_throwDerivedAsBase(self, f: Ice.Future[None]) -> None:
        ex = f.exception()
        assert isinstance(ex, Test.DerivedException)
        test(ex.ice_id() == "::Test::DerivedException")
        e = ex
        test(e.sbe == "sbe")
        assert e.pb
        test(e.pb.sb == "sb1")
        test(e.pb.pb == e.pb)
        test(e.sde == "sde1")
        assert e.pd1
        test(e.pd1.sb == "sb2")
        test(e.pd1.pb == e.pd1)
        test(e.pd1.sd1 == "sd2")
        test(e.pd1.pd1 == e.pd1)
        self.called()

    def exception_throwDerivedAsDerived(self, f: Ice.Future[None]) -> None:
        ex = f.exception()
        assert isinstance(ex, Test.DerivedException)
        test(ex.ice_id() == "::Test::DerivedException")
        e = ex
        test(e.sbe == "sbe")
        assert e.pb
        test(e.pb.sb == "sb1")
        test(e.pb.pb == e.pb)
        test(e.sde == "sde1")
        assert e.pd1
        test(e.pd1.sb == "sb2")
        test(e.pd1.pb == e.pd1)
        test(e.pd1.sd1 == "sd2")
        test(e.pd1.pd1 == e.pd1)
        self.called()

    def exception_throwUnknownDerivedAsBase(self, f: Ice.Future[None]) -> None:
        ex = f.exception()
        assert isinstance(ex, Test.BaseException)
        test(ex.ice_id() == "::Test::BaseException")
        e = ex
        test(e.sbe == "sbe")
        assert e.pb
        test(e.pb.sb == "sb d2")
        test(e.pb.pb == e.pb)
        self.called()

    def response_useForward(self, f: Ice.Future[Test.Forward]) -> None:
        fwd = f.result()
        test(fwd)
        self.called()

    def response_preserved1(self, f: Ice.Future[Test.Preserved]) -> None:
        r = f.result()
        assert r is not None
        assert isinstance(r, Test.PDerived)
        test(r.pi == 3)
        test(r.ps == "preserved")
        test(r.pb == r)
        self.called()

    def response_preserved2encoding10(self, f: Ice.Future[Test.Preserved]) -> None:
        r = f.result()
        assert r is not None
        test(not isinstance(r, ClientPrivateTest.PCUnknown))
        test(r.pi == 3)
        self.called()

    def response_preserved2encoding11(self, f: Ice.Future[Test.Preserved]) -> None:
        r = f.result()
        assert r is not None
        assert isinstance(r, ClientPrivateTest.PCUnknown)
        test(r.pi == 3)
        test(r.pu == "preserved")
        self.called()

    def response_preserved3(self, f: Ice.Future[Test.Preserved]) -> None:
        #
        # Encoding 1.0
        #
        r = f.result()
        test(not isinstance(r, ClientPrivateTest.PCDerived))
        test(r.pi == 3)
        self.called()

    def response_preserved4(self, f: Ice.Future[Test.Preserved]) -> None:
        #
        # Encoding > 1.0
        #
        r = f.result()
        assert isinstance(r, ClientPrivateTest.PCDerived)
        test(r.pi == 3)
        test(r.pbs[0] == r)
        self.called()

    def response_preserved5(self, f: Ice.Future[Test.Preserved]) -> None:
        r = f.result()
        assert isinstance(r, ClientPrivateTest.PCDerived3)
        test(r.pi == 3)
        for i in range(0, 300):
            p2 = r.pbs[i]
            assert isinstance(p2, ClientPrivateTest.PCDerived2)
            test(p2.pi == i)
            test(len(p2.pbs) == 1)
            test(not p2.pbs[0])
            test(p2.pcd2 == i)
        test(r.pcd2 == r.pi)
        test(r.pcd3 == r.pbs[10])
        self.called()

    def response_compactPreserved1(self, f: Ice.Future[Test.Preserved]) -> None:
        #
        # Encoding 1.0
        #
        r = f.result()
        test(not isinstance(r, ClientPrivateTest.CompactPCDerived))
        test(r.pi == 3)
        self.called()

    def response_compactPreserved2(self, f: Ice.Future[Test.Preserved]) -> None:
        #
        # Encoding > 1.0
        #
        r = f.result()
        assert isinstance(r, ClientPrivateTest.CompactPCDerived)
        test(r.pi == 3)
        test(r.pbs[0] == r)
        self.called()


class PNodeI(Test.PNode):
    counter = 0

    def __init__(self) -> None:
        PNodeI.counter = PNodeI.counter + 1

    def __del__(self) -> None:
        PNodeI.counter = PNodeI.counter - 1


class PreservedI(Test.Preserved):
    counter = 0

    def __init__(self) -> None:
        PreservedI.counter = PreservedI.counter + 1

    def __del__(self) -> None:
        PreservedI.counter = PreservedI.counter - 1


def customSliceLoader(typeId: str) -> PreservedI | PNodeI | None:
    if typeId == Test.Preserved.ice_staticId():
        return PreservedI()
    elif typeId == Test.PNode.ice_staticId():
        return PNodeI()
    else:
        return None


def testValueSlicing(t: Test.TestIntfPrx):
    sys.stdout.write("base as Object... ")
    sys.stdout.flush()
    o = None
    try:
        o = t.SBaseAsObject()
        assert o is not None
        test(o.ice_id() == "::Test::SBase")
    except Ice.Exception:
        test(False)
    sb = o

    assert isinstance(sb, Test.SBase)
    assert sb is not None
    test(sb.sb == "SBase.sb")
    print("ok")

    sys.stdout.write("base as Object (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.SBaseAsObjectAsync()).add_done_callback(cb.response_SBaseAsObject)
    cb.check()
    print("ok")

    sys.stdout.write("base as base... ")
    sys.stdout.flush()
    try:
        sb = t.SBaseAsSBase()
        assert sb is not None
        test(sb.sb == "SBase.sb")
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("base as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.SBaseAsSBaseAsync()).add_done_callback(cb.response_SBaseAsSBase)
    cb.check()
    print("ok")

    sys.stdout.write("base with known derived as base... ")
    sys.stdout.flush()
    try:
        sb = t.SBSKnownDerivedAsSBase()
        assert sb is not None
        test(sb.sb == "SBSKnownDerived.sb")
    except Ice.Exception:
        test(False)
    sbskd = sb
    assert isinstance(sbskd, Test.SBSKnownDerived)
    assert sbskd is not None
    test(sbskd.sbskd == "SBSKnownDerived.sbskd")
    print("ok")

    sys.stdout.write("base with known derived as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.SBSKnownDerivedAsSBaseAsync()).add_done_callback(cb.response_SBSKnownDerivedAsSBase)
    cb.check()
    print("ok")

    sys.stdout.write("base with known derived as known derived... ")
    sys.stdout.flush()
    try:
        sbskd = t.SBSKnownDerivedAsSBSKnownDerived()
        assert sbskd is not None
        test(sbskd.sbskd == "SBSKnownDerived.sbskd")
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("base with known derived as known derived (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.SBSKnownDerivedAsSBSKnownDerivedAsync()).add_done_callback(
        cb.response_SBSKnownDerivedAsSBSKnownDerived
    )
    cb.check()
    print("ok")

    sys.stdout.write("base with unknown derived as base... ")
    sys.stdout.flush()
    try:
        sb = t.SBSUnknownDerivedAsSBase()
        assert sb is not None
        test(sb.sb == "SBSUnknownDerived.sb")
    except Ice.Exception:
        test(False)
    if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
        try:
            #
            # This test succeeds for the 1.0 encoding.
            #
            sb = t.SBSUnknownDerivedAsSBaseCompact()
            assert sb is not None
            test(sb.sb == "SBSUnknownDerived.sb")
        except Ice.OperationNotExistException:
            pass
        except Exception as ex:
            print(f"Unexpected exception: {ex}")
            test(False)
    else:
        try:
            #
            # This test fails when using the compact format because the instance cannot
            # be sliced to a known type.
            #
            sb = t.SBSUnknownDerivedAsSBaseCompact()
            test(False)
        except Ice.OperationNotExistException:
            pass
        except Ice.MarshalException:
            # Expected.
            pass
        except Exception as ex:
            print(f"Unexpected exception: {ex}")
            test(False)
    print("ok")

    sys.stdout.write("base with unknown derived as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.SBSUnknownDerivedAsSBaseAsync()).add_done_callback(
        cb.response_SBSUnknownDerivedAsSBase
    )
    cb.check()
    if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
        #
        # This test succeeds for the 1.0 encoding.
        #
        cb = Callback()
        cast(Ice.InvocationFuture, t.SBSUnknownDerivedAsSBaseCompactAsync()).add_done_callback(
            cb.response_SBSUnknownDerivedAsSBase
        )
        cb.check()
    else:
        #
        # This test fails when using the compact format because the instance cannot
        # be sliced to a known type.
        #
        cb = Callback()
        cast(Ice.InvocationFuture, t.SBSUnknownDerivedAsSBaseCompactAsync()).add_done_callback(
            cb.exception_SBSUnknownDerivedAsSBaseCompact
        )
        cb.check()
    print("ok")

    sys.stdout.write("unknown with Object as Object... ")
    sys.stdout.flush()
    try:
        o = t.SUnknownAsObject()
        test(t.ice_getEncodingVersion() != Ice.Encoding_1_0)
        assert isinstance(o, Ice.UnknownSlicedValue)
        test(o.unknownTypeId == "::Test::SUnknown")
        test(o.ice_getSlicedData())
        t.checkSUnknown(o)
    except Ice.MarshalException:
        test(t.ice_getEncodingVersion() == Ice.Encoding_1_0)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("unknown with Object as Object (AMI)... ")
    sys.stdout.flush()
    try:
        cb = Callback()
        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            cast(Ice.InvocationFuture, t.SUnknownAsObjectAsync()).add_done_callback(cb.exception_SUnknownAsObject10)
        else:
            cast(Ice.InvocationFuture, t.SUnknownAsObjectAsync()).add_done_callback(cb.response_SUnknownAsObject11)
        cb.check()
    except Ice.Exception:
        test(False)
    print("ok")


def testCycles(t: Test.TestIntfPrx):
    sys.stdout.write("one-element cycle... ")
    sys.stdout.flush()
    try:
        b = t.oneElementCycle()
        assert b is not None
        test(b.ice_id() == "::Test::B")
        test(b.sb == "B1.sb")
        test(b.pb == b)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("one-element cycle (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.oneElementCycleAsync()).add_done_callback(cb.response_oneElementCycle)
    cb.check()
    print("ok")

    sys.stdout.write("two-element cycle... ")
    sys.stdout.flush()
    try:
        b1 = t.twoElementCycle()
        assert b1 is not None
        test(b1.ice_id() == "::Test::B")
        test(b1.sb == "B1.sb")

        b2 = b1.pb
        assert b2 is not None
        test(b2.ice_id() == "::Test::B")
        test(b2.sb == "B2.sb")
        test(b2.pb == b1)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("two-element cycle (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.twoElementCycleAsync()).add_done_callback(cb.response_twoElementCycle)
    cb.check()
    print("ok")

    sys.stdout.write("known derived pointer slicing as base... ")
    sys.stdout.flush()
    try:
        b1 = t.D1AsB()
        assert b1 is not None
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb)
        test(b1.pb != b1)
        d1 = b1
        assert isinstance(d1, Test.D1)
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1)
        test(d1.pd1 != b1)
        test(b1.pb == d1.pd1)

        b2 = b1.pb
        assert b2
        test(b2.pb == b1)
        test(b2.sb == "D2.sb")
        test(b2.ice_id() == "::Test::B")
    except Ice.Exception:
        test(False)
    print("ok")


def testPointerSlicing(t: Test.TestIntfPrx) -> None:
    sys.stdout.write("known derived pointer slicing as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.D1AsBAsync()).add_done_callback(cb.response_D1AsB)
    cb.check()
    print("ok")

    sys.stdout.write("known derived pointer slicing as derived... ")
    sys.stdout.flush()
    try:
        d1 = t.D1AsD1()
        assert d1 is not None
        test(d1.ice_id() == "::Test::D1")
        test(d1.sb == "D1.sb")
        test(d1.pb)
        test(d1.pb != d1)

        b2 = d1.pb
        assert b2 is not None
        test(b2.ice_id() == "::Test::B")
        test(b2.sb == "D2.sb")
        test(b2.pb == d1)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("known derived pointer slicing as derived (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.D1AsD1Async()).add_done_callback(cb.response_D1AsD1)
    cb.check()
    print("ok")

    sys.stdout.write("unknown derived pointer slicing as base... ")
    sys.stdout.flush()
    try:
        b2 = t.D2AsB()
        assert b2 is not None
        test(b2.ice_id() == "::Test::B")
        test(b2.sb == "D2.sb")
        test(b2.pb)
        test(b2.pb != b2)

        b1 = b2.pb
        assert b1 is not None
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb == b2)
        d1 = b1
        assert isinstance(d1, Test.D1)
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1 == b2)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("unknown derived pointer slicing as base (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.D2AsBAsync()).add_done_callback(cb.response_D2AsB)
    cb.check()
    print("ok")

    sys.stdout.write("param ptr slicing with known first... ")
    sys.stdout.flush()
    try:
        b1, b2 = t.paramTest1()

        assert b1 is not None
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb == b2)
        d1 = b1
        assert isinstance(d1, Test.D1)
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1 == b2)

        assert b2 is not None
        test(b2.ice_id() == "::Test::B")  # No factory, must be sliced
        test(b2.sb == "D2.sb")
        test(b2.pb == b1)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("param ptr slicing with known first (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.paramTest1Async()).add_done_callback(cb.response_paramTest1)
    cb.check()
    print("ok")

    sys.stdout.write("param ptr slicing with unknown first... ")
    sys.stdout.flush()
    try:
        b2, b1 = t.paramTest2()

        assert b1 is not None
        test(b1.ice_id() == "::Test::D1")
        test(b1.sb == "D1.sb")
        test(b1.pb == b2)
        d1 = b1
        assert isinstance(d1, Test.D1)
        test(d1.sd1 == "D1.sd1")
        test(d1.pd1 == b2)

        assert b2 is not None
        test(b2.ice_id() == "::Test::B")  # No factory, must be sliced
        test(b2.sb == "D2.sb")
        test(b2.pb == b1)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("return value identity with known first... ")
    sys.stdout.flush()
    try:
        r, p1, p2 = t.returnTest1()
        test(r == p1)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("return value identity with known first (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.returnTest1Async()).add_done_callback(cb.response_returnTest1)
    cb.check()
    print("ok")

    sys.stdout.write("return value identity with unknown first... ")
    sys.stdout.flush()
    try:
        r, p1, p2 = t.returnTest2()
        test(r == p1)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("return value identity with unknown first (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.returnTest2Async()).add_done_callback(cb.response_returnTest2)
    cb.check()
    print("ok")

    sys.stdout.write("return value identity for input params known first... ")
    sys.stdout.flush()
    try:
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d3 = ClientPrivateTest.D3()
        d3.pb = d1
        d3.sb = "D3.sb"
        d3.sd3 = "D3.sd3"
        d3.pd3 = d1
        d1.pb = d3
        d1.pd1 = d3

        b1 = t.returnTest3(d1, d3)

        assert b1 is not None
        test(b1.sb == "D1.sb")
        test(b1.ice_id() == "::Test::D1")
        p1 = b1
        assert isinstance(p1, Test.D1)
        test(p1.sd1 == "D1.sd1")
        test(p1.pd1 == b1.pb)

        b2 = b1.pb
        assert b2 is not None
        test(b2.sb == "D3.sb")
        test(b2.pb == b1)
        p3 = b2
        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(not isinstance(p3, ClientPrivateTest.D3))
        else:
            assert isinstance(p3, ClientPrivateTest.D3)
            test(p3.pd3 == p1)
            test(p3.sd3 == "D3.sd3")

        test(b1 != d1)
        test(b1 != d3)
        test(b2 != d1)
        test(b2 != d3)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("return value identity for input params known first (AMI)... ")
    sys.stdout.flush()
    try:
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d3 = ClientPrivateTest.D3()
        d3.pb = d1
        d3.sb = "D3.sb"
        d3.sd3 = "D3.sd3"
        d3.pd3 = d1
        d1.pb = d3
        d1.pd1 = d3

        cb = Callback[Test.B]()
        cast(Ice.InvocationFuture, t.returnTest3Async(d1, d3)).add_done_callback(cb.response_returnTest3)
        cb.check()
        b1 = cb.r

        assert b1 is not None
        test(b1.sb == "D1.sb")
        test(b1.ice_id() == "::Test::D1")
        p1 = b1
        assert isinstance(p1, Test.D1)
        test(p1.sd1 == "D1.sd1")
        test(p1.pd1 == b1.pb)

        b2 = b1.pb
        assert b2 is not None
        test(b2.sb == "D3.sb")
        test(b2.pb == b1)
        p3 = b2
        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(not isinstance(p3, ClientPrivateTest.D3))
        else:
            assert isinstance(p3, ClientPrivateTest.D3)
            test(p3.pd3 == p1)
            test(p3.sd3 == "D3.sd3")

        test(b1 != d1)
        test(b1 != d3)
        test(b2 != d1)
        test(b2 != d3)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("return value identity for input params unknown first... ")
    sys.stdout.flush()
    try:
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d3 = ClientPrivateTest.D3()
        d3.pb = d1
        d3.sb = "D3.sb"
        d3.sd3 = "D3.sd3"
        d3.pd3 = d1
        d1.pb = d3
        d1.pd1 = d3

        b1 = t.returnTest3(d3, d1)

        assert b1 is not None
        test(b1.sb == "D3.sb")

        b2 = b1.pb
        assert b2 is not None
        test(b2.sb == "D1.sb")
        test(b2.ice_id() == "::Test::D1")
        test(b2.pb == b1)

        p3 = b2
        assert isinstance(p3, Test.D1)
        test(p3.sd1 == "D1.sd1")
        test(p3.pd1 == b1)

        p1 = b1
        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(not isinstance(p1, ClientPrivateTest.D3))
        else:
            assert isinstance(p1, ClientPrivateTest.D3)
            test(p1.pd3 == b2)
            test(p1.sd3 == "D3.sd3")

        test(b1 != d1)
        test(b1 != d3)
        test(b2 != d1)
        test(b2 != d3)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("return value identity for input params unknown first (AMI)... ")
    sys.stdout.flush()
    try:
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d3 = ClientPrivateTest.D3()
        d3.pb = d1
        d3.sb = "D3.sb"
        d3.sd3 = "D3.sd3"
        d3.pd3 = d1
        d1.pb = d3
        d1.pd1 = d3

        cb = Callback[Test.B]()
        cast(Ice.InvocationFuture, t.returnTest3Async(d3, d1)).add_done_callback(cb.response_returnTest3)
        cb.check()
        b1 = cb.r
        assert b1 is not None

        b2 = b1.pb
        assert b2 is not None
        test(b2.sb == "D1.sb")
        test(b2.ice_id() == "::Test::D1")
        test(b2.pb == b1)

        p3 = b2
        assert isinstance(p3, Test.D1)
        test(p3.sd1 == "D1.sd1")
        test(p3.pd1 == b1)

        p1 = b1
        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(not isinstance(p1, ClientPrivateTest.D3))
        else:
            assert isinstance(p1, ClientPrivateTest.D3)
            test(p1.pd3 == b2)
            test(p1.sd3 == "D3.sd3")

        test(b1 != d1)
        test(b1 != d3)
        test(b2 != d1)
        test(b2 != d3)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("remainder unmarshaling (3 instances)... ")
    sys.stdout.flush()
    try:
        ret, p1, p2 = t.paramTest3()

        assert p1 is not None
        test(p1.sb == "D2.sb (p1 1)")
        test(p1.pb is None)
        test(p1.ice_id() == "::Test::B")

        assert p2 is not None
        test(p2.sb == "D2.sb (p2 1)")
        test(p2.pb is None)
        test(p2.ice_id() == "::Test::B")

        assert ret is not None
        test(ret.sb == "D1.sb (p2 2)")
        test(ret.pb is None)
        test(ret.ice_id() == "::Test::D1")
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("remainder unmarshaling (3 instances) (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.paramTest3Async()).add_done_callback(cb.response_paramTest3)
    cb.check()
    print("ok")

    sys.stdout.write("remainder unmarshaling (4 instances)... ")
    sys.stdout.flush()
    try:
        ret, b = t.paramTest4()

        assert b is not None
        test(b.sb == "D4.sb (1)")
        test(b.pb is None)
        test(b.ice_id() == "::Test::B")

        assert ret is not None
        test(ret.sb == "B.sb (2)")
        test(ret.pb is None)
        test(ret.ice_id() == "::Test::B")
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("remainder unmarshaling (4 instances) (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.paramTest4Async()).add_done_callback(cb.response_paramTest4)
    cb.check()
    print("ok")

    sys.stdout.write("param ptr slicing, instance marshaled in unknown derived as base... ")
    sys.stdout.flush()
    try:
        b1 = Test.B()
        b1.sb = "B.sb(1)"
        b1.pb = b1

        d3 = ClientPrivateTest.D3()
        d3.sb = "D3.sb"
        d3.pb = d3
        d3.sd3 = "D3.sd3"
        d3.pd3 = b1

        b2 = Test.B()
        b2.sb = "B.sb(2)"
        b2.pb = b1

        r = t.returnTest3(d3, b2)

        test(r)

        p3 = r
        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(not isinstance(p3, ClientPrivateTest.D3))
        else:
            assert isinstance(p3, ClientPrivateTest.D3)

            test(p3.sb == "D3.sb")
            test(p3.pb == r)
            test(p3.sd3 == "D3.sd3")

            assert p3.pd3 is not None
            test(p3.pd3.ice_id() == "::Test::B")
            test(p3.pd3.sb == "B.sb(1)")
            test(p3.pd3.pb == p3.pd3)

    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("param ptr slicing, instance marshaled in unknown derived as base (AMI)... ")
    sys.stdout.flush()
    try:
        b1 = Test.B()
        b1.sb = "B.sb(1)"
        b1.pb = b1

        d3 = ClientPrivateTest.D3()
        d3.sb = "D3.sb"
        d3.pb = d3
        d3.sd3 = "D3.sd3"
        d3.pd3 = b1

        b2 = Test.B()
        b2.sb = "B.sb(2)"
        b2.pb = b1

        cb = Callback[Test.B]()
        cast(Ice.InvocationFuture, t.returnTest3Async(d3, b2)).add_done_callback(cb.response_returnTest3)
        cb.check()
        r = cb.r

        p3 = r
        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(not isinstance(p3, ClientPrivateTest.D3))
        else:
            assert isinstance(p3, ClientPrivateTest.D3)

            test(p3.sb == "D3.sb")
            test(p3.pb == r)
            test(p3.sd3 == "D3.sd3")

            assert p3.pd3 is not None
            test(p3.pd3.ice_id() == "::Test::B")
            test(p3.pd3.sb == "B.sb(1)")
            test(p3.pd3.pb == p3.pd3)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("param ptr slicing, instance marshaled in unknown derived as derived... ")
    sys.stdout.flush()
    try:
        d11 = Test.D1()
        d11.sb = "D1.sb(1)"
        d11.pb = d11
        d11.sd1 = "D1.sd1(1)"
        d11.pd1 = None

        d3 = ClientPrivateTest.D3()
        d3.sb = "D3.sb"
        d3.pb = d3
        d3.sd3 = "D3.sd3"
        d3.pd3 = d11

        d12 = Test.D1()
        d12.sb = "D1.sb(2)"
        d12.pb = d12
        d12.sd1 = "D1.sd1(2)"
        d12.pd1 = d11

        r = t.returnTest3(d3, d12)

        assert r is not None
        test(r.sb == "D3.sb")
        test(r.pb == r)
        p3 = r
        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(not isinstance(p3, ClientPrivateTest.D3))
        else:
            assert isinstance(p3, ClientPrivateTest.D3)
            test(p3.sd3 == "D3.sd3")
            assert p3.pd3 is not None
            test(p3.pd3.ice_id() == "::Test::D1")

    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("param ptr slicing, instance marshaled in unknown derived as derived (AMI)... ")
    sys.stdout.flush()
    try:
        d11 = Test.D1()
        d11.sb = "D1.sb(1)"
        d11.pb = d11
        d11.sd1 = "D1.sd1(1)"
        d11.pd1 = None

        d3 = ClientPrivateTest.D3()
        d3.sb = "D3.sb"
        d3.pb = d3
        d3.sd3 = "D3.sd3"
        d3.pd3 = d11

        d12 = Test.D1()
        d12.sb = "D1.sb(2)"
        d12.pb = d12
        d12.sd1 = "D1.sd1(2)"
        d12.pd1 = d11

        cb = Callback[Test.B]()
        cast(Ice.InvocationFuture, t.returnTest3Async(d3, d12)).add_done_callback(cb.response_returnTest3)
        cb.check()
        r = cb.r
        assert r is not None

        test(r.sb == "D3.sb")
        test(r.pb == r)
        p3 = r
        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(not isinstance(p3, ClientPrivateTest.D3))
        else:
            assert isinstance(p3, ClientPrivateTest.D3)
            assert p3.pd3 is not None
            test(p3.sd3 == "D3.sd3")
            test(p3.pd3.ice_id() == "::Test::D1")

    except Ice.Exception:
        test(False)
    print("ok")


def testSequenceSlicing(t: Test.TestIntfPrx) -> None:
    sys.stdout.write("sequence slicing... ")
    sys.stdout.flush()
    try:
        ss = Test.SS3()
        ss1b = Test.B()
        ss1b.sb = "B.sb"
        ss1b.pb = ss1b

        ss1d1 = Test.D1()
        ss1d1.sb = "D1.sb"
        ss1d1.sd1 = "D1.sd1"
        ss1d1.pb = ss1b

        ss1d3 = ClientPrivateTest.D3()
        ss1d3.sb = "D3.sb"
        ss1d3.sd3 = "D3.sd3"
        ss1d3.pb = ss1b

        ss2b = Test.B()
        ss2b.sb = "B.sb"
        ss2b.pb = ss1b

        ss2d1 = Test.D1()
        ss2d1.sb = "D1.sb"
        ss2d1.sd1 = "D1.sd1"
        ss2d1.pb = ss2b

        ss2d3 = ClientPrivateTest.D3()
        ss2d3.sb = "D3.sb"
        ss2d3.sd3 = "D3.sd3"
        ss2d3.pb = ss2b

        ss1d1.pd1 = ss2b
        ss1d3.pd3 = ss2d1

        ss2d1.pd1 = ss1d3
        ss2d3.pd3 = ss1d1

        ss1 = Test.SS1()
        ss1.s = [ss1b, ss1d1, ss1d3]

        ss2 = Test.SS2()
        ss2.s = [ss2b, ss2d1, ss2d3]

        ss = t.sequenceTest(ss1, ss2)

        assert ss.c1 is not None
        ss1b = ss.c1.s[0]
        ss1d1 = ss.c1.s[1]
        test(ss.c2)
        ss1d3 = ss.c1.s[2]

        assert ss.c2 is not None
        ss2b = ss.c2.s[0]
        ss2d1 = ss.c2.s[1]
        ss2d3 = ss.c2.s[2]

        assert ss1b is not None
        assert ss1d1 is not None
        assert ss1d3 is not None
        test(ss1b.pb == ss1b)
        test(ss1d1.pb == ss1b)
        test(ss1d3.pb == ss1b)

        assert ss2b is not None
        assert ss2d1 is not None
        assert ss2d3 is not None
        test(ss2b.pb == ss1b)
        test(ss2d1.pb == ss2b)
        test(ss2d3.pb == ss2b)

        test(ss1b.ice_id() == "::Test::B")
        test(ss1d1.ice_id() == "::Test::D1")

        test(ss2b.ice_id() == "::Test::B")
        test(ss2d1.ice_id() == "::Test::D1")

        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(ss1d3.ice_id() == "::Test::B")
            test(ss2d3.ice_id() == "::Test::B")
        else:
            test(ss1d3.ice_id() == "::Test::D3")
            test(ss2d3.ice_id() == "::Test::D3")

    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("sequence slicing (AMI)... ")
    sys.stdout.flush()
    try:
        ss = Test.SS3()
        ss1b = Test.B()
        ss1b.sb = "B.sb"
        ss1b.pb = ss1b

        ss1d1 = Test.D1()
        ss1d1.sb = "D1.sb"
        ss1d1.sd1 = "D1.sd1"
        ss1d1.pb = ss1b

        ss1d3 = ClientPrivateTest.D3()
        ss1d3.sb = "D3.sb"
        ss1d3.sd3 = "D3.sd3"
        ss1d3.pb = ss1b

        ss2b = Test.B()
        ss2b.sb = "B.sb"
        ss2b.pb = ss1b

        ss2d1 = Test.D1()
        ss2d1.sb = "D1.sb"
        ss2d1.sd1 = "D1.sd1"
        ss2d1.pb = ss2b

        ss2d3 = ClientPrivateTest.D3()
        ss2d3.sb = "D3.sb"
        ss2d3.sd3 = "D3.sd3"
        ss2d3.pb = ss2b

        ss1d1.pd1 = ss2b
        ss1d3.pd3 = ss2d1

        ss2d1.pd1 = ss1d3
        ss2d3.pd3 = ss1d1

        ss1 = Test.SS1()
        ss1.s = [ss1b, ss1d1, ss1d3]

        ss2 = Test.SS2()
        ss2.s = [ss2b, ss2d1, ss2d3]

        cb = Callback[Test.SS3]()
        cast(Ice.InvocationFuture, t.sequenceTestAsync(ss1, ss2)).add_done_callback(cb.response_sequenceTest)
        cb.check()
        ss = cb.r
        assert ss is not None

        assert ss.c1 is not None
        ss1b = ss.c1.s[0]
        ss1d1 = ss.c1.s[1]
        assert ss.c2 is not None
        ss1d3 = ss.c1.s[2]

        assert ss.c2 is not None
        ss2b = ss.c2.s[0]
        ss2d1 = ss.c2.s[1]
        ss2d3 = ss.c2.s[2]

        assert ss1b is not None
        assert ss1d1 is not None
        assert ss1d3 is not None
        test(ss1b.pb == ss1b)
        test(ss1d1.pb == ss1b)
        test(ss1d3.pb == ss1b)

        assert ss2b is not None
        assert ss2d1 is not None
        assert ss2d3 is not None
        test(ss2b.pb == ss1b)
        test(ss2d1.pb == ss2b)
        test(ss2d3.pb == ss2b)

        test(ss1b.ice_id() == "::Test::B")
        test(ss1d1.ice_id() == "::Test::D1")

        test(ss2b.ice_id() == "::Test::B")
        test(ss2d1.ice_id() == "::Test::D1")

        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(ss1d3.ice_id() == "::Test::B")
            test(ss2d3.ice_id() == "::Test::B")
        else:
            test(ss1d3.ice_id() == "::Test::D3")
            test(ss2d3.ice_id() == "::Test::D3")

    except Ice.Exception:
        test(False)
    print("ok")


def testDictionarySlicing(t: Test.TestIntfPrx) -> None:
    sys.stdout.write("dictionary slicing... ")
    sys.stdout.flush()
    try:
        bin = {}
        for i in range(0, 10):
            d1 = Test.D1()
            s = "D1." + str(i)
            d1.sb = s
            d1.pb = d1
            d1.sd1 = s
            d1.pd1 = None
            bin[i] = d1

        r, bout = t.dictionaryTest(bin)

        test(len(bout) == 10)
        for i in range(0, 10):
            b = bout[i * 10]
            assert b is not None
            s = "D1." + str(i)
            test(b.sb == s)
            assert b.pb is not None
            test(b.pb != b)
            test(b.pb.sb == s)
            test(b.pb.pb == b.pb)

        test(len(r) == 10)
        for i in range(0, 10):
            b = r[i * 20]
            assert b is not None
            s = "D1." + str(i * 20)
            test(b.sb == s)
            if i == 0:
                test(b.pb is None)
            else:
                test(b.pb == r[(i - 1) * 20])
            d1 = b
            assert isinstance(d1, Test.D1)
            test(d1.sd1 == s)
            test(d1.pd1 == d1)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("dictionary slicing (AMI)... ")
    sys.stdout.flush()
    try:
        bin = {}
        for i in range(0, 10):
            d1 = Test.D1()
            s = "D1." + str(i)
            d1.sb = s
            d1.pb = d1
            d1.sd1 = s
            d1.pd1 = None
            bin[i] = d1

        cb = Callback()
        cast(Ice.InvocationFuture, t.dictionaryTestAsync(bin)).add_done_callback(cb.response_dictionaryTest)
        cb.check()
        bout = cb.bout
        r = cb.r
        assert bout is not None
        assert r is not None

        test(len(bout) == 10)
        for i in range(0, 10):
            b = bout[i * 10]
            test(b)
            s = "D1." + str(i)
            test(b.sb == s)
            assert b.pb is not None
            test(b.pb != b)
            test(b.pb.sb == s)
            test(b.pb.pb == b.pb)

        test(len(r) == 10)
        for i in range(0, 10):
            b = r[i * 20]
            test(b)
            s = "D1." + str(i * 20)
            test(b.sb == s)
            if i == 0:
                test(b.pb is None)
            else:
                test(b.pb == r[(i - 1) * 20])
            d1 = b
            test(isinstance(d1, Test.D1))
            test(d1.sd1 == s)
            test(d1.pd1 == d1)
    except Ice.Exception:
        test(False)
    print("ok")


def testExceptionSlicing(t: Test.TestIntfPrx) -> None:
    sys.stdout.write("base exception thrown as base exception... ")
    sys.stdout.flush()
    try:
        t.throwBaseAsBase()
        test(False)
    except Test.BaseException as e:
        test(e.ice_id() == "::Test::BaseException")
        test(e.sbe == "sbe")
        assert e.pb is not None
        test(e.pb.sb == "sb")
        test(e.pb.pb == e.pb)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("base exception thrown as base exception (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.throwBaseAsBaseAsync()).add_done_callback(cb.exception_throwBaseAsBase)
    cb.check()
    print("ok")

    sys.stdout.write("derived exception thrown as base exception... ")
    sys.stdout.flush()
    try:
        t.throwDerivedAsBase()
        test(False)
    except Test.DerivedException as e:
        test(e.ice_id() == "::Test::DerivedException")
        test(e.sbe == "sbe")
        assert e.pb is not None
        test(e.pb.sb == "sb1")
        test(e.pb.pb == e.pb)
        test(e.sde == "sde1")
        assert e.pd1 is not None
        test(e.pd1.sb == "sb2")
        test(e.pd1.pb == e.pd1)
        test(e.pd1.sd1 == "sd2")
        test(e.pd1.pd1 == e.pd1)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("derived exception thrown as base exception (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.throwDerivedAsBaseAsync()).add_done_callback(cb.exception_throwDerivedAsBase)
    cb.check()
    print("ok")

    sys.stdout.write("derived exception thrown as derived exception... ")
    sys.stdout.flush()
    try:
        t.throwDerivedAsDerived()
        test(False)
    except Test.DerivedException as e:
        test(e.ice_id() == "::Test::DerivedException")
        test(e.sbe == "sbe")
        assert e.pb is not None
        test(e.pb.sb == "sb1")
        test(e.pb.pb == e.pb)
        test(e.sde == "sde1")
        assert e.pd1 is not None
        test(e.pd1.sb == "sb2")
        test(e.pd1.pb == e.pd1)
        test(e.pd1.sd1 == "sd2")
        test(e.pd1.pd1 == e.pd1)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("derived exception thrown as derived exception (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.throwDerivedAsDerivedAsync()).add_done_callback(cb.exception_throwDerivedAsDerived)
    cb.check()
    print("ok")

    sys.stdout.write("unknown derived exception thrown as base exception... ")
    sys.stdout.flush()
    try:
        t.throwUnknownDerivedAsBase()
        test(False)
    except Test.BaseException as e:
        test(e.ice_id() == "::Test::BaseException")
        test(e.sbe == "sbe")
        assert e.pb is not None
        test(e.pb.sb == "sb d2")
        test(e.pb.pb == e.pb)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("unknown derived exception thrown as base exception (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.throwUnknownDerivedAsBaseAsync()).add_done_callback(
        cb.exception_throwUnknownDerivedAsBase
    )
    cb.check()
    print("ok")


def testClassSlicing(t: Test.TestIntfPrx) -> None:
    sys.stdout.write("forward-declared class... ")
    sys.stdout.flush()
    try:
        f = t.useForward()
        test(f)
    except Ice.Exception:
        test(False)
    print("ok")

    sys.stdout.write("forward-declared class (AMI)... ")
    sys.stdout.flush()
    cb = Callback()
    cast(Ice.InvocationFuture, t.useForwardAsync()).add_done_callback(cb.response_useForward)
    cb.check()
    print("ok")

    sys.stdout.write("preserved classes... ")
    sys.stdout.flush()

    try:
        #
        # Server knows the most-derived class PDerived.
        #
        pd = Test.PDerived()
        pd.pi = 3
        pd.ps = "preserved"
        pd.pb = pd

        r = t.exchangePBase(pd)
        assert isinstance(r, Test.PDerived)
        test(r.pi == 3)
        test(r.ps == "preserved")
        test(r.pb == r)

        #
        # Server only knows the base (non-preserved) type, so the object is sliced.
        #
        pu = ClientPrivateTest.PCUnknown()
        pu.pi = 3
        pu.pu = "preserved"

        r = t.exchangePBase(pu)
        assert r is not None
        test(r.pi == 3)

        p2 = r
        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(not isinstance(p2, ClientPrivateTest.PCUnknown))
        else:
            assert isinstance(p2, ClientPrivateTest.PCUnknown)
            test(p2.pu == "preserved")

        #
        # Server only knows the intermediate type Preserved. The object will be sliced to
        # Preserved for the 1.0 encoding; otherwise it should be returned intact.
        #
        pcd = ClientPrivateTest.PCDerived()
        pcd.pi = 3
        pcd.pbs = [pcd]

        r = t.exchangePBase(pcd)
        assert r is not None
        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(not isinstance(r, ClientPrivateTest.PCDerived))
            test(r.pi == 3)
        else:
            assert isinstance(r, ClientPrivateTest.PCDerived)
            test(r.pi == 3)
            test(r.pbs[0] == r)

        #
        # Server only knows the intermediate type CompactPDerived. The object will be sliced to
        # CompactPDerived for the 1.0 encoding; otherwise it should be returned intact.
        #
        pcd = ClientPrivateTest.CompactPCDerived()
        pcd.pi = 3
        pcd.pbs = [pcd]

        r = t.exchangePBase(pcd)
        assert r is not None
        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(not isinstance(r, ClientPrivateTest.CompactPCDerived))
            test(r.pi == 3)
        else:
            assert isinstance(r, ClientPrivateTest.CompactPCDerived)
            test(r.pi == 3)
            test(r.pbs[0] == r)

        #
        # Send an object that will have multiple preserved slices in the server.
        # The object will be sliced to Preserved for the 1.0 encoding.
        #
        pcd = ClientPrivateTest.PCDerived3()
        pcd.pi = 3
        #
        # Sending more than 254 objects exercises the encoding for object ids.
        #
        pcd.pbs = []
        for i in range(0, 300):
            p2 = ClientPrivateTest.PCDerived2()
            p2.pi = i
            p2.pbs = [None]  # Nil reference. This slice should not have an indirection table.
            p2.pcd2 = i
            pcd.pbs.append(p2)
        pcd.pcd2 = pcd.pi
        pcd.pcd3 = pcd.pbs[10]

        r = t.exchangePBase(pcd)
        assert r is not None
        if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
            test(not isinstance(r, ClientPrivateTest.PCDerived3))
            test(isinstance(r, Test.Preserved))
            test(r.pi == 3)
        else:
            assert isinstance(r, ClientPrivateTest.PCDerived3)
            test(r.pi == 3)
            for i in range(0, 300):
                p2 = r.pbs[i]
                assert isinstance(p2, ClientPrivateTest.PCDerived2)
                test(p2.pi == i)
                test(len(p2.pbs) == 1)
                test(not p2.pbs[0])
                test(p2.pcd2 == i)
            test(r.pcd2 == r.pi)
            test(r.pcd3 == r.pbs[10])

        #
        # Obtain an object with preserved slices and send it back to the server.
        # The preserved slices should be excluded for the 1.0 encoding, otherwise
        # they should be included.
        #
        p = t.PBSUnknownAsPreserved()
        assert p is not None
        t.checkPBSUnknown(p)
        if t.ice_getEncodingVersion() != Ice.Encoding_1_0:
            slicedData = p.ice_getSlicedData()
            assert slicedData is not None
            # TODO: type hinting for SlicedData
            test(len(slicedData.slices) == 1)  # pyright: ignore
            test(slicedData.slices[0].typeId == "::Test::PSUnknown")  # pyright: ignore
            t.ice_encodingVersion(Ice.Encoding_1_0).checkPBSUnknown(p)
        else:
            test(p.ice_getSlicedData() is None)
    except Ice.OperationNotExistException:
        pass

    print("ok")

    sys.stdout.write("preserved classes (AMI)... ")
    sys.stdout.flush()

    #
    # Server knows the most-derived class PDerived.
    #
    pd = Test.PDerived()
    pd.pi = 3
    pd.ps = "preserved"
    pd.pb = pd

    cb = Callback()
    cast(Ice.InvocationFuture, t.exchangePBaseAsync(pd)).add_done_callback(cb.response_preserved1)
    cb.check()

    #
    # Server only knows the base (non-preserved) type, so the object is sliced.
    #
    pu = ClientPrivateTest.PCUnknown()
    pu.pi = 3
    pu.pu = "preserved"

    cb = Callback()
    if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
        cast(Ice.InvocationFuture, t.exchangePBaseAsync(pu)).add_done_callback(cb.response_preserved2encoding10)
    else:
        cast(Ice.InvocationFuture, t.exchangePBaseAsync(pu)).add_done_callback(cb.response_preserved2encoding11)
    cb.check()

    #
    # Server only knows the intermediate type Preserved. The object will be sliced to
    # Preserved for the 1.0 encoding; otherwise it should be returned intact.
    #
    pcd = ClientPrivateTest.PCDerived()
    pcd.pi = 3
    pcd.pbs = [pcd]

    cb = Callback()
    if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
        cast(Ice.InvocationFuture, t.exchangePBaseAsync(pcd)).add_done_callback(cb.response_preserved3)
    else:
        cast(Ice.InvocationFuture, t.exchangePBaseAsync(pcd)).add_done_callback(cb.response_preserved4)
    cb.check()

    #
    # Server only knows the intermediate type CompactPDerived. The object will be sliced to
    # CompactPDerived for the 1.0 encoding; otherwise it should be returned intact.
    #
    pcd = ClientPrivateTest.CompactPCDerived()
    pcd.pi = 3
    pcd.pbs = [pcd]

    cb = Callback()
    if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
        cast(Ice.InvocationFuture, t.exchangePBaseAsync(pcd)).add_done_callback(cb.response_compactPreserved1)
    else:
        cast(Ice.InvocationFuture, t.exchangePBaseAsync(pcd)).add_done_callback(cb.response_compactPreserved2)
    cb.check()

    #
    # Send an object that will have multiple preserved slices in the server.
    # The object will be sliced to Preserved for the 1.0 encoding.
    #
    pcd = ClientPrivateTest.PCDerived3()
    pcd.pi = 3
    #
    # Sending more than 254 objects exercises the encoding for object ids.
    #
    pcd.pbs = []
    for i in range(0, 300):
        p2 = ClientPrivateTest.PCDerived2()
        p2.pi = i
        p2.pbs = [None]  # Nil reference. This slice should not have an indirection table.
        p2.pcd2 = i
        pcd.pbs.append(p2)
    pcd.pcd2 = pcd.pi
    pcd.pcd3 = pcd.pbs[10]

    cb = Callback()
    if t.ice_getEncodingVersion() == Ice.Encoding_1_0:
        cast(Ice.InvocationFuture, t.exchangePBaseAsync(pcd)).add_done_callback(cb.response_preserved3)
    else:
        cast(Ice.InvocationFuture, t.exchangePBaseAsync(pcd)).add_done_callback(cb.response_preserved5)
    cb.check()

    print("ok")

    sys.stdout.write("garbage collection of preserved classes... ")
    sys.stdout.flush()
    try:
        #
        # Relay a graph through the server. This test uses a preserved class
        # with a class member.
        #
        c = Test.PNode()
        c.next = Test.PNode()
        c.next.next = Test.PNode()
        c.next.next.next = c  # Create a cyclic graph.

        test(PNodeI.counter == 0)
        n = t.exchangePNode(c)
        assert n is not None
        test(PNodeI.counter == 3)
        assert n.next is not None
        assert n.next.next is not None
        test(n.next != n.next.next)
        test(n.next.next != n.next.next.next)
        test(n.next.next.next == n)
        n = None  # Release reference.
        gc.collect()
        test(PNodeI.counter == 0)

        #
        # Obtain a preserved object from the server where the most-derived
        # type is unknown. The preserved slice refers to a graph of PNode
        # objects.
        #
        test(PNodeI.counter == 0)
        p = t.PBSUnknownAsPreservedWithGraph()
        test(p)
        test(PNodeI.counter == 3)
        t.checkPBSUnknownWithGraph(p)
        p = None  # Release reference.
        gc.collect()
        test(PNodeI.counter == 0)

        #
        # Obtain a preserved object from the server where the most-derived
        # type is unknown. A data member in the preserved slice refers to the
        # outer object, so the chain of references looks like this:
        #
        # outer->slicedData->outer
        #
        test(PreservedI.counter == 0)
        p = t.PBSUnknown2AsPreservedWithGraph()
        assert p is not None
        test(PreservedI.counter == 1)
        t.checkPBSUnknown2WithGraph(p)
        p._ice_slicedData = None  # Break the cycle. # pyright: ignore
        p = None  # Release reference.
        test(PreservedI.counter == 0)
    except Ice.OperationNotExistException:
        pass

    print("ok")


def allTests(helper: TestHelper, communicator: Ice.Communicator) -> Test.TestIntfPrx:
    t = Test.TestIntfPrx(communicator, f"Test:{helper.getTestEndpoint()}")

    testValueSlicing(t)
    testCycles(t)
    testPointerSlicing(t)
    testSequenceSlicing(t)
    testDictionarySlicing(t)
    testExceptionSlicing(t)
    testClassSlicing(t)

    return t
