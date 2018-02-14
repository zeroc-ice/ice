// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestI.h>
#include <Ice/Ice.h>
#include <TestCommon.h>

using namespace Test;

TestI::TestI()
{
}

Ice::ObjectPtr
TestI::SBaseAsObject(const ::Ice::Current&)
{
    SBasePtr sb = new SBase;
    sb->sb = "SBase.sb";
    return sb;
}

SBasePtr
TestI::SBaseAsSBase(const ::Ice::Current&)
{
    SBasePtr sb = new SBase;
    sb->sb = "SBase.sb";
    return sb;
}

SBasePtr
TestI::SBSKnownDerivedAsSBase(const ::Ice::Current&)
{
    SBSKnownDerivedPtr sbskd = new SBSKnownDerived;
    sbskd->sb = "SBSKnownDerived.sb";
    sbskd->sbskd = "SBSKnownDerived.sbskd";
    return sbskd;
}

SBSKnownDerivedPtr
TestI::SBSKnownDerivedAsSBSKnownDerived(const ::Ice::Current&)
{
    SBSKnownDerivedPtr sbskd = new SBSKnownDerived;
    sbskd->sb = "SBSKnownDerived.sb";
    sbskd->sbskd = "SBSKnownDerived.sbskd";
    return sbskd;
}

SBasePtr
TestI::SBSUnknownDerivedAsSBase(const ::Ice::Current&)
{
    SBSUnknownDerivedPtr sbsud = new SBSUnknownDerived;
    sbsud->sb = "SBSUnknownDerived.sb";
    sbsud->sbsud = "SBSUnknownDerived.sbsud";
    return sbsud;
}

SBasePtr
TestI::SBSUnknownDerivedAsSBaseCompact(const ::Ice::Current&)
{
    SBSUnknownDerivedPtr sbsud = new SBSUnknownDerived;
    sbsud->sb = "SBSUnknownDerived.sb";
    sbsud->sbsud = "SBSUnknownDerived.sbsud";
    return sbsud;
}

Ice::ObjectPtr
TestI::SUnknownAsObject(const ::Ice::Current&)
{
    SUnknownPtr su = new SUnknown;
    su->su = "SUnknown.su";
    return su;
}

void
TestI::checkSUnknown(const Ice::ObjectPtr& obj, const ::Ice::Current& current)
{
    SUnknownPtr su = SUnknownPtr::dynamicCast(obj);
    if(current.encoding == Ice::Encoding_1_0)
    {
        test(!su);
    }
    else
    {
        test(su);
        test(su->su == "SUnknown.su");
    }
}

BPtr
TestI::oneElementCycle(const ::Ice::Current&)
{
    BPtr b = new B;
    b->sb = "B1.sb";
    b->pb = b;
    b->ice_collectable(true);
    return b;
}

BPtr
TestI::twoElementCycle(const ::Ice::Current&)
{
    BPtr b1 = new B;
    b1->sb = "B1.sb";
    BPtr b2 = new B;
    b2->sb = "B2.sb";
    b2->pb = b1;
    b1->pb = b2;
    b1->ice_collectable(true);
    return b1;
}

BPtr
TestI::D1AsB(const ::Ice::Current&)
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
    d1->ice_collectable(true);
    return d1;
}

D1Ptr
TestI::D1AsD1(const ::Ice::Current&)
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
    d1->ice_collectable(true);
    return d1;
}

BPtr
TestI::D2AsB(const ::Ice::Current&)
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
    d1->ice_collectable(true);
    return d2;
}

void
TestI::paramTest1(BPtr& p1, BPtr& p2, const ::Ice::Current&)
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
    p1 = d1;
    p2 = d2;
    d1->ice_collectable(true);
}

void
TestI::paramTest2(BPtr& p1, BPtr& p2, const ::Ice::Current&)
{
    ::Ice::Current c;
    paramTest1(p2, p1, c);
}

