// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "TestHelper.h"
#include "TestI.h" // For TestFacetI

using namespace std;
using namespace Test;
using namespace Ice;

static void
testFacets(const CommunicatorPtr& com, bool builtInFacets = true)
{
    if (builtInFacets)
    {
        test(com->findAdminFacet("Properties"));
        test(com->findAdminFacet("Process"));
        test(com->findAdminFacet("Logger"));
        test(com->findAdminFacet("Metrics"));
    }

    TestFacetPtr f1 = std::make_shared<TestFacetI>();
    TestFacetPtr f2 = std::make_shared<TestFacetI>();
    TestFacetPtr f3 = std::make_shared<TestFacetI>();

    com->addAdminFacet(f1, "Facet1");
    com->addAdminFacet(f2, "Facet2");
    com->addAdminFacet(f3, "Facet3");

    test(com->findAdminFacet<TestFacetI>("Facet1") == f1);
    test(com->findAdminFacet<TestFacetI>("Facet2") == f2);
    test(com->findAdminFacet<TestFacetI>("Facet3") == f3);
    test(!com->findAdminFacet("Bogus"));

    const FacetMap facetMap = com->findAllAdminFacets();

    if (builtInFacets)
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
    catch (const AlreadyRegisteredException&)
    {
        // Expected
    }

    try
    {
        com->removeAdminFacet("Bogus");
        test(false);
    }
    catch (const NotRegisteredException&)
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
    catch (const NotRegisteredException&)
    {
        // Expected
    }
}

class RemoteLoggerI final : public RemoteLogger
{
public:
    RemoteLoggerI();

    void init(string, LogMessageSeq, const Current&) final;
    void log(LogMessage, const Current&) final;

    void checkNextInit(const string&, LogMessageType, const string&, const string& = "");
    void checkNextLog(LogMessageType, const string&, const string& = "");

    bool wait(int);

private:
    mutex _mutex;
    condition_variable _condition;
    int _receivedCalls{0};
    string _prefix;
    LogMessageSeq _initMessages;
    LogMessageSeq _logMessages;
};

using RemoteLoggerIPtr = std::shared_ptr<RemoteLoggerI>;

RemoteLoggerI::RemoteLoggerI() = default;

void
RemoteLoggerI::init(string prefix, LogMessageSeq logMessages, const Current&)
{
    lock_guard lock(_mutex);
    _prefix = prefix;
    _initMessages.insert(_initMessages.end(), logMessages.begin(), logMessages.end());
    _receivedCalls++;
    _condition.notify_all();
}

void
RemoteLoggerI::log(LogMessage logMessage, const Current&)
{
    lock_guard lock(_mutex);
    _logMessages.push_back(logMessage);
    _receivedCalls++;
    _condition.notify_all();
}

void
RemoteLoggerI::checkNextInit(const string& prefix, LogMessageType type, const string& message, const string& category)
{
    lock_guard lock(_mutex);
    test(_prefix == prefix);
    test(_initMessages.size() > 0);
    LogMessage front = _initMessages.front();
    test(front.type == type);
    test(front.message == message);
    test(front.traceCategory == category);
    _initMessages.pop_front();
}

void
RemoteLoggerI::checkNextLog(LogMessageType type, const string& message, const string& category)
{
    lock_guard lock(_mutex);
    test(_logMessages.size() > 0);
    LogMessage front = _logMessages.front();
    test(front.type == type);
    test(front.message == message);
    test(front.traceCategory == category);
    _logMessages.pop_front();
}

bool
RemoteLoggerI::wait(int calls)
{
    unique_lock lock(_mutex);
    _receivedCalls -= calls;
    auto now = chrono::steady_clock::now();
    const auto start = now;
    auto delay = chrono::seconds(20);
    while (_receivedCalls < 0)
    {
        _condition.wait_for(lock, delay);
        now = chrono::steady_clock::now();
        if (now - start >= chrono::seconds(20))
        {
            cerr << "expected `" << calls << "' received: `" << (calls + _receivedCalls) << "'" << endl;
            return false; // Waited for more than 20s for close, something's wrong.
        }
        delay = chrono::duration_cast<chrono::seconds>(start + chrono::seconds(20) - now);
    }
    return true;
}

