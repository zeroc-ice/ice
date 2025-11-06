// Copyright (c) ZeroC, Inc.

#include "TestI.h"

using namespace Test;
using namespace std;
using namespace Ice;

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

InitialI::InitialI(ObjectAdapterPtr adapter)
    : _adapter(std::move(adapter)),
      _b1(new BI),
      _b2(new BI),
      _c(new CI),
      _d(new DI)
{
    _b1->theA = _b2;     // Cyclic reference to another B
    _b1->theB = _b1;     // Self reference.
    _b1->theC = nullptr; // Null reference.

    _b2->theA = _b2; // Self reference, using base.
    _b2->theB = _b1; // Cyclic reference to another B
    _b2->theC = _c;  // Cyclic reference to a C.

    _c->theB = _b2; // Cyclic reference to a B.

    _d->theA = _b1;     // Reference to a B.
    _d->theB = _b2;     // Reference to a B.
    _d->theC = nullptr; // Reference to a C.

    _b1->postUnmarshalInvoked = false;
    _b2->postUnmarshalInvoked = false;
    _c->postUnmarshalInvoked = false;
    _d->postUnmarshalInvoked = false;
}

InitialI::~InitialI()
{
    _b1->theA = nullptr;
    _b1->theB = nullptr;

    _b2->theA = nullptr;
    _b2->theB = nullptr;
    _b2->theC = nullptr;

    _c->theB = nullptr;
}

void
InitialI::shutdown(const Current&)
{
    _adapter->getCommunicator()->shutdown();
}

BPtr
InitialI::getB1(const Current&)
{
    _b1->preMarshalInvoked = false;
    _b2->preMarshalInvoked = false;
    _c->preMarshalInvoked = false;
    return _b1;
}

BPtr
InitialI::getB2(const Current&)
{
    _b1->preMarshalInvoked = false;
    _b2->preMarshalInvoked = false;
    _c->preMarshalInvoked = false;
    return _b2;
}

CPtr
InitialI::getC(const Current&)
{
    _b1->preMarshalInvoked = false;
    _b2->preMarshalInvoked = false;
    _c->preMarshalInvoked = false;
    return _c;
}

DPtr
InitialI::getD(const Current&)
{
    _b1->preMarshalInvoked = false;
    _b2->preMarshalInvoked = false;
    _c->preMarshalInvoked = false;
    _d->preMarshalInvoked = false;
    return _d;
}

void
InitialI::setRecursive(RecursivePtr, const Current&)
{
}

void
InitialI::setCycle(RecursivePtr r, const Current&)
{
    // break the cycle
    assert(r);
    r->v = nullptr;
}

bool
InitialI::acceptsClassCycles(const Current& c)
{
    return c.adapter->getCommunicator()->getProperties()->getIcePropertyAsInt("Ice.AcceptClassCycles") > 0;
}

InitialI::GetMBMarshaledResult
InitialI::getMB(const Current& current)
{
    return {_b1, current};
}

void
InitialI::getAMDMBAsync(
    function<void(GetAMDMBMarshaledResult)> response,
    function<void(exception_ptr)>,
    const Current& current)
{
    response(GetAMDMBMarshaledResult(_b1, current));
}

void
InitialI::getAll(BPtr& b1, BPtr& b2, CPtr& c, DPtr& d, const Current&)
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

Test::CppClassPtr
InitialI::opRenamedClass(Test::CppClassPtr r, const Ice::Current&)
{
    return r;
}

void
InitialI::setG(GPtr, const Current&)
{
}

BaseSeq
InitialI::opBaseSeq(BaseSeq inSeq, BaseSeq& outSeq, const Current&)
{
    outSeq = inSeq;
    return inSeq;
}

CompactPtr
InitialI::getCompact(const Current&)
{
    return make_shared<CompactExt>();
}

Inner::APtr
InitialI::getInnerA(const Current&)
{
    return make_shared<Inner::A>(_b1);
}

Inner::Sub::APtr
InitialI::getInnerSubA(const Current&)
{
    return make_shared<Inner::Sub::A>(make_shared<Inner::A>(_b1));
}

void
InitialI::throwInnerEx(const Current&)
{
    throw Inner::Ex{"Inner::Ex"};
}

void
InitialI::throwInnerSubEx(const Current&)
{
    throw Inner::Sub::Ex{"Inner::Sub::Ex"};
}

KPtr
InitialI::getK(const Current&)
{
    return make_shared<K>(make_shared<L>("l"));
}

ValuePtr
InitialI::opValue(ValuePtr v1, ValuePtr& v2, const Current&)
{
    v2 = v1;
    return v1;
}

ValueSeq
InitialI::opValueSeq(ValueSeq v1, ValueSeq& v2, const Current&)
{
    v2 = v1;
    return v1;
}

ValueMap
InitialI::opValueMap(ValueMap v1, ValueMap& v2, const Current&)
{
    v2 = v1;
    return v1;
}

D1Ptr
InitialI::getD1(D1Ptr d1, const Current&)
{
    return d1;
}

void
InitialI::throwEDerived(const Current&)
{
    throw EDerived(make_shared<A1>("a1"), make_shared<A1>("a2"), make_shared<A1>("a3"), make_shared<A1>("a4"));
}

MPtr
InitialI::opM(MPtr v1, MPtr& v2, const Current&)
{
    v2 = v1;
    return v1;
}

bool
UnexpectedObjectExceptionTestI::ice_invoke(vector<byte>, vector<byte>& outParams, const Current& current)
{
    CommunicatorPtr communicator = current.adapter->getCommunicator();
    OutputStream out(communicator);
    out.startEncapsulation(current.encoding, nullopt);
    AlsoEmptyPtr obj = make_shared<AlsoEmpty>();
    out.write(obj);
    out.writePendingValues();
    out.endEncapsulation();
    out.finished(outParams);
    return true;
}

F1Ptr
InitialI::opF1(F1Ptr f11, F1Ptr& f12, const Current&)
{
    f12 = make_shared<F1>("F12");
    return f11;
}

optional<F2Prx>
InitialI::opF2(optional<F2Prx> f21, optional<F2Prx>& f22, const Current& current)
{
    f22 = F2Prx(current.adapter->getCommunicator(), "F22");
    return f21;
}

F3Ptr
InitialI::opF3(F3Ptr f31, F3Ptr& f32, const Current& current)
{
    f32 = make_shared<F3>();
    f32->f1 = make_shared<F1>("F12");
    f32->f2 = F2Prx(current.adapter->getCommunicator(), "F22");
    return f31;
}

bool
InitialI::hasF3(const Current&)
{
    return true;
}
