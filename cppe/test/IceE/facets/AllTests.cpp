// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

class EmptyI : virtual public Empty
{
};

GPrx
allTests(const Ice::CommunicatorPtr& communicator)
{
    tprintf("testing facet registration exceptions... ");
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("FacetExceptionTestAdapter");
    Ice::ObjectPtr obj = new EmptyI;
    adapter->add(obj, communicator->stringToIdentity("d"));
    adapter->addFacet(obj, communicator->stringToIdentity("d"), "facetABCD");
    try
    {
	adapter->addFacet(obj, communicator->stringToIdentity("d"), "facetABCD");
	test(false);
    }
    catch(Ice::AlreadyRegisteredException&)
    {
    }
    adapter->removeFacet(communicator->stringToIdentity("d"), "facetABCD");
    try
    {
	adapter->removeFacet(communicator->stringToIdentity("d"), "facetABCD");
	test(false);
    }
    catch(Ice::NotRegisteredException&)
    {
    }
    tprintf("ok\n");

    tprintf("testing removeAllFacets... ");
    Ice::ObjectPtr obj1 = new EmptyI;
    Ice::ObjectPtr obj2 = new EmptyI;
    adapter->addFacet(obj1, communicator->stringToIdentity("id1"), "f1");
    adapter->addFacet(obj2, communicator->stringToIdentity("id1"), "f2");
    Ice::ObjectPtr obj3 = new EmptyI;
    adapter->addFacet(obj1, communicator->stringToIdentity("id2"), "f1");
    adapter->addFacet(obj2, communicator->stringToIdentity("id2"), "f2");
    adapter->addFacet(obj3, communicator->stringToIdentity("id2"), "");
    Ice::FacetMap fm = adapter->removeAllFacets(communicator->stringToIdentity("id1"));
    test(fm.size() == 2);
    test(fm["f1"] == obj1);
    test(fm["f2"] == obj2);
    try
    {
	adapter->removeAllFacets(communicator->stringToIdentity("id1"));
	test(false);
    }
    catch(Ice::NotRegisteredException&)
    {
    }
    fm = adapter->removeAllFacets(communicator->stringToIdentity("id2"));
    test(fm.size() == 3);
    test(fm["f1"] == obj1);
    test(fm["f2"] == obj2);
    test(fm[""] == obj3);
    tprintf("ok\n");

    adapter->deactivate();

    tprintf("testing stringToProxy... ");
    string ref = communicator->getProperties()->getPropertyWithDefault("Facets.Proxy", "d:default -p 12010 -t 10000");
    Ice::ObjectPrx db = communicator->stringToProxy(ref);
    test(db);
    tprintf("ok\n");

    tprintf("testing unchecked cast... ");
    Ice::ObjectPrx prx = Ice::ObjectPrx::uncheckedCast(db);
    test(prx->ice_getFacet().empty());
    prx = Ice::ObjectPrx::uncheckedCast(db, "facetABCD");
    test(prx->ice_getFacet() == "facetABCD");
    Ice::ObjectPrx prx2 = Ice::ObjectPrx::uncheckedCast(prx);
    test(prx2->ice_getFacet() == "facetABCD");
    Ice::ObjectPrx prx3 = Ice::ObjectPrx::uncheckedCast(prx, "");
    test(prx3->ice_getFacet().empty());
    DPrx d = Test::DPrx::uncheckedCast(db);
    test(d->ice_getFacet().empty());
    DPrx df = Test::DPrx::uncheckedCast(db, "facetABCD");
    test(df->ice_getFacet() == "facetABCD");
    DPrx df2 = Test::DPrx::uncheckedCast(df);
    test(df2->ice_getFacet() == "facetABCD");
    DPrx df3 = Test::DPrx::uncheckedCast(df, "");
    test(df3->ice_getFacet().empty());
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    prx = Ice::ObjectPrx::checkedCast(db);
    test(prx->ice_getFacet().empty());
    prx = Ice::ObjectPrx::checkedCast(db, "facetABCD");
    test(prx->ice_getFacet() == "facetABCD");
    prx2 = Ice::ObjectPrx::checkedCast(prx);
    test(prx2->ice_getFacet() == "facetABCD");
    prx3 = Ice::ObjectPrx::checkedCast(prx, "");
    test(prx3->ice_getFacet().empty());
    d = Test::DPrx::checkedCast(db);
    test(d->ice_getFacet().empty());
    df = Test::DPrx::checkedCast(db, "facetABCD");
    test(df->ice_getFacet() == "facetABCD");
    df2 = Test::DPrx::checkedCast(df);
    test(df2->ice_getFacet() == "facetABCD");
    df3 = Test::DPrx::checkedCast(df, "");
    test(df3->ice_getFacet().empty());
    tprintf("ok\n");

    tprintf("testing non-facets A, B, C, and D... ");
    d = DPrx::checkedCast(db);
    test(d);
    test(d == db);
    test(d->callA() == "A");
    test(d->callB() == "B");
    test(d->callC() == "C");
    test(d->callD() == "D");
    tprintf("ok\n");

    tprintf("testing facets A, B, C, and D... ");
    df = DPrx::checkedCast(d, "facetABCD");
    test(df);
    test(df->callA() == "A");
    test(df->callB() == "B");
    test(df->callC() == "C");
    test(df->callD() == "D");
    tprintf("ok\n");

    tprintf("testing facets E and F... ");
    FPrx ff = FPrx::checkedCast(d, "facetEF");
    test(ff);
    test(ff->callE() == "E");
    test(ff->callF() == "F");
    tprintf("ok\n");

    tprintf("testing facet G... ");
    GPrx gf = GPrx::checkedCast(ff, "facetGH");
    test(gf);
    test(gf->callG() == "G");
    tprintf("ok\n");

    tprintf("testing whether casting preserves the facet... ");
    HPrx hf = HPrx::checkedCast(gf);
    test(hf);
    test(hf->callG() == "G");
    test(hf->callH() == "H");
    tprintf("ok\n");

    return gf;
}
