// Copyright (c) ZeroC, Inc.

#include "Glacier2/Router.h"
#include "Ice/Ice.h"
#include "IceGrid/IceGrid.h"
#include "TestHelper.h"
#include <thread>

using namespace std;
using namespace Ice;
using namespace IceGrid;
using namespace Test;

class ObserverBase
{
public:
    ObserverBase(const string& name) : _name(name) { _observers.insert(make_pair(name, this)); }

    virtual ~ObserverBase() { _observers.erase(_name); }

    static void printStack()
    {
        for (const auto& p : _observers)
        {
            auto q = p.second->_stack.begin();
            if (p.second->_stack.size() > 10)
            {
                q = p.second->_stack.begin() +
                    static_cast<vector<string>::difference_type>(p.second->_stack.size() - 10);
            }
            cerr << "Last 10 updates of observer `" << p.second->_name << "':" << endl;
            for (; q != p.second->_stack.end(); ++q)
            {
                cerr << "  " << *q << endl;
            }
            p.second->_stack.clear();
        }
    }

    void waitForUpdate(int line)
    {
        unique_lock<mutex> lg(_mutex);

        _stack.push_back("wait for update from line " + to_string(line));
        while (!_updated)
        {
            _condVar.wait(lg);
        }
        --_updated;
    }

protected:
    void updated(const string& update)
    {
        _stack.push_back(update);
        ++_updated;
        _condVar.notify_all();
    }

    string _name;
    vector<string> _stack;
    int _updated{0};
    mutex _mutex;

    static map<string, ObserverBase*> _observers;

private:
    condition_variable _condVar;
};
map<string, ObserverBase*> ObserverBase::_observers;

class ApplicationObserverI final : public ApplicationObserver, public ObserverBase
{
public:
    ApplicationObserverI(const string& name) : ObserverBase(name) {}

    void applicationInit(int serialP, ApplicationInfoSeq apps, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        for (const auto& p : apps)
        {
            if (p.descriptor.name != "Test") // Ignore the test application from application.xml!
            {
                applications.insert(make_pair(p.descriptor.name, p));
            }
        }
        updated(updateSerial(serialP, "init update"));
    }

    void applicationAdded(int serialP, ApplicationInfo app, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        applications.insert(make_pair(app.descriptor.name, app));
        updated(updateSerial(serialP, "application added `" + app.descriptor.name + "'"));
    }

    void applicationRemoved(int serialP, std::string name, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        applications.erase(name);
        updated(updateSerial(serialP, "application removed `" + name + "'"));
    }

    void applicationUpdated(int serialP, ApplicationUpdateInfo info, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        const ApplicationUpdateDescriptor& desc = info.descriptor;
        for (const auto& p : desc.removeVariables)
        {
            applications[desc.name].descriptor.variables.erase(p);
        }
        for (const auto& p : desc.variables)
        {
            applications[desc.name].descriptor.variables[p.first] = p.second;
        }
        updated(updateSerial(serialP, "application updated `" + desc.name + "'"));
    }

    int serial;
    map<string, ApplicationInfo> applications;

private:
    string updateSerial(int serialP, const string& update)
    {
        serial = serialP;
        ostringstream os;
        os << update << " (serial = " << serial << ")";
        return os.str();
    }
};

class AdapterObserverI final : public AdapterObserver, public ObserverBase
{
public:
    AdapterObserverI(const string& name) : ObserverBase(name) {}

    void adapterInit(AdapterInfoSeq adaptersP, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        for (const auto& p : adaptersP)
        {
            adapters.insert(make_pair(p.id, p));
        }
        updated(updateSerial(0, "init update"));
    }

    void adapterAdded(AdapterInfo info, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        adapters.insert(make_pair(info.id, info));
        updated(updateSerial(0, "adapter added `" + info.id + "'"));
    }

    void adapterUpdated(AdapterInfo info, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        adapters[info.id] = info;
        updated(updateSerial(0, "adapter updated `" + info.id + "'"));
    }

    void adapterRemoved(string id, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        adapters.erase(id);
        updated(updateSerial(0, "adapter removed `" + id + "'"));
    }

    int serial;
    map<string, AdapterInfo> adapters;

private:
    string updateSerial(int serialP, const string& update)
    {
        serial = serialP;
        ostringstream os;
        os << update << " (serial = " << serial << ")";
        return os.str();
    }
};

class ObjectObserverI final : public ObjectObserver, public ObserverBase
{
public:
    ObjectObserverI(const string& name) : ObserverBase(name) {}

    void objectInit(ObjectInfoSeq objectsP, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        for (const auto& p : objectsP)
        {
            objects.insert(make_pair(p.proxy->ice_getIdentity(), p));
        }
        updated(updateSerial(0, "init update"));
    }

    void objectAdded(ObjectInfo info, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        objects.insert(make_pair(info.proxy->ice_getIdentity(), info));
        updated(updateSerial(0, "object added `" + info.proxy->ice_toString() + "'"));
    }

    void objectUpdated(ObjectInfo info, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        objects[info.proxy->ice_getIdentity()] = info;
        updated(updateSerial(0, "object updated `" + info.proxy->ice_toString() + "'"));
    }

    void objectRemoved(Ice::Identity id, const Ice::Current& current) override
    {
        lock_guard<mutex> lg(_mutex);
        objects.erase(id);
        updated(updateSerial(0, "object removed `" + current.adapter->getCommunicator()->identityToString(id) + "'"));
    }

    int serial;
    map<Ice::Identity, ObjectInfo> objects;

private:
    string updateSerial(int serialP, const string& update)
    {
        serial = serialP;
        ostringstream os;
        os << update << " (serial = " << serial << ")";
        return os.str();
    }
};

class NodeObserverI final : public NodeObserver, public ObserverBase
{
public:
    NodeObserverI(const string& name) : ObserverBase(name) {}