void
allTests(Test::TestHelper* helper)
{
    CommunicatorPtr communicator = helper->communicator();
    cout << "testing communicator operations... " << flush;
    {
        //
        // Test: Exercise addAdminFacet, findAdminFacet, removeAdminFacet with a typical configuration.
        //
        InitializationData init;
        init.properties = createProperties();
        init.properties->setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        init.properties->setProperty("Ice.Admin.InstanceName", "Test");
        CommunicatorHolder ich(init);
        testFacets(ich.communicator());
    }
    {
        //
        // Test: Verify that the operations work correctly in the presence of facet filters.
        //
        InitializationData init;
        init.properties = createProperties();
        init.properties->setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        init.properties->setProperty("Ice.Admin.InstanceName", "Test");
        init.properties->setProperty("Ice.Admin.Facets", "Properties");
        CommunicatorPtr com = initialize(init);
        testFacets(com, false);
        com->destroy();
    }
    {
        //
        // Test: Verify that the operations work correctly with the Admin object disabled.
        //
        CommunicatorPtr com = initialize();
        testFacets(com, false);
        com->destroy();
    }
    {
        //
        // Test: Verify that the operations work correctly when Ice.Admin.Enabled is set
        //
        InitializationData init;
        init.properties = createProperties();
        init.properties->setProperty("Ice.Admin.Enabled", "1");
        CommunicatorPtr com = initialize(init);
        test(!com->getAdmin());

        Identity id = stringToIdentity("test-admin");
        try
        {
            com->createAdmin(nullptr, id);
            test(false);
        }
        catch (const InitializationException&)
        {
        }

        ObjectAdapterPtr adapter = com->createObjectAdapter("");
        com->createAdmin(adapter, id);
        test(com->getAdmin());

        testFacets(com);
        com->destroy();
    }
    {
        //
        // Test: Verify that the operations work correctly when creation of the Admin object is delayed.
        //
        InitializationData init;
        init.properties = createProperties();
        init.properties->setProperty("Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        init.properties->setProperty("Ice.Admin.InstanceName", "Test");
        init.properties->setProperty("Ice.Admin.DelayCreation", "1");
        CommunicatorPtr com = initialize(init);
        testFacets(com);
        com->getAdmin();
        testFacets(com);
        com->destroy();
    }
    cout << "ok" << endl;

    RemoteCommunicatorFactoryPrx factory(communicator, "factory:" + helper->getTestEndpoint() + " -t 10000");

    string defaultHost = communicator->getProperties()->getIceProperty("Ice.Default.Host");

    cout << "testing process facet... " << flush;
    {
        //
        // Test: Verify that Process::shutdown() operation shuts down the communicator.
        //
        PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        optional<RemoteCommunicatorPrx> com = factory->createCommunicator(props);
        optional<ObjectPrx> obj = com->getAdmin();
        auto proc = obj->ice_facet<ProcessPrx>("Process");
        proc->shutdown();
        com->waitForShutdown();
        com->destroy();
    }
    cout << "ok" << endl;

    cout << "testing properties facet... " << flush;
    {
        PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Prop1"] = "1";
        props["Prop2"] = "2";
        props["Prop3"] = "3";
        optional<RemoteCommunicatorPrx> com = factory->createCommunicator(props);
        optional<ObjectPrx> obj = com->getAdmin();
        auto pa = obj->ice_facet<PropertiesAdminPrx>("Properties");
        //
        // Test: PropertiesAdmin::getProperty()
        //
        test(pa->getProperty("Prop2") == "2");
        test(pa->getProperty("Bogus") == "");

        //
        // Test: PropertiesAdmin::getProperties()
        //
        PropertyDict pd = pa->getPropertiesForPrefix("");
        test(pd.size() == 5);
        test(pd["Ice.Admin.Endpoints"] == "tcp -h " + defaultHost);
        test(pd["Ice.Admin.InstanceName"] == "Test");
        test(pd["Prop1"] == "1");
        test(pd["Prop2"] == "2");
        test(pd["Prop3"] == "3");

        PropertyDict changes;

        //
        // Test: PropertiesAdmin::setProperties()
        //
        PropertyDict setProps;
        setProps["Prop1"] = "10"; // Changed
        setProps["Prop2"] = "20"; // Changed
        setProps["Prop3"] = "";   // Removed
        setProps["Prop4"] = "4";  // Added
        setProps["Prop5"] = "5";  // Added
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
        PropertyDict moreProps;
        moreProps["Prop1"] = "11"; // Changed
        moreProps["Prop2"] = "";   // Removed
        moreProps["Prop6"] = "6";  // Added
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

    cout << "testing logger facet... " << flush;
    {
        PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["NullLogger"] = "1";
        optional<RemoteCommunicatorPrx> com = factory->createCommunicator(props);

        com->trace("testCat", "trace");
        com->warning("warning");
        com->error("error");
        com->print("print");

        optional<ObjectPrx> obj = com->getAdmin();
        auto logger = obj->ice_facet<LoggerAdminPrx>("Logger");
        string prefix;

        //
        // Get all
        //
        LogMessageSeq logMessages = logger->getLog(LogMessageTypeSeq(), StringSeq(), -1, prefix);

        test(logMessages.size() == 4);
        test(prefix == "NullLogger");
        auto p = logMessages.begin();
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

        LogMessageTypeSeq messageTypes;
        messageTypes.push_back(LogMessageType::ErrorMessage);
        messageTypes.push_back(LogMessageType::WarningMessage);

        logMessages = logger->getLog(messageTypes, StringSeq(), -1, prefix);
        test(logMessages.size() == 4);
        test(prefix == "NullLogger");

        p = logMessages.begin();
        while (p != logMessages.end())
        {
            test(p->type == LogMessageType::ErrorMessage || p->type == LogMessageType::WarningMessage);
            ++p;
        }

        //
        // Get only errors and traces with Cat = "testCat"
        //
        com->trace("testCat2", "A");
        com->trace("testCat", "trace3");
        com->trace("testCat2", "B");

        messageTypes.clear();
        messageTypes.push_back(LogMessageType::ErrorMessage);
        messageTypes.push_back(LogMessageType::TraceMessage);

        StringSeq categories;
        categories.emplace_back("testCat");

        logMessages = logger->getLog(messageTypes, categories, -1, prefix);
        test(logMessages.size() == 5);
        test(prefix == "NullLogger");

        p = logMessages.begin();
        while (p != logMessages.end())
        {
            test(
                p->type == LogMessageType::ErrorMessage ||
                (p->type == LogMessageType::TraceMessage && p->traceCategory == "testCat"));
            ++p;
        }

        //
        // Same, but limited to last 2 messages (trace3 + error3)
        //
        com->error("error3");

        logMessages = logger->getLog(messageTypes, categories, 2, prefix);
        test(logMessages.size() == 2);
        test(prefix == "NullLogger");

        p = logMessages.begin();
        test(p++->message == "trace3");
        test(p->message == "error3");

        //
        // Now, test RemoteLogger
        //

        ObjectAdapterPtr adapter =
            communicator->createObjectAdapterWithEndpoints("RemoteLoggerAdapter", "tcp -h localhost");

        RemoteLoggerIPtr remoteLogger = std::make_shared<RemoteLoggerI>();

        auto myProxy = adapter->addWithUUID<RemoteLoggerPrx>(remoteLogger);

        adapter->activate();

        //
        // No filtering
        //
        logMessages = logger->getLog(LogMessageTypeSeq(), StringSeq(), -1, prefix);
        logger->attachRemoteLogger(myProxy, LogMessageTypeSeq(), StringSeq(), -1);
        test(remoteLogger->wait(1));

        for (const auto& logMessage : logMessages)
        {
            remoteLogger->checkNextInit(prefix, logMessage.type, logMessage.message, logMessage.traceCategory);
        }

        com->trace("testCat", "rtrace");
        com->warning("rwarning");
        com->error("rerror");
        com->print("rprint");
        test(remoteLogger->wait(4));

        remoteLogger->checkNextLog(LogMessageType::TraceMessage, "rtrace", "testCat");
        remoteLogger->checkNextLog(LogMessageType::WarningMessage, "rwarning");
        remoteLogger->checkNextLog(LogMessageType::ErrorMessage, "rerror");
        remoteLogger->checkNextLog(LogMessageType::PrintMessage, "rprint");

        test(logger->detachRemoteLogger(myProxy));
        test(!logger->detachRemoteLogger(myProxy));

        //
        // Use Error + Trace with "traceCat" filter with 4 limit
        //
        logMessages = logger->getLog(messageTypes, categories, 4, prefix);
        test(logMessages.size() == 4);
        logger->attachRemoteLogger(myProxy, messageTypes, categories, 4);
        test(remoteLogger->wait(1));

        for (const auto& logMessage : logMessages)
        {
            remoteLogger->checkNextInit(prefix, logMessage.type, logMessage.message, logMessage.traceCategory);
        }

        com->warning("rwarning2");
        com->trace("testCat", "rtrace2");
        com->warning("rwarning3");
        com->error("rerror2");
        com->print("rprint2");
        test(remoteLogger->wait(2));

        remoteLogger->checkNextLog(LogMessageType::TraceMessage, "rtrace2", "testCat");
        remoteLogger->checkNextLog(LogMessageType::ErrorMessage, "rerror2");

        //
        // Attempt reconnection with slightly different proxy
        //
        try
        {
            logger->attachRemoteLogger(myProxy->ice_oneway(), messageTypes, categories, 4);
            test(false);
        }
        catch (const RemoteLoggerAlreadyAttachedException&)
        {
            // expected
        }

        com->destroy();
    }
    cout << "ok" << endl;

    cout << "testing custom facet... " << flush;
    {
        //
        // Test: Verify that the custom facet is present.
        //
        PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        optional<RemoteCommunicatorPrx> com = factory->createCommunicator(props);
        optional<ObjectPrx> obj = com->getAdmin();
        auto tf = obj->ice_facet<Test::TestFacetPrx>("TestFacet");
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
        PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "Properties";
        optional<RemoteCommunicatorPrx> com = factory->createCommunicator(props);
        optional<ObjectPrx> obj = com->getAdmin();

        try
        {
            checkedCast<ProcessPrx>(obj, "Process");
            test(false);
        }
        catch (const Ice::FacetNotExistException&)
        {
            // expected
        }
        try
        {
            checkedCast<Test::TestFacetPrx>(obj, "TestFacet");
            test(false);
        }
        catch (const Ice::FacetNotExistException&)
        {
            // expected
        }
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose only the Process facet,
        // meaning no other facet is available.
        //
        PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "Process";
        optional<RemoteCommunicatorPrx> com = factory->createCommunicator(props);
        optional<ObjectPrx> obj = com->getAdmin();
        try
        {
            checkedCast<PropertiesAdminPrx>(obj, "Properties");
            test(false);
        }
        catch (const Ice::FacetNotExistException&)
        {
            // expected
        }

        try
        {
            checkedCast<Test::TestFacetPrx>(obj, "TestFacet");
            test(false);
        }
        catch (const Ice::FacetNotExistException&)
        {
            // expected
        }
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose only the TestFacet facet,
        // meaning no other facet is available.
        //
        PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "TestFacet";
        optional<RemoteCommunicatorPrx> com = factory->createCommunicator(props);
        optional<ObjectPrx> obj = com->getAdmin();
        try
        {
            checkedCast<PropertiesAdminPrx>(obj, "Properties");
            test(false);
        }
        catch (const Ice::FacetNotExistException&)
        {
            // expected
        }
        try
        {
            checkedCast<ProcessPrx>(obj, "Process");
            test(false);
        }
        catch (const Ice::FacetNotExistException&)
        {
            // expected
        }
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose two facets. Use whitespace to separate the
        // facet names.
        //
        PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "Properties TestFacet";
        optional<RemoteCommunicatorPrx> com = factory->createCommunicator(props);
        optional<ObjectPrx> obj = com->getAdmin();
        auto pa = obj->ice_facet<PropertiesAdminPrx>("Properties");
        test(pa->getProperty("Ice.Admin.InstanceName") == "Test");
        auto tf = obj->ice_facet<Test::TestFacetPrx>("TestFacet");
        tf->op();
        try
        {
            checkedCast<ProcessPrx>(obj, "Process");
            test(false);
        }
        catch (const Ice::FacetNotExistException&)
        {
            // expected
        }
        com->destroy();
    }
    {
        //
        // Test: Set Ice.Admin.Facets to expose two facets. Use a comma to separate the
        // facet names.
        //
        PropertyDict props;
        props["Ice.Admin.Endpoints"] = "tcp -h " + defaultHost;
        props["Ice.Admin.InstanceName"] = "Test";
        props["Ice.Admin.Facets"] = "TestFacet, Process";
        optional<RemoteCommunicatorPrx> com = factory->createCommunicator(props);
        optional<ObjectPrx> obj = com->getAdmin();
        try
        {
            checkedCast<PropertiesAdminPrx>(obj, "Properties");
            test(false);
        }
        catch (const Ice::FacetNotExistException&)
        {
            // expected
        }
        auto tf = obj->ice_facet<Test::TestFacetPrx>("TestFacet");
        tf->op();
        auto proc = obj->ice_facet<ProcessPrx>("Process");
        proc->shutdown();
        com->waitForShutdown();
        com->destroy();
    }
    cout << "ok" << endl;

    factory->shutdown();
}
