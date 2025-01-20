// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "Test.h"
#include "TestHelper.h"

using namespace std;
using namespace Ice;
using namespace Test;

class EmptyI : public virtual Empty
{
};

GPrx
allTests(Test::TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();

    cout << "testing Ice.Admin.Facets property... " << flush;
    test(communicator->getProperties()->getIcePropertyAsList("Ice.Admin.Facets").empty());
    communicator->getProperties()->setProperty("Ice.Admin.Facets", "foobar");
    StringSeq facetFilter = communicator->getProperties()->getIcePropertyAsList("Ice.Admin.Facets");
    test(facetFilter.size() == 1 && facetFilter[0] == "foobar");
    communicator->getProperties()->setProperty("Ice.Admin.Facets", "foo\\'bar");
    facetFilter = communicator->getProperties()->getIcePropertyAsList("Ice.Admin.Facets");
    test(facetFilter.size() == 1 && facetFilter[0] == "foo'bar");
    communicator->getProperties()->setProperty("Ice.Admin.Facets", "'foo bar' toto 'titi'");
    facetFilter = communicator->getProperties()->getIcePropertyAsList("Ice.Admin.Facets");
    test(
        facetFilter.size() == 3 && facetFilter[0] == "foo bar" && facetFilter[1] == "toto" && facetFilter[2] == "titi");
    communicator->getProperties()->setProperty("Ice.Admin.Facets", "'foo bar\\' toto' 'titi'");
    facetFilter = communicator->getProperties()->getIcePropertyAsList("Ice.Admin.Facets");
    test(facetFilter.size() == 2 && facetFilter[0] == "foo bar' toto" && facetFilter[1] == "titi");
    // communicator->getProperties()->setProperty("Ice.Admin.Facets", "'foo bar' 'toto titi");
    // facetFilter = communicator->getProperties()->getIcePropertyAsList("Ice.Admin.Facets");
    // test(facetFilter.size() == 0);
    communicator->getProperties()->setProperty("Ice.Admin.Facets", "");
    cout << "ok" << endl;

    cout << "testing facet registration exceptions... " << flush;
    string localOAEndpoint;
    {
        ostringstream ostr;
        if (communicator->getProperties()->getIceProperty("Ice.Default.Protocol") == "bt")
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
    if (communicator->getProperties()->getIceProperty("Ice.Default.Protocol") != "ssl" &&
        communicator->getProperties()->getIceProperty("Ice.Default.Protocol") != "wss")
    {
        ObjectAdapterPtr adapter = communicator->createObjectAdapter("FacetExceptionTestAdapter");
        ObjectPtr obj = std::make_shared<EmptyI>();
        adapter->add(obj, stringToIdentity("d"));
        adapter->addFacet(obj, stringToIdentity("d"), "facetABCD");
        try
        {
            adapter->addFacet(obj, stringToIdentity("d"), "facetABCD");
            test(false);
        }
        catch (const AlreadyRegisteredException&)
        {
        }
        adapter->removeFacet(stringToIdentity("d"), "facetABCD");
        try
        {
            adapter->removeFacet(stringToIdentity("d"), "facetABCD");
            test(false);
        }
        catch (const NotRegisteredException&)
        {
        }
        cout << "ok" << endl;

        cout << "testing removeAllFacets... " << flush;
        ObjectPtr obj1 = std::make_shared<EmptyI>();
        ObjectPtr obj2 = std::make_shared<EmptyI>();
        adapter->addFacet(obj1, stringToIdentity("id1"), "f1");
        adapter->addFacet(obj2, stringToIdentity("id1"), "f2");
        ObjectPtr obj3 = std::make_shared<EmptyI>();
        adapter->addFacet(obj1, stringToIdentity("id2"), "f1");
        adapter->addFacet(obj2, stringToIdentity("id2"), "f2");
        adapter->addFacet(obj3, stringToIdentity("id2"), "");
        FacetMap fm = adapter->removeAllFacets(stringToIdentity("id1"));
        test(fm.size() == 2);
        test(ObjectPtr(fm["f1"]) == obj1);
        test(ObjectPtr(fm["f2"]) == obj2);
        try
        {
            adapter->removeAllFacets(stringToIdentity("id1"));
            test(false);
        }
        catch (const NotRegisteredException&)
        {
        }
        fm = adapter->removeAllFacets(stringToIdentity("id2"));
        test(fm.size() == 3);
        test(ObjectPtr(fm["f1"]) == obj1);
        test(ObjectPtr(fm["f2"]) == obj2);
        test(ObjectPtr(fm[""]) == obj3);
        cout << "ok" << endl;

        adapter->deactivate();
    }

    string ref = "d:" + helper->getTestEndpoint();
    ObjectPrx db(communicator, ref);

    cout << "testing unchecked cast... " << flush;
    optional<ObjectPrx> prx = uncheckedCast<ObjectPrx>(db);
    test(prx->ice_getFacet().empty());
    prx = uncheckedCast<ObjectPrx>(db, "facetABCD");
    test(prx->ice_getFacet() == "facetABCD");
    optional<ObjectPrx> prx2 = uncheckedCast<ObjectPrx>(prx);
    test(prx2->ice_getFacet() == "facetABCD");

    optional<ObjectPrx> prx3 = uncheckedCast<ObjectPrx>(prx, "");
    test(prx3->ice_getFacet().empty());
    optional<DPrx> d = uncheckedCast<Test::DPrx>(db);
    test(d->ice_getFacet().empty());
    optional<DPrx> df = uncheckedCast<Test::DPrx>(db, "facetABCD");
    test(df->ice_getFacet() == "facetABCD");
    optional<DPrx> df2 = uncheckedCast<Test::DPrx>(df);
    test(df2->ice_getFacet() == "facetABCD");
    optional<DPrx> df3 = uncheckedCast<Test::DPrx>(df, "");
    test(df3->ice_getFacet().empty());
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    prx = checkedCast<ObjectPrx>(db);
    test(prx->ice_getFacet().empty());
    prx = checkedCast<ObjectPrx>(db, "facetABCD");
    test(prx->ice_getFacet() == "facetABCD");
    prx2 = checkedCast<ObjectPrx>(prx);
    test(prx2->ice_getFacet() == "facetABCD");
    prx3 = checkedCast<ObjectPrx>(prx, "");
    test(prx3->ice_getFacet().empty());
    d = checkedCast<Test::DPrx>(db);
    test(d->ice_getFacet().empty());
    df = checkedCast<Test::DPrx>(db, "facetABCD");
    test(df->ice_getFacet() == "facetABCD");
    df2 = checkedCast<Test::DPrx>(df);
    test(df2->ice_getFacet() == "facetABCD");
    df3 = checkedCast<Test::DPrx>(df, "");
    test(df3->ice_getFacet().empty());
    cout << "ok" << endl;

    cout << "testing non-facets A, B, C, and D... " << flush;
    d = checkedCast<DPrx>(db);
    test(d);
    test(d == db);
    test(d->callA() == "A");
    test(d->callB() == "B");
    test(d->callC() == "C");
    test(d->callD() == "D");
    cout << "ok" << endl;

    cout << "testing facets A, B, C, and D... " << flush;
    df = checkedCast<DPrx>(d, "facetABCD");
    test(df);
    test(df->callA() == "A");
    test(df->callB() == "B");
    test(df->callC() == "C");
    test(df->callD() == "D");
    cout << "ok" << endl;

    cout << "testing facets E and F... " << flush;
    auto ff = checkedCast<FPrx>(d, "facetEF");
    test(ff);
    test(ff->callE() == "E");
    test(ff->callF() == "F");
    cout << "ok" << endl;

    cout << "testing facet G... " << flush;
    auto gf = checkedCast<GPrx>(ff, "facetGH");
    test(gf);
    test(gf->callG() == "G");
    cout << "ok" << endl;

    cout << "testing whether casting preserves the facet... " << flush;
    optional<HPrx> hf = checkedCast<HPrx>(gf);
    test(hf);
    test(hf->callG() == "G");
    test(hf->callH() == "H");
    cout << "ok" << endl;

    return gf.value();
}