BPtr
TestI::paramTest3(BPtr& p1, BPtr& p2, const ::Ice::Current&)
{
    D2Ptr d2 = new D2;
    d2->sb = "D2.sb (p1 1)";
    d2->pb = 0;
    d2->sd2 = "D2.sd2 (p1 1)";
    p1 = d2;

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
    p2 = d4;

    D1Ptr d3 = new D1;
    d3->sb = "D1.sb (p2 2)";
    d3->pb = 0;
    d3->sd1 = "D1.sd2 (p2 2)";
    d3->pd1 = 0;
    d4->pd2 = d3;

    return d3;
}

BPtr
TestI::paramTest4(BPtr& p1, const ::Ice::Current&)
{
    D4Ptr d4 = new D4;
    d4->sb = "D4.sb (1)";
    d4->pb = 0;
    d4->p1 = new B;
    d4->p1->sb = "B.sb (1)";
    d4->p2 = new B;
    d4->p2->sb = "B.sb (2)";
    p1 = d4;
    return d4->p2;
}

BPtr
TestI::returnTest1(BPtr& p1, BPtr& p2, const ::Ice::Current&)
{
    ::Ice::Current c;
    paramTest1(p1, p2, c);
    return p1;
}

BPtr
TestI::returnTest2(BPtr& p1, BPtr& p2, const ::Ice::Current&)
{
    ::Ice::Current c;
    paramTest1(p2, p1, c);
    return p1;
}

BPtr
TestI::returnTest3(const BPtr& p1, const BPtr&, const ::Ice::Current&)
{
    return p1;
}

SS3
TestI::sequenceTest(const SS1Ptr& p1, const SS2Ptr& p2, const ::Ice::Current&)
{
    SS3 ss;
    ss.c1 = p1;
    ss.c2 = p2;
    return ss;
}

Test::BDict
TestI::dictionaryTest(const BDict& bin, BDict& bout, const ::Ice::Current&)
{
    int i;
    for(i = 0; i < 10; ++i)
    {
        BPtr b = bin.find(i)->second;
        D2Ptr d2 = new D2;
        d2->sb = b->sb;
        d2->pb = b->pb;
        d2->sd2 = "D2";
        d2->pd2 = d2;
        d2->ice_collectable(true);
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
        d1->ice_collectable(true);
        r[i * 20] = d1;
    }
    return r;
}

Test::PBasePtr
TestI::exchangePBase(const Test::PBasePtr& pb, const Ice::Current&)
{
    return pb;
}

Test::PreservedPtr
TestI::PBSUnknownAsPreserved(const Ice::Current& current)
{
    PSUnknownPtr r = new PSUnknown;
    r->pi = 5;
    r->ps = "preserved";
    r->psu = "unknown";
    r->graph = 0;
    if(current.encoding != Ice::Encoding_1_0)
    {
        //
        // 1.0 encoding doesn't support unmarshaling unknown classes even if referenced
        // from unread slice.
        //
        r->cl = new MyClass(15);
    }
    return r;
}

void
TestI::checkPBSUnknown(const Test::PreservedPtr& p, const Ice::Current& current)
{
    PSUnknownPtr pu = PSUnknownPtr::dynamicCast(p);
    if(current.encoding == Ice::Encoding_1_0)
    {
        test(!pu);
        test(p->pi == 5);
        test(p->ps == "preserved");
    }
    else
    {
        test(pu);
        test(pu->pi == 5);
        test(pu->ps == "preserved");
        test(pu->psu == "unknown");
        test(!pu->graph);
        test(pu->cl && pu->cl->i == 15);
    }
}

void
TestI::PBSUnknownAsPreservedWithGraph_async(const Test::AMD_TestIntf_PBSUnknownAsPreservedWithGraphPtr& cb,
                                            const Ice::Current&)
{
    PSUnknownPtr r = new PSUnknown;
    r->pi = 5;
    r->ps = "preserved";
    r->psu = "unknown";
    r->graph = new PNode;
    r->graph->next = new PNode;
    r->graph->next->next = new PNode;
    r->graph->next->next->next = r->graph;
    cb->ice_response(r);
    r->graph->next->next->next = 0; // Break the cycle.
}

