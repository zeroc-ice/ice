// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;

InitialPrx
allTests(const Ice::CommunicatorPtr& communicator, bool collocated)
{
    cout << "testing stringToProxy... " << flush;
    string ref = "initial:default -p 12345 -t 10000";
    Ice::ObjectPrx base = communicator->stringToProxy(ref);
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    InitialPrx initial = InitialPrx::checkedCast(base);
    test(initial);
    test(initial == base);
    cout << "ok" << endl;

    cout << "getting B1... " << flush;
    BPtr b1 = initial->getB1();
    test(b1);
    cout << "ok" << endl;
    
    cout << "getting B2... " << flush;
    BPtr b2 = initial->getB2();
    test(b2);
    cout << "ok" << endl;
    
    cout << "getting C... " << flush;
    CPtr c = initial->getC();
    test(c);
    cout << "ok" << endl;
    
    cout << "getting D... " << flush;
    DPtr d = initial->getD();
    test(d);
    cout << "ok" << endl;
    
    cout << "checking consistency... " << flush;
    test(b1 != b2);
    test(b1 != c);
    test(b1 != d);
    test(b2 != c);
    test(b2 != d);
    test(c != d);
    test(b1->theB == b1);
    test(b1->theC == 0);
    test(BPtr::dynamicCast(b1->theA));
    test(BPtr::dynamicCast(b1->theA)->theA == b1->theA);
    test(BPtr::dynamicCast(b1->theA)->theB == b1);
    test(CPtr::dynamicCast(BPtr::dynamicCast(b1->theA)->theC));
    test(CPtr::dynamicCast(BPtr::dynamicCast(b1->theA)->theC)->theB == b1->theA);
    if(!collocated)
    {
        test(b1->preMarshalInvoked);
        test(b1->postUnmarshalInvoked());
        test(b1->theA->preMarshalInvoked);
        test(b1->theA->postUnmarshalInvoked());
        test(BPtr::dynamicCast(b1->theA)->theC->preMarshalInvoked);
        test(BPtr::dynamicCast(b1->theA)->theC->postUnmarshalInvoked());
    }
    // More tests possible for b2 and d, but I think this is already sufficient.
    test(b2->theA == b2);
    test(d->theC == 0);
    cout << "ok" << endl;

    //
    // Break cyclic dependencies
    //
    if(!collocated)
    {
	BPtr::dynamicCast(b1->theA)->theA = 0;
	BPtr::dynamicCast(b1->theA)->theB = 0;
	BPtr::dynamicCast(b1->theA)->theC = 0;
	b1->theA = 0;
	b1->theB = 0;

	b2->theA = 0;
	b2->theB->theA = 0;
	b2->theB->theB = 0;
	b2->theC = 0;

	c->theB->theA = 0;
	c->theB->theB->theA = 0;
	c->theB->theB->theB = 0;
	c->theB = 0;

	BPtr::dynamicCast(BPtr::dynamicCast(d->theA)->theA)->theA = 0;
	BPtr::dynamicCast(BPtr::dynamicCast(d->theA)->theA)->theB = 0;
	BPtr::dynamicCast(d->theA)->theB->theA = 0;
	BPtr::dynamicCast(d->theA)->theB->theB = 0;
	d->theB->theA = 0;
	d->theB->theB = 0;
	d->theB->theC = 0;
    }

    cout << "getting B1, B2, C, and D all at once... " << flush;
    initial->getAll(b1, b2, c, d);
    test(b1);
    test(b2);
    test(c);
    test(d);
    cout << "ok" << endl;
    
    cout << "checking consistency... " << flush;
    test(b1 != b2);
    test(b1 != c);
    test(b1 != d);
    test(b2 != c);
    test(b2 != d);
    test(c != d);
    test(b1->theA == b2);
    test(b1->theB == b1);
    test(b1->theC == 0);
    test(b2->theA == b2);
    test(b2->theB == b1);
    test(b2->theC == c);
    test(c->theB == b2);
    test(d->theA == b1);
    test(d->theB == b2);
    test(d->theC == 0);
    if(!collocated)
    {
        test(d->preMarshalInvoked);
        test(d->postUnmarshalInvoked());
        test(d->theA->preMarshalInvoked);
        test(d->theA->postUnmarshalInvoked());
        test(d->theB->preMarshalInvoked);
        test(d->theB->postUnmarshalInvoked());
        test(d->theB->theC->preMarshalInvoked);
        test(d->theB->theC->postUnmarshalInvoked());
    }
    cout << "ok" << endl;

    //
    // Break cyclic dependencies
    //
    if(!collocated)
    {
	b1->theA = 0;
	b1->theB = 0;
	b2->theA = 0;
	b2->theB = 0;
	b2->theC = 0;
	c->theB = 0;
	d->theA = 0;
	d->theB = 0;
    }

    cout << "adding facets to B1... " << flush;
    initial->addFacetsToB1();
    cout << "ok" << endl;

    cout << "getting B1 with facets... " << flush;
    b1 = initial->getB1();
    test(b1);
    cout << "ok" << endl;
    
    cout << "checking consistency... " << flush;
    test(b1->theB == b1);
    test(b1->theC == 0);
    test(BPtr::dynamicCast(b1->theA));
    test(BPtr::dynamicCast(b1->theA)->theA == b1->theA);
    test(BPtr::dynamicCast(b1->theA)->theB == b1);
    test(CPtr::dynamicCast(BPtr::dynamicCast(b1->theA)->theC));
    test(CPtr::dynamicCast(BPtr::dynamicCast(b1->theA)->theC)->theB == b1->theA);
    cout << "ok" << endl;

    cout << "checking facet consistency... " << flush;
    BPtr fb1 = BPtr::dynamicCast(b1->ice_findFacet("b1"));
    test(fb1);
    BPtr fb2 = BPtr::dynamicCast(b1->ice_findFacet("b2"));
    test(fb2);
    CPtr fc = CPtr::dynamicCast(fb2->ice_findFacet("c"));
    test(fc);
    DPtr fd = DPtr::dynamicCast(fb2->ice_findFacet("d"));
    test(fd);
    test(b1 == fb1);
    test(fb1->theA == fb2);
    test(fb1->theB == fb1);
    test(fb1->theC == 0);
    test(fb2->theA == fb2);
    test(fb2->theB == fb1);
    test(fb2->theC == fc);
    test(fc->theB == fb2);
    test(fd->theA == fb1);
    test(fd->theB == fb2);
    test(fd->theC == 0);
    cout << "ok" << endl;

    //
    // Break cyclic dependencies
    //
    if(!collocated)
    {
	BPtr::dynamicCast(fb1->theA)->theA = 0;
	BPtr::dynamicCast(fb1->theA)->theB = 0;
	fb1->theA = 0;
	fb1->theB = 0;
	fb1->ice_removeAllFacets();
	fb2->theA = 0;
	fb2->theB = 0;
	fb2->theC = 0;
	fc->theB = 0;
	fd->theA = 0;
	fd->theB = 0;
    }

    cout << "getting B1 with facets, and B2, C, and D all at once... " << flush;
    initial->getAll(b1, b2, c, d);
    test(b1);
    test(b2);
    test(c);
    test(d);
    cout << "ok" << endl;

    cout << "checking consistency... " << flush;
    test(b1 != b2);
    test(b1 != c);
    test(b1 != d);
    test(b2 != c);
    test(b2 != d);
    test(c != d);
    test(b1->theA == b2);
    test(b1->theB == b1);
    test(b1->theC == 0);
    test(b2->theA == b2);
    test(b2->theB == b1);
    test(b2->theC == c);
    test(c->theB == b2);
    test(d->theA == b1);
    test(d->theB == b2);
    test(d->theC == 0);
    cout << "ok" << endl;

    cout << "checking facet consistency... " << flush;
    fb1 = BPtr::dynamicCast(b1->ice_findFacet("b1"));
    test(fb1);
    fb2 = BPtr::dynamicCast(b1->ice_findFacet("b2"));
    test(fb2);
    fc = CPtr::dynamicCast(fb2->ice_findFacet("c"));
    test(fc);
    fd = DPtr::dynamicCast(fb2->ice_findFacet("d"));
    test(fd);
    test(b1 == fb1);
    test(b2 == fb2);
    test(c == fc);
    test(d == fd);
    test(fb1->theA == fb2);
    test(fb1->theA == fb2);
    test(fb1->theB == fb1);
    test(fb1->theC == 0);
    test(fb2->theA == fb2);
    test(fb2->theB == fb1);
    test(fb2->theC == fc);
    test(fc->theB == fb2);
    test(fd->theA == fb1);
    test(fd->theB == fb2);
    test(fd->theC == 0);
    cout << "ok" << endl;

    //
    // Break cyclic dependencies
    //
    if(!collocated)
    {
	fb1->theA = 0;
	fb1->theB = 0;
	fb1->ice_removeAllFacets();
	fb2->theA = 0;
	fb2->theB = 0;
	fb2->theC = 0;
	fc->theB = 0;
	fd->theA = 0;
	fd->theB = 0;
    }

    return initial;
}
