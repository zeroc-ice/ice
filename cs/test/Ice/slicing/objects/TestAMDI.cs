// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using Test;

public sealed class TestI : TestIntfDisp_
{
    public TestI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }
    
    public override void shutdown_async(AMD_TestIntf_shutdown cb, Ice.Current current)
    {
        _adapter.getCommunicator().shutdown();
        cb.ice_response();
    }
    
    public override void SBaseAsObject_async(AMD_TestIntf_SBaseAsObject cb, Ice.Current current)
    {
        SBase sb = new SBase();
        sb.sb = "SBase.sb";
        cb.ice_response(sb);
    }
    
    public override void SBaseAsSBase_async(AMD_TestIntf_SBaseAsSBase cb, Ice.Current current)
    {
        SBase sb = new SBase();
        sb.sb = "SBase.sb";
        cb.ice_response(sb);
    }
    
    public override void SBSKnownDerivedAsSBase_async(AMD_TestIntf_SBSKnownDerivedAsSBase cb, Ice.Current current)
    {
        SBSKnownDerived sbskd = new SBSKnownDerived();
        sbskd.sb = "SBSKnownDerived.sb";
        sbskd.sbskd = "SBSKnownDerived.sbskd";
        cb.ice_response(sbskd);
    }
    
    public override void SBSKnownDerivedAsSBSKnownDerived_async(AMD_TestIntf_SBSKnownDerivedAsSBSKnownDerived cb, Ice.Current current)
    {
        SBSKnownDerived sbskd = new SBSKnownDerived();
        sbskd.sb = "SBSKnownDerived.sb";
        sbskd.sbskd = "SBSKnownDerived.sbskd";
        cb.ice_response(sbskd);
    }
    
    public override void SBSUnknownDerivedAsSBase_async(AMD_TestIntf_SBSUnknownDerivedAsSBase cb, Ice.Current current)
    {
        SBSUnknownDerived sbsud = new SBSUnknownDerived();
        sbsud.sb = "SBSUnknownDerived.sb";
        sbsud.sbsud = "SBSUnknownDerived.sbsud";
        cb.ice_response(sbsud);
    }
    
    public override void SUnknownAsObject_async(AMD_TestIntf_SUnknownAsObject cb, Ice.Current current)
    {
        SUnknown su = new SUnknown();
        su.su = "SUnknown.su";
        cb.ice_response(su);
    }
    
    public override void oneElementCycle_async(AMD_TestIntf_oneElementCycle cb, Ice.Current current)
    {
        B b = new B();
        b.sb = "B1.sb";
        b.pb = b;
        cb.ice_response(b);
    }
    
    public override void twoElementCycle_async(AMD_TestIntf_twoElementCycle cb, Ice.Current current)
    {
        B b1 = new B();
        b1.sb = "B1.sb";
        B b2 = new B();
        b2.sb = "B2.sb";
        b2.pb = b1;
        b1.pb = b2;
        cb.ice_response(b1);
    }
    
    public override void D1AsB_async(AMD_TestIntf_D1AsB cb, Ice.Current current)
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
    
    public override void D1AsD1_async(AMD_TestIntf_D1AsD1 cb, Ice.Current current)
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
    
    public override void D2AsB_async(AMD_TestIntf_D2AsB cb, Ice.Current current)
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
    
    public override void paramTest1_async(AMD_TestIntf_paramTest1 cb, Ice.Current current)
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
    
    public override void paramTest2_async(AMD_TestIntf_paramTest2 cb, Ice.Current current)
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
    
    public override void paramTest3_async(AMD_TestIntf_paramTest3 cb, Ice.Current current)
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
    
    public override void paramTest4_async(AMD_TestIntf_paramTest4 cb, Ice.Current current)
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
    
    public override void returnTest1_async(AMD_TestIntf_returnTest1 cb, Ice.Current current)
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
    
    public override void returnTest2_async(AMD_TestIntf_returnTest2 cb, Ice.Current current)
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
    
    public override void returnTest3_async(AMD_TestIntf_returnTest3 cb, B p1, B p2, Ice.Current current)
    {
        cb.ice_response(p1);
    }
    
    public override void sequenceTest_async(AMD_TestIntf_sequenceTest cb, SS1 p1, SS2 p2, Ice.Current current)
    {
        SS ss = new SS();
        ss.c1 = p1;
        ss.c2 = p2;
        cb.ice_response(ss);
    }
    
    public override void dictionaryTest_async(AMD_TestIntf_dictionaryTest cb, BDict bin, Ice.Current current)
    {
        BDict bout = new BDict();
        int i;
        for(i = 0; i < 10; ++i)
        {
            B b = bin[i];
            D2 d2 = new D2();
            d2.sb = b.sb;
            d2.pb = b.pb;
            d2.sd2 = "D2";
            d2.pd2 = d2;
            bout[i * 10] = d2;
        }
        BDict r = new BDict();
        for(i = 0; i < 10; ++i)
        {
            string s = "D1." + (i * 20).ToString();
            D1 d1 = new D1();
            d1.sb = s;
            d1.pb = (i == 0 ? null : r[(i - 1) * 20]);
            d1.sd1 = s;
            d1.pd1 = d1;
            r[i * 20] = d1;
        }
        cb.ice_response(r, bout);
    }
    
    public override void throwBaseAsBase_async(AMD_TestIntf_throwBaseAsBase cb, Ice.Current current)
    {
        BaseException be = new BaseException();
        be.sbe = "sbe";
        be.pb = new B();
        be.pb.sb = "sb";
        be.pb.pb = be.pb;
        cb.ice_exception(be);
    }
    
    public override void throwDerivedAsBase_async(AMD_TestIntf_throwDerivedAsBase cb, Ice.Current current)
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
    
    public override void throwDerivedAsDerived_async(AMD_TestIntf_throwDerivedAsDerived cb, Ice.Current current)
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
    
    public override void throwUnknownDerivedAsBase_async(AMD_TestIntf_throwUnknownDerivedAsBase cb, Ice.Current current)
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
    
    public override void useForward_async(AMD_TestIntf_useForward cb, Ice.Current current)
    {
        Forward f = new Forward();
        f = new Forward();
        f.h = new Hidden();
        f.h.f = f;
        cb.ice_response(f);
    }
    
    private Ice.ObjectAdapter _adapter;
}
