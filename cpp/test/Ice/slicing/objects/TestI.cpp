// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestI.h>
#include <Ice/Ice.h>
#include <sstream>

using namespace Test;

TestI::TestI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter)
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

Ice::ObjectPtr
TestI::SUnknownAsObject(const ::Ice::Current&)
{
    SUnknownPtr su = new SUnknown;
    su->su = "SUnknown.su";
    return su;
}

BPtr
TestI::oneElementCycle(const ::Ice::Current&)
{
    BPtr b = new B;
    b->sb = "B1.sb";
    b->pb = b;
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
TestI::returnTest3(const BPtr& p1, const BPtr& p2, const ::Ice::Current&)
{
    return p1;
}

SS
TestI::sequenceTest(const SS1Ptr& p1, const SS2Ptr& p2, const ::Ice::Current&)
{
    SS ss;
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
    return r;
}

void
TestI::throwBaseAsBase(const ::Ice::Current&)
{
    BaseException be;
    be.sbe = "sbe";
    be.pb = new B;
    be.pb->sb = "sb";
    be.pb->pb = be.pb;
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

    UnknownDerivedException ude;
    ude.sbe = "sbe";
    ude.pb = d2;
    ude.sude = "sude";
    ude.pd2 = d2;
    throw ude;
}

void
TestI::useForward(ForwardPtr& f, const ::Ice::Current&)
{
    f = new Forward;
    f->h = new Hidden;
    f->h->f = f;
}

void
TestI::shutdown(const ::Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
}
