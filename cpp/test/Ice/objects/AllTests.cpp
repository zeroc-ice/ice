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
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    string ref("initial:tcp -p 12345 -t 2000");
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
    BPtr fb1 = BPtr::dynamicCast(b1->_findFacet("b1"));
    test(fb1);
    BPtr fb2 = BPtr::dynamicCast(b1->_findFacet("b2"));
    test(fb2);
    CPtr fc = CPtr::dynamicCast(b1->_findFacet("c"));
    test(fc);
    DPtr fd = DPtr::dynamicCast(b1->_findFacet("d"));
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
    fb1 = BPtr::dynamicCast(b1->_findFacet("b1"));
    test(fb1);
    fb2 = BPtr::dynamicCast(b1->_findFacet("b2"));
    test(fb2);
    fc = CPtr::dynamicCast(b1->_findFacet("c"));
    test(fc);
    fd = DPtr::dynamicCast(b1->_findFacet("d"));
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

    return initial;
}
