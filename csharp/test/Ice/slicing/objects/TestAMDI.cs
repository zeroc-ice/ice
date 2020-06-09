//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Slicing.Objects
{
    public sealed class TestIntfAsync : ITestIntfAsync
    {
        public ValueTask shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<AnyClass?>
        SBaseAsObjectAsync(Current current) => new ValueTask<AnyClass?>(new SBase("SBase.sb"));

        public ValueTask<SBase?> SBaseAsSBaseAsync(Current current) => new ValueTask<SBase?>(new SBase("SBase.sb"));

        public ValueTask<SBase?>
        SBSKnownDerivedAsSBaseAsync(Current current) =>
            new ValueTask<SBase?>(new SBSKnownDerived("SBSKnownDerived.sb", "SBSKnownDerived.sbskd"));

        public ValueTask<SBSKnownDerived?>
        SBSKnownDerivedAsSBSKnownDerivedAsync(Current current) =>
            new ValueTask<SBSKnownDerived?>(new SBSKnownDerived("SBSKnownDerived.sb", "SBSKnownDerived.sbskd"));

        public ValueTask<SBase?>
        SBSUnknownDerivedAsSBaseAsync(Current current) =>
            new ValueTask<SBase?>(new SBSUnknownDerived("SBSUnknownDerived.sb", "SBSUnknownDerived.sbsud"));

        public ValueTask<SBase?>
        SBSUnknownDerivedAsSBaseCompactAsync(Current current) =>
            new ValueTask<SBase?>(new SBSUnknownDerived("SBSUnknownDerived.sb", "SBSUnknownDerived.sbsud"));

        public ValueTask<AnyClass?> SUnknownAsObjectAsync(Current current)
        {
            var su = new SUnknown("SUnknown.su", null);
            su.cycle = su;
            return new ValueTask<AnyClass?>(su);
        }

        public ValueTask checkSUnknownAsync(AnyClass? obj, Current current)
        {
            TestHelper.Assert(obj != null);
            var su = (SUnknown)obj;
            TestHelper.Assert(su.su.Equals("SUnknown.su"));
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<B?> oneElementCycleAsync(Current current)
        {
            var b = new B();
            b.sb = "B1.sb";
            b.pb = b;
            return new ValueTask<B?>(b);
        }

        public ValueTask<B?> twoElementCycleAsync(Current current)
        {
            var b1 = new B();
            b1.sb = "B1.sb";
            var b2 = new B();
            b2.sb = "B2.sb";
            b2.pb = b1;
            b1.pb = b2;
            return new ValueTask<B?>(b1);
        }

        public ValueTask<B?> D1AsBAsync(Current current)
        {
            var d1 = new D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            var d2 = new D2();
            d2.pb = d1;
            d2.sb = "D2.sb";
            d2.sd2 = "D2.sd2";
            d2.pd2 = d1;
            d1.pb = d2;
            d1.pd1 = d2;
            return new ValueTask<B?>(d1);
        }

        public ValueTask<D1?> D1AsD1Async(Current current)
        {
            var d1 = new D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            var d2 = new D2();
            d2.pb = d1;
            d2.sb = "D2.sb";
            d2.sd2 = "D2.sd2";
            d2.pd2 = d1;
            d1.pb = d2;
            d1.pd1 = d2;
            return new ValueTask<D1?>(d1);
        }

        public ValueTask<B?> D2AsBAsync(Current current)
        {
            var d2 = new D2();
            d2.sb = "D2.sb";
            d2.sd2 = "D2.sd2";
            var d1 = new D1();
            d1.pb = d2;
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            d1.pd1 = d2;
            d2.pb = d1;
            d2.pd2 = d1;
            return new ValueTask<B?>(d2);
        }

        public ValueTask<(B?, B?)>
        paramTest1Async(Current current)
        {
            var d1 = new D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            var d2 = new D2();
            d2.pb = d1;
            d2.sb = "D2.sb";
            d2.sd2 = "D2.sd2";
            d2.pd2 = d1;
            d1.pb = d2;
            d1.pd1 = d2;
            return MakeValueTask(((B?)d1, (B?)d2));
        }

        public ValueTask<(B?, B?)>
        paramTest2Async(Current current)
        {
            var d1 = new D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            var d2 = new D2();
            d2.pb = d1;
            d2.sb = "D2.sb";
            d2.sd2 = "D2.sd2";
            d2.pd2 = d1;
            d1.pb = d2;
            d1.pd1 = d2;
            return MakeValueTask(((B?)d2, (B?)d1));
        }

        public ValueTask<(B?, B?, B?)>
        paramTest3Async(Current current)
        {
            var d2 = new D2();
            d2.sb = "D2.sb (p1 1)";
            d2.pb = null;
            d2.sd2 = "D2.sd2 (p1 1)";

            var d1 = new D1();
            d1.sb = "D1.sb (p1 2)";
            d1.pb = null;
            d1.sd1 = "D1.sd2 (p1 2)";
            d1.pd1 = null;
            d2.pd2 = d1;

            var d4 = new D2();
            d4.sb = "D2.sb (p2 1)";
            d4.pb = null;
            d4.sd2 = "D2.sd2 (p2 1)";

            var d3 = new D1();
            d3.sb = "D1.sb (p2 2)";
            d3.pb = null;
            d3.sd1 = "D1.sd2 (p2 2)";
            d3.pd1 = null;
            d4.pd2 = d3;

            return MakeValueTask(((B?)d3, (B?)d2, (B?)d4));
        }

        public ValueTask<(B?, B?)>
        paramTest4Async(Current current)
        {
            var d4 = new D4();
            d4.sb = "D4.sb (1)";
            d4.pb = null;
            d4.p1 = new B();
            d4.p1.sb = "B.sb (1)";
            d4.p2 = new B();
            d4.p2.sb = "B.sb (2)";
            return MakeValueTask(((B?)d4.p2, (B?)d4));
        }

        public ValueTask<(B?, B?, B?)>
        returnTest1Async(Current current)
        {
            var d1 = new D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            var d2 = new D2();
            d2.pb = d1;
            d2.sb = "D2.sb";
            d2.sd2 = "D2.sd2";
            d2.pd2 = d1;
            d1.pb = d2;
            d1.pd1 = d2;
            return MakeValueTask(((B?)d2, (B?)d2, (B?)d1));
        }

        public ValueTask<(B?, B?, B?)>
        returnTest2Async(Current current)
        {
            var d1 = new D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            var d2 = new D2();
            d2.pb = d1;
            d2.sb = "D2.sb";
            d2.sd2 = "D2.sd2";
            d2.pd2 = d1;
            d1.pb = d2;
            d1.pd1 = d2;
            return new ValueTask<(B?, B?, B?)>((d1, d1, d2));
        }

        public ValueTask<B?>
        returnTest3Async(B? p1, B? p2, Current current) => MakeValueTask(p1);

        public ValueTask<SS3>
        sequenceTestAsync(SS1? p1, SS2? p2, Current current) => MakeValueTask(new SS3(p1, p2));

        public ValueTask<(IReadOnlyDictionary<int, B?>, IReadOnlyDictionary<int, B?>)> dictionaryTestAsync(
            Dictionary<int, B?> bin, Current current)
        {
            var bout = new Dictionary<int, B?>();
            int i;
            for (i = 0; i < 10; ++i)
            {
                B? b = bin[i];
                TestHelper.Assert(b != null);
                var d2 = new D2();
                d2.sb = b.sb;
                d2.pb = b.pb;
                d2.sd2 = "D2";
                d2.pd2 = d2;
                bout[i * 10] = d2;
            }
            var r = new Dictionary<int, B?>();
            for (i = 0; i < 10; ++i)
            {
                string s = "D1." + (i * 20).ToString();
                var d1 = new D1();
                d1.sb = s;
                d1.pb = i == 0 ? null : r[(i - 1) * 20];
                d1.sd1 = s;
                d1.pd1 = d1;
                r[i * 20] = d1;
            }
            return new ValueTask<(IReadOnlyDictionary<int, B?>, IReadOnlyDictionary<int, B?>)>((r, bout));
        }

        public ValueTask<PBase?>
        exchangePBaseAsync(PBase? pb, Current current) => MakeValueTask(pb);

        public ValueTask<Preserved?>
        PBSUnknownAsPreservedAsync(Current current)
        {
            var r = new PSUnknown(5, "preserved", "unknown", null, new MyClass(15));
            return new ValueTask<Preserved?>(r);
        }

        public ValueTask
        checkPBSUnknownAsync(Preserved? p, Current current)
        {
            var pu = p as PSUnknown;
            TestHelper.Assert(pu != null);
            TestHelper.Assert(pu.pi == 5);
            TestHelper.Assert(pu.ps.Equals("preserved"));
            TestHelper.Assert(pu.psu.Equals("unknown"));
            TestHelper.Assert(pu.graph == null);
            TestHelper.Assert(pu.cl != null && pu.cl.i == 15);
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<Preserved?>
        PBSUnknownAsPreservedWithGraphAsync(Current current)
        {
            var graph = new PNode();
            graph.next = new PNode();
            graph.next.next = new PNode();
            graph.next.next.next = graph;

            var r = new PSUnknown(5, "preserved", "unknown", graph, null);
            return new ValueTask<Preserved?>(r);
        }

        public ValueTask
        checkPBSUnknownWithGraphAsync(Preserved? p, Current current)
        {
            TestHelper.Assert(p is PSUnknown);
            var pu = (PSUnknown)p;
            TestHelper.Assert(pu.pi == 5);
            TestHelper.Assert(pu.ps.Equals("preserved"));
            TestHelper.Assert(pu.psu.Equals("unknown"));
            TestHelper.Assert(pu.graph != pu.graph!.next);
            TestHelper.Assert(pu.graph.next != pu.graph!.next!.next);
            TestHelper.Assert(pu.graph!.next!.next!.next == pu.graph);
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<Preserved?>
        PBSUnknown2AsPreservedWithGraphAsync(Current current)
        {
            var r = new PSUnknown2(5, "preserved", null);
            r.pb = r;
            return new ValueTask<Preserved?>(r);
        }

        public ValueTask
        checkPBSUnknown2WithGraphAsync(Preserved? p, Current current)
        {
            TestHelper.Assert(p is PSUnknown2);
            var pu = (PSUnknown2)p;
            TestHelper.Assert(pu.pi == 5);
            TestHelper.Assert(pu.ps.Equals("preserved"));
            TestHelper.Assert(pu.pb == pu);
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask<PNode?>
        exchangePNodeAsync(PNode? pn, Current current) => MakeValueTask(pn);

        public ValueTask throwBaseAsBaseAsync(Current current)
        {
            var b = new B("sb", null);
            b.pb = b;
            throw new BaseException("sbe", b);
        }

        public ValueTask throwDerivedAsBaseAsync(Current current)
        {
            var b = new B("sb1", null);
            b.pb = b;

            var d = new D1("sb2", null, "sd2", null);
            d.pb = d;
            d.pd1 = d;

            throw new DerivedException("sbe", b, "sde1", d);
        }

        public ValueTask
        throwDerivedAsDerivedAsync(Current current)
        {
            var b = new B("sb1", null);
            b.pb = b;

            var d = new D1("sb2", null, "sd2", null);
            d.pb = d;
            d.pd1 = d;

            throw new DerivedException("sbe", b, "sde1", d);
        }

        public ValueTask throwUnknownDerivedAsBaseAsync(Current current)
        {
            var d2 = new D2();
            d2.sb = "sb d2";
            d2.pb = d2;
            d2.sd2 = "sd2 d2";
            d2.pd2 = d2;

            throw new UnknownDerivedException("sbe", d2, "sude", d2);
        }

        public ValueTask throwPreservedExceptionAsync(Current current)
        {
            var ue = new PSUnknownException();
            ue.p = new PSUnknown2(5, "preserved", null);
            ue.p.pb = ue.p;
            throw ue;
        }

        public ValueTask<Forward?>
        useForwardAsync(Current current)
        {
            var f = new Forward();
            f.h = new Hidden();
            f.h.f = f;
            return new ValueTask<Forward?>(f);
        }

        // Type-inference helper method
        private static ValueTask<T> MakeValueTask<T>(T result) => new ValueTask<T>(result);
    }

    public sealed class TestIntf2Async : ITestIntf2Async
    {
        public ValueTask<SBase?> SBSUnknownDerivedAsSBaseAsync(Current current) =>
            new ValueTask<SBase?>(new SBSUnknownDerived("SBSUnknownDerived.sb", "SBSUnknownDerived.sbsud"));

        public ValueTask CUnknownAsSBaseAsync(SBase? cUnknown, Current current)
        {
            if (cUnknown!.sb != "CUnknown.sb")
            {
                throw new Exception();
            }
            return new ValueTask();
        }
    }
}
