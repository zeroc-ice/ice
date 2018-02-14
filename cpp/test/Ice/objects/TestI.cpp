// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

using namespace Test;

BI::BI() :
    _postUnmarshalInvoked(false)
{
}

bool
BI::postUnmarshalInvoked(const Ice::Current&)
{
    return _postUnmarshalInvoked;
}

void
BI::ice_preMarshal()
{
    preMarshalInvoked = true;
}

void
BI::ice_postUnmarshal()
{
    _postUnmarshalInvoked = true;
}

CI::CI() :
    _postUnmarshalInvoked(false)
{
}

bool
CI::postUnmarshalInvoked(const Ice::Current&)
{
    return _postUnmarshalInvoked;
}

void
CI::ice_preMarshal()
{
    preMarshalInvoked = true;
}

void
CI::ice_postUnmarshal()
{
    _postUnmarshalInvoked = true;
}

DI::DI() :
    _postUnmarshalInvoked(false)
{
}

bool
DI::postUnmarshalInvoked(const Ice::Current&)
{
    return _postUnmarshalInvoked;
}

void
DI::ice_preMarshal()
{
    preMarshalInvoked = true;
}

void
DI::ice_postUnmarshal()
{
    _postUnmarshalInvoked = true;
}

EI::EI() :
    E(1, "hello")
{
}

bool
EI::checkValues(const Ice::Current&)
{
    return i == 1 && s == "hello";
}

FI::FI()
{
}

FI::FI(const EPtr& e) :
    F(e, e)
{
}

bool
FI::checkValues(const Ice::Current&)
{
    return e1 && e1 == e2;
}

InitialI::InitialI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter),
    _b1(new BI),
    _b2(new BI),
    _c(new CI),
    _d(new DI),
    _e(new EI),
    _f(new FI(_e))
{
    _b1->ice_collectable(true);
    _b2->ice_collectable(true);
    _c->ice_collectable(true);
    _d->ice_collectable(true);

    _b1->theA = _b2; // Cyclic reference to another B
    _b1->theB = _b1; // Self reference.
    _b1->theC = 0; // Null reference.

    _b2->theA = _b2; // Self reference, using base.
    _b2->theB = _b1; // Cyclic reference to another B
    _b2->theC = _c; // Cyclic reference to a C.

    _c->theB = _b2; // Cyclic reference to a B.

    _d->theA = _b1; // Reference to a B.
    _d->theB = _b2; // Reference to a B.
    _d->theC = 0; // Reference to a C.
}

void
InitialI::shutdown(const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();
}

BPtr
InitialI::getB1(const Ice::Current&)
{
    _b1->preMarshalInvoked = false;
    _b2->preMarshalInvoked = false;
    _c->preMarshalInvoked = false;
    return _b1;
}

BPtr
InitialI::getB2(const Ice::Current&)
{
    _b1->preMarshalInvoked = false;
    _b2->preMarshalInvoked = false;
    _c->preMarshalInvoked = false;
    return _b2;
}

CPtr
InitialI::getC(const Ice::Current&)
{
    _b1->preMarshalInvoked = false;
    _b2->preMarshalInvoked = false;
    _c->preMarshalInvoked = false;
    return _c;
}

DPtr
InitialI::getD(const Ice::Current&)
{
    _b1->preMarshalInvoked = false;
    _b2->preMarshalInvoked = false;
    _c->preMarshalInvoked = false;
    _d->preMarshalInvoked = false;
    return _d;
}

EPtr
InitialI::getE(const Ice::Current&)
{
    return _e;
}

FPtr
InitialI::getF(const Ice::Current&)
{
    return _f;
}

void
InitialI::setRecursive(const RecursivePtr&, const Ice::Current&)
{
}

bool
InitialI::supportsClassGraphDepthMax(const Ice::Current&)
{
    return true;
}

void
InitialI::getAll(BPtr& b1, BPtr& b2, CPtr& c, DPtr& d, const Ice::Current&)
{
    _b1->preMarshalInvoked = false;
    _b2->preMarshalInvoked = false;
    _c->preMarshalInvoked = false;
    _d->preMarshalInvoked = false;
    b1 = _b1;
    b2 = _b2;
    c = _c;
    d = _d;
}

IPtr
InitialI::getI(const Ice::Current&)
{
    return new II();
}

void
InitialI::setI(const IPtr&, const Ice::Current&)
{
}

BaseSeq
InitialI::opBaseSeq(const BaseSeq& inSeq, BaseSeq& outSeq, const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

CompactPtr
InitialI::getCompact(const Ice::Current&)
{
    return new CompactExt();
}

Test::Inner::APtr
InitialI::getInnerA(const Ice::Current&)
{
    return new Inner::A(_b1);
}

Test::Inner::Sub::APtr
InitialI::getInnerSubA(const Ice::Current&)
{
    return new Inner::Sub::A(new Inner::A(_b1));
}

void
InitialI::throwInnerEx(const Ice::Current&)
{
    Inner::Ex ex;
    ex.reason = "Inner::Ex";
    throw ex;
}

void
InitialI::throwInnerSubEx(const Ice::Current&)
{
    Inner::Sub::Ex ex;
    ex.reason = "Inner::Sub::Ex";
    throw ex;
}

IPtr
InitialI::getJ(const Ice::Current&)
{
    return new JI();
}

IPtr
InitialI::getH(const Ice::Current&)
{
    return new HI();
}

D1Ptr
InitialI::getD1(const Test::D1Ptr& d1, const Ice::Current&)
{
    return d1;
}

void
InitialI::throwEDerived(const Ice::Current&)
{
    throw EDerived(new A1("a1"), new A1("a2"), new A1("a3"), new A1("a4"));
}

bool
UnexpectedObjectExceptionTestI::ice_invoke(const std::vector<Ice::Byte>&,
                                           std::vector<Ice::Byte>& outParams,
                                           const Ice::Current& current)
{
    Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
    Ice::OutputStreamPtr out = Ice::createOutputStream(communicator);
    out->startEncapsulation(current.encoding, Ice::DefaultFormat);
    AlsoEmptyPtr ae = new AlsoEmpty;
    out->write(ae);
    out->writePendingObjects();
    out->endEncapsulation();
    out->finished(outParams);
    return true;
}
