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
#include <TestCommon.h>
#include <Test.h>

using namespace std;

InitialPrx
allTests(const Ice::CommunicatorPtr& communicator, bool collocated)
{
    cout << "testing stringToProxy... " << flush;
    string ref = "initial:default -p 12345 -t 2000";
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
    test(b1->b == b1);
    test(b1->c == 0);
    test(BPtr::dynamicCast(b1->a));
    test(BPtr::dynamicCast(b1->a)->a == b1->a);
    test(BPtr::dynamicCast(b1->a)->b == b1);
    test(CPtr::dynamicCast(BPtr::dynamicCast(b1->a)->c));
    test(CPtr::dynamicCast(BPtr::dynamicCast(b1->a)->c)->b == b1->a);
    // More tests possible for b2 and d, but I think this is already sufficient.
    test(b2->a == b2);
    test(d->c == 0);
    cout << "ok" << endl;

    //
    // Break cyclic dependencies
    //
    if(!collocated)
    {
	BPtr::dynamicCast(b1->a)->a = 0;
	BPtr::dynamicCast(b1->a)->b = 0;
	BPtr::dynamicCast(b1->a)->c = 0;
	b1->a = 0;
	b1->b = 0;

	b2->a = 0;
	b2->b->a = 0;
	b2->b->b = 0;
	b2->c = 0;

	c->b->a = 0;
	c->b->b->a = 0;
	c->b->b->b = 0;
	c->b = 0;

	BPtr::dynamicCast(BPtr::dynamicCast(d->a)->a)->a = 0;
	BPtr::dynamicCast(BPtr::dynamicCast(d->a)->a)->b = 0;
	BPtr::dynamicCast(d->a)->b->a = 0;
	BPtr::dynamicCast(d->a)->b->b = 0;
	d->b->a = 0;
	d->b->b = 0;
	d->b->c = 0;
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
    test(b1->a == b2);
    test(b1->b == b1);
    test(b1->c == 0);
    test(b2->a == b2);
    test(b2->b == b1);
    test(b2->c == c);
    test(c->b == b2);
    test(d->a == b1);
    test(d->b == b2);
    test(d->c == 0);
    cout << "ok" << endl;

    //
    // Break cyclic dependencies
    //
    if(!collocated)
    {
	b1->a = 0;
	b1->b = 0;
	b2->a = 0;
	b2->b = 0;
	b2->c = 0;
	c->b = 0;
	d->a = 0;
	d->b = 0;
    }

    cout << "adding facets to B1... " << flush;
    initial->addFacetsToB1();
    cout << "ok" << endl;

    cout << "getting B1 with facets... " << flush;
    b1 = initial->getB1();
    test(b1);
    cout << "ok" << endl;
    
    cout << "checking consistency... " << flush;
    test(b1->b == b1);
    test(b1->c == 0);
    test(BPtr::dynamicCast(b1->a));
    test(BPtr::dynamicCast(b1->a)->a == b1->a);
    test(BPtr::dynamicCast(b1->a)->b == b1);
    test(CPtr::dynamicCast(BPtr::dynamicCast(b1->a)->c));
    test(CPtr::dynamicCast(BPtr::dynamicCast(b1->a)->c)->b == b1->a);
    cout << "ok" << endl;

    cout << "checking facet consistency... " << flush;
    BPtr fb1 = BPtr::dynamicCast(b1->ice_findFacet("b1"));
    test(fb1);
    BPtr fb2 = BPtr::dynamicCast(b1->ice_findFacet("b2"));
    test(fb2);
    CPtr fc = CPtr::dynamicCast(b1->ice_findFacet("c"));
    test(fc);
    DPtr fd = DPtr::dynamicCast(b1->ice_findFacet("d"));
    test(fd);
    test(b1 == fb1);
    test(fb1->a == fb2);
    test(fb1->b == fb1);
    test(fb1->c == 0);
    test(fb2->a == fb2);
    test(fb2->b == fb1);
    test(fb2->c == fc);
    test(fc->b == fb2);
    test(fd->a == fb1);
    test(fd->b == fb2);
    test(fd->c == 0);
    cout << "ok" << endl;

    //
    // Break cyclic dependencies
    //
    if(!collocated)
    {
	BPtr::dynamicCast(fb1->a)->a = 0;
	BPtr::dynamicCast(fb1->a)->b = 0;
	fb1->a = 0;
	fb1->b = 0;
	fb1->ice_removeAllFacets();
	fb2->a = 0;
	fb2->b = 0;
	fb2->c = 0;
	fc->b = 0;
	fd->a = 0;
	fd->b = 0;
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
    test(b1->a == b2);
    test(b1->b == b1);
    test(b1->c == 0);
    test(b2->a == b2);
    test(b2->b == b1);
    test(b2->c == c);
    test(c->b == b2);
    test(d->a == b1);
    test(d->b == b2);
    test(d->c == 0);
    cout << "ok" << endl;

    cout << "checking facet consistency... " << flush;
    fb1 = BPtr::dynamicCast(b1->ice_findFacet("b1"));
    test(fb1);
    fb2 = BPtr::dynamicCast(b1->ice_findFacet("b2"));
    test(fb2);
    fc = CPtr::dynamicCast(b1->ice_findFacet("c"));
    test(fc);
    fd = DPtr::dynamicCast(b1->ice_findFacet("d"));
    test(fd);
    test(b1 == fb1);
    test(b2 == fb2);
    test(c == fc);
    test(d == fd);
    test(fb1->a == fb2);
    test(fb1->a == fb2);
    test(fb1->b == fb1);
    test(fb1->c == 0);
    test(fb2->a == fb2);
    test(fb2->b == fb1);
    test(fb2->c == fc);
    test(fc->b == fb2);
    test(fd->a == fb1);
    test(fd->b == fb2);
    test(fd->c == 0);
    cout << "ok" << endl;

    //
    // Break cyclic dependencies
    //
    if(!collocated)
    {
	fb1->a = 0;
	fb1->b = 0;
	fb1->ice_removeAllFacets();
	fb2->a = 0;
	fb2->b = 0;
	fb2->c = 0;
	fc->b = 0;
	fd->a = 0;
	fd->b = 0;
    }

    return initial;
}
