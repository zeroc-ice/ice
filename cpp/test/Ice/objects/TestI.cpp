// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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
    _b1->a = _b2; // Cyclic reference to another B
    _b1->b = _b1; // Self reference.
    _b1->c = 0; // Null reference.

    _b2->a = _b2; // Self reference, using base.
    _b2->b = _b1; // Cyclic reference to another B
    _b2->c = _c; // Cyclic reference to a C.

    _c->b = _b2; // Cyclic reference to a B.

    _d->a = _b1; // Reference to a B.
    _d->b = _b2; // Reference to a B.
    _d->c = 0; // Reference to a C.
}

void
InitialI::shutdown(const Ice::Current&)
{
    _adapter->getCommunicator()->shutdown();

    //
    // Break cyclic dependencies
    //
    _b1->a = 0;
    _b1->b = 0;
    _b1->c = 0;
    _b1->ice_removeAllFacets();
    _b2->a = 0;
    _b2->b = 0;
    _b2->c = 0;
    _b2->ice_removeAllFacets();
    _c->b = 0;
    _c->ice_removeAllFacets();
    _d->a = 0;
    _d->b = 0;
    _d->c = 0;
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
    _b1->ice_addFacet(_c, "c");
    _b1->ice_addFacet(_d, "d");
}
