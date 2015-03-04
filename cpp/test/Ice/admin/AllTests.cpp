// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <TestI.h> // For TestFacetI

using namespace std;
using namespace Test;

static void
testFacets(const Ice::CommunicatorPtr& com)
{
    test(com->findAdminFacet("Properties"));
    test(com->findAdminFacet("Process"));

    TestFacetPtr f1 = new TestFacetI;
    TestFacetPtr f2 = new TestFacetI;
    TestFacetPtr f3 = new TestFacetI;

    com->addAdminFacet(f1, "Facet1");
    com->addAdminFacet(f2, "Facet2");
    com->addAdminFacet(f3, "Facet3");

    test(com->findAdminFacet("Facet1") == f1);
    test(com->findAdminFacet("Facet2") == f2);
    test(com->findAdminFacet("Facet3") == f3);
    test(!com->findAdminFacet("Bogus"));

    try
    {
        com->addAdminFacet(f1, "Facet1");
        test(false);
    }
    catch(const Ice::AlreadyRegisteredException&)
    {
        // Expected
    }

    try
    {
        com->removeAdminFacet("Bogus");
        test(false);
    }
    catch(const Ice::NotRegisteredException&)
    {
        // Expected
    }

    com->removeAdminFacet("Facet1");
    com->removeAdminFacet("Facet2");
    com->removeAdminFacet("Facet3");

    try
    {
        com->removeAdminFacet("Facet1");
        test(false);
    }
    catch(const Ice::NotRegisteredException&)
    {
        // Expected
    }
}

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing communicator operations... " << flush;
    {
        //
        // Test: Exercise addAdminFacet, findAdminFacet, removeAdminFacet with a typical configuration.
        //
        Ice::InitializationData init;
        init.properties = Ice::createProperties();
        init.properties->setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        init.properties->setProperty("Ice.Admin.InstanceName", "Test");
        Ice::CommunicatorPtr com = Ice::initialize(init);
        testFacets(com);
        com->destroy();
    }
    {
        //
        // Test: Verify that the operations work correctly in the presence of facet filters.
        //
        Ice::InitializationData init;
        init.properties = Ice::createProperties();
        init.properties->setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        init.properties->setProperty("Ice.Admin.InstanceName", "Test");
        init.properties->setProperty("Ice.Admin.Facets", "Properties");
        Ice::CommunicatorPtr com = Ice::initialize(init);
        testFacets(com);
        com->destroy();
    }
    {
        //
        // Test: Verify that the operations work correctly with the Admin object disabled.
        //
        Ice::CommunicatorPtr com = Ice::initialize();
        testFacets(com);
        com->destroy();
    }
    {
        //
        // Test: Verify that the operations work correctly when creation of the Admin object is delayed.
        //
        Ice::InitializationData init;
        init.properties = Ice::createProperties();
        init.properties->setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        init.properties->setProperty("Ice.Admin.InstanceName", "Test");
        init.properties->setProperty("Ice.Admin.DelayCreation", "1");
        Ice::CommunicatorPtr com = Ice::initialize(init);
        testFacets(com);
        com->getAdmin();
        testFacets(com);
        com->destroy();
    }
    cout << "ok" << endl;

    string ref = "factory:default -p 12010 -t 10000";
    RemoteCommunicatorFactoryPrx factory =
        RemoteCommunicatorFactoryPrx::uncheckedCast(communicator->stringToProxy(ref));

    cout << "testing process facet... " << flush;
    {
        //
        // Test: Verify that Process::shutdown() operation shuts down the communicator.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1";
        props["Ice.Admin.InstanceName"] = "Test";
        RemoteCommunicatorPrx com = factory->createCommunicator(props);
        Ice::ObjectPrx obj = com->getAdmin();
        Ice::ProcessPrx proc = Ice::ProcessPrx::checkedCast(obj, "Process");
        proc->shutdown();
        com->waitForShutdown();
        com->destroy();
    }
    cout << "ok" << endl;

    cout << "testing properties facet... " << flush;
    {
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1";
        props["Ice.Admin.InstanceName"] = "Test";
        props["Prop1"] = "1";
        props["Prop2"] = "2";
        props["Prop3"] = "3";
        RemoteCommunicatorPrx com = factory->createCommunicator(props);
        Ice::ObjectPrx obj = com->getAdmin();
        Ice::PropertiesAdminPrx pa = Ice::PropertiesAdminPrx::checkedCast(obj, "Properties");

        //
        // Test: PropertiesAdmin::getProperty()
        //
        test(pa->getProperty("Prop2") == "2");
        test(pa->getProperty("Bogus") == "");

        //
        // Test: PropertiesAdmin::getProperties()
        //
        Ice::PropertyDict pd = pa->getPropertiesForPrefix("");
        test(pd.size() == 5);
        test(pd["Ice.Admin.Endpoints"] == "tcp -h 127.0.0.1");
        test(pd["Ice.Admin.InstanceName"] == "Test");
        test(pd["Prop1"] == "1");
        test(pd["Prop2"] == "2");
        test(pd["Prop3"] == "3");

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
        changes = com->getChanges();
        test(changes.size() == 5);
        test(changes["Prop1"] == "10");
        test(changes["Prop2"] == "20");
        test(changes["Prop3"] == "");
        test(changes["Prop4"] == "4");
        test(changes["Prop5"] == "5");
        pa->setProperties(setProps);
        changes = com->getChanges();
        test(changes.empty());

        com->destroy();
    }
    cout << "ok" << endl;

    cout << "testing custom facet... " << flush;
    {
        //
        // Test: Verify that the custom facet is present.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1";
        props["Ice.Admin.InstanceName"] = "Test";
        RemoteCommunicatorPrx com = factory->createCommunicator(props);
        Ice::ObjectPrx obj = com->getAdmin();
        Test::TestFacetPrx tf = Test::TestFacetPrx::checkedCast(obj, "TestFacet");
        tf->op();
        com->destroy();
    }
    cout << "ok" << endl;

    cout << "testing facet filtering... " << flush;
    {
        //
        // Test: Set Ice.Admin.Facets to expose only the Properties facet,
        // meaning no other facet is available.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1";
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "Properties";
        RemoteCommunicatorPrx com = factory->createCommunicator(props);
        Ice::ObjectPrx obj = com->getAdmin();
        Ice::ProcessPrx proc = Ice::ProcessPrx::checkedCast(obj, "Process");
        test(!proc);
        Test::TestFacetPrx tf = Test::TestFacetPrx::checkedCast(obj, "TestFacet");
        test(!tf);
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose only the Process facet,
        // meaning no other facet is available.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1";
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "Process";
        RemoteCommunicatorPrx com = factory->createCommunicator(props);
        Ice::ObjectPrx obj = com->getAdmin();
        Ice::PropertiesAdminPrx pa = Ice::PropertiesAdminPrx::checkedCast(obj, "Properties");
        test(!pa);
        Test::TestFacetPrx tf = Test::TestFacetPrx::checkedCast(obj, "TestFacet");
        test(!tf);
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose only the TestFacet facet,
        // meaning no other facet is available.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1";
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "TestFacet";
        RemoteCommunicatorPrx com = factory->createCommunicator(props);
        Ice::ObjectPrx obj = com->getAdmin();
        Ice::PropertiesAdminPrx pa = Ice::PropertiesAdminPrx::checkedCast(obj, "Properties");
        test(!pa);
        Ice::ProcessPrx proc = Ice::ProcessPrx::checkedCast(obj, "Process");
        test(!proc);
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose two facets. Use whitespace to separate the
        // facet names.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1";
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "Properties TestFacet";
        RemoteCommunicatorPrx com = factory->createCommunicator(props);
        Ice::ObjectPrx obj = com->getAdmin();
        Ice::PropertiesAdminPrx pa = Ice::PropertiesAdminPrx::checkedCast(obj, "Properties");
        test(pa->getProperty("Ice.Admin.InstanceName") == "Test");
        Test::TestFacetPrx tf = Test::TestFacetPrx::checkedCast(obj, "TestFacet");
        tf->op();
        Ice::ProcessPrx proc = Ice::ProcessPrx::checkedCast(obj, "Process");
        test(!proc);
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose two facets. Use a comma to separate the
        // facet names.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h 127.0.0.1";
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "TestFacet, Process";
        RemoteCommunicatorPrx com = factory->createCommunicator(props);
        Ice::ObjectPrx obj = com->getAdmin();
        Ice::PropertiesAdminPrx pa = Ice::PropertiesAdminPrx::checkedCast(obj, "Properties");
        test(!pa);
        Test::TestFacetPrx tf = Test::TestFacetPrx::checkedCast(obj, "TestFacet");
        tf->op();
        Ice::ProcessPrx proc = Ice::ProcessPrx::checkedCast(obj, "Process");
        proc->shutdown();
        com->waitForShutdown();
        com->destroy();
    }
    cout << "ok" << endl;

    factory->shutdown();
}
