// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestI.h>

InitialI::InitialI(const Ice::ObjectAdapterPtr& adapter) :
    _adapter(adapter),
    _b1(new B),
    _b2(new B),
    _c(new C),
    _d(new D)
{
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

    //
    // Break cyclic dependencies
    //
    _b1->theA = 0;
    _b1->theB = 0;
    _b1->theC = 0;
    _b1->ice_removeAllFacets();
    _b2->theA = 0;
    _b2->theB = 0;
    _b2->theC = 0;
    _b2->ice_removeAllFacets();
    _c->theB = 0;
    _c->ice_removeAllFacets();
    _d->theA = 0;
    _d->theB = 0;
    _d->theC = 0;
    _d->ice_removeAllFacets();
}

BPtr
InitialI::getB1(const Ice::Current&)
{
    return _b1;
}

BPtr
InitialI::getB2(const Ice::Current&)
{
    return _b2;
}

CPtr
InitialI::getC(const Ice::Current&)
{
    return _c;
}

DPtr
InitialI::getD(const Ice::Current&)
{
    return _d;
}

void
InitialI::getAll(BPtr& b1, BPtr& b2, CPtr& c, DPtr& d, const Ice::Current&)
{
    b1 = _b1;
    b2 = _b2;
    c = _c;
    d = _d;
}

void
InitialI::addFacetsToB1(const Ice::Current&)
{
    _b1->ice_addFacet(_b1, "b1");
    _b1->ice_addFacet(_b2, "b2");
    _b2->ice_addFacet(_c, "c");
    _b2->ice_addFacet(_d, "d");
}
