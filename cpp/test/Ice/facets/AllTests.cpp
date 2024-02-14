//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Test;

class EmptyI : public virtual Empty
{
};

GPrxPtr
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();

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

    cout << "testing facet registration exceptions... " << flush;
    string localOAEndpoint;
    {
        ostringstream ostr;
        if(communicator->getProperties()->getProperty("Ice.Default.Protocol") == "bt")
        {
            ostr << "default -a *";
        }
        else
        {
            ostr << "default -h *";
        }
        localOAEndpoint = ostr.str();
    }
    communicator->getProperties()->setProperty("FacetExceptionTestAdapter.Endpoints", localOAEndpoint);
    if(communicator->getProperties()->getProperty("Ice.Default.Protocol") != "ssl" &&
       communicator->getProperties()->getProperty("Ice.Default.Protocol") != "wss")
    {
        Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("FacetExceptionTestAdapter");
        Ice::ObjectPtr obj = std::make_shared<EmptyI>();
        adapter->add(obj, Ice::stringToIdentity("d"));
        adapter->addFacet(obj, Ice::stringToIdentity("d"), "facetABCD");
        try
        {
            adapter->addFacet(obj, Ice::stringToIdentity("d"), "facetABCD");
            test(false);
        }
        catch(const Ice::AlreadyRegisteredException&)
        {
        }
        adapter->removeFacet(Ice::stringToIdentity("d"), "facetABCD");
        try
        {
            adapter->removeFacet(Ice::stringToIdentity("d"), "facetABCD");
            test(false);
        }
        catch(const Ice::NotRegisteredException&)
        {
        }
        cout << "ok" << endl;

        cout << "testing removeAllFacets... " << flush;
        Ice::ObjectPtr obj1 = std::make_shared<EmptyI>();
        Ice::ObjectPtr obj2 = std::make_shared<EmptyI>();
        adapter->addFacet(obj1, Ice::stringToIdentity("id1"), "f1");
        adapter->addFacet(obj2, Ice::stringToIdentity("id1"), "f2");
        Ice::ObjectPtr obj3 = std::make_shared<EmptyI>();
        adapter->addFacet(obj1, Ice::stringToIdentity("id2"), "f1");
        adapter->addFacet(obj2, Ice::stringToIdentity("id2"), "f2");
        adapter->addFacet(obj3, Ice::stringToIdentity("id2"), "");
        Ice::FacetMap fm = adapter->removeAllFacets(Ice::stringToIdentity("id1"));
        test(fm.size() == 2);
        test(Ice::ObjectPtr(fm["f1"]) == obj1);
        test(Ice::ObjectPtr(fm["f2"]) == obj2);
        try
        {
            adapter->removeAllFacets(Ice::stringToIdentity("id1"));
            test(false);
        }
        catch(const Ice::NotRegisteredException&)
        {
        }
        fm = adapter->removeAllFacets(Ice::stringToIdentity("id2"));
        test(fm.size() == 3);
        test(Ice::ObjectPtr(fm["f1"]) == obj1);
        test(Ice::ObjectPtr(fm["f2"]) == obj2);
        test(Ice::ObjectPtr(fm[""]) == obj3);
        cout << "ok" << endl;

        adapter->deactivate();
    }

    cout << "testing stringToProxy... " << flush;
    string ref = "d:" + helper->getTestEndpoint();
    Ice::ObjectPrx db = communicator->stringToProxy(ref);
    test(db);
    cout << "ok" << endl;

    cout << "testing unchecked cast... " << flush;
    Ice::ObjectPrx prx = Ice::uncheckedCast<Ice::ObjectPrx>(db);
    test(prx->ice_getFacet().empty());
    prx = Ice::uncheckedCast<Ice::ObjectPrx>(db, "facetABCD");
    test(prx->ice_getFacet() == "facetABCD");
    Ice::ObjectPrx prx2 = Ice::uncheckedCast<Ice::ObjectPrx>(prx);
    test(prx2->ice_getFacet() == "facetABCD");

    Ice::ObjectPrx prx3 = Ice::uncheckedCast<Ice::ObjectPrx>(prx, "");
    test(prx3->ice_getFacet().empty());
    DPrxPtr d = Ice::uncheckedCast<Test::DPrx>(db);
    test(d->ice_getFacet().empty());
    DPrxPtr df = Ice::uncheckedCast<Test::DPrx>(db, "facetABCD");
    test(df->ice_getFacet() == "facetABCD");
    DPrxPtr df2 = Ice::uncheckedCast<Test::DPrx>(df);
    test(df2->ice_getFacet() == "facetABCD");
    DPrxPtr df3 = Ice::uncheckedCast<Test::DPrx>(df, "");
    test(df3->ice_getFacet().empty());
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    prx = Ice::checkedCast<Ice::ObjectPrx>(db);
    test(prx->ice_getFacet().empty());
    prx = Ice::checkedCast<Ice::ObjectPrx>(db, "facetABCD");
    test(prx->ice_getFacet() == "facetABCD");
    prx2 = Ice::checkedCast<Ice::ObjectPrx>(prx);
    test(prx2->ice_getFacet() == "facetABCD");
    prx3 = Ice::checkedCast<Ice::ObjectPrx>(prx, "");
    test(prx3->ice_getFacet().empty());
    d = Ice::checkedCast<Test::DPrx>(db);
    test(d->ice_getFacet().empty());
    df = Ice::checkedCast<Test::DPrx>(db, "facetABCD");
    test(df->ice_getFacet() == "facetABCD");
    df2 = Ice::checkedCast<Test::DPrx>(df);
    test(df2->ice_getFacet() == "facetABCD");
    df3 = Ice::checkedCast<Test::DPrx>(df, "");
    test(df3->ice_getFacet().empty());
    cout << "ok" << endl;

    cout << "testing non-facets A, B, C, and D... " << flush;
    d = Ice::checkedCast<DPrx>(db);
    test(d);
    test(Ice::targetEqualTo(d, db));
    test(d->callA() == "A");
    test(d->callB() == "B");
    test(d->callC() == "C");
    test(d->callD() == "D");
    cout << "ok" << endl;

    cout << "testing facets A, B, C, and D... " << flush;
    df = Ice::checkedCast<DPrx>(d, "facetABCD");
    test(df);
    test(df->callA() == "A");
    test(df->callB() == "B");
    test(df->callC() == "C");
    test(df->callD() == "D");
    cout << "ok" << endl;

    cout << "testing facets E and F... " << flush;
    auto ff = Ice::checkedCast<FPrx>(d, "facetEF");
    test(ff);
    test(ff->callE() == "E");
    test(ff->callF() == "F");
    cout << "ok" << endl;

    cout << "testing facet G... " << flush;
    auto gf = Ice::checkedCast<GPrx>(ff, "facetGH");
    test(gf);
    test(gf->callG() == "G");
    cout << "ok" << endl;

    cout << "testing whether casting preserves the facet... " << flush;
    HPrxPtr hf = Ice::checkedCast<HPrx>(gf);
    test(hf);
    test(hf->callG() == "G");
    test(hf->callH() == "H");
    cout << "ok" << endl;

    return gf;
}
