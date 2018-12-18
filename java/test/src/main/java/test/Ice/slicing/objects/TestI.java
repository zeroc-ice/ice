// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.slicing.objects;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import test.Ice.slicing.objects.server.Test.*;

public final class TestI implements TestIntf
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public com.zeroc.Ice.Value SBaseAsObject(com.zeroc.Ice.Current current)
    {
        SBase sb = new SBase();
        sb.sb = "SBase.sb";
        return sb;
    }

    @Override
    public SBase SBaseAsSBase(com.zeroc.Ice.Current current)
    {
        SBase sb = new SBase();
        sb.sb = "SBase.sb";
        return sb;
    }

    @Override
    public SBase SBSKnownDerivedAsSBase(com.zeroc.Ice.Current current)
    {
        SBSKnownDerived sbskd = new SBSKnownDerived();
        sbskd.sb = "SBSKnownDerived.sb";
        sbskd.sbskd = "SBSKnownDerived.sbskd";
        return sbskd;
    }

    @Override
    public SBSKnownDerived SBSKnownDerivedAsSBSKnownDerived(com.zeroc.Ice.Current current)
    {
        SBSKnownDerived sbskd = new SBSKnownDerived();
        sbskd.sb = "SBSKnownDerived.sb";
        sbskd.sbskd = "SBSKnownDerived.sbskd";
        return sbskd;
    }

    @Override
    public SBase SBSUnknownDerivedAsSBase(com.zeroc.Ice.Current current)
    {
        SBSUnknownDerived sbsud = new SBSUnknownDerived();
        sbsud.sb = "SBSUnknownDerived.sb";
        sbsud.sbsud = "SBSUnknownDerived.sbsud";
        return sbsud;
    }

    @Override
    public SBase SBSUnknownDerivedAsSBaseCompact(com.zeroc.Ice.Current current)
    {
        SBSUnknownDerived sbsud = new SBSUnknownDerived();
        sbsud.sb = "SBSUnknownDerived.sb";
        sbsud.sbsud = "SBSUnknownDerived.sbsud";
        return sbsud;
    }

    @Override
    public com.zeroc.Ice.Value SUnknownAsObject(com.zeroc.Ice.Current current)
    {
        SUnknown su = new SUnknown();
        su.su = "SUnknown.su";
        su.cycle = su;
        return su;
    }

    @Override
    public void checkSUnknown(com.zeroc.Ice.Value obj, com.zeroc.Ice.Current current)
    {
        if(current.encoding.equals(com.zeroc.Ice.Util.Encoding_1_0))
        {
            test(!(obj instanceof SUnknown));
        }
        else
        {
            SUnknown su = (SUnknown)obj;
            test(su.su.equals("SUnknown.su"));
        }
    }

    @Override
    public B oneElementCycle(com.zeroc.Ice.Current current)
    {
        B b = new B();
        b.sb = "B1.sb";
        b.pb = b;
        return b;
    }

    @Override
    public B twoElementCycle(com.zeroc.Ice.Current current)
    {
        B b1 = new B();
        b1.sb = "B1.sb";
        B b2 = new B();
        b2.sb = "B2.sb";
        b2.pb = b1;
        b1.pb = b2;
        return b1;
    }

    @Override
    public B D1AsB(com.zeroc.Ice.Current current)
    {
        D1 d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        D2 d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        return d1;
    }

    @Override
    public D1 D1AsD1(com.zeroc.Ice.Current current)
    {
        D1 d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        D2 d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        return d1;
    }

    @Override
    public B D2AsB(com.zeroc.Ice.Current current)
    {
        D2 d2 = new D2();
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        D1 d1 = new D1();
        d1.pb = d2;
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        d1.pd1 = d2;
        d2.pb = d1;
        d2.pd2 = d1;
        return d2;
    }

    @Override
    public TestIntf.ParamTest1Result paramTest1(com.zeroc.Ice.Current current)
    {
        D1 d1 = new D1();
        d1.sb = "D1.sb";
        d1.sd1 = "D1.sd1";
        D2 d2 = new D2();
        d2.pb = d1;
        d2.sb = "D2.sb";
        d2.sd2 = "D2.sd2";
        d2.pd2 = d1;
        d1.pb = d2;
        d1.pd1 = d2;
        return new TestIntf.ParamTest1Result(d1, d2);
    }

    @Override
    public TestIntf.ParamTest2Result paramTest2(com.zeroc.Ice.Current current)
    {
        TestIntf.ParamTest1Result r1 = paramTest1(current);
        return new TestIntf.ParamTest2Result(r1.p2, r1.p1);
    }

    @Override
    public TestIntf.ParamTest3Result paramTest3(com.zeroc.Ice.Current current)
    {
        TestIntf.ParamTest3Result r = new TestIntf.ParamTest3Result();

        D2 d2 = new D2();
        d2.sb = "D2.sb (p1 1)";
        d2.pb = null;
        d2.sd2 = "D2.sd2 (p1 1)";
        r.p1 = d2;

        D1 d1 = new D1();
        d1.sb = "D1.sb (p1 2)";
        d1.pb = null;
        d1.sd1 = "D1.sd2 (p1 2)";
        d1.pd1 = null;
        d2.pd2 = d1;

        D2 d4 = new D2();
        d4.sb = "D2.sb (p2 1)";
        d4.pb = null;
        d4.sd2 = "D2.sd2 (p2 1)";
        r.p2 = d4;

        D1 d3 = new D1();
        d3.sb = "D1.sb (p2 2)";
        d3.pb = null;
        d3.sd1 = "D1.sd2 (p2 2)";
        d3.pd1 = null;
        d4.pd2 = d3;
        r.returnValue = d3;

        return r;
    }

    @Override
    public TestIntf.ParamTest4Result paramTest4(com.zeroc.Ice.Current current)
    {
        D4 d4 = new D4();
        d4.sb = "D4.sb (1)";
        d4.pb = null;
        d4.p1 = new B();
        d4.p1.sb = "B.sb (1)";
        d4.p2 = new B();
        d4.p2.sb = "B.sb (2)";

        return new TestIntf.ParamTest4Result(d4.p2, d4);
    }

    @Override
    public TestIntf.ReturnTest1Result returnTest1(com.zeroc.Ice.Current current)
    {
        TestIntf.ParamTest1Result r1 = paramTest1(current);
        return new TestIntf.ReturnTest1Result(r1.p1, r1.p1, r1.p2);
    }

    @Override
    public TestIntf.ReturnTest2Result returnTest2(com.zeroc.Ice.Current current)
    {
        TestIntf.ParamTest1Result r1 = paramTest1(current);
        return new TestIntf.ReturnTest2Result(r1.p1, r1.p1, r1.p2);
    }

    @Override
    public B returnTest3(B p1, B p2, com.zeroc.Ice.Current current)
    {
        return p1;
    }

    @Override
    public SS3 sequenceTest(SS1 p1, SS2 p2, com.zeroc.Ice.Current current)
    {
        SS3 ss = new SS3();
        ss.c1 = p1;
        ss.c2 = p2;
        return ss;
    }

    @Override
    public TestIntf.DictionaryTestResult dictionaryTest(java.util.Map<Integer, B> bin, com.zeroc.Ice.Current current)
    {
        TestIntf.DictionaryTestResult r = new TestIntf.DictionaryTestResult();
        r.bout = new java.util.HashMap<>();
        int i;
        for(i = 0; i < 10; ++i)
        {
            B b = bin.get(i);
            D2 d2 = new D2();
            d2.sb = b.sb;
            d2.pb = b.pb;
            d2.sd2 = "D2";
            d2.pd2 = d2;
            r.bout.put(i * 10, d2);
        }
        r.returnValue = new java.util.HashMap<>();
        for(i = 0; i < 10; ++i)
        {
            String s = "D1." + Integer.valueOf(i * 20).toString();
            D1 d1 = new D1();
            d1.sb = s;
            d1.pb = (i == 0 ? null : r.returnValue.get((i - 1) * 20));
            d1.sd1 = s;
            d1.pd1 = d1;
            r.returnValue.put(i * 20, d1);
        }
        return r;
    }

    @Override
    public PBase exchangePBase(PBase pb, com.zeroc.Ice.Current current)
    {
        return pb;
    }

    @Override
    public Preserved PBSUnknownAsPreserved(com.zeroc.Ice.Current current)
    {
        PSUnknown r = new PSUnknown();
        r.pi = 5;
        r.ps = "preserved";
        r.psu = "unknown";
        r.graph = null;
        if(!current.encoding.equals(com.zeroc.Ice.Util.Encoding_1_0))
        {
            //
            // 1.0 encoding doesn't support unmarshaling unknown classes even if referenced
            // from unread slice.
            //
            r.cl = new MyClass(15);
        }
        return r;
    }

    @Override
    public void checkPBSUnknown(Preserved p, com.zeroc.Ice.Current current)
    {
        if(current.encoding.equals(com.zeroc.Ice.Util.Encoding_1_0))
        {
            test(!(p instanceof PSUnknown));
            test(p.pi == 5);
            test(p.ps.equals("preserved"));
        }
        else
        {
            PSUnknown pu = (PSUnknown)p;
            test(pu.pi == 5);
            test(pu.ps.equals("preserved"));
            test(pu.psu.equals("unknown"));
            test(pu.graph == null);
            test(pu.cl != null && pu.cl.i == 15);
        }
    }

    @Override
    public CompletionStage<Preserved> PBSUnknownAsPreservedWithGraphAsync(com.zeroc.Ice.Current current)
    {
        PSUnknown r = new PSUnknown();
        r.pi = 5;
        r.ps = "preserved";
        r.psu = "unknown";
        r.graph = new PNode();
        r.graph.next = new PNode();
        r.graph.next.next = new PNode();
        r.graph.next.next.next = r.graph;
        return CompletableFuture.completedFuture(r);
        //
        // Can't do this with the Java 8 mapping:
        //
        //r.graph.next.next.next = null; // Break the cycle.
    }

    @Override
    public void checkPBSUnknownWithGraph(Preserved p, com.zeroc.Ice.Current current)
    {
        if(current.encoding.equals(com.zeroc.Ice.Util.Encoding_1_0))
        {
            test(!(p instanceof PSUnknown));
            test(p.pi == 5);
            test(p.ps.equals("preserved"));
        }
        else
        {
            PSUnknown pu = (PSUnknown)p;
            test(pu.pi == 5);
            test(pu.ps.equals("preserved"));
            test(pu.psu.equals("unknown"));
            test(pu.graph != pu.graph.next);
            test(pu.graph.next != pu.graph.next.next);
            test(pu.graph.next.next.next == pu.graph);
            pu.graph.next.next.next = null;          // Break the cycle.
        }
    }

    @Override
    public CompletionStage<Preserved> PBSUnknown2AsPreservedWithGraphAsync(com.zeroc.Ice.Current current)
    {
        PSUnknown2 r = new PSUnknown2();
        r.pi = 5;
        r.ps = "preserved";
        r.pb = r;
        return CompletableFuture.completedFuture(r);
        //
        // Can't do this with the Java 8 mapping:
        //
        //r.pb = null; // Break the cycle.
    }

    @Override
    public void checkPBSUnknown2WithGraph(Preserved p, com.zeroc.Ice.Current current)
    {
        if(current.encoding.equals(com.zeroc.Ice.Util.Encoding_1_0))
        {
            test(!(p instanceof PSUnknown2));
            test(p.pi == 5);
            test(p.ps.equals("preserved"));
        }
        else
        {
            PSUnknown2 pu = (PSUnknown2)p;
            test(pu.pi == 5);
            test(pu.ps.equals("preserved"));
            test(pu.pb == pu);
            pu.pb = null;          // Break the cycle.
        }
    }

    @Override
    public PNode exchangePNode(PNode pn, com.zeroc.Ice.Current current)
    {
        return pn;
    }

    @Override
    public void throwBaseAsBase(com.zeroc.Ice.Current current)
        throws BaseException
    {
        BaseException be = new BaseException();
        be.sbe = "sbe";
        be.pb = new B();
        be.pb.sb = "sb";
        be.pb.pb = be.pb;
        throw be;
    }

    @Override
    public void throwDerivedAsBase(com.zeroc.Ice.Current current)
        throws BaseException
    {
        DerivedException de = new DerivedException();
        de.sbe = "sbe";
        de.pb = new B();
        de.pb.sb = "sb1";
        de.pb.pb = de.pb;
        de.sde = "sde1";
        de.pd1 = new D1();
        de.pd1.sb = "sb2";
        de.pd1.pb = de.pd1;
        de.pd1.sd1 = "sd2";
        de.pd1.pd1 = de.pd1;
        throw de;
    }

    @Override
    public void throwDerivedAsDerived(com.zeroc.Ice.Current current)
        throws DerivedException
    {
        DerivedException de = new DerivedException();
        de.sbe = "sbe";
        de.pb = new B();
        de.pb.sb = "sb1";
        de.pb.pb = de.pb;
        de.sde = "sde1";
        de.pd1 = new D1();
        de.pd1.sb = "sb2";
        de.pd1.pb = de.pd1;
        de.pd1.sd1 = "sd2";
        de.pd1.pd1 = de.pd1;
        throw de;
    }

    @Override
    public void throwUnknownDerivedAsBase(com.zeroc.Ice.Current current)
        throws BaseException
    {
        D2 d2 = new D2();
        d2.sb = "sb d2";
        d2.pb = d2;
        d2.sd2 = "sd2 d2";
        d2.pd2 = d2;

        UnknownDerivedException ude = new UnknownDerivedException();
        ude.sbe = "sbe";
        ude.pb = d2;
        ude.sude = "sude";
        ude.pd2 = d2;
        throw ude;
    }

    @Override
    public CompletionStage<Void> throwPreservedExceptionAsync(com.zeroc.Ice.Current current)
    {
        PSUnknownException ue = new PSUnknownException();
        ue.p = new PSUnknown2();
        ue.p.pi = 5;
        ue.p.ps = "preserved";
        ue.p.pb = ue.p;
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(ue);
        return r;
        //
        // Can't do this with the Java 8 mapping:
        //
        //ue.p.pb = null; // Break the cycle.
    }

    @Override
    public Forward useForward(com.zeroc.Ice.Current current)
    {
        Forward f = new Forward();
        f.h = new Hidden();
        f.h.f = f;
        return f;
    }
}
