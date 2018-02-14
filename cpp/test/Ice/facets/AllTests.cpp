// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
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
#ifdef ICE_OS_WINRT
    bool winrt = true;
#else
    bool winrt = false;
#endif
    cout << "testing Ice.Admin.Facets property... " << flush;
    test(communicator->getProperties()->getPropertyAsList("Ice.Admin.Facets").empty());
    communicator->getProperties()->setProperty("Ice.Admin.Facets", "foobar");
    Ice::StringSeq facetFilter = communicator->getProperties()->getPropertyAsList("Ice.Admin.Facets");
    test(facetFilter.size() == 1 && facetFilter[0] == "foobar");
    communicator->getProperties()->setProperty("Ice.Admin.Facets", "foo\\'bar");
    facetFilter = communicator->getProperties()->getPropertyAsList("Ice.Admin.Facets");
    test(facetFilter.size() == 1 && facetFilter[0] == "foo'bar");
    communicator->getProperties()->setProperty("Ice.Admin.Facets", "'foo bar' toto 'titi'");
    facetFilter = communicator->getProperties()->getPropertyAsList("Ice.Admin.Facets");
    test(facetFilter.size() == 3 && facetFilter[0] == "foo bar" && facetFilter[1] == "toto" &&
         facetFilter[2] == "titi");
    communicator->getProperties()->setProperty("Ice.Admin.Facets", "'foo bar\\' toto' 'titi'");
    facetFilter = communicator->getProperties()->getPropertyAsList("Ice.Admin.Facets");
    test(facetFilter.size() == 2 && facetFilter[0] == "foo bar' toto" && facetFilter[1] == "titi");
    // communicator->getProperties()->setProperty("Ice.Admin.Facets", "'foo bar' 'toto titi");
    // facetFilter = communicator->getProperties()->getPropertyAsList("Ice.Admin.Facets");
    // test(facetFilter.size() == 0);
    communicator->getProperties()->setProperty("Ice.Admin.Facets", "");
    cout << "ok" << endl;

    if(!winrt || (communicator->getProperties()->getProperty("Ice.Default.Protocol") != "ssl" &&
                  communicator->getProperties()->getProperty("Ice.Default.Protocol") != "wss"))
    {
        cout << "testing facet registration exceptions... " << flush;
        string host = communicator->getProperties()->getPropertyAsIntWithDefault("Ice.IPv6", 0) == 0 ?
            "127.0.0.1" : "\"0:0:0:0:0:0:0:1\"";
        communicator->getProperties()->setProperty("FacetExceptionTestAdapter.Endpoints", "default -h " + host);
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("FacetExceptionTestAdapter");
        Ice::ObjectPtr obj = new EmptyI;
        adapter->add(obj, communicator->stringToIdentity("d"));
        adapter->addFacet(obj, communicator->stringToIdentity("d"), "facetABCD");
        try
        {
            adapter->addFacet(obj, communicator->stringToIdentity("d"), "facetABCD");
            test(false);
        }
        catch(const Ice::AlreadyRegisteredException&)
        {
        }
        adapter->removeFacet(communicator->stringToIdentity("d"), "facetABCD");
        try
        {
            adapter->removeFacet(communicator->stringToIdentity("d"), "facetABCD");
            test(false);
        }
        catch(const Ice::NotRegisteredException&)
        {
        }
        cout << "ok" << endl;

        cout << "testing removeAllFacets... " << flush;
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
        catch(const Ice::NotRegisteredException&)
        {
        }
        fm = adapter->removeAllFacets(communicator->stringToIdentity("id2"));
        test(fm.size() == 3);
        test(fm["f1"] == obj1);
        test(fm["f2"] == obj2);
        test(fm[""] == obj3);
        cout << "ok" << endl;

        adapter->deactivate();
    }

    cout << "testing stringToProxy... " << flush;
    string ref = "d:default -p 12010";
    Ice::ObjectPrx db = communicator->stringToProxy(ref);
    test(db);
    cout << "ok" << endl;

    cout << "testing unchecked cast... " << flush;
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
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
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
    cout << "ok" << endl;

    cout << "testing non-facets A, B, C, and D... " << flush;
    d = DPrx::checkedCast(db);
    test(d);
    test(d == db);
    test(d->callA() == "A");
    test(d->callB() == "B");
    test(d->callC() == "C");
    test(d->callD() == "D");
    cout << "ok" << endl;

    cout << "testing facets A, B, C, and D... " << flush;
    df = DPrx::checkedCast(d, "facetABCD");
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

    cout << "testing facet G... " << flush;
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