void
TestI::checkPBSUnknownWithGraph(const Test::PreservedPtr& p, const Ice::Current& current)
{
    PSUnknownPtr pu = PSUnknownPtr::dynamicCast(p);
    if(current.encoding == Ice::Encoding_1_0)
    {
        test(!pu);
        test(p->pi == 5);
        test(p->ps == "preserved");
    }
    else
    {
        test(pu);
        test(pu->pi == 5);
        test(pu->ps == "preserved");
        test(pu->psu == "unknown");
        test(pu->graph != pu->graph->next);
        test(pu->graph->next != pu->graph->next->next);
        test(pu->graph->next->next->next == pu->graph);
        pu->graph->next->next->next = 0;          // Break the cycle.
    }
}

void
TestI::PBSUnknown2AsPreservedWithGraph_async(const Test::AMD_TestIntf_PBSUnknown2AsPreservedWithGraphPtr& cb,
                                             const Ice::Current&)
{
    PSUnknown2Ptr r = new PSUnknown2;
    r->pi = 5;
    r->ps = "preserved";
    r->pb = r;
    cb->ice_response(r);
    r->pb = 0; // Break the cycle.
}

void
TestI::checkPBSUnknown2WithGraph(const Test::PreservedPtr& p, const Ice::Current& current)
{
    PSUnknown2Ptr pu = PSUnknown2Ptr::dynamicCast(p);
    if(current.encoding == Ice::Encoding_1_0)
    {
        test(!pu);
        test(p->pi == 5);
        test(p->ps == "preserved");
    }
    else
    {
        test(pu);
        test(pu->pi == 5);
        test(pu->ps == "preserved");
        test(pu->pb == pu);
        pu->pb = 0;          // Break the cycle.
    }
}

Test::PNodePtr
TestI::exchangePNode(const Test::PNodePtr& pn, const Ice::Current&)
{
    return pn;
}

void
TestI::throwBaseAsBase(const ::Ice::Current&)
{
    BaseException be;
    be.sbe = "sbe";
    be.pb = new B;
    be.pb->sb = "sb";
    be.pb->pb = be.pb;
    be.pb->ice_collectable(true);
    throw be;
}

void
TestI::throwDerivedAsBase(const ::Ice::Current&)
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
    de.pb->ice_collectable(true);
    de.pd1->ice_collectable(true);
    throw de;
}

void
TestI::throwDerivedAsDerived(const ::Ice::Current&)
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
    de.pb->ice_collectable(true);
    de.pd1->ice_collectable(true);
    throw de;
}

void
TestI::throwUnknownDerivedAsBase(const ::Ice::Current&)
{
    D2Ptr d2 = new D2;
    d2->sb = "sb d2";
    d2->pb = d2;
    d2->sd2 = "sd2 d2";
    d2->pd2 = d2;
    d2->ice_collectable(true);
    
    UnknownDerivedException ude;
    ude.sbe = "sbe";
    ude.pb = d2;
    ude.sude = "sude";
    ude.pd2 = d2;
    throw ude;
}

void
TestI::throwPreservedException_async(const AMD_TestIntf_throwPreservedExceptionPtr& cb, const ::Ice::Current&)
{
    PSUnknownException ue;
    ue.p = new PSUnknown2;
    ue.p->pi = 5;
    ue.p->ps = "preserved";
    ue.p->pb = ue.p;
    cb->ice_exception(ue);
    ue.p->pb = 0; // Break the cycle.
}

void
TestI::useForward(ForwardPtr& f, const ::Ice::Current&)
{
    f = new Forward;
    f->h = new Hidden;
    f->h->f = f;
    f->ice_collectable(true);
}

void
TestI::shutdown(const ::Ice::Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
