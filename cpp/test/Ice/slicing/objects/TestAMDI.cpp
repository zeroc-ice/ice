// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestAMDI.h>
#include <Ice/Ice.h>
#include <sstream>

using namespace Test;

TestI::TestI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
{
}

void
TestI::SBaseAsObject_async(const AMD_TestIntf_SBaseAsObjectPtr& cb, const ::Ice::Current&)
{
    SBasePtr sb = new SBase;
    sb->sb = "SBase.sb";
    cb->ice_response(sb);
}

void
TestI::SBaseAsSBase_async(const AMD_TestIntf_SBaseAsSBasePtr& cb, const ::Ice::Current&)
{
    SBasePtr sb = new SBase;
    sb->sb = "SBase.sb";
    cb->ice_response(sb);
}

void
TestI::SBSKnownDerivedAsSBase_async(const AMD_TestIntf_SBSKnownDerivedAsSBasePtr& cb, const ::Ice::Current&)
{
    SBSKnownDerivedPtr sbskd = new SBSKnownDerived;
    sbskd->sb = "SBSKnownDerived.sb";
    sbskd->sbskd = "SBSKnownDerived.sbskd";
    cb->ice_response(sbskd);
}

void
TestI::SBSKnownDerivedAsSBSKnownDerived_async(const AMD_TestIntf_SBSKnownDerivedAsSBSKnownDerivedPtr& cb,
                                              const ::Ice::Current&)
{
    SBSKnownDerivedPtr sbskd = new SBSKnownDerived;
    sbskd->sb = "SBSKnownDerived.sb";
    sbskd->sbskd = "SBSKnownDerived.sbskd";
    cb->ice_response(sbskd);
}

void
TestI::SBSUnknownDerivedAsSBase_async(const AMD_TestIntf_SBSUnknownDerivedAsSBasePtr& cb, const ::Ice::Current&)
{
    SBSUnknownDerivedPtr sbsud = new SBSUnknownDerived;
    sbsud->sb = "SBSUnknownDerived.sb";
    sbsud->sbsud = "SBSUnknownDerived.sbsud";
    cb->ice_response(sbsud);
}

void
TestI::SUnknownAsObject_async(const AMD_TestIntf_SUnknownAsObjectPtr& cb, const ::Ice::Current&)
{
    SUnknownPtr su = new SUnknown;
    su->su = "SUnknown.su";
    cb->ice_response(su);
}

void
TestI::oneElementCycle_async(const AMD_TestIntf_oneElementCyclePtr& cb, const ::Ice::Current&)
{
    BPtr b = new B;
    b->sb = "B1.sb";
    b->pb = b;
    cb->ice_response(b);
}

void
TestI::twoElementCycle_async(const AMD_TestIntf_twoElementCyclePtr& cb, const ::Ice::Current&)
{
    BPtr b1 = new B;
    b1->sb = "B1.sb";
    BPtr b2 = new B;
    b2->sb = "B2.sb";
    b2->pb = b1;
    b1->pb = b2;
    cb->ice_response(b1);
}

void
TestI::D1AsB_async(const AMD_TestIntf_D1AsBPtr& cb, const ::Ice::Current&)
{
    D1Ptr d1 = new D1;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    D2Ptr d2 = new D2;
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    cb->ice_response(d1);
}

void
TestI::D1AsD1_async(const AMD_TestIntf_D1AsD1Ptr& cb, const ::Ice::Current&)
{
    D1Ptr d1 = new D1;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    D2Ptr d2 = new D2;
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    cb->ice_response(d1);
}

void
TestI::D2AsB_async(const AMD_TestIntf_D2AsBPtr& cb, const ::Ice::Current&)
{
    D2Ptr d2 = new D2;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    D1Ptr d1 = new D1;
    d1->pb = d2;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    d1->pd1 = d2;
    d2->pb = d1;
    d2->pd2 = d1;
    cb->ice_response(d2);
}

void
TestI::paramTest1_async(const AMD_TestIntf_paramTest1Ptr& cb, const ::Ice::Current&)
{
    D1Ptr d1 = new D1;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    D2Ptr d2 = new D2;
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    cb->ice_response(d1, d2);
}

void
TestI::paramTest2_async(const AMD_TestIntf_paramTest2Ptr& cb, const ::Ice::Current&)
{
    D1Ptr d1 = new D1;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    D2Ptr d2 = new D2;
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    cb->ice_response(d2, d1);
}

void
TestI::paramTest3_async(const AMD_TestIntf_paramTest3Ptr& cb, const ::Ice::Current&)
{
    D2Ptr d2 = new D2;
    d2->sb = "D2.sb (p1 1)";
    d2->pb = 0;
    d2->sd2 = "D2.sd2 (p1 1)";

    D1Ptr d1 = new D1;
    d1->sb = "D1.sb (p1 2)";
    d1->pb = 0;
    d1->sd1 = "D1.sd2 (p1 2)";
    d1->pd1 = 0;
    d2->pd2 = d1;

    D2Ptr d4 = new D2;
    d4->sb = "D2.sb (p2 1)";
    d4->pb = 0;
    d4->sd2 = "D2.sd2 (p2 1)";

    D1Ptr d3 = new D1;
    d3->sb = "D1.sb (p2 2)";
    d3->pb = 0;
    d3->sd1 = "D1.sd2 (p2 2)";
    d3->pd1 = 0;
    d4->pd2 = d3;

    cb->ice_response(d3, d2, d4);
}