    void nodeInit(NodeDynamicInfoSeq info, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        for (const auto& p : info)
        {
            nodes[p.info.name] = filter(p);
        }
        updated("init");
    }

    void nodeUp(NodeDynamicInfo info, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        nodes[info.info.name] = filter(info);
        updated("node `" + info.info.name + "' up");
    }

    void nodeDown(string name, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        nodes.erase(name);
        updated("node `" + name + "' down");
    }

    void updateServer(string node, ServerDynamicInfo info, const Ice::Current&) override
    {
        if (info.id == "Glacier2" || info.id == "Glacier2Admin" || info.id == "PermissionsVerifierServer")
        {
            return;
        }

        lock_guard<mutex> lg(_mutex);
        // cerr << node << " " << info.id << " " << info.state << " " << info.pid << endl;
        ServerDynamicInfoSeq& servers = nodes[node].servers;
        ServerDynamicInfoSeq::iterator p;
        for (p = servers.begin(); p != servers.end(); ++p)
        {
            if (p->id == info.id)
            {
                if (info.state == ServerState::Destroyed)
                {
                    servers.erase(p);
                }
                else
                {
                    *p = info;
                }
                break;
            }
        }
        if (info.state != ServerState::Destroyed && p == servers.end())
        {
            servers.push_back(info);
        }

        ostringstream os;
        os << "server `" << info.id << "' on node `" << node << "' state updated: " << static_cast<int>(info.state)
           << " (pid = " << info.pid << ")";
        updated(os.str());
    }

    void updateAdapter(string node, AdapterDynamicInfo info, const Ice::Current&) override
    {
        if (info.id == "PermissionsVerifierServer.Server")
        {
            return;
        }

        lock_guard<mutex> lg(_mutex);
        // cerr << "update adapter: " << info.id << " " << (info.proxy ? "active" : "inactive") << endl;
        AdapterDynamicInfoSeq& adapters = nodes[node].adapters;
        AdapterDynamicInfoSeq::iterator p;
        for (p = adapters.begin(); p != adapters.end(); ++p)
        {
            if (p->id == info.id)
            {
                if (info.proxy)
                {
                    *p = info;
                }
                else
                {
                    adapters.erase(p);
                }
                break;
            }
        }
        if (info.proxy && p == adapters.end())
        {
            adapters.push_back(info);
        }

        ostringstream os;
        os << "adapter `" << info.id << " on node `" << node
           << "' state updated: " << (info.proxy ? "active" : "inactive");
        updated(os.str());
    }

    NodeDynamicInfo filter(NodeDynamicInfo info)
    {
        if (info.info.name != "localnode")
        {
            return info;
        }

        NodeDynamicInfo filtered;
        filtered.info = info.info;

        for (const auto& p : info.servers)
        {
            if (p.id == "Glacier2" || p.id == "Glacier2Admin" || p.id == "PermissionsVerifierServer")
            {
                continue;
            }
            filtered.servers.push_back(p);
        }

        for (const auto& a : info.adapters)
        {
            if (a.id == "PermissionsVerifierServer.Server")
            {
                continue;
            }
            filtered.adapters.push_back(a);
        }

        return filtered;
    }

    map<string, NodeDynamicInfo> nodes;
};

class RegistryObserverI final : public RegistryObserver, public ObserverBase
{
public:
    RegistryObserverI(const string& name) : ObserverBase(name) {}

    void registryInit(RegistryInfoSeq info, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        for (const auto& p : info)
        {
            registries[p.name] = p;
        }
        updated("init");
    }

    void registryUp(RegistryInfo info, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        registries[info.name] = info;
        updated("registry `" + info.name + "' up");
    }

    void registryDown(string name, const Ice::Current&) override
    {
        lock_guard<mutex> lg(_mutex);
        registries.erase(name);
        updated("registry `" + name + "' down");
    }

    map<string, RegistryInfo> registries;
};

void
testFailedAndPrintObservers(const char* expr, const char* file, unsigned int line)
{
    ObserverBase::printStack();
    Test::testFailed(expr, file, line);
}

#undef test
#define test(ex) ((ex) ? ((void)0) : testFailedAndPrintObservers(#ex, __FILE__, __LINE__))

