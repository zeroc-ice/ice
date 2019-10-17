//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <TestHelper.h>
#include <TestI.h> // For TestFacetI

using namespace std;
using namespace Test;
using namespace Ice;

static void
testFacets(const Ice::CommunicatorPtr& com, bool builtInFacets = true)
{
    if(builtInFacets)
    {
        test(com->findAdminFacet("Properties"));
        test(com->findAdminFacet("Process"));
        test(com->findAdminFacet("Logger"));
        test(com->findAdminFacet("Metrics"));
    }

    TestFacetPtr f1 = ICE_MAKE_SHARED(TestFacetI);
    TestFacetPtr f2 = ICE_MAKE_SHARED(TestFacetI);
    TestFacetPtr f3 = ICE_MAKE_SHARED(TestFacetI);

    com->addAdminFacet(f1, "Facet1");
    com->addAdminFacet(f2, "Facet2");
    com->addAdminFacet(f3, "Facet3");

    test(com->findAdminFacet("Facet1") == f1);
    test(com->findAdminFacet("Facet2") == f2);
    test(com->findAdminFacet("Facet3") == f3);
    test(!com->findAdminFacet("Bogus"));

    const Ice::FacetMap facetMap = com->findAllAdminFacets();

    if(builtInFacets)
    {
        test(facetMap.size() == 7);
        test(facetMap.find("Properties") != facetMap.end());
        test(facetMap.find("Process") != facetMap.end());
        test(facetMap.find("Logger") != facetMap.end());
        test(facetMap.find("Metrics") != facetMap.end());
    }
    else
    {
        test(facetMap.size() >= 3);
    }
    test(facetMap.find("Facet1") != facetMap.end());
    test(facetMap.find("Facet2") != facetMap.end());
    test(facetMap.find("Facet3") != facetMap.end());

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

class RemoteLoggerI : public Ice::RemoteLogger
{
public:

    RemoteLoggerI();

    virtual void init(ICE_IN(string), ICE_IN(Ice::LogMessageSeq), const Ice::Current&);
    virtual void log(ICE_IN(Ice::LogMessage), const Ice::Current&);

    void checkNextInit(const string&, Ice::LogMessageType, const string&, const string& = "");
    void checkNextLog(Ice::LogMessageType, const string&, const string& = "");

    void wait(int);

private:

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    int _receivedCalls;
    string _prefix;
    Ice::LogMessageSeq _initMessages;
    Ice::LogMessageSeq _logMessages;
};

ICE_DEFINE_PTR(RemoteLoggerIPtr, RemoteLoggerI);

RemoteLoggerI::RemoteLoggerI() : _receivedCalls(0)
{
}

void
RemoteLoggerI::init(ICE_IN(string) prefix, ICE_IN(Ice::LogMessageSeq) logMessages, const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    _prefix = prefix;
    _initMessages.insert(_initMessages.end(), logMessages.begin(), logMessages.end());
    _receivedCalls++;
    _monitor.notifyAll();
}

void
RemoteLoggerI::log(ICE_IN(Ice::LogMessage) logMessage, const Ice::Current&)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    _logMessages.push_back(logMessage);
    _receivedCalls++;
    _monitor.notifyAll();
}

void
RemoteLoggerI::checkNextInit(const string& prefix, Ice::LogMessageType type, const string& message,
                             const string& category)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    test(_prefix == prefix);
    test(_initMessages.size() > 0);
    Ice::LogMessage front = _initMessages.front();
    test(front.type == type);
    test(front.message == message);
    test(front.traceCategory == category);
    _initMessages.pop_front();
}

void
RemoteLoggerI::checkNextLog(Ice::LogMessageType type, const string& message, const string& category)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    test(_logMessages.size() > 0);
    Ice::LogMessage front = _logMessages.front();
    test(front.type == type);
    test(front.message == message);
    test(front.traceCategory == category);
    _logMessages.pop_front();
}

