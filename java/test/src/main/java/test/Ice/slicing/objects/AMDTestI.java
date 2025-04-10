// Copyright (c) ZeroC, Inc.

package test.Ice.slicing.objects;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.Util;
import com.zeroc.Ice.Value;

import test.Ice.slicing.objects.serverAMD.Test.B;
import test.Ice.slicing.objects.serverAMD.Test.BaseException;
import test.Ice.slicing.objects.serverAMD.Test.D1;
import test.Ice.slicing.objects.serverAMD.Test.D2;
import test.Ice.slicing.objects.serverAMD.Test.D4;
import test.Ice.slicing.objects.serverAMD.Test.DerivedException;
import test.Ice.slicing.objects.serverAMD.Test.Forward;
import test.Ice.slicing.objects.serverAMD.Test.Hidden;
import test.Ice.slicing.objects.serverAMD.Test.MyClass;
import test.Ice.slicing.objects.serverAMD.Test.PBase;
import test.Ice.slicing.objects.serverAMD.Test.PNode;
import test.Ice.slicing.objects.serverAMD.Test.PSUnknown;
import test.Ice.slicing.objects.serverAMD.Test.PSUnknown2;
import test.Ice.slicing.objects.serverAMD.Test.PSUnknownException;
import test.Ice.slicing.objects.serverAMD.Test.Preserved;
import test.Ice.slicing.objects.serverAMD.Test.SBSKnownDerived;
import test.Ice.slicing.objects.serverAMD.Test.SBSUnknownDerived;
import test.Ice.slicing.objects.serverAMD.Test.SBase;
import test.Ice.slicing.objects.serverAMD.Test.SS1;
import test.Ice.slicing.objects.serverAMD.Test.SS2;
import test.Ice.slicing.objects.serverAMD.Test.SS3;
import test.Ice.slicing.objects.serverAMD.Test.SUnknown;
import test.Ice.slicing.objects.serverAMD.Test.TestIntf;
import test.Ice.slicing.objects.serverAMD.Test.UnknownDerivedException;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public final class AMDTestI implements TestIntf {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    @Override
    public CompletionStage<Void> shutdownAsync(Current current) {
        current.adapter.getCommunicator().shutdown();
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Value> SBaseAsObjectAsync(Current current) {
        SBase sb = new SBase();
        sb.sb = "SBase.sb";
        return CompletableFuture.completedFuture(sb);
    }

    @Override
    public CompletionStage<SBase> SBaseAsSBaseAsync(Current current) {
        SBase sb = new SBase();
        sb.sb = "SBase.sb";
        return CompletableFuture.completedFuture(sb);
    }

    @Override
    public CompletionStage<SBase> SBSKnownDerivedAsSBaseAsync(Current current) {
        SBSKnownDerived sbskd = new SBSKnownDerived();
        sbskd.sb = "SBSKnownDerived.sb";
        sbskd.sbskd = "SBSKnownDerived.sbskd";
        return CompletableFuture.completedFuture(sbskd);
    }

    @Override
    public CompletionStage<SBSKnownDerived> SBSKnownDerivedAsSBSKnownDerivedAsync(
            Current current) {
        SBSKnownDerived sbskd = new SBSKnownDerived();
        sbskd.sb = "SBSKnownDerived.sb";
        sbskd.sbskd = "SBSKnownDerived.sbskd";
        return CompletableFuture.completedFuture(sbskd);
    }

    @Override
    public CompletionStage<SBase> SBSUnknownDerivedAsSBaseAsync(Current current) {
        SBSUnknownDerived sbsud = new SBSUnknownDerived();
        sbsud.sb = "SBSUnknownDerived.sb";
        sbsud.sbsud = "SBSUnknownDerived.sbsud";
        return CompletableFuture.completedFuture(sbsud);
    }

    @Override
    public CompletionStage<SBase> SBSUnknownDerivedAsSBaseCompactAsync(
            Current current) {
        SBSUnknownDerived sbsud = new SBSUnknownDerived();
        sbsud.sb = "SBSUnknownDerived.sb";
        sbsud.sbsud = "SBSUnknownDerived.sbsud";
        return CompletableFuture.completedFuture(sbsud);
    }

    @Override
    public CompletionStage<Value> SUnknownAsObjectAsync(
            Current current) {
        SUnknown su = new SUnknown();
        su.su = "SUnknown.su";
        return CompletableFuture.completedFuture(su);
    }

    @Override
    public CompletionStage<Void> checkSUnknownAsync(
            Value obj, Current current) {
        if (current.encoding.equals(Util.Encoding_1_0)) {
            test(!(obj instanceof SUnknown));
        } else {
            SUnknown su = (SUnknown) obj;
            test("SUnknown.su".equals(su.su));
        }
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<B> oneElementCycleAsync(Current current) {
        B b = new B();
        b.sb = "B1.sb";
        b.pb = b;
        return CompletableFuture.completedFuture(b);
    }

    @Override
    public CompletionStage<B> twoElementCycleAsync(Current current) {
        B b1 = new B();
        b1.sb = "B1.sb";
        B b2 = new B();
        b2.sb = "B2.sb";
        b2.pb = b1;
        b1.pb = b2;
        return CompletableFuture.completedFuture(b1);
    }

    @Override
    public CompletionStage<B> D1AsBAsync(Current current) {
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
        return CompletableFuture.completedFuture(d1);
    }

    @Override
    public CompletionStage<D1> D1AsD1Async(Current current) {
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
        return CompletableFuture.completedFuture(d1);
    }

    @Override
    public CompletionStage<B> D2AsBAsync(Current current) {
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
        return CompletableFuture.completedFuture(d2);
    }

    @Override
    public CompletionStage<TestIntf.ParamTest1Result> paramTest1Async(
            Current current) {
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
        return CompletableFuture.completedFuture(new TestIntf.ParamTest1Result(d1, d2));
    }

    @Override
    public CompletionStage<TestIntf.ParamTest2Result> paramTest2Async(
            Current current) {
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
        return CompletableFuture.completedFuture(new TestIntf.ParamTest2Result(d2, d1));
    }

    @Override
    public CompletionStage<TestIntf.ParamTest3Result> paramTest3Async(
            Current current) {
        D2 d2 = new D2();
        d2.sb = "D2.sb (p1 1)";
        d2.pb = null;
        d2.sd2 = "D2.sd2 (p1 1)";

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

        D1 d3 = new D1();
        d3.sb = "D1.sb (p2 2)";
        d3.pb = null;
        d3.sd1 = "D1.sd2 (p2 2)";
        d3.pd1 = null;
        d4.pd2 = d3;
        return CompletableFuture.completedFuture(new TestIntf.ParamTest3Result(d3, d2, d4));
    }

    @Override
    public CompletionStage<TestIntf.ParamTest4Result> paramTest4Async(
            Current current) {
        D4 d4 = new D4();
        d4.sb = "D4.sb (1)";
        d4.pb = null;
        d4.p1 = new B();
        d4.p1.sb = "B.sb (1)";
        d4.p2 = new B();
        d4.p2.sb = "B.sb (2)";
        return CompletableFuture.completedFuture(new TestIntf.ParamTest4Result(d4.p2, d4));
    }

    @Override
    public CompletionStage<TestIntf.ReturnTest1Result> returnTest1Async(
            Current current) {
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
        return CompletableFuture.completedFuture(new TestIntf.ReturnTest1Result(d2, d2, d1));
    }

    @Override
    public CompletionStage<TestIntf.ReturnTest2Result> returnTest2Async(
            Current current) {
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
        return CompletableFuture.completedFuture(new TestIntf.ReturnTest2Result(d1, d1, d2));
    }

    @Override
    public CompletionStage<B> returnTest3Async(B p1, B p2, Current current) {
        return CompletableFuture.completedFuture(p1);
    }

    @Override
    public CompletionStage<SS3> sequenceTestAsync(SS1 p1, SS2 p2, Current current) {
        SS3 ss = new SS3();
        ss.c1 = p1;
        ss.c2 = p2;
        return CompletableFuture.completedFuture(ss);
    }

    @Override
    public CompletionStage<TestIntf.DictionaryTestResult> dictionaryTestAsync(
            Map<Integer, B> bin, Current current) {
        TestIntf.DictionaryTestResult r = new TestIntf.DictionaryTestResult();
        r.bout = new HashMap<>();
        int i;
        for (i = 0; i < 10; i++) {
            B b = bin.get(i);
            D2 d2 = new D2();
            d2.sb = b.sb;
            d2.pb = b.pb;
            d2.sd2 = "D2";
            d2.pd2 = d2;
            r.bout.put(i * 10, d2);
        }
        r.returnValue = new HashMap<>();
        for (i = 0; i < 10; i++) {
            String s = "D1." + Integer.valueOf(i * 20).toString();
            D1 d1 = new D1();
            d1.sb = s;
            d1.pb = i == 0 ? null : r.returnValue.get((i - 1) * 20);
            d1.sd1 = s;
            d1.pd1 = d1;
            r.returnValue.put(i * 20, d1);
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<PBase> exchangePBaseAsync(PBase pb, Current current) {
        return CompletableFuture.completedFuture(pb);
    }

    @Override
    public CompletionStage<Preserved> PBSUnknownAsPreservedAsync(Current current) {
        PSUnknown r = new PSUnknown();
        r.pi = 5;
        r.ps = "preserved";
        r.psu = "unknown";
        r.graph = null;
        if (!current.encoding.equals(Util.Encoding_1_0)) {
            //
            // 1.0 encoding doesn't support unmarshaling unknown classes even if referenced
            // from unread slice.
            //
            r.cl = new MyClass(15);
        }
        return CompletableFuture.completedFuture(r);
    }

    @Override
    public CompletionStage<Void> checkPBSUnknownAsync(Preserved p, Current current) {
        if (current.encoding.equals(Util.Encoding_1_0)) {
            test(!(p instanceof PSUnknown));
            test(p.pi == 5);
            test("preserved".equals(p.ps));
        } else {
            PSUnknown pu = (PSUnknown) p;
            test(pu.pi == 5);
            test("preserved".equals(pu.ps));
            test("unknown".equals(pu.psu));
            test(pu.graph == null);
            test(pu.cl != null && pu.cl.i == 15);
        }
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Preserved> PBSUnknownAsPreservedWithGraphAsync(
            Current current) {
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
        // r.graph.next.next.next = null; // Break the cycle.
    }

    @Override
    public CompletionStage<Void> checkPBSUnknownWithGraphAsync(
            Preserved p, Current current) {
        if (current.encoding.equals(Util.Encoding_1_0)) {
            test(!(p instanceof PSUnknown));
            test(p.pi == 5);
            test("preserved".equals(p.ps));
        } else {
            PSUnknown pu = (PSUnknown) p;
            test(pu.pi == 5);
            test("preserved".equals(pu.ps));
            test("unknown".equals(pu.psu));
            test(pu.graph != pu.graph.next);
            test(pu.graph.next != pu.graph.next.next);
            test(pu.graph.next.next.next == pu.graph);
            pu.graph.next.next.next = null; // Break the cycle.
        }
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Preserved> PBSUnknown2AsPreservedWithGraphAsync(
            Current current) {
        PSUnknown2 r = new PSUnknown2();
        r.pi = 5;
        r.ps = "preserved";
        r.pb = r;
        return CompletableFuture.completedFuture(r);
        //
        // Can't do this with the Java 8 mapping:
        //
        // r.pb = null; // Break the cycle.
    }

    @Override
    public CompletionStage<Void> checkPBSUnknown2WithGraphAsync(
            Preserved p, Current current) {
        if (current.encoding.equals(Util.Encoding_1_0)) {
            test(!(p instanceof PSUnknown2));
            test(p.pi == 5);
            test("preserved".equals(p.ps));
        } else {
            PSUnknown2 pu = (PSUnknown2) p;
            test(pu.pi == 5);
            test("preserved".equals(pu.ps));
            test(pu.pb == pu);
            pu.pb = null; // Break the cycle.
        }
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<PNode> exchangePNodeAsync(PNode pn, Current current) {
        return CompletableFuture.completedFuture(pn);
    }

    @Override
    public CompletionStage<Void> throwBaseAsBaseAsync(Current current)
        throws BaseException {
        BaseException be = new BaseException();
        be.sbe = "sbe";
        be.pb = new B();
        be.pb.sb = "sb";
        be.pb.pb = be.pb;
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(be);
        return r;
    }

    @Override
    public CompletionStage<Void> throwDerivedAsBaseAsync(Current current)
        throws BaseException {
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
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(de);
        return r;
    }

    @Override
    public CompletionStage<Void> throwDerivedAsDerivedAsync(Current current)
        throws DerivedException {
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
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(de);
        return r;
    }

    @Override
    public CompletionStage<Void> throwUnknownDerivedAsBaseAsync(Current current)
        throws BaseException {
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
        CompletableFuture<Void> r = new CompletableFuture<>();
        r.completeExceptionally(ude);
        return r;
    }

    @Override
    public CompletionStage<Void> throwPreservedExceptionAsync(Current current) {
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
        // ue.p.pb = null; // Break the cycle.
    }

    @Override
    public CompletionStage<Forward> useForwardAsync(Current current) {
        Forward f = new Forward();
        f = new Forward();
        f.h = new Hidden();
        f.h.f = f;
        return CompletableFuture.completedFuture(f);
    }
}