void
TestI::paramTest4_async(const AMD_TestIntf_paramTest4Ptr& cb, const ::Ice::Current&)
{
    D4Ptr d4 = new D4;
    d4->sb = "D4.sb (1)";
    d4->pb = 0;
    d4->p1 = new B;
    d4->p1->sb = "B.sb (1)";
    d4->p2 = new B;
    d4->p2->sb = "B.sb (2)";
    cb->ice_response(d4->p2, d4);
}

void
TestI::returnTest1_async(const AMD_TestIntf_returnTest1Ptr& cb, const ::Ice::Current&)
{
    D1Ptr d1 = new D1;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    D2Ptr d2 = new D2;
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    cb->ice_response(d2, d2, d1);
}

void
TestI::returnTest2_async(const AMD_TestIntf_returnTest2Ptr& cb, const ::Ice::Current&)
{
    D1Ptr d1 = new D1;
    d1->sb = "D1.sb";
    d1->sd1 = "D1.sd1";
    D2Ptr d2 = new D2;
    d2->pb = d1;
    d2->sb = "D2.sb";
    d2->sd2 = "D2.sd2";
    d2->pd2 = d1;
    d1->pb = d2;
    d1->pd1 = d2;
    cb->ice_response(d1, d1, d2);
}

void
TestI::returnTest3_async(const AMD_TestIntf_returnTest3Ptr& cb, const BPtr& p1, const BPtr& p2, const ::Ice::Current&)
{
    cb->ice_response(p1);
}

void
TestI::sequenceTest_async(const AMD_TestIntf_sequenceTestPtr& cb,
                          const SS1Ptr& p1, const SS2Ptr& p2, const ::Ice::Current&)
{
    SS ss;
    ss.c1 = p1;
    ss.c2 = p2;
    cb->ice_response(ss);
}

void
TestI::dictionaryTest_async(const AMD_TestIntf_dictionaryTestPtr& cb, const BDict& bin, const ::Ice::Current&)
{
    BDict bout;
    int i;
    for(i = 0; i < 10; ++i)
    {
        BPtr b = bin.find(i)->second;
        D2Ptr d2 = new D2;
        d2->sb = b->sb;
        d2->pb = b->pb;
        d2->sd2 = "D2";
        d2->pd2 = d2;
        bout[i * 10] = d2;
    }
    BDict r;
    for(i = 0; i < 10; ++i)
    {
        std::ostringstream s;
        s << "D1." << i * 20;
        D1Ptr d1 = new D1;
        d1->sb = s.str();
        d1->pb = (i == 0 ? BPtr(0) : r.find((i - 1) * 20)->second);
        d1->sd1 = s.str();
        d1->pd1 = d1;
        r[i * 20] = d1;
    }
    cb->ice_response(r, bout);
}

void
TestI::throwBaseAsBase_async(const AMD_TestIntf_throwBaseAsBasePtr& cb, const ::Ice::Current&)
{
    BaseException be;
    be.sbe = "sbe";
    be.pb = new B;
    be.pb->sb = "sb";
    be.pb->pb = be.pb;
    cb->ice_exception(be);
}

void
TestI::throwDerivedAsBase_async(const AMD_TestIntf_throwDerivedAsBasePtr& cb, const ::Ice::Current&)
{
    DerivedException de;
    de.sbe = "sbe";
    de.pb = new B;
    de.pb->sb = "sb1";
    de.pb->pb = de.pb;
    de.sde = "sde1";
    de.pd1 = new D1;
    de.pd1->sb = "sb2";
    de.pd1->pb = de.pd1;
    de.pd1->sd1 = "sd2";
    de.pd1->pd1 = de.pd1;
    cb->ice_exception(de);
}

void
TestI::throwDerivedAsDerived_async(const AMD_TestIntf_throwDerivedAsDerivedPtr& cb, const ::Ice::Current&)
{
    DerivedException de;
    de.sbe = "sbe";
    de.pb = new B;
    de.pb->sb = "sb1";
    de.pb->pb = de.pb;
    de.sde = "sde1";
    de.pd1 = new D1;
    de.pd1->sb = "sb2";
    de.pd1->pb = de.pd1;
    de.pd1->sd1 = "sd2";
    de.pd1->pd1 = de.pd1;
    cb->ice_exception(de);
}

void
TestI::throwUnknownDerivedAsBase_async(const AMD_TestIntf_throwUnknownDerivedAsBasePtr& cb, const ::Ice::Current&)
{
    D2Ptr d2 = new D2;
    d2->sb = "sb d2";
    d2->pb = d2;
    d2->sd2 = "sd2 d2";
    d2->pd2 = d2;

    UnknownDerivedException ude;
    ude.sbe = "sbe";
    ude.pb = d2;
    ude.sude = "sude";
    ude.pd2 = d2;
    cb->ice_exception(ude);
}

void
TestI::useForward_async(const AMD_TestIntf_useForwardPtr& cb, const ::Ice::Current&)
{
    ForwardPtr f = new Forward;
    f->h = new Hidden;
    f->h->f = f;
    cb->ice_response(f);
}

void
TestI::shutdown_async(const AMD_TestIntf_shutdownPtr& cb, const ::Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
    cb->ice_response();
}
