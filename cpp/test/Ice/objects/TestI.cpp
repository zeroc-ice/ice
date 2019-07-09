//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestI.h>

using namespace Test;
using namespace std;

void
BI::ice_preMarshal()
{
    preMarshalInvoked = true;
}

void
BI::ice_postUnmarshal()
{
    postUnmarshalInvoked = true;
}

void
CI::ice_preMarshal()
{
    preMarshalInvoked = true;
}

void
CI::ice_postUnmarshal()
{
    postUnmarshalInvoked = true;
}

void
DI::ice_preMarshal()
{
    preMarshalInvoked = true;
}

void
DI::ice_postUnmarshal()
{
    postUnmarshalInvoked = true;
}

EI::EI() :
    E(1, "hello")
{
}

bool
EI::checkValues()
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
FI::checkValues()
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
#ifndef ICE_CPP11_MAPPING
    _b1->ice_collectable(true);
    _b2->ice_collectable(true);
    _c->ice_collectable(true);
    _d->ice_collectable(true);
#endif

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

    _b1->postUnmarshalInvoked = false;
    _b2->postUnmarshalInvoked = false;
    _c->postUnmarshalInvoked = false;
    _d->postUnmarshalInvoked = false;
}

InitialI::~InitialI()
{
#ifdef ICE_CPP11_MAPPING
    // No GC with the C++11 mapping
    _b1->theA = ICE_NULLPTR;
    _b1->theB = ICE_NULLPTR;

    _b2->theA = ICE_NULLPTR;
    _b2->theB = ICE_NULLPTR;
    _b2->theC = ICE_NULLPTR;

    _c->theB = ICE_NULLPTR;
#endif
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
InitialI::setRecursive(ICE_IN(RecursivePtr), const Ice::Current&)
{
}

bool
InitialI::supportsClassGraphDepthMax(const Ice::Current&)
{
    return true;
}

#ifdef ICE_CPP11_MAPPING
InitialI::GetMBMarshaledResult
InitialI::getMB(const Ice::Current& current)
{
    return GetMBMarshaledResult(_b1, current);
}

void
InitialI::getAMDMBAsync(function<void(const GetAMDMBMarshaledResult&)> response,
                        function<void(exception_ptr)>,
                        const Ice::Current& current)
{
    response(GetAMDMBMarshaledResult(_b1, current));
}
#else
Test::BPtr
InitialI::getMB(const Ice::Current&)
{
    return _b1;
}

void
InitialI::getAMDMB_async(const Test::AMD_Initial_getAMDMBPtr& cb, const Ice::Current&)
{
    cb->ice_response(_b1);
}
#endif

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

void
InitialI::setG(ICE_IN(Test::GPtr), const Ice::Current&)
{
}

#ifdef ICE_CPP11_MAPPING
shared_ptr<Ice::Value>
InitialI::getI(const Ice::Current&)
{
    return make_shared<II>();
}

void
InitialI::setI(ICE_IN(shared_ptr<Ice::Value>), const Ice::Current&)
{
}
#else
IPtr
InitialI::getI(const Ice::Current&)
{
    return new II();
}

void
InitialI::setI(const IPtr&, const Ice::Current&)
{
}
#endif

BaseSeq
InitialI::opBaseSeq(ICE_IN(BaseSeq) inSeq, BaseSeq& outSeq, const Ice::Current&)
{
    outSeq = inSeq;
    return inSeq;
}

CompactPtr
InitialI::getCompact(const Ice::Current&)
{
    return ICE_MAKE_SHARED(CompactExt);
}

Test::Inner::APtr
InitialI::getInnerA(const Ice::Current&)
{
    return ICE_MAKE_SHARED(Inner::A, _b1);
}

Test::Inner::Sub::APtr
InitialI::getInnerSubA(const Ice::Current&)
{
    return ICE_MAKE_SHARED(Inner::Sub::A, ICE_MAKE_SHARED(Inner::A, _b1));
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

#ifdef ICE_CPP11_MAPPING
shared_ptr<Ice::Value>
InitialI::getJ(const Ice::Current&)
{
    return make_shared<JI>();
}

shared_ptr<Ice::Value>
InitialI::getH(const Ice::Current&)
{
    return make_shared<HI>();
}
#else
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
#endif

KPtr
InitialI::getK(const Ice::Current&)
{
    return ICE_MAKE_SHARED(K, ICE_MAKE_SHARED(L, "l"));
}

Ice::ValuePtr
InitialI::opValue(ICE_IN(Ice::ValuePtr) v1, Ice::ValuePtr& v2, const Ice::Current&)
{
    v2 = v1;
    return v1;
}

Test::ValueSeq
InitialI::opValueSeq(ICE_IN(Test::ValueSeq) v1, Test::ValueSeq& v2, const Ice::Current&)
{
    v2 = v1;
    return v1;
}

Test::ValueMap
InitialI::opValueMap(ICE_IN(Test::ValueMap) v1, Test::ValueMap& v2, const Ice::Current&)
{
    v2 = v1;
    return v1;
}

D1Ptr
InitialI::getD1(ICE_IN(Test::D1Ptr) d1, const Ice::Current&)
{
    return d1;
}

void
InitialI::throwEDerived(const Ice::Current&)
{
    throw EDerived(ICE_MAKE_SHARED(A1, "a1"),
                   ICE_MAKE_SHARED(A1, "a2"),
                   ICE_MAKE_SHARED(A1, "a3"),
                   ICE_MAKE_SHARED(A1, "a4"));
}

Test::MPtr
InitialI::opM(ICE_IN(Test::MPtr) v1, Test::MPtr& v2, const Ice::Current&)
{
    v2 = v1;
    return v1;
}

bool
UnexpectedObjectExceptionTestI::ice_invoke(ICE_IN(std::vector<Ice::Byte>),
                                           std::vector<Ice::Byte>& outParams,
                                           const Ice::Current& current)
{
    Ice::CommunicatorPtr communicator = current.adapter->getCommunicator();
    Ice::OutputStream out(communicator);
    out.startEncapsulation(current.encoding, Ice::ICE_ENUM(FormatType, DefaultFormat));
    AlsoEmptyPtr obj = ICE_MAKE_SHARED(AlsoEmpty);
    out.write(obj);
    out.writePendingValues();
    out.endEncapsulation();
    out.finished(outParams);
    return true;
}

Test::F1Ptr
InitialI::opF1(ICE_IN(Test::F1Ptr) f11, Test::F1Ptr& f12, const Ice::Current&)
{
    f12 = ICE_MAKE_SHARED(F1, "F12");
    return f11;
}

Test::F2PrxPtr
InitialI::opF2(ICE_IN(Test::F2PrxPtr) f21, Test::F2PrxPtr& f22, const Ice::Current& current)
{
    f22 = ICE_UNCHECKED_CAST(F2Prx, current.adapter->getCommunicator()->stringToProxy("F22"));
    return f21;
}

Test::F3Ptr
InitialI::opF3(ICE_IN(Test::F3Ptr) f31, Test::F3Ptr& f32, const Ice::Current& current)
{
    f32 = ICE_MAKE_SHARED(F3);
    f32->f1 = ICE_MAKE_SHARED(F1, "F12");
    f32->f2 = ICE_UNCHECKED_CAST(F2Prx, current.adapter->getCommunicator()->stringToProxy("F22"));
    return f31;
}

bool
InitialI::hasF3(const Ice::Current&)
{
    return true;
}