void
allTests(TestHelper* helper)
{
    auto communicator = helper->communicator();

    RegistryPrx registry(communicator, communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry");

    auto session = registry->createAdminSession("admin3", "test3");
    auto admin = session->getAdmin();
    test(admin);

    cout << "starting router... " << flush;
    try
    {
        admin->startServer("Glacier2");
    }
    catch (const ServerStartException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "starting admin router... " << flush;
    try
    {
        admin->startServer("Glacier2Admin");
    }
    catch (const ServerStartException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }
    cout << "ok" << endl;

    auto properties = communicator->getProperties();

    auto registry1 = registry->ice_connectionId("reg1");
    auto registry2 = registry->ice_connectionId("reg2");

    Glacier2::RouterPrx router(communicator, "Glacier2/router:default -p 12347 -h 127.0.0.1");
    Glacier2::RouterPrx adminRouter(communicator, "Glacier2/router:default -p 12348 -h 127.0.0.1");

    Glacier2::RouterPrx router1 = router->ice_connectionId("router1");
    Glacier2::RouterPrx router2 = router->ice_connectionId("router2");

    Glacier2::RouterPrx adminRouter1 = adminRouter->ice_connectionId("admRouter1");
    Glacier2::RouterPrx adminRouter2 = adminRouter->ice_connectionId("admRouter2");

    //
    // TODO: Find a better way to wait for the Glacier2 router to be
    // fully started...
    //
    while (true)
    {
        try
        {
            router1->ice_ping();
            adminRouter1->ice_ping();
            break;
        }
        catch (const Ice::LocalException&)
        {
            this_thread::sleep_for(100ms);
        }
    }

    {
        cout << "testing username/password sessions... " << flush;

        SessionPrx session1(registry1->createSession("client1", "test1")->ice_connectionId("reg1"));
        SessionPrx session2(registry2->createSession("client2", "test2")->ice_connectionId("reg2"));
        try
        {
            registry1->createSession("client3", "test1");
            test(false);
        }
        catch (const PermissionDeniedException&)
        {
        }
        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            registry1->createSession("client3", "test1", ctx);
            test(false);
        }
        catch (const PermissionDeniedException& ex)
        {
            test(ex.reason == "reason");
        }

        session1->ice_ping();
        session2->ice_ping();

        try
        {
            session1->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            session2->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        try
        {
            session1->ice_connectionId("reg2")->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            session2->ice_connectionId("reg1")->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        session1->destroy();
        session2->destroy();

        AdminSessionPrx adminSession1(registry1->createAdminSession("admin1", "test1")->ice_connectionId("reg1"));

        AdminSessionPrx adminSession2(registry2->createAdminSession("admin2", "test2")->ice_connectionId("reg2"));

        try
        {
            registry1->createAdminSession("admin3", "test1");
            test(false);
        }
        catch (const PermissionDeniedException&)
        {
        }
        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            registry1->createSession("admin3", "test1", ctx);
            test(false);
        }
        catch (const PermissionDeniedException& ex)
        {
            test(ex.reason == "reason");
        }

        adminSession1->ice_ping();
        adminSession2->ice_ping();

        try
        {
            adminSession1->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            adminSession2->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        adminSession1->getAdmin()->ice_connectionId("reg1")->ice_ping();
        adminSession2->getAdmin()->ice_connectionId("reg2")->ice_ping();

        try
        {
            adminSession1->getAdmin()->ice_connectionId("reg2")->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            adminSession2->getAdmin()->ice_connectionId("reg1")->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        adminSession1->destroy();
        adminSession2->destroy();

        cout << "ok" << endl;
    }

    if (properties->getIceProperty("Ice.Default.Protocol") == "ssl")
    {
        cout << "testing sessions from secure connection... " << flush;

        SessionPrx session1(registry1->createSessionFromSecureConnection()->ice_connectionId("reg1"));
        SessionPrx session2(registry2->createSessionFromSecureConnection()->ice_connectionId("reg2"));

        session1->ice_ping();
        session2->ice_ping();

        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            registry1->createSessionFromSecureConnection(ctx);
            test(false);
        }
        catch (const PermissionDeniedException& ex)
        {
            test(ex.reason == "reason");
        }

        try
        {
            session1->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            session2->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        session1->destroy();
        session2->destroy();

        AdminSessionPrx adminSession1(registry1->createAdminSessionFromSecureConnection()->ice_connectionId("reg1"));
        AdminSessionPrx adminSession2(registry2->createAdminSessionFromSecureConnection()->ice_connectionId("reg2"));

        adminSession1->ice_ping();
        adminSession2->ice_ping();

        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            registry1->createAdminSessionFromSecureConnection(ctx);
            test(false);
        }
        catch (const PermissionDeniedException& ex)
        {
            test(ex.reason == "reason");
        }

        try
        {
            adminSession1->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            adminSession2->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        adminSession1->destroy();
        adminSession2->destroy();

        cout << "ok" << endl;
    }
    else
    {
        cout << "testing sessions from secure connection... " << flush;
        try
        {
            registry1->createSessionFromSecureConnection();
            test(false);
        }
        catch (const PermissionDeniedException&)
        {
        }
        try
        {
            registry1->createAdminSessionFromSecureConnection();
            test(false);
        }
        catch (const PermissionDeniedException&)
        {
        }
        cout << "ok" << endl;
    }

    {
        cout << "testing Glacier2 username/password sessions... " << flush;

        auto session1 = uncheckedCast<SessionPrx>(
            router1->createSession("client1", "test1")->ice_connectionId("router1")->ice_router(router1));

        auto session2 = uncheckedCast<SessionPrx>(
            router2->createSession("client2", "test2")->ice_connectionId("router2")->ice_router(router2));

        try
        {
            router1->createSession("client3", "test1");
            test(false);
        }
        catch (const Glacier2::CannotCreateSessionException&)
        {
        }
        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            router->ice_connectionId("routerex")->createSession("client3", "test1", ctx);
            test(false);
        }
        catch (const Glacier2::PermissionDeniedException& ex)
        {
            test(ex.reason == "reason");
        }

        session1->ice_ping();
        session2->ice_ping();

        try
        {
            session1->ice_connectionId("router2")->ice_router(router2)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            session2->ice_connectionId("router1")->ice_router(router1)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        ObjectPrx obj(communicator, "TestIceGrid/Query");
        obj->ice_connectionId("router1")->ice_router(router1)->ice_ping();
        obj->ice_connectionId("router2")->ice_router(router2)->ice_ping();

        obj = ObjectPrx(communicator, "TestIceGrid/Registry");
        try
        {
            obj->ice_connectionId("router1")->ice_router(router1)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            obj->ice_connectionId("router2")->ice_router(router2)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        router1->destroySession();
        router2->destroySession();

        auto admSession1 = uncheckedCast<AdminSessionPrx>(
            adminRouter1->createSession("admin1", "test1")->ice_connectionId("admRouter1")->ice_router(adminRouter1));

        auto admSession2 = uncheckedCast<AdminSessionPrx>(
            adminRouter2->createSession("admin2", "test2")->ice_connectionId("admRouter2")->ice_router(adminRouter2));

        try
        {
            adminRouter1->createSession("admin3", "test1");
            test(false);
        }
        catch (const Glacier2::CannotCreateSessionException&)
        {
        }
        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            adminRouter->ice_connectionId("routerex")->createSession("admin3", "test1", ctx);
            test(false);
        }
        catch (const Glacier2::PermissionDeniedException& ex)
        {
            test(ex.reason == "reason");
        }

        admSession1->ice_ping();
        admSession2->ice_ping();

        auto admin1 = admSession1->getAdmin()->ice_router(adminRouter1)->ice_connectionId("admRouter1");
        auto admin2 = admSession2->getAdmin()->ice_router(adminRouter2)->ice_connectionId("admRouter2");

        admin1->ice_ping();
        admin2->ice_ping();

        obj = ObjectPrx(communicator, "TestIceGrid/Query");
        obj->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
        obj->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();

        try
        {
            admSession1->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            admSession2->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        try
        {
            admin1->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            admin2->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        adminRouter1->destroySession();
        adminRouter2->destroySession();

        cout << "ok" << endl;
    }

    if (properties->getIceProperty("Ice.Default.Protocol") == "ssl")
    {
        cout << "testing Glacier2 sessions from secure connection... " << flush;

        //
        // BUGFIX: We can't re-use the same router proxies because of bug 1034.
        //
        router1 = router1->ice_connectionId("router11");
        router2 = router2->ice_connectionId("router21");

        auto session1 = uncheckedCast<SessionPrx>(
            router1->createSessionFromSecureConnection()->ice_connectionId("router11")->ice_router(router1));

        auto session2 = uncheckedCast<SessionPrx>(
            router2->createSessionFromSecureConnection()->ice_connectionId("router21")->ice_router(router2));

        session1->ice_ping();
        session2->ice_ping();

        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            router->ice_connectionId("routerex")->createSessionFromSecureConnection(ctx);
            test(false);
        }
        catch (const Glacier2::PermissionDeniedException& ex)
        {
            test(ex.reason == "reason");
        }

        try
        {
            session1->ice_connectionId("router21")->ice_router(router2)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            session2->ice_connectionId("router11")->ice_router(router1)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        Ice::ObjectPrx obj(communicator, "TestIceGrid/Query");
        obj->ice_connectionId("router11")->ice_router(router1)->ice_ping();
        obj->ice_connectionId("router21")->ice_router(router2)->ice_ping();

        obj = Ice::ObjectPrx(communicator, "TestIceGrid/Registry");
        try
        {
            obj->ice_connectionId("router11")->ice_router(router1)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            obj->ice_connectionId("router21")->ice_router(router2)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        router1->destroySession();
        router2->destroySession();

        // BUGFIX: We can't re-use the same router proxies because of bug 1034.
        adminRouter1 = adminRouter->ice_connectionId("admRouter11");
        adminRouter2 = adminRouter->ice_connectionId("admRouter21");

        auto admSession1 = uncheckedCast<AdminSessionPrx>(adminRouter1->createSessionFromSecureConnection()
                                                              ->ice_connectionId("admRouter11")
                                                              ->ice_router(adminRouter1));

        auto admSession2 = uncheckedCast<AdminSessionPrx>(adminRouter2->createSessionFromSecureConnection()
                                                              ->ice_connectionId("admRouter21")
                                                              ->ice_router(adminRouter2));

        admSession1->ice_ping();
        admSession2->ice_ping();

        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            adminRouter->ice_connectionId("routerex")->createSessionFromSecureConnection(ctx);
            test(false);
        }
        catch (const Glacier2::PermissionDeniedException& ex)
        {
            test(ex.reason == "reason");
        }

        auto admin1 = admSession1->getAdmin()->ice_router(adminRouter1)->ice_connectionId("admRouter11");
        auto admin2 = admSession2->getAdmin()->ice_router(adminRouter2)->ice_connectionId("admRouter21");

        admin1->ice_ping();
        admin2->ice_ping();

        obj = ObjectPrx(communicator, "TestIceGrid/Query");
        obj->ice_connectionId("admRouter11")->ice_router(adminRouter1)->ice_ping();
        obj->ice_connectionId("admRouter21")->ice_router(adminRouter2)->ice_ping();

        try
        {
            admSession1->ice_connectionId("admRouter21")->ice_router(adminRouter2)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            admSession2->ice_connectionId("admRouter11")->ice_router(adminRouter1)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        try
        {
            admin1->ice_connectionId("admRouter21")->ice_router(adminRouter2)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            admin2->ice_connectionId("admRouter11")->ice_router(adminRouter1)->ice_ping();
            test(false);
        }
        catch (const Ice::ObjectNotExistException&)
        {
        }

        adminRouter1->destroySession();
        adminRouter2->destroySession();

        cout << "ok" << endl;
    }
    else
    {
        cout << "testing Glacier2 sessions from secure connection... " << flush;
        try
        {
            router1->createSessionFromSecureConnection();
            test(false);
        }
        catch (const Glacier2::PermissionDeniedException&)
        {
        }
        try
        {
            adminRouter1->createSessionFromSecureConnection();
            test(false);
        }
        catch (const Glacier2::PermissionDeniedException&)
        {
        }
        cout << "ok" << endl;
    }

    {
        cout << "testing updates with admin sessions... " << flush;
        auto session1 = registry->createAdminSession("admin1", "test1");
        auto session2 = registry->createAdminSession("admin2", "test2");

        auto admin1 = session1->getAdmin();
        auto admin2 = session2->getAdmin();

        auto adpt1 = communicator->createObjectAdapter("");
        auto appObs1 = make_shared<ApplicationObserverI>("appObs1.1");
        auto app1 = adpt1->addWithUUID(appObs1);
        auto nodeObs1 = make_shared<NodeObserverI>("nodeObs1");
        auto no1 = adpt1->addWithUUID(nodeObs1);
        adpt1->activate();
        registry->ice_getConnection()->setAdapter(adpt1);
        session1->setObserversByIdentity(
            Ice::Identity(),
            no1->ice_getIdentity(),
            app1->ice_getIdentity(),
            Ice::Identity(),
            Ice::Identity());

        auto adpt2 = communicator->createObjectAdapterWithEndpoints("Observer2", "tcp");
        auto appObs2 = make_shared<ApplicationObserverI>("appObs2");
        auto app2 = adpt2->addWithUUID<ApplicationObserverPrx>(appObs2);
        auto nodeObs2 = make_shared<NodeObserverI>("nodeObs1");
        auto no2 = adpt2->addWithUUID<NodeObserverPrx>(nodeObs2);
        adpt2->activate();
        session2->setObservers(nullopt, no2, app2, nullopt, nullopt);

        appObs1->waitForUpdate(__LINE__);
        appObs2->waitForUpdate(__LINE__);

        int serial = appObs1->serial;
        test(serial == appObs2->serial);

        try
        {
            session1->getAdmin()->ice_ping();
        }
        catch (const Ice::LocalException&)
        {
            test(false);
        }

        try
        {
            int s = session1->startUpdate();
            test(s != serial + 1);
        }
        catch (const AccessDeniedException&)
        {
            test(false);
        }

        try
        {
            int s = session1->startUpdate();
            test(s == serial);
        }
        catch (const Ice::UserException&)
        {
            test(false);
        }

        try
        {
            session2->startUpdate();
            test(false);
        }
        catch (const AccessDeniedException& ex)
        {
            test(ex.lockUserId == "admin1");
        }

        try
        {
            session1->finishUpdate();
        }
        catch (const Ice::UserException&)
        {
            test(false);
        }

        try
        {
            int s = session2->startUpdate();
            test(s == appObs2->serial);
        }
        catch (const Ice::UserException&)
        {
            test(false);
        }

        try
        {
            ApplicationDescriptor app;
            app.name = "Application";
            admin2->addApplication(app);
        }
        catch (const Ice::UserException&)
        {
            test(false);
        }

        try
        {
            admin1->addApplication(ApplicationDescriptor());
            test(false);
        }
        catch (const AccessDeniedException&)
        {
        }

        try
        {
            session2->finishUpdate();
        }
        catch (const Ice::UserException&)
        {
            test(false);
        }

        appObs1->waitForUpdate(__LINE__);
        appObs2->waitForUpdate(__LINE__);

        test(serial + 1 == appObs1->serial);
        test(serial + 1 == appObs2->serial);
        ++serial;

        try
        {
            int s = session1->startUpdate();
            test(s == serial);
            ApplicationUpdateDescriptor update;
            update.name = "Application";
            update.variables.insert(make_pair(string("test"), string("test")));
            admin1->updateApplication(update);
            session1->finishUpdate();
        }
        catch (const Ice::UserException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        appObs1->waitForUpdate(__LINE__);
        appObs2->waitForUpdate(__LINE__);

        test(serial + 1 == appObs1->serial);
        test(serial + 1 == appObs2->serial);
        ++serial;

        try
        {
            int s = session2->startUpdate();
            test(s == serial);
            admin2->removeApplication("Application");
            session2->finishUpdate();
        }
        catch (const Ice::UserException&)
        {
            test(false);
        }

        appObs1->waitForUpdate(__LINE__);
        appObs2->waitForUpdate(__LINE__);

        test(serial + 1 == appObs1->serial);
        test(serial + 1 == appObs2->serial);
        ++serial;

        try
        {
            int s = session1->startUpdate();
            test(s == serial);
        }
        catch (const Ice::UserException&)
        {
            test(false);
        }
        session1->destroy();

        try
        {
            int s = session2->startUpdate();
            test(s == serial);
            session2->finishUpdate();
        }
        catch (const Ice::UserException&)
        {
            test(false);
        }
        session2->destroy();

        adpt1->destroy();
        adpt2->destroy();

        cout << "ok" << endl;
    }

    {
        cout << "testing invalid configuration... " << flush;
        auto session1 = registry->createAdminSession("admin1", "test1");
        auto admin1 = session1->getAdmin();

        auto locatorRegistry = communicator->getDefaultLocator()->getRegistry();

        try
        {
            ApplicationDescriptor app;
            app.name = string(512, 'A');
            admin1->addApplication(app);
            test(false);
        }
        catch (const DeploymentException&)
        {
        }

        ObjectPrx obj(communicator, "dummy:tcp -p 10000");
        try
        {
            locatorRegistry->setAdapterDirectProxy(string(512, 'A'), obj);
            test(false);
        }
        catch (const Ice::UnknownException&)
        {
        }

        try
        {
            locatorRegistry->setReplicatedAdapterDirectProxy("Adapter", string(512, 'A'), obj);
            test(false);
        }
        catch (const Ice::UnknownException&)
        {
        }

        try
        {
            admin1->addObjectWithType(obj, string(512, 'T'));
            test(false);
        }
        catch (const DeploymentException&)
        {
        }

        cout << "ok" << endl;
    }

    {
        cout << "testing application observer... " << flush;
        auto session1 = registry->createAdminSession("admin1", "test1");
        auto admin1 = session1->getAdmin();
        auto adpt1 = communicator->createObjectAdapter("");
        auto appObs1 = make_shared<ApplicationObserverI>("appObs1.2");
        auto app1 = adpt1->addWithUUID(appObs1);
        adpt1->activate();
        registry->ice_getConnection()->setAdapter(adpt1);
        session1->setObserversByIdentity(
            Ice::Identity(),
            Ice::Identity(),
            app1->ice_getIdentity(),
            Ice::Identity(),
            Ice::Identity());

        appObs1->waitForUpdate(__LINE__);

        int serial = appObs1->serial;
        test(appObs1->applications.empty());

        try
        {
            ApplicationDescriptor app;
            app.name = "Application";
            int s = session1->startUpdate();
            test(s == serial);
            admin1->addApplication(app);
            appObs1->waitForUpdate(__LINE__);
            test(appObs1->applications.find("Application") != appObs1->applications.end());
            test(++serial == appObs1->serial);
        }
        catch (const Ice::UserException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
            ApplicationUpdateDescriptor update;
            update.name = "Application";
            update.variables.insert(make_pair(string("test"), string("test")));
            admin1->updateApplication(update);
            appObs1->waitForUpdate(__LINE__);
            test(appObs1->applications.find("Application") != appObs1->applications.end());
            test(appObs1->applications["Application"].descriptor.variables["test"] == "test");
            test(++serial == appObs1->serial);
        }
        catch (const Ice::UserException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
            ApplicationDescriptor app;
            app = appObs1->applications["Application"].descriptor;
            app.variables.clear();
            app.variables["test1"] = "test";
            admin1->syncApplication(app);
            appObs1->waitForUpdate(__LINE__);
            test(appObs1->applications.find("Application") != appObs1->applications.end());
            test(appObs1->applications["Application"].descriptor.variables.size() == 1);
            test(appObs1->applications["Application"].descriptor.variables["test1"] == "test");
            test(++serial == appObs1->serial);
        }
        catch (const Ice::UserException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
            admin1->removeApplication("Application");
            appObs1->waitForUpdate(__LINE__);
            test(appObs1->applications.empty());
            test(++serial == appObs1->serial);
        }
        catch (const Ice::UserException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        session1->destroy();
        adpt1->destroy();

        cout << "ok" << endl;
    }

    {
        cout << "testing adapter observer... " << flush;

        optional<AdminSessionPrx> session1(registry->createAdminSession("admin1", "test1"));
        auto admin1 = session1->getAdmin();
        auto adpt1 = communicator->createObjectAdapter("");
        auto adptObs1 = make_shared<AdapterObserverI>("adptObs1");
        auto adapter1 = adpt1->addWithUUID(adptObs1);
        adpt1->activate();
        registry->ice_getConnection()->setAdapter(adpt1);
        session1->setObserversByIdentity(
            Ice::Identity(),
            Ice::Identity(),
            Ice::Identity(),
            adapter1->ice_getIdentity(),
            Ice::Identity());

        adptObs1->waitForUpdate(__LINE__); // init

        try
        {
            ObjectPrx obj(communicator, "dummy:tcp -p 10000");

            auto locatorRegistry = communicator->getDefaultLocator()->getRegistry();
            locatorRegistry->setAdapterDirectProxy("DummyAdapter", obj);
            adptObs1->waitForUpdate(__LINE__);
            test(adptObs1->adapters.find("DummyAdapter") != adptObs1->adapters.end());
            test(adptObs1->adapters["DummyAdapter"].proxy == obj);

            obj = ObjectPrx(communicator, "dummy:tcp -p 10000 -h localhost");
            locatorRegistry->setAdapterDirectProxy("DummyAdapter", obj);
            adptObs1->waitForUpdate(__LINE__);
            test(adptObs1->adapters.find("DummyAdapter") != adptObs1->adapters.end());
            test(adptObs1->adapters["DummyAdapter"].proxy == obj);

            obj = ObjectPrx(communicator, "dummy:tcp -p 10000 -h localhost");
            locatorRegistry->setReplicatedAdapterDirectProxy("DummyAdapter", "DummyReplicaGroup", obj);
            adptObs1->waitForUpdate(__LINE__);
            test(adptObs1->adapters.find("DummyAdapter") != adptObs1->adapters.end());
            test(adptObs1->adapters["DummyAdapter"].proxy == obj);
            test(adptObs1->adapters["DummyAdapter"].replicaGroupId == "DummyReplicaGroup");

            obj = ObjectPrx(communicator, "dummy:tcp -p 10000 -h localhost");
            locatorRegistry->setReplicatedAdapterDirectProxy("DummyAdapter1", "DummyReplicaGroup", obj);
            adptObs1->waitForUpdate(__LINE__);
            test(adptObs1->adapters.find("DummyAdapter1") != adptObs1->adapters.end());
            test(adptObs1->adapters["DummyAdapter1"].proxy == obj);
            test(adptObs1->adapters["DummyAdapter1"].replicaGroupId == "DummyReplicaGroup");

            obj = ObjectPrx(communicator, "dummy:tcp -p 10000 -h localhost");
            locatorRegistry->setReplicatedAdapterDirectProxy("DummyAdapter2", "DummyReplicaGroup", obj);
            adptObs1->waitForUpdate(__LINE__);
            test(adptObs1->adapters.find("DummyAdapter2") != adptObs1->adapters.end());
            test(adptObs1->adapters["DummyAdapter2"].proxy == obj);
            test(adptObs1->adapters["DummyAdapter2"].replicaGroupId == "DummyReplicaGroup");

            admin->removeAdapter("DummyAdapter2");
            adptObs1->waitForUpdate(__LINE__);
            test(adptObs1->adapters.find("DummyAdapter2") == adptObs1->adapters.end());

            admin->removeAdapter("DummyReplicaGroup");
            adptObs1->waitForUpdate(__LINE__);
            adptObs1->waitForUpdate(__LINE__);
            test(adptObs1->adapters["DummyAdapter"].replicaGroupId == "");
            test(adptObs1->adapters["DummyAdapter1"].replicaGroupId == "");

            locatorRegistry->setAdapterDirectProxy("DummyAdapter", nullopt);
            adptObs1->waitForUpdate(__LINE__);
            test(adptObs1->adapters.find("DummyAdapter") == adptObs1->adapters.end());
        }
        catch (const Ice::UserException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        session1->destroy();
        adpt1->destroy();

        cout << "ok" << endl;
    }

    {
        cout << "testing object observer... " << flush;

        optional<AdminSessionPrx> session1(registry->createAdminSession("admin1", "test1"));
        auto admin1 = session1->getAdmin();
        auto adpt1 = communicator->createObjectAdapter("");
        auto objectObs1 = make_shared<ObjectObserverI>("objectObs1");
        auto object1 = adpt1->addWithUUID(objectObs1);
        adpt1->activate();
        registry->ice_getConnection()->setAdapter(adpt1);
        session1->setObserversByIdentity(
            Ice::Identity(),
            Ice::Identity(),
            Ice::Identity(),
            Ice::Identity(),
            object1->ice_getIdentity());

        objectObs1->waitForUpdate(__LINE__); // init

        try
        {
            ObjectPrx obj(communicator, "dummy:tcp -p 10000");

            admin->addObjectWithType(obj, "::Dummy");
            objectObs1->waitForUpdate(__LINE__);
            test(objectObs1->objects.find(Ice::stringToIdentity("dummy")) != objectObs1->objects.end());
            test(objectObs1->objects[Ice::stringToIdentity("dummy")].type == "::Dummy");
            test(objectObs1->objects[Ice::stringToIdentity("dummy")].proxy == obj);

            obj = ObjectPrx(communicator, "dummy:tcp -p 10000 -h localhost");
            admin->updateObject(obj);
            objectObs1->waitForUpdate(__LINE__);
            test(objectObs1->objects.find(Ice::stringToIdentity("dummy")) != objectObs1->objects.end());
            test(objectObs1->objects[Ice::stringToIdentity("dummy")].type == "::Dummy");
            test(objectObs1->objects[Ice::stringToIdentity("dummy")].proxy == obj);

            admin->removeObject(obj->ice_getIdentity());
            objectObs1->waitForUpdate(__LINE__);
            test(objectObs1->objects.find(Ice::stringToIdentity("dummy")) == objectObs1->objects.end());
        }
        catch (const Ice::UserException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        session1->destroy();
        adpt1->destroy();

        cout << "ok" << endl;
    }

    {
        cout << "testing node observer... " << flush;
        auto session1 = registry->createAdminSession("admin1", "test1");
        auto adpt1 = communicator->createObjectAdapter("");
        auto appObs1 = make_shared<ApplicationObserverI>("appObs1.3");
        auto app1 = adpt1->addWithUUID(appObs1);
        auto nodeObs1 = make_shared<NodeObserverI>("nodeObs1");
        auto no1 = adpt1->addWithUUID(nodeObs1);
        adpt1->activate();
        registry->ice_getConnection()->setAdapter(adpt1);
        session1->setObserversByIdentity(
            Ice::Identity(),
            no1->ice_getIdentity(),
            app1->ice_getIdentity(),
            Ice::Identity(),
            Ice::Identity());

        appObs1->waitForUpdate(__LINE__);
        nodeObs1->waitForUpdate(__LINE__); // init

        test(nodeObs1->nodes.find("localnode") != nodeObs1->nodes.end());
        test(appObs1->applications.empty());

        ApplicationDescriptor nodeApp;
        nodeApp.name = "NodeApp";
        auto server = make_shared<ServerDescriptor>();
        server->id = "node-1";
        server->exe = properties->getProperty("IceGridNodeExe");
        server->pwd = ".";
        server->allocatable = false;
        server->propertySet.properties.push_back(PropertyDescriptor{"IceGrid.Node.Name", "node-1"});
        server->propertySet.properties.push_back(
            PropertyDescriptor{"IceGrid.Node.Data", properties->getProperty("TestDir") + "/db/node-1"});
        server->propertySet.properties.push_back(PropertyDescriptor{"IceGrid.Node.Endpoints", "default"});
        server->propertySet.properties.push_back(PropertyDescriptor{"Ice.Admin.Endpoints", "tcp -h 127.0.0.1"});

        NodeDescriptor node;
        node.servers.push_back(std::move(server));
        nodeApp.nodes["localnode"] = node;

        session->startUpdate();
        admin->addApplication(nodeApp);
        session->finishUpdate();
        appObs1->waitForUpdate(__LINE__);

        admin->startServer("node-1");

        nodeObs1->waitForUpdate(__LINE__); // serverUpdate
        nodeObs1->waitForUpdate(__LINE__); // serverUpdate
        do
        {
            nodeObs1->waitForUpdate(__LINE__); // nodeUp
        } while (nodeObs1->nodes.find("node-1") == nodeObs1->nodes.end());

        test(nodeObs1->nodes["localnode"].servers.size() == 1);
        test(nodeObs1->nodes["localnode"].servers[0].state == ServerState::Active);
        admin->stopServer("node-1");

        nodeObs1->waitForUpdate(__LINE__); // serverUpdate(Deactivating)
        nodeObs1->waitForUpdate(__LINE__); // serverUpdate(Inactive)
        nodeObs1->waitForUpdate(__LINE__); // nodeDown
        test(nodeObs1->nodes["localnode"].servers[0].state == ServerState::Inactive);

        session->startUpdate();
        admin->removeApplication("NodeApp");
        session->finishUpdate();
        nodeObs1->waitForUpdate(__LINE__); // serverUpdate(Destroying)
        nodeObs1->waitForUpdate(__LINE__); // serverUpdate(Destroyed)

        appObs1->waitForUpdate(__LINE__);
        test(nodeObs1->nodes.find("node-1") == nodeObs1->nodes.end());

        ApplicationDescriptor testApp;
        testApp.name = "TestApp";

        server = make_shared<ServerDescriptor>();
        server->id = "Server";
        server->exe = properties->getProperty("ServerDir") + "/server";
        server->pwd = properties->getProperty("TestDir");

        server->allocatable = false;
        AdapterDescriptor adapter;
        adapter.name = "Server";
        adapter.id = "ServerAdapter";
        adapter.registerProcess = false;
        adapter.serverLifetime = true;
        server->adapters.push_back(std::move(adapter));
        server->propertySet.properties.push_back(PropertyDescriptor{"Server.Endpoints", "default"});
        server->propertySet.properties.push_back(PropertyDescriptor{"Ice.Admin.Endpoints", "tcp -h 127.0.0.1"});
        node = NodeDescriptor();
        node.servers.push_back(server);
        testApp.nodes["localnode"] = node;

        session->startUpdate();
        admin->addApplication(testApp);
        session->finishUpdate();
        appObs1->waitForUpdate(__LINE__);

        session->startUpdate();
        admin->startServer("Server");
        session->finishUpdate();

        nodeObs1->waitForUpdate(__LINE__); // serverUpdate
        nodeObs1->waitForUpdate(__LINE__); // serverUpdate
        nodeObs1->waitForUpdate(__LINE__); // adapterUpdate

        test(nodeObs1->nodes.find("localnode") != nodeObs1->nodes.end());
        test(nodeObs1->nodes["localnode"].servers.size() == 1);
        test(nodeObs1->nodes["localnode"].servers[0].state == ServerState::Active);
        test(nodeObs1->nodes["localnode"].adapters.size() == 1);
        test(nodeObs1->nodes["localnode"].adapters[0].proxy);

        test(nodeObs1->nodes["localnode"].servers[0].enabled);
        admin->enableServer("Server", false);
        nodeObs1->waitForUpdate(__LINE__); // serverUpdate
        test(!nodeObs1->nodes["localnode"].servers[0].enabled);
        admin->enableServer("Server", true);
        nodeObs1->waitForUpdate(__LINE__); // serverUpdate
        test(nodeObs1->nodes["localnode"].servers[0].enabled);

        admin->stopServer("Server");

        nodeObs1->waitForUpdate(__LINE__); // serverUpdate
        nodeObs1->waitForUpdate(__LINE__); // serverUpdate
        nodeObs1->waitForUpdate(__LINE__); // adapterUpdate

        test(nodeObs1->nodes.find("localnode") != nodeObs1->nodes.end());
        test(nodeObs1->nodes["localnode"].servers.size() == 1);
        test(nodeObs1->nodes["localnode"].servers[0].state == ServerState::Inactive);
        test(nodeObs1->nodes["localnode"].adapters.empty());

        session->startUpdate();
        admin->removeApplication("TestApp");
        session->finishUpdate();

        nodeObs1->waitForUpdate(__LINE__); // serverUpdate(Destroying)
        nodeObs1->waitForUpdate(__LINE__); // serverUpdate(Destroyed)
        test(nodeObs1->nodes["localnode"].servers.empty());

        appObs1->waitForUpdate(__LINE__);

        cout << "ok" << endl;
    }

    {
        cout << "testing registry observer... " << flush;
        auto session1 = registry->createAdminSession("admin1", "test1");
        auto adpt1 = communicator->createObjectAdapter("");
        auto appObs1 = make_shared<ApplicationObserverI>("appObs1.4");
        auto app1 = adpt1->addWithUUID(appObs1);
        auto registryObs1 = make_shared<RegistryObserverI>("registryObs1");
        auto ro1 = adpt1->addWithUUID(registryObs1);
        adpt1->activate();
        registry->ice_getConnection()->setAdapter(adpt1);
        session1->setObserversByIdentity(
            ro1->ice_getIdentity(),
            Ice::Identity(),
            app1->ice_getIdentity(),
            Ice::Identity(),
            Ice::Identity());

        appObs1->waitForUpdate(__LINE__);
        registryObs1->waitForUpdate(__LINE__); // init

        test(registryObs1->registries.find("Master") != registryObs1->registries.end());
        test(appObs1->applications.empty());

        QueryPrx query(communicator, "TestIceGrid/Query");
        auto registries = query->findAllObjectsByType("::IceGrid::Registry");
        const string prefix("Registry-");
        for (const auto& p : registries)
        {
            string name = p->ice_getIdentity().name;
            string::size_type pos = name.find(prefix);
            if (pos != string::npos)
            {
                name = name.substr(prefix.size());
                test(registryObs1->registries.find(name) != registryObs1->registries.end());
            }
        }
        cout << "ok" << endl;
    }

    {
        cout << "testing observer with direct proxy... " << flush;
        auto session1 = registry->createAdminSession("admin1", "test1");
        auto adpt1 = communicator->createObjectAdapterWithEndpoints("", "tcp");
        auto nodeObs1 = make_shared<NodeObserverI>("nodeObs1");
        auto no1 = adpt1->addWithUUID<NodeObserverPrx>(nodeObs1);
        adpt1->activate();

        session1->setObservers(nullopt, no1, nullopt, nullopt, nullopt);
        nodeObs1->waitForUpdate(__LINE__); // init

        session1->destroy();
        adpt1->destroy();

        cout << "ok" << endl;
    }

    {
        cout << "testing observer with indirect proxy... " << flush;
        auto session1 = registry->createAdminSession("admin1", "test1");
        communicator->getProperties()->setProperty("IndirectAdpt1.Endpoints", "tcp");
        communicator->getProperties()->setProperty("IndirectAdpt1.AdapterId", "adapter1");
        auto adpt1 = communicator->createObjectAdapter("IndirectAdpt1");
        test(communicator->getDefaultLocator());
        auto nodeObs1 = make_shared<NodeObserverI>("nodeObs1");

        auto no1 = adpt1->addWithUUID<NodeObserverPrx>(nodeObs1);
        assert(no1->ice_getAdapterId() == "adapter1");
        adpt1->activate();

        session1->setObservers(nullopt, no1, nullopt, nullopt, nullopt);
        nodeObs1->waitForUpdate(__LINE__); // init

        session1->destroy();
        adpt1->destroy();

        cout << "ok" << endl;
    }

    admin->stopServer("PermissionsVerifierServer");

    cout << "shutting down admin router... " << flush;
    admin->stopServer("Glacier2Admin");
    cout << "ok" << endl;

    cout << "shutting down router... " << flush;
    admin->stopServer("Glacier2");
    cout << "ok" << endl;

    session->destroy();
}
