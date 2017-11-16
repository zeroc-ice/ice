// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.slicing.objects;

import test.Ice.slicing.objects.serverAMD.Test.*;

public final class AMDTestI extends _TestIntfDisp
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    @Override
    public void
    shutdown_async(AMD_TestIntf_shutdown cb, Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
        cb.ice_response();
    }

    @Override
    public void
    SBaseAsObject_async(AMD_TestIntf_SBaseAsObject cb, Ice.Current current)
    {
        SBase sb = new SBase();
        sb.sb = "SBase.sb";
        cb.ice_response(sb);
    }

    @Override
    public void
    SBaseAsSBase_async(AMD_TestIntf_SBaseAsSBase cb, Ice.Current current)
    {
        SBase sb = new SBase();
        sb.sb = "SBase.sb";
        cb.ice_response(sb);
    }

    @Override
    public void
    SBSKnownDerivedAsSBase_async(AMD_TestIntf_SBSKnownDerivedAsSBase cb, Ice.Current current)
    {
        SBSKnownDerived sbskd = new SBSKnownDerived();
        sbskd.sb = "SBSKnownDerived.sb";
        sbskd.sbskd = "SBSKnownDerived.sbskd";
        cb.ice_response(sbskd);
    }

    @Override
    public void
    SBSKnownDerivedAsSBSKnownDerived_async(AMD_TestIntf_SBSKnownDerivedAsSBSKnownDerived cb, Ice.Current current)
    {
        SBSKnownDerived sbskd = new SBSKnownDerived();
        sbskd.sb = "SBSKnownDerived.sb";
        sbskd.sbskd = "SBSKnownDerived.sbskd";
        cb.ice_response(sbskd);
    }

    @Override
    public void
    SBSUnknownDerivedAsSBase_async(AMD_TestIntf_SBSUnknownDerivedAsSBase cb, Ice.Current current)
    {
        SBSUnknownDerived sbsud = new SBSUnknownDerived();
        sbsud.sb = "SBSUnknownDerived.sb";
        sbsud.sbsud = "SBSUnknownDerived.sbsud";
        cb.ice_response(sbsud);
    }

    @Override
    public void
    SBSUnknownDerivedAsSBaseCompact_async(AMD_TestIntf_SBSUnknownDerivedAsSBaseCompact cb, Ice.Current current)
    {
        SBSUnknownDerived sbsud = new SBSUnknownDerived();
        sbsud.sb = "SBSUnknownDerived.sb";
        sbsud.sbsud = "SBSUnknownDerived.sbsud";
        cb.ice_response(sbsud);
    }

    @Override
    public void
    SUnknownAsObject_async(AMD_TestIntf_SUnknownAsObject cb, Ice.Current current)
    {
        SUnknown su = new SUnknown();
        su.su = "SUnknown.su";
        cb.ice_response(su);
    }

    @Override
    public void
    checkSUnknown_async(AMD_TestIntf_checkSUnknown cb, Ice.Object obj, Ice.Current current)
    {
        if(current.encoding.equals(Ice.Util.Encoding_1_0))
        {
            test(!(obj instanceof SUnknown));
        }
        else
        {
            SUnknown su = (SUnknown)obj;
            test(su.su.equals("SUnknown.su"));
        }
        cb.ice_response();
    }

    @Override
    public void
    oneElementCycle_async(AMD_TestIntf_oneElementCycle cb, Ice.Current current)
    {
        B b = new B();
        b.sb = "B1.sb";
        b.pb = b;
        cb.ice_response(b);
    }

    @Override
    public void
    twoElementCycle_async(AMD_TestIntf_twoElementCycle cb, Ice.Current current)
    {
        B b1 = new B();
        b1.sb = "B1.sb";
        B b2 = new B();
        b2.sb = "B2.sb";
        b2.pb = b1;
        b1.pb = b2;
        cb.ice_response(b1);
    }

    @Override
    public void
    D1AsB_async(AMD_TestIntf_D1AsB cb, Ice.Current current)
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
        cb.ice_response(d1);
    }

    @Override
    public void
    D1AsD1_async(AMD_TestIntf_D1AsD1 cb, Ice.Current current)
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
        cb.ice_response(d1);
    }

    @Override
    public void
    D2AsB_async(AMD_TestIntf_D2AsB cb, Ice.Current current)
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
        cb.ice_response(d2);
    }

    @Override
    public void
    paramTest1_async(AMD_TestIntf_paramTest1 cb, Ice.Current current)
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
        cb.ice_response(d1, d2);
    }

    @Override
    public void
    paramTest2_async(AMD_TestIntf_paramTest2 cb, Ice.Current current)
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
        cb.ice_response(d2, d1);
    }

    @Override
    public void
    paramTest3_async(AMD_TestIntf_paramTest3 cb, Ice.Current current)
    {
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
        cb.ice_response(d3, d2, d4);
    }

    @Override
    public void
    paramTest4_async(AMD_TestIntf_paramTest4 cb, Ice.Current current)
    {
        D4 d4 = new D4();
        d4.sb = "D4.sb (1)";
        d4.pb = null;
        d4.p1 = new B();
        d4.p1.sb = "B.sb (1)";
        d4.p2 = new B();
        d4.p2.sb = "B.sb (2)";
        cb.ice_response(d4.p2, d4);
    }

    @Override
    public void
    returnTest1_async(AMD_TestIntf_returnTest1 cb, Ice.Current current)
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
        cb.ice_response(d2, d2, d1);
    }

    @Override
    public void
    returnTest2_async(AMD_TestIntf_returnTest2 cb, Ice.Current current)
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
        cb.ice_response(d1, d1, d2);
    }

    @Override
    public void
    returnTest3_async(AMD_TestIntf_returnTest3 cb, B p1, B p2, Ice.Current current)
    {
        cb.ice_response(p1);
    }

    @Override
    public void
    sequenceTest_async(AMD_TestIntf_sequenceTest cb, SS1 p1, SS2 p2, Ice.Current current)
    {
        SS3 ss = new SS3();
        ss.c1 = p1;
        ss.c2 = p2;
        cb.ice_response(ss);
    }

    @Override
    public void
    dictionaryTest_async(AMD_TestIntf_dictionaryTest cb, java.util.Map<Integer, B> bin, Ice.Current current)
    {
        java.util.Map<Integer, B> bout = new java.util.HashMap<Integer, B>();
        int i;
        for(i = 0; i < 10; ++i)
        {
            B b = bin.get(i);
            D2 d2 = new D2();
            d2.sb = b.sb;
            d2.pb = b.pb;
            d2.sd2 = "D2";
            d2.pd2 = d2;
            bout.put(i * 10, d2);
        }
        java.util.Map<Integer, B> r = new java.util.HashMap<Integer, B>();
        for(i = 0; i < 10; ++i)
        {
            String s = "D1." + Integer.valueOf(i * 20).toString();
            D1 d1 = new D1();
            d1.sb = s;
            d1.pb = (i == 0 ? null : r.get((i - 1) * 20));
            d1.sd1 = s;
            d1.pd1 = d1;
            r.put(i * 20, d1);
        }
        cb.ice_response(r, bout);
    }

    @Override
    public void
    exchangePBase_async(AMD_TestIntf_exchangePBase cb, PBase pb, Ice.Current current)
    {
        cb.ice_response(pb);
    }

    @Override
    public void
    PBSUnknownAsPreserved_async(AMD_TestIntf_PBSUnknownAsPreserved cb, Ice.Current current)
    {
        PSUnknown r = new PSUnknown();
        r.pi = 5;
        r.ps = "preserved";
        r.psu = "unknown";
        r.graph = null;
        if(!current.encoding.equals(Ice.Util.Encoding_1_0))
        {
            //
            // 1.0 encoding doesn't support unmarshaling unknown classes even if referenced
            // from unread slice.
            //
            r.cl = new MyClass(15);
        }
        cb.ice_response(r);
    }

    @Override
    public void
    checkPBSUnknown_async(AMD_TestIntf_checkPBSUnknown cb, Preserved p, Ice.Current current)
    {
        if(current.encoding.equals(Ice.Util.Encoding_1_0))
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
        cb.ice_response();
    }

    @Override
    public void
    PBSUnknownAsPreservedWithGraph_async(AMD_TestIntf_PBSUnknownAsPreservedWithGraph cb, Ice.Current current)
    {
        PSUnknown r = new PSUnknown();
        r.pi = 5;
        r.ps = "preserved";
        r.psu = "unknown";
        r.graph = new PNode();
        r.graph.next = new PNode();
        r.graph.next.next = new PNode();
        r.graph.next.next.next = r.graph;
        cb.ice_response(r);
        r.graph.next.next.next = null; // Break the cycle.
    }

    @Override
    public void
    checkPBSUnknownWithGraph_async(AMD_TestIntf_checkPBSUnknownWithGraph cb, Preserved p, Ice.Current current)
    {
        if(current.encoding.equals(Ice.Util.Encoding_1_0))
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
        cb.ice_response();
    }

    @Override
    public void
    PBSUnknown2AsPreservedWithGraph_async(AMD_TestIntf_PBSUnknown2AsPreservedWithGraph cb, Ice.Current current)
    {
        PSUnknown2 r = new PSUnknown2();
        r.pi = 5;
        r.ps = "preserved";
        r.pb = r;
        cb.ice_response(r);
        r.pb = null; // Break the cycle.
    }

    @Override
    public void
    checkPBSUnknown2WithGraph_async(AMD_TestIntf_checkPBSUnknown2WithGraph cb, Preserved p, Ice.Current current)
    {
        if(current.encoding.equals(Ice.Util.Encoding_1_0))
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
            pu.pb = null; // Break the cycle.
        }
        cb.ice_response();
    }

    @Override
    public void
    exchangePNode_async(AMD_TestIntf_exchangePNode cb, PNode pn, Ice.Current current)
    {
        cb.ice_response(pn);
    }

    @Override
    public void
    throwBaseAsBase_async(AMD_TestIntf_throwBaseAsBase cb, Ice.Current current)
        throws BaseException
    {
        BaseException be = new BaseException();
        be.sbe = "sbe";
        be.pb = new B();
        be.pb.sb = "sb";
        be.pb.pb = be.pb;
        cb.ice_exception(be);
    }

    @Override
    public void
    throwDerivedAsBase_async(AMD_TestIntf_throwDerivedAsBase cb, Ice.Current current)
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
        cb.ice_exception(de);
    }

    @Override
    public void
    throwDerivedAsDerived_async(AMD_TestIntf_throwDerivedAsDerived cb, Ice.Current current)
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
        cb.ice_exception(de);
    }

    @Override
    public void
    throwUnknownDerivedAsBase_async(AMD_TestIntf_throwUnknownDerivedAsBase cb, Ice.Current current)
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
        cb.ice_exception(ude);
    }

    @Override
    public void
    throwPreservedException_async(AMD_TestIntf_throwPreservedException cb, Ice.Current current)
    {
        PSUnknownException ue = new PSUnknownException();
        ue.p = new PSUnknown2();
        ue.p.pi = 5;
        ue.p.ps = "preserved";
        ue.p.pb = ue.p;
        cb.ice_exception(ue);
        ue.p.pb = null; // Break the cycle.
    }

    @Override
    public void
    useForward_async(AMD_TestIntf_useForward cb, Ice.Current current)
    {
        Forward f = new Forward();
        f = new Forward();
        f.h = new Hidden();
        f.h.f = f;
        cb.ice_response(f);
    }
}