void
RemoteLoggerI::wait(int calls)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    _receivedCalls -= calls;
    IceUtil::Time now = IceUtil::Time::now(IceUtil::Time::Monotonic);
    IceUtil::Time start = now;
    IceUtil::Time delay = IceUtil::Time::seconds(10);
    while(_receivedCalls < 0)
    {
        _monitor.timedWait(delay);
        now = IceUtil::Time::now(IceUtil::Time::Monotonic);
        if(now - start >= IceUtil::Time::seconds(10))
        {
            test(false); // Waited for more than 2s for close, something's wrong.
        }
        delay = start + IceUtil::Time::seconds(10) - now;
    }
}

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    cout << "testing communicator operations... " << flush;
    {
        //
        // Test: Exercise addAdminFacet, findAdminFacet, removeAdminFacet with a typical configuration.
        //
        Ice::InitializationData init;
        init.properties = Ice::createProperties();
        init.properties->setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        init.properties->setProperty("Ice.Admin.InstanceName", "Test");
        Ice::CommunicatorHolder ich(init);
        testFacets(ich.communicator());

#ifdef ICE_CPP11_MAPPING
        // Test move assignment on CommunicatorHolder
        Ice::CommunicatorHolder ich2;
        test(!ich2.communicator());
        ich2 = std::move(ich);
        test(ich2.communicator());
        test(!ich.communicator());

        // Equivalent with = and release
        Ice::CommunicatorHolder ich3;
        test(!ich3.communicator());
        ich3 = ich2.release();
        test(ich3.communicator());
        test(!ich2.communicator());
#else
        Ice::CommunicatorHolder ich2;
        test(!ich2.communicator());
        ich2 = ich.release();
        test(ich2.communicator());
        test(!ich.communicator());
#endif
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
        testFacets(com, false);
        com->destroy();
    }
    {
        //
        // Test: Verify that the operations work correctly with the Admin object disabled.
        //
        Ice::CommunicatorPtr com = Ice::initialize();
        testFacets(com, false);
        com->destroy();
    }
    {
        //
        // Test: Verify that the operations work correctly when Ice.Admin.Enabled is set
        //
        Ice::InitializationData init;
        init.properties = Ice::createProperties();
        init.properties->setProperty("Ice.Admin.Enabled", "1");
        Ice::CommunicatorPtr com = Ice::initialize(init);
        test(!com->getAdmin());

        Ice::Identity id = stringToIdentity("test-admin");
        try
        {
            com->createAdmin(0, id);
            test(false);
        }
        catch(const Ice::InitializationException&)
        {
        }

        Ice::ObjectAdapterPtr adapter = com->createObjectAdapter("");
        test(com->createAdmin(adapter, id));
        test(com->getAdmin());

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

    string ref = "factory:" + helper->getTestEndpoint() + " -t 10000";
    RemoteCommunicatorFactoryPrxPtr factory =
        ICE_UNCHECKED_CAST(RemoteCommunicatorFactoryPrx, communicator->stringToProxy(ref));

    string defaultHost = communicator->getProperties()->getProperty("Ice.Default.Host");

    cout << "testing process facet... " << flush;
    {
        //
        // Test: Verify that Process::shutdown() operation shuts down the communicator.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
#ifdef ICE_CPP11_MAPPING
        shared_ptr<Ice::ProcessPrx> proc = Ice::checkedCast<Ice::ProcessPrx>(obj, "Process");
#else
        Ice::ProcessPrx proc = Ice::ProcessPrx::checkedCast(obj, "Process");
#endif
        proc->shutdown();
        com->waitForShutdown();
        com->destroy();
    }
    cout << "ok" << endl;

    cout << "testing properties facet... " << flush;
    {
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Prop1"] = "1";
        props["Prop2"] = "2";
        props["Prop3"] = "3";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
#ifdef ICE_CPP11_MAPPING
        shared_ptr<Ice::PropertiesAdminPrx> pa = Ice::checkedCast<Ice::PropertiesAdminPrx>(obj, "Properties");
#else
        Ice::PropertiesAdminPrx pa = Ice::PropertiesAdminPrx::checkedCast(obj, "Properties");
#endif
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
        test(pd["Ice.Admin.Endpoints"] == "tcp -h " + defaultHost);
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

        com->removeUpdateCallback();
        Ice::PropertyDict moreProps;
        moreProps["Prop1"] = "11"; // Changed
        moreProps["Prop2"] = ""; // Removed
        moreProps["Prop6"] = "6"; // Added
        pa->setProperties(moreProps);
        changes = com->getChanges();
        test(changes.empty());
        com->addUpdateCallback();
        pa->setProperties(moreProps);
        changes = com->getChanges();
        test(changes.empty());
        moreProps["Prop1"] = "10"; // Changed back
        pa->setProperties(moreProps);
        changes = com->getChanges();
        test(changes.size() == 1);
        test(changes["Prop1"] == "10");

        com->destroy();
    }
    cout << "ok" << endl;

#ifndef ICE_OS_UWP
    //
    // This doesn't work well with UWP because connection to localhost are
    // restricted to the same process.
    //
    cout << "testing logger facet... " << flush;
    {
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["NullLogger"] = "1";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);

        com->trace("testCat", "trace");
        com->warning("warning");
        com->error("error");
        com->print("print");

        Ice::ObjectPrxPtr obj = com->getAdmin();
#ifdef ICE_CPP11_MAPPING
        shared_ptr<Ice::LoggerAdminPrx> logger = Ice::checkedCast<Ice::LoggerAdminPrx>(obj, "Logger");
#else
        Ice::LoggerAdminPrx logger = Ice::LoggerAdminPrx::checkedCast(obj, "Logger");
#endif
        test(logger);

        string prefix;

        //
        // Get all
        //
        Ice::LogMessageSeq logMessages =
            logger->getLog(Ice::LogMessageTypeSeq(), Ice::StringSeq(), -1, prefix);

        test(logMessages.size() == 4);
        test(prefix == "NullLogger");
        Ice::LogMessageSeq::const_iterator p = logMessages.begin();
        test(p->traceCategory == "testCat" && p++->message == "trace");
        test(p++->message == "warning");
        test(p++->message == "error");
        test(p++->message == "print");

        //
        // Get only errors and warnings
        //
        com->error("error2");
        com->print("print2");
        com->trace("testCat", "trace2");
        com->warning("warning2");

        Ice::LogMessageTypeSeq messageTypes;
        messageTypes.push_back(ICE_ENUM(LogMessageType, ErrorMessage));
        messageTypes.push_back(ICE_ENUM(LogMessageType, WarningMessage));

        logMessages =
            logger->getLog(messageTypes, Ice::StringSeq(), -1, prefix);
        test(logMessages.size() == 4);
        test(prefix == "NullLogger");

        p = logMessages.begin();
        while(p != logMessages.end())
        {
            test(p->type == ICE_ENUM(LogMessageType, ErrorMessage) || p->type == ICE_ENUM(LogMessageType, WarningMessage));
            ++p;
        }

        //
        // Get only errors and traces with Cat = "testCat"
        //
        com->trace("testCat2", "A");
        com->trace("testCat", "trace3");
        com->trace("testCat2", "B");

        messageTypes.clear();
        messageTypes.push_back(ICE_ENUM(LogMessageType, ErrorMessage));
        messageTypes.push_back(ICE_ENUM(LogMessageType, TraceMessage));

        Ice::StringSeq categories;
        categories.push_back("testCat");

        logMessages =
            logger->getLog(messageTypes, categories, -1, prefix);
        test(logMessages.size() == 5);
        test(prefix == "NullLogger");

        p = logMessages.begin();
        while(p != logMessages.end())
        {
            test(p->type == ICE_ENUM(LogMessageType, ErrorMessage) ||
                (p->type == ICE_ENUM(LogMessageType, TraceMessage) && p->traceCategory == "testCat"));
            ++p;
        }

        //
        // Same, but limited to last 2 messages (trace3 + error3)
        //
        com->error("error3");

        logMessages =
            logger->getLog(messageTypes, categories, 2, prefix);
        test(logMessages.size() == 2);
        test(prefix == "NullLogger");

        p = logMessages.begin();
        test(p++->message == "trace3");
        test(p->message == "error3");

        //
        // Now, test RemoteLogger
        //

        Ice::ObjectAdapterPtr adapter =
            communicator->createObjectAdapterWithEndpoints("RemoteLoggerAdapter", "tcp -h localhost");

        RemoteLoggerIPtr remoteLogger = ICE_MAKE_SHARED(RemoteLoggerI);

        Ice::RemoteLoggerPrxPtr myProxy =
            ICE_UNCHECKED_CAST(Ice::RemoteLoggerPrx, adapter->addWithUUID(remoteLogger));

        adapter->activate();

        //
        // No filtering
        //
        logMessages = logger->getLog(Ice::LogMessageTypeSeq(), Ice::StringSeq(), -1, prefix);
        logger->attachRemoteLogger(myProxy, Ice::LogMessageTypeSeq(), Ice::StringSeq(), -1);
        remoteLogger->wait(1);

        for(LogMessageSeq::const_iterator i = logMessages.begin(); i != logMessages.end(); ++i)
        {
            remoteLogger->checkNextInit(prefix, i->type, i->message, i->traceCategory);
        }

        com->trace("testCat", "rtrace");
        com->warning("rwarning");
        com->error("rerror");
        com->print("rprint");
        remoteLogger->wait(4);

        remoteLogger->checkNextLog(ICE_ENUM(LogMessageType, TraceMessage), "rtrace", "testCat");
        remoteLogger->checkNextLog(ICE_ENUM(LogMessageType, WarningMessage), "rwarning");
        remoteLogger->checkNextLog(ICE_ENUM(LogMessageType, ErrorMessage), "rerror");
        remoteLogger->checkNextLog(ICE_ENUM(LogMessageType, PrintMessage), "rprint");

        test(logger->detachRemoteLogger(myProxy));
        test(!logger->detachRemoteLogger(myProxy));

        //
        // Use Error + Trace with "traceCat" filter with 4 limit
        //
        logMessages = logger->getLog(messageTypes, categories, 4, prefix);
        test(logMessages.size() == 4);
        logger->attachRemoteLogger(myProxy, messageTypes, categories, 4);
        remoteLogger->wait(1);

        for(LogMessageSeq::const_iterator i = logMessages.begin(); i != logMessages.end(); ++i)
        {
            remoteLogger->checkNextInit(prefix, i->type, i->message, i->traceCategory);
        }

        com->warning("rwarning2");
        com->trace("testCat", "rtrace2");
        com->warning("rwarning3");
        com->error("rerror2");
        com->print("rprint2");
        remoteLogger->wait(2);

        remoteLogger->checkNextLog(ICE_ENUM(LogMessageType, TraceMessage), "rtrace2", "testCat");
        remoteLogger->checkNextLog(ICE_ENUM(LogMessageType, ErrorMessage), "rerror2");

        //
        // Attempt reconnection with slightly different proxy
        //
        try
        {
            logger->attachRemoteLogger(myProxy->ice_oneway(), messageTypes, categories, 4);
            test(false);
        }
        catch(const Ice::RemoteLoggerAlreadyAttachedException&)
        {
            // expected
        }

        com->destroy();
    }
    cout << "ok" << endl;
