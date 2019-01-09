// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Test;

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    string ref = "DemoIceBox/admin:default -p 9996 -t 10000";
    Ice::ObjectPrxPtr admin = communicator->stringToProxy(ref);

    TestFacetPrxPtr facet;

    cout << "testing custom facet... " << flush;
    {
        //
        // Test: Verify that the custom facet is present.
        //
#ifdef ICE_CPP11_MAPPING
        facet = Ice::checkedCast<Test::TestFacetPrx>(admin, "TestFacet");
#else
        facet = Test::TestFacetPrx::checkedCast(admin, "TestFacet");
#endif
        facet->ice_ping();
    }
    cout << "ok" << endl;

    cout << "testing properties facet... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto pa = Ice::checkedCast<Ice::PropertiesAdminPrx>(admin, "IceBox.Service.TestService.Properties");
#else
        Ice::PropertiesAdminPrx pa =
            Ice::PropertiesAdminPrx::checkedCast(admin, "IceBox.Service.TestService.Properties");
#endif
        //
        // Test: PropertiesAdmin::getProperty()
        //
        test(pa->getProperty("Prop1") == "1");
        test(pa->getProperty("Bogus") == "");

        //
        // Test: PropertiesAdmin::getProperties()
        //
        Ice::PropertyDict pd = pa->getPropertiesForPrefix("");
        test(pd.size() == 6);
        test(pd["Prop1"] == "1");
        test(pd["Prop2"] == "2");
        test(pd["Prop3"] == "3");
        test(pd["Ice.Config"] == "config.service");
        test(pd["Ice.ProgramName"] == "IceBox-TestService");
        test(pd["Ice.Admin.Enabled"] == "1");

        Ice::PropertyDict changes;

        //
        // Test: PropertiesAdmin::setProperties()
        //
        Ice::PropertyDict setProps;
        setProps["Prop1"] = "10"; // Changed
        setProps["Prop2"] = "20"; // Changed
        setProps["Prop3"] = ""; // Removed
        setProps["Prop4"] = "4"; // Added
        setProps["Prop5"] = "5"; // Added
        pa->setProperties(setProps);
        test(pa->getProperty("Prop1") == "10");
        test(pa->getProperty("Prop2") == "20");
        test(pa->getProperty("Prop3") == "");
        test(pa->getProperty("Prop4") == "4");
        test(pa->getProperty("Prop5") == "5");
        changes = facet->getChanges();
        test(changes.size() == 5);
        test(changes["Prop1"] == "10");
        test(changes["Prop2"] == "20");
        test(changes["Prop3"] == "");
        test(changes["Prop4"] == "4");
        test(changes["Prop5"] == "5");
        pa->setProperties(setProps);
        changes = facet->getChanges();
        test(changes.empty());
    }
    cout << "ok" << endl;

    cout << "testing metrics admin facet... " << flush;
    {
#ifdef ICE_CPP11_MAPPING
        auto ma = Ice::checkedCast<IceMX::MetricsAdminPrx>(admin, "IceBox.Service.TestService.Metrics");
        auto pa = Ice::checkedCast<Ice::PropertiesAdminPrx>(admin, "IceBox.Service.TestService.Properties");
#else
        IceMX::MetricsAdminPrx ma = IceMX::MetricsAdminPrx::checkedCast(admin, "IceBox.Service.TestService.Metrics");
        Ice::PropertiesAdminPrx pa =
            Ice::PropertiesAdminPrx::checkedCast(admin, "IceBox.Service.TestService.Properties");
#endif
        Ice::StringSeq views;
        Ice::StringSeq disabledViews;
        views = ma->getMetricsViewNames(disabledViews);
        test(views.empty());

        Ice::PropertyDict setProps;
        setProps["IceMX.Metrics.Debug.GroupBy"] = "id";
        setProps["IceMX.Metrics.All.GroupBy"] = "none";
        setProps["IceMX.Metrics.Parent.GroupBy"] = "parent";
        pa->setProperties(setProps);
        pa->setProperties(Ice::PropertyDict());

        views = ma->getMetricsViewNames(disabledViews);
        test(views.size() == 3);

        // Make sure that the IceBox communicator metrics admin is a separate instance.
#ifdef ICE_CPP11_MAPPING
        test(Ice::checkedCast<IceMX::MetricsAdminPrx>(admin, "Metrics")->getMetricsViewNames(disabledViews).empty());
#else
        test(IceMX::MetricsAdminPrx::checkedCast(admin, "Metrics")->getMetricsViewNames(disabledViews).empty());
#endif
    }
    cout << "ok" << endl;
}
