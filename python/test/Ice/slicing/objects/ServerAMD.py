#!/usr/bin/env python3

# Copyright (c) ZeroC, Inc.

import Test
from TestHelper import TestHelper

import Ice


def test(b):
    if not b:
        raise RuntimeError("test assertion failed")


class TestI(Test.TestIntf):
    def SBaseAsObject(self, current):
        sb = Test.SBase()
        sb.sb = "SBase.sb"
        return Ice.Future.completed(sb)

    def SBaseAsSBase(self, current):
        sb = Test.SBase()
        sb.sb = "SBase.sb"
        return Ice.Future.completed(sb)

    def SBSKnownDerivedAsSBase(self, current):
        sbskd = Test.SBSKnownDerived()
        sbskd.sb = "SBSKnownDerived.sb"
        sbskd.sbskd = "SBSKnownDerived.sbskd"
        return Ice.Future.completed(sbskd)

    def SBSKnownDerivedAsSBSKnownDerived(self, current):
        sbskd = Test.SBSKnownDerived()
        sbskd.sb = "SBSKnownDerived.sb"
        sbskd.sbskd = "SBSKnownDerived.sbskd"
        return Ice.Future.completed(sbskd)

    def SBSUnknownDerivedAsSBase(self, current):
        sbsud = Test.SBSUnknownDerived()
        sbsud.sb = "SBSUnknownDerived.sb"
        sbsud.sbsud = "SBSUnknownDerived.sbsud"
        return Ice.Future.completed(sbsud)

    def SBSUnknownDerivedAsSBaseCompact(self, current):
        sbsud = Test.SBSUnknownDerived()
        sbsud.sb = "SBSUnknownDerived.sb"
        sbsud.sbsud = "SBSUnknownDerived.sbsud"
        return Ice.Future.completed(sbsud)

    def SUnknownAsObject(self, current):
        su = Test.SUnknown()
        su.su = "SUnknown.su"
        return Ice.Future.completed(su)

    def checkSUnknown(self, obj, current):
        if current.encoding == Ice.Encoding_1_0:
            test(not isinstance(obj, Test.SUnknown))
        else:
            test(isinstance(obj, Test.SUnknown))
            test(obj.su == "SUnknown.su")
        return Ice.Future.completed(None)

    def oneElementCycle(self, current):
        b = Test.B()
        b.sb = "B1.sb"
        b.pb = b
        return Ice.Future.completed(b)

    def twoElementCycle(self, current):
        b1 = Test.B()
        b1.sb = "B1.sb"
        b2 = Test.B()
        b2.sb = "B2.sb"
        b2.pb = b1
        b1.pb = b2
        return Ice.Future.completed(b1)

    def D1AsB(self, current):
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d2 = Test.D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        return Ice.Future.completed(d1)

    def D1AsD1(self, current):
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d2 = Test.D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        return Ice.Future.completed(d1)

    def D2AsB(self, current):
        d2 = Test.D2()
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d1 = Test.D1()
        d1.pb = d2
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d1.pd1 = d2
        d2.pb = d1
        d2.pd2 = d1
        return Ice.Future.completed(d2)

    def paramTest1(self, current):
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d2 = Test.D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        return Ice.Future.completed((d1, d2))

    def paramTest2(self, current):
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d2 = Test.D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        return Ice.Future.completed((d2, d1))

    def paramTest3(self, current):
        d2 = Test.D2()
        d2.sb = "D2.sb (p1 1)"
        d2.pb = None
        d2.sd2 = "D2.sd2 (p1 1)"

        d1 = Test.D1()
        d1.sb = "D1.sb (p1 2)"
        d1.pb = None
        d1.sd1 = "D1.sd2 (p1 2)"
        d1.pd1 = None
        d2.pd2 = d1

        d4 = Test.D2()
        d4.sb = "D2.sb (p2 1)"
        d4.pb = None
        d4.sd2 = "D2.sd2 (p2 1)"

        d3 = Test.D1()
        d3.sb = "D1.sb (p2 2)"
        d3.pb = None
        d3.sd1 = "D1.sd2 (p2 2)"
        d3.pd1 = None
        d4.pd2 = d3

        return Ice.Future.completed((d3, d2, d4))

    def paramTest4(self, current):
        d4 = Test.D4()
        d4.sb = "D4.sb (1)"
        d4.pb = None
        d4.p1 = Test.B()
        d4.p1.sb = "B.sb (1)"
        d4.p1.pb = None
        d4.p2 = Test.B()
        d4.p2.sb = "B.sb (2)"
        d4.p2.pb = None
        return Ice.Future.completed((d4.p2, d4))

    def returnTest1(self, current):
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d2 = Test.D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        return Ice.Future.completed((d2, d2, d1))

    def returnTest2(self, current):
        d1 = Test.D1()
        d1.sb = "D1.sb"
        d1.sd1 = "D1.sd1"
        d2 = Test.D2()
        d2.pb = d1
        d2.sb = "D2.sb"
        d2.sd2 = "D2.sd2"
        d2.pd2 = d1
        d1.pb = d2
        d1.pd1 = d2
        return Ice.Future.completed((d1, d1, d2))

    def returnTest3(self, p1, p2, current):
        return Ice.Future.completed(p1)

    def sequenceTest(self, p1, p2, current):
        ss = Test.SS3()
        ss.c1 = p1
        ss.c2 = p2
        return Ice.Future.completed(ss)

    def dictionaryTest(self, bin, current):
        bout = {}
        for i in range(0, 10):
            b = bin[i]
            d2 = Test.D2()
            d2.sb = b.sb
            d2.pb = b.pb
            d2.sd2 = "D2"
            d2.pd2 = d2
            bout[i * 10] = d2

        r = {}
        for i in range(0, 10):
            s = "D1." + str(i * 20)
            d1 = Test.D1()
            d1.sb = s
            if i == 0:
                d1.pb = None
            else:
                d1.pb = r[(i - 1) * 20]
            d1.sd1 = s
            d1.pd1 = d1
            r[i * 20] = d1

        return Ice.Future.completed((r, bout))

    def exchangePBase(self, pb, current):
        return Ice.Future.completed(pb)

    def PBSUnknownAsPreserved(self, current):
        r = Test.PSUnknown()
        r.pi = 5
        r.ps = "preserved"
        r.psu = "unknown"
        r.graph = None
        return Ice.Future.completed(r)

    def checkPBSUnknown(self, p, current):
        if current.encoding == Ice.Encoding_1_0:
            test(not isinstance(p, Test.PSUnknown))
            test(p.pi == 5)
            test(p.ps == "preserved")
        else:
            test(isinstance(p, Test.PSUnknown))
            test(p.pi == 5)
            test(p.ps == "preserved")
            test(p.psu == "unknown")
            test(not p.graph)
        return Ice.Future.completed(None)

    def PBSUnknownAsPreservedWithGraph(self, current):
        r = Test.PSUnknown()
        r.pi = 5
        r.ps = "preserved"
        r.psu = "unknown"
        r.graph = Test.PNode()
        r.graph.next = Test.PNode()
        r.graph.next.next = Test.PNode()
        r.graph.next.next.next = r.graph
        return Ice.Future.completed(r)
        # r.graph.next.next.next = None   # Break the cycle.

    def checkPBSUnknownWithGraph(self, p, current):
        if current.encoding == Ice.Encoding_1_0:
            test(not isinstance(p, Test.PSUnknown))
            test(p.pi == 5)
            test(p.ps == "preserved")
        else:
            test(isinstance(p, Test.PSUnknown))
            test(p.pi == 5)
            test(p.ps == "preserved")
            test(p.psu == "unknown")
            test(p.graph != p.graph.next)
            test(p.graph.next != p.graph.next.next)
            test(p.graph.next.next.next == p.graph)
            p.graph.next.next.next = None  # Break the cycle.
        return Ice.Future.completed(None)

    def PBSUnknown2AsPreservedWithGraph(self, current):
        r = Test.PSUnknown2()
        r.pi = 5
        r.ps = "preserved"
        r.pb = r
        return Ice.Future.completed(r)
        # r.pb = None         # Break the cycle.

    def checkPBSUnknown2WithGraph(self, p, current):
        if current.encoding == Ice.Encoding_1_0:
            test(not isinstance(p, Test.PSUnknown2))
            test(p.pi == 5)
            test(p.ps == "preserved")
        else:
            test(isinstance(p, Test.PSUnknown2))
            test(p.pi == 5)
            test(p.ps == "preserved")
            test(p.pb == p)
            p.pb = None  # Break the cycle.
        return Ice.Future.completed(None)

    def exchangePNode(self, pn, current):
        return Ice.Future.completed(pn)

    def throwBaseAsBase(self, current):
        be = Test.BaseException()
        be.sbe = "sbe"
        be.pb = Test.B()
        be.pb.sb = "sb"
        be.pb.pb = be.pb
        f = Ice.Future()
        f.set_exception(be)
        return f

    def throwDerivedAsBase(self, current):
        de = Test.DerivedException()
        de.sbe = "sbe"
        de.pb = Test.B()
        de.pb.sb = "sb1"
        de.pb.pb = de.pb
        de.sde = "sde1"
        de.pd1 = Test.D1()
        de.pd1.sb = "sb2"
        de.pd1.pb = de.pd1
        de.pd1.sd1 = "sd2"
        de.pd1.pd1 = de.pd1
        f = Ice.Future()
        f.set_exception(de)
        return f

    def throwDerivedAsDerived(self, current):
        de = Test.DerivedException()
        de.sbe = "sbe"
        de.pb = Test.B()
        de.pb.sb = "sb1"
        de.pb.pb = de.pb
        de.sde = "sde1"
        de.pd1 = Test.D1()
        de.pd1.sb = "sb2"
        de.pd1.pb = de.pd1
        de.pd1.sd1 = "sd2"
        de.pd1.pd1 = de.pd1
        f = Ice.Future()
        f.set_exception(de)
        return f

    def throwUnknownDerivedAsBase(self, current):
        d2 = Test.D2()
        d2.sb = "sb d2"
        d2.pb = d2
        d2.sd2 = "sd2 d2"
        d2.pd2 = d2

        ude = Test.UnknownDerivedException()
        ude.sbe = "sbe"
        ude.pb = d2
        ude.sude = "sude"
        ude.pd2 = d2
        f = Ice.Future()
        f.set_exception(ude)
        return f

    def useForward(self, current):
        fwd = Test.Forward()
        fwd.h = Test.Hidden()
        fwd.h.f = fwd
        return Ice.Future.completed(fwd)

    def shutdown(self, current):
        current.adapter.getCommunicator().shutdown()


class ServerAMD(TestHelper):
    def run(self, args):
        properties = self.createTestProperties(args)
        properties.setProperty("Ice.Warn.Dispatch", "0")
        with self.initialize(properties=properties) as communicator:
            communicator.getProperties().setProperty(
                "TestAdapter.Endpoints", "{0} -t 10000".format(self.getTestEndpoint())
            )
            adapter = communicator.createObjectAdapter("TestAdapter")
            adapter.add(TestI(), Ice.stringToIdentity("Test"))
            adapter.activate()
            communicator.waitForShutdown()