#endif

    cout << "testing custom facet... " << flush;
    {
        //
        // Test: Verify that the custom facet is present.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
#ifdef ICE_CPP11_MAPPING
        shared_ptr<Test::TestFacetPrx> tf = Ice::checkedCast<Test::TestFacetPrx>(obj, "TestFacet");
#else
        Test::TestFacetPrx tf = Test::TestFacetPrx::checkedCast(obj, "TestFacet");
#endif
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
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "Properties";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
#ifdef ICE_CPP11_MAPPING
        shared_ptr<Ice::ProcessPrx> proc = Ice::checkedCast<Ice::ProcessPrx>(obj, "Process");
        test(!proc);
        shared_ptr<Test::TestFacetPrx> tf = Ice::checkedCast<Test::TestFacetPrx>(obj, "TestFacet");
        test(!tf);
#else
        Ice::ProcessPrx proc = Ice::ProcessPrx::checkedCast(obj, "Process");
        test(!proc);
        Test::TestFacetPrx tf = Test::TestFacetPrx::checkedCast(obj, "TestFacet");
        test(!tf);
#endif
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose only the Process facet,
        // meaning no other facet is available.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "Process";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
#ifdef ICE_CPP11_MAPPING
        shared_ptr<Ice::PropertiesAdminPrx> pa = Ice::checkedCast<Ice::PropertiesAdminPrx>(obj, "Properties");
        test(!pa);
        shared_ptr<Test::TestFacetPrx> tf = Ice::checkedCast<Test::TestFacetPrx>(obj, "TestFacet");
        test(!tf);
#else
        Ice::PropertiesAdminPrx pa = Ice::PropertiesAdminPrx::checkedCast(obj, "Properties");
        test(!pa);
        Test::TestFacetPrx tf = Test::TestFacetPrx::checkedCast(obj, "TestFacet");
        test(!tf);
#endif
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose only the TestFacet facet,
        // meaning no other facet is available.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "TestFacet";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
#ifdef ICE_CPP11_MAPPING
        shared_ptr<Ice::PropertiesAdminPrx> pa = Ice::checkedCast<Ice::PropertiesAdminPrx>(obj, "Properties");
        test(!pa);
        shared_ptr<Ice::ProcessPrx> proc = Ice::checkedCast<Ice::ProcessPrx>(obj, "Process");
        test(!proc);
#else
        Ice::PropertiesAdminPrx pa = Ice::PropertiesAdminPrx::checkedCast(obj, "Properties");
        test(!pa);
        Ice::ProcessPrx proc = Ice::ProcessPrx::checkedCast(obj, "Process");
        test(!proc);
#endif
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose two facets. Use whitespace to separate the
        // facet names.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "Properties TestFacet";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
#ifdef ICE_CPP11_MAPPING
        shared_ptr<Ice::PropertiesAdminPrx> pa = Ice::checkedCast<Ice::PropertiesAdminPrx>(obj, "Properties");
        test(pa->getProperty("Ice.Admin.InstanceName") == "Test");
        shared_ptr<Test::TestFacetPrx> tf = Ice::checkedCast<Test::TestFacetPrx>(obj, "TestFacet");
        tf->op();
        shared_ptr<Ice::ProcessPrx> proc = Ice::checkedCast<Ice::ProcessPrx>(obj, "Process");
        test(!proc);
#else
        Ice::PropertiesAdminPrx pa = Ice::PropertiesAdminPrx::checkedCast(obj, "Properties");
        test(pa->getProperty("Ice.Admin.InstanceName") == "Test");
        Test::TestFacetPrx tf = Test::TestFacetPrx::checkedCast(obj, "TestFacet");
        tf->op();
        Ice::ProcessPrx proc = Ice::ProcessPrx::checkedCast(obj, "Process");
        test(!proc);
#endif
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose two facets. Use a comma to separate the
        // facet names.
        //
        Ice::PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "TestFacet, Process";
        RemoteCommunicatorPrxPtr com = factory->createCommunicator(props);
        Ice::ObjectPrxPtr obj = com->getAdmin();
#ifdef ICE_CPP11_MAPPING
        shared_ptr<Ice::PropertiesAdminPrx> pa = Ice::checkedCast<Ice::PropertiesAdminPrx>(obj, "Properties");
        test(!pa);
        shared_ptr<Test::TestFacetPrx> tf = Ice::checkedCast<Test::TestFacetPrx>(obj, "TestFacet");
        tf->op();
        shared_ptr<Ice::ProcessPrx> proc = Ice::checkedCast<Ice::ProcessPrx>(obj, "Process");
#else
        Ice::PropertiesAdminPrx pa = Ice::PropertiesAdminPrx::checkedCast(obj, "Properties");
        test(!pa);
        Test::TestFacetPrx tf = Test::TestFacetPrx::checkedCast(obj, "TestFacet");
        tf->op();
        Ice::ProcessPrx proc = Ice::ProcessPrx::checkedCast(obj, "Process");
#endif
        proc->shutdown();
        com->waitForShutdown();
        com->destroy();
    }
    cout << "ok" << endl;

    factory->shutdown();
}
