// **********************************************************************
//
// Copyright (c) 2001
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

GPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    string ref = "d:default -p 12345 -t 2000";
    Ice::ObjectPrx db = communicator->stringToProxy(ref);
    test(db);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    DPrx d = DPrx::checkedCast(db);
    test(d);
    test(d == db);
    cout << "ok" << endl;

    cout << "testing non-facets A, B, C, and D... " << flush;
    test(d->callA() == "A");
    test(d->callB() == "B");
    test(d->callC() == "C");
    test(d->callD() == "D");
    cout << "ok" << endl;

    cout << "testing facets A, B, C, and D... " << flush;
    DPrx df = DPrx::checkedCast(d, "facetABCD");
    test(df);
    test(df->callA() == "A");
    test(df->callB() == "B");
    test(df->callC() == "C");
    test(df->callD() == "D");
    cout << "ok" << endl;

    cout << "testing facets E and F... " << flush;
    FPrx ff = FPrx::checkedCast(d, "facetEF");
    test(ff);
    test(ff->callE() == "E");
    test(ff->callF() == "F");
    cout << "ok" << endl;

    cout << "testing facet G, which is a sub-facet of E and F... " << flush;
    GPrx gf = GPrx::checkedCast(ff, "facetGH");
    test(gf);
    test(gf->callG() == "G");
    cout << "ok" << endl;

    cout << "testing whether casting preserves the facet... " << flush;
    HPrx hf = HPrx::checkedCast(gf);
    test(hf);
    test(hf->callG() == "G");
    test(hf->callH() == "H");
    cout << "ok" << endl;

    return gf;
}
