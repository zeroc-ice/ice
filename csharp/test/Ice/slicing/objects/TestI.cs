// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Slicing.Objects
{
    public sealed class TestIntf : ITestIntf
    {
        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();

        public AnyClass SBaseAsObject(Current current, CancellationToken cancel)
        {
            var sb = new SBase();
            sb.Sb = "SBase.sb";
            return sb;
        }

        public SBase SBaseAsSBase(Current current, CancellationToken cancel)
        {
            var sb = new SBase();
            sb.Sb = "SBase.sb";
            return sb;
        }

        public SBase SBSKnownDerivedAsSBase(Current current, CancellationToken cancel)
        {
            var sbskd = new SBSKnownDerived();
            sbskd.Sb = "SBSKnownDerived.sb";
            sbskd.Sbskd = "SBSKnownDerived.sbskd";
            return sbskd;
        }

        public void CUnknownAsSBase(SBase? cUnknown, Current current, CancellationToken cancel)
        {
            if (cUnknown!.Sb != "CUnknown.sb")
            {
                throw new Exception();
            }
        }

        public SBSKnownDerived SBSKnownDerivedAsSBSKnownDerived(Current current, CancellationToken cancel)
        {
            var sbskd = new SBSKnownDerived();
            sbskd.Sb = "SBSKnownDerived.sb";
            sbskd.Sbskd = "SBSKnownDerived.sbskd";
            return sbskd;
        }

        public SBase SBSUnknownDerivedAsSBase(Current current, CancellationToken cancel) =>
            new SBSUnknownDerived("SBSUnknownDerived.sb", "SBSUnknownDerived.sbsud");

        public SBase SBSUnknownDerivedAsSBaseCompact(Current current, CancellationToken cancel) =>
            new SBSUnknownDerived("SBSUnknownDerived.sb", "SBSUnknownDerived.sbsud");

        public AnyClass SUnknownAsObject(Current current, CancellationToken cancel)
        {
            var su = new SUnknown("SUnknown.su", null);
            su.Cycle = su;
            return su;
        }

        public void CheckSUnknown(AnyClass? obj, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(obj is SUnknown);
            var su = (SUnknown)obj;
            TestHelper.Assert(su.Su.Equals("SUnknown.su"));
        }

        public B OneElementCycle(Current current, CancellationToken cancel)
        {
            var b = new B();
            b.Sb = "B1.sb";
            b.Pb = b;
            return b;
        }

        public B TwoElementCycle(Current current, CancellationToken cancel)
        {
            var b1 = new B();
            b1.Sb = "B1.sb";
            var b2 = new B();
            b2.Sb = "B2.sb";
            b2.Pb = b1;
            b1.Pb = b2;
            return b1;
        }

        public B D1AsB(Current current, CancellationToken cancel)
        {
            var d1 = new D1();
            d1.Sb = "D1.sb";
            d1.Sd1 = "D1.sd1";
            var d2 = new D2();
            d2.Pb = d1;
            d2.Sb = "D2.sb";
            d2.Sd2 = "D2.sd2";
            d2.Pd2 = d1;
            d1.Pb = d2;
            d1.Pd1 = d2;
            return d1;
        }

        public D1 D1AsD1(Current current, CancellationToken cancel)
        {
            var d1 = new D1();
            d1.Sb = "D1.sb";
            d1.Sd1 = "D1.sd1";
            var d2 = new D2();
            d2.Pb = d1;
            d2.Sb = "D2.sb";
            d2.Sd2 = "D2.sd2";
            d2.Pd2 = d1;
            d1.Pb = d2;
            d1.Pd1 = d2;
            return d1;
        }

        public B D2AsB(Current current, CancellationToken cancel)
        {
            var d2 = new D2();
            d2.Sb = "D2.sb";
            d2.Sd2 = "D2.sd2";
            var d1 = new D1();
            d1.Pb = d2;
            d1.Sb = "D1.sb";
            d1.Sd1 = "D1.sd1";
            d1.Pd1 = d2;
            d2.Pb = d1;
            d2.Pd2 = d1;
            return d2;
        }

        public (B, B) ParamTest1(Current current, CancellationToken cancel)
        {
            var d1 = new D1();
            d1.Sb = "D1.sb";
            d1.Sd1 = "D1.sd1";
            var d2 = new D2();
            d2.Pb = d1;
            d2.Sb = "D2.sb";
            d2.Sd2 = "D2.sd2";
            d2.Pd2 = d1;
            d1.Pb = d2;
            d1.Pd1 = d2;
            return (d1, d2);
        }

        public (B, B) ParamTest2(Current current, CancellationToken cancel)
        {
            (B p1, B p2) = ParamTest1(current, cancel);
            return (p2, p1);
        }

        public (B, B, B) ParamTest3(Current current, CancellationToken cancel)
        {
            var d2 = new D2();
            d2.Sb = "D2.sb (p1 1)";
            d2.Pb = null;
            d2.Sd2 = "D2.sd2 (p1 1)";

            var d1 = new D1();
            d1.Sb = "D1.sb (p1 2)";
            d1.Pb = null;
            d1.Sd1 = "D1.sd2 (p1 2)";
            d1.Pd1 = null;
            d2.Pd2 = d1;

            var d4 = new D2();
            d4.Sb = "D2.sb (p2 1)";
            d4.Pb = null;
            d4.Sd2 = "D2.sd2 (p2 1)";

            var d3 = new D1();
            d3.Sb = "D1.sb (p2 2)";
            d3.Pb = null;
            d3.Sd1 = "D1.sd2 (p2 2)";
            d3.Pd1 = null;
            d4.Pd2 = d3;

            return (d3, d2, d4);
        }

        public (B, B) ParamTest4(Current current, CancellationToken cancel)
        {
            var d4 = new D4();
            d4.Sb = "D4.sb (1)";
            d4.Pb = null;
            d4.P1 = new B();
            d4.P1.Sb = "B.sb (1)";
            d4.P2 = new B();
            d4.P2.Sb = "B.sb (2)";
            return (d4.P2, d4);
        }

        public (B, B, B) ReturnTest1(Current current, CancellationToken cancel)
        {
            (B p1, B p2) = ParamTest1(current, cancel);
            return (p1, p1, p2);
        }

        public (B, B, B) ReturnTest2(Current current, CancellationToken cancel)
        {
            (B p1, B p2) = ParamTest1(current, cancel);
            return (p2, p2, p1);
        }

        public B? ReturnTest3(B? p1, B? p2, Current current, CancellationToken cancel) => p1;

        public SS3 SequenceTest(SS1? p1, SS2? p2, Current current, CancellationToken cancel)
        {
            var ss = new SS3();
            ss.C1 = p1;
            ss.C2 = p2;
            return ss;
        }

        public (IReadOnlyDictionary<int, B?>, IReadOnlyDictionary<int, B?>) DictionaryTest(
            Dictionary<int, B?> bin,
            Current current,
            CancellationToken cancel)
        {
            var bout = new Dictionary<int, B?>();
            int i;
            for (i = 0; i < 10; ++i)
            {
                B? b = bin[i];
                TestHelper.Assert(b != null);
                var d2 = new D2();
                d2.Sb = b.Sb;
                d2.Pb = b.Pb;
                d2.Sd2 = "D2";
                d2.Pd2 = d2;
                bout[i * 10] = d2;
            }
            var r = new Dictionary<int, B?>();
            for (i = 0; i < 10; ++i)
            {
                string s = "D1." + (i * 20).ToString();
                var d1 = new D1();
                d1.Sb = s;
                d1.Pb = i == 0 ? null : r[(i - 1) * 20];
                d1.Sd1 = s;
                d1.Pd1 = d1;
                r[i * 20] = d1;
            }
            return (r, bout);
        }

        public PBase? ExchangePBase(PBase? pb, Current current, CancellationToken cancel) => pb;

        public Preserved PBSUnknownAsPreserved(Current current, CancellationToken cancel) =>
            new PSUnknown(5, "preserved", "unknown", null, new MyClass(15));

        public void CheckPBSUnknown(Preserved? p, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(p is PSUnknown);
            var pu = (PSUnknown)p;
            TestHelper.Assert(pu.Pi == 5);
            TestHelper.Assert(pu.Ps.Equals("preserved"));
            TestHelper.Assert(pu.Psu.Equals("unknown"));
            TestHelper.Assert(pu.Graph == null);
            TestHelper.Assert(pu.Cl != null && pu.Cl.I == 15);
        }

        public ValueTask<Preserved?>
        PBSUnknownAsPreservedWithGraphAsync(Current current, CancellationToken cancel)
        {
            var graph = new PNode();
            graph.Next = new PNode();
            graph.Next.Next = new PNode();
            graph.Next.Next.Next = graph;

            var r = new PSUnknown(5, "preserved", "unknown", graph, null);
            return new ValueTask<Preserved?>(r);
        }

        public void CheckPBSUnknownWithGraph(Preserved? p, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(p is PSUnknown);
            var pu = (PSUnknown)p;
            TestHelper.Assert(pu.Pi == 5);
            TestHelper.Assert(pu.Ps.Equals("preserved"));
            TestHelper.Assert(pu.Psu.Equals("unknown"));
            TestHelper.Assert(pu.Graph != pu.Graph!.Next);
            TestHelper.Assert(pu.Graph.Next != pu.Graph.Next!.Next);
            TestHelper.Assert(pu.Graph.Next.Next!.Next == pu.Graph);
        }

        public ValueTask<Preserved?>
        PBSUnknown2AsPreservedWithGraphAsync(Current current, CancellationToken cancel)
        {
            var r = new PSUnknown2(5, "preserved", null);
            r.Pb = r;
            return new ValueTask<Preserved?>(r);
        }

        public void CheckPBSUnknown2WithGraph(Preserved? p, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(p is PSUnknown2);
            var pu = (PSUnknown2)p;
            TestHelper.Assert(pu.Pi == 5);
            TestHelper.Assert(pu.Ps.Equals("preserved"));
            TestHelper.Assert(pu.Pb == pu);
        }

        public PNode? ExchangePNode(PNode? pn, Current current, CancellationToken cancel) => pn;

        public void ThrowBaseAsBase(Current current, CancellationToken cancel)
        {
            var b = new B("sb", null);
            b.Pb = b;
            throw new BaseException("sbe", b);
        }

        public void ThrowDerivedAsBase(Current current, CancellationToken cancel)
        {
            var b = new B("sb1", null);
            b.Pb = b;

            var d = new D1("sb2", null, "sd2", null);
            d.Pb = d;
            d.Pd1 = d;

            throw new DerivedException("sbe", b, "sde1", d);
        }

        public void ThrowDerivedAsDerived(Current current, CancellationToken cancel)
        {
            var b = new B("sb1", null);
            b.Pb = b;

            var d = new D1("sb2", null, "sd2", null);
            d.Pb = d;
            d.Pd1 = d;

            throw new DerivedException("sbe", b, "sde1", d);
        }

        public void ThrowUnknownDerivedAsBase(Current current, CancellationToken cancel)
        {
            var d2 = new D2();
            d2.Sb = "sb d2";
            d2.Pb = d2;
            d2.Sd2 = "sd2 d2";
            d2.Pd2 = d2;

            throw new UnknownDerivedException("sbe", d2, "sude", d2);
        }

        public ValueTask ThrowPreservedExceptionAsync(Current current, CancellationToken cancel)
        {
            var ue = new PSUnknownException();
            ue.P = new PSUnknown2(5, "preserved", null);
            ue.P.Pb = ue.P;
            throw ue;
        }

        public Forward UseForward(Current current, CancellationToken cancel)
        {
            var f = new Forward();
            f.H = new Hidden();
            f.H.F = f;
            return f;
        }
    }
}
