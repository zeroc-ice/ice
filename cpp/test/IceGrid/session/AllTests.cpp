//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Thread.h>
#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <Glacier2/Router.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace IceGrid;

void
addProperty(const CommunicatorDescriptorPtr& communicator, const string& name, const string& value)
{
    PropertyDescriptor prop;
    prop.name = name;
    prop.value = value;
    communicator->propertySet.properties.push_back(prop);
}

class ObserverBase : public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    ObserverBase(const string& name) : _name(name), _updated(0)
    {
        _observers.insert(make_pair(name, this));
    }

    virtual ~ObserverBase()
    {
        _observers.erase(_name);
    }

    static void
    printStack()
    {
        map<string, ObserverBase*>::const_iterator p;
        for(p = _observers.begin(); p != _observers.end(); ++p)
        {
            vector<string>::const_iterator q = p->second->_stack.begin();
            if(p->second->_stack.size() > 10)
            {
                q = p->second->_stack.begin() +
                    static_cast<vector<string>::difference_type>(p->second->_stack.size() - 10);
            }
            cerr << "Last 10 updates of observer `" << p->second->_name << "':" << endl;
            for(; q != p->second->_stack.end(); ++q)
            {
                cerr << "  " << *q << endl;
            }
            p->second->_stack.clear();
        }
    }

    void
    trace(const string& msg)
    {
        _stack.push_back(msg);
    }

    void
    waitForUpdate(const char*, int line)
    {
        Lock sync(*this);

        ostringstream os;
        os << "wait for update from line " << line;
        trace(os.str());

        while(!_updated)
        {
            wait();
        }
        --_updated;
    }

protected:

    void
    updated(const string& update)
    {
        trace(update);
        ++_updated;
        notifyAll();
    }

    string _name;
    vector<string> _stack;
    int _updated;
    static map<string, ObserverBase*> _observers;
};
map<string, ObserverBase*> ObserverBase::_observers;

class ApplicationObserverI : public ApplicationObserver, public ObserverBase
{
public:

    ApplicationObserverI(const string& name) : ObserverBase(name)
    {
    }

    virtual void
    applicationInit(int serialP, const ApplicationInfoSeq& apps, const Ice::Current&)
    {
        Lock sync(*this);
        for(ApplicationInfoSeq::const_iterator p = apps.begin(); p != apps.end(); ++p)
        {
            if(p->descriptor.name != "Test") // Ignore the test application from application.xml!
            {
                this->applications.insert(make_pair(p->descriptor.name, *p));
            }
        }
        updated(updateSerial(serialP, "init update"));
    }

    virtual void
    applicationAdded(int serialP, const ApplicationInfo& app, const Ice::Current&)
    {
        Lock sync(*this);
        this->applications.insert(make_pair(app.descriptor.name, app));
        updated(updateSerial(serialP, "application added `" + app.descriptor.name + "'"));
    }

    virtual void
    applicationRemoved(int serialP, const std::string& name, const Ice::Current&)
    {
        Lock sync(*this);
        this->applications.erase(name);
        updated(updateSerial(serialP, "application removed `" + name + "'"));
    }

    virtual void
    applicationUpdated(int serialP, const ApplicationUpdateInfo& info, const Ice::Current&)
    {
        Lock sync(*this);
        const ApplicationUpdateDescriptor& desc = info.descriptor;
        for(Ice::StringSeq::const_iterator q = desc.removeVariables.begin(); q != desc.removeVariables.end(); ++q)
        {
            this->applications[desc.name].descriptor.variables.erase(*q);
        }
        for(map<string, string>::const_iterator p = desc.variables.begin(); p != desc.variables.end(); ++p)
        {
            this->applications[desc.name].descriptor.variables[p->first] = p->second;
        }
        updated(updateSerial(serialP, "application updated `" + desc.name + "'"));
    }

    int serial;
    map<string, ApplicationInfo> applications;

private:

    string
    updateSerial(int serialP, const string& update)
    {
        serial = serialP;
        ostringstream os;
        os << update << " (serial = " << serial << ")";
        return os.str();
    }
};
typedef IceUtil::Handle<ApplicationObserverI> ApplicationObserverIPtr;

class AdapterObserverI : public AdapterObserver, public ObserverBase
{
public:

    AdapterObserverI(const string& name) : ObserverBase(name)
    {
    }

    virtual void
    adapterInit(const AdapterInfoSeq& adaptersP, const Ice::Current&)
    {
        Lock sync(*this);
        for(AdapterInfoSeq::const_iterator q = adaptersP.begin(); q != adaptersP.end(); ++q)
        {
            adapters.insert(make_pair(q->id, *q));
        }
        updated(updateSerial(0, "init update"));
    }

    void
    adapterAdded(const AdapterInfo& info, const Ice::Current&)
    {
        Lock sync(*this);
        adapters.insert(make_pair(info.id, info));
        updated(updateSerial(0, "adapter added `" + info.id + "'"));
    }

    void
    adapterUpdated(const AdapterInfo& info, const Ice::Current&)
    {
        Lock sync(*this);
        adapters[info.id] = info;
        updated(updateSerial(0, "adapter updated `" + info.id + "'"));
    }

    void
    adapterRemoved(const string& id, const Ice::Current&)
    {
        Lock sync(*this);
        adapters.erase(id);
        updated(updateSerial(0, "adapter removed `" + id + "'"));
    }

    int serial;
    map<string, AdapterInfo> adapters;

private:

    string
    updateSerial(int serialP, const string& update)
    {
        serial = serialP;
        ostringstream os;
        os << update << " (serial = " << serial << ")";
        return os.str();
    }
};
typedef IceUtil::Handle<AdapterObserverI> AdapterObserverIPtr;

class ObjectObserverI : public ObjectObserver, public ObserverBase
{
public:

    ObjectObserverI(const string& name) : ObserverBase(name)
    {
    }

    virtual void
    objectInit(const ObjectInfoSeq& objectsP, const Ice::Current&)
    {
        Lock sync(*this);
        for(ObjectInfoSeq::const_iterator r = objectsP.begin(); r != objectsP.end(); ++r)
        {
            objects.insert(make_pair(r->proxy->ice_getIdentity(), *r));
        }
        updated(updateSerial(0, "init update"));
    }

    void
    objectAdded(const ObjectInfo& info, const Ice::Current&)
    {
        Lock sync(*this);
        objects.insert(make_pair(info.proxy->ice_getIdentity(), info));
        updated(updateSerial(0, "object added `" + info.proxy->ice_toString() + "'"));
    }

    void
    objectUpdated(const ObjectInfo& info, const Ice::Current&)
    {
        Lock sync(*this);
        objects[info.proxy->ice_getIdentity()] = info;
        updated(updateSerial(0, "object updated `" + info.proxy->ice_toString() + "'"));
    }

    void
    objectRemoved(const Ice::Identity& id, const Ice::Current& current)
    {
        Lock sync(*this);
        objects.erase(id);
        updated(updateSerial(0, "object removed `" +
                             current.adapter->getCommunicator()->identityToString(id) + "'"));
    }

    int serial;
    map<Ice::Identity, ObjectInfo> objects;

private:

    string
    updateSerial(int serialP, const string& update)
    {
        serial = serialP;
        ostringstream os;
        os << update << " (serial = " << serial << ")";
        return os.str();
    }
};
typedef IceUtil::Handle<ObjectObserverI> ObjectObserverIPtr;

class NodeObserverI : public NodeObserver, public ObserverBase
{
public:

    NodeObserverI(const string& name) : ObserverBase(name)
    {
    }

    virtual void
    nodeInit(const NodeDynamicInfoSeq& info, const Ice::Current&)
    {
        Lock sync(*this);
        for(NodeDynamicInfoSeq::const_iterator p = info.begin(); p != info.end(); ++p)
        {
            this->nodes[p->info.name] = filter(*p);
        }
        updated("init");
    }

    virtual void
    nodeUp(const NodeDynamicInfo& info, const Ice::Current&)
    {
        Lock sync(*this);
        this->nodes[info.info.name] = filter(info);
        updated("node `" + info.info.name + "' up");
    }

    virtual void
    nodeDown(const string& name, const Ice::Current&)
    {
        Lock sync(*this);
        this->nodes.erase(name);
        updated("node `" + name + "' down");
    }

    virtual void
    updateServer(const string& node, const ServerDynamicInfo& info, const Ice::Current&)
    {
        if(info.id == "Glacier2" || info.id == "Glacier2Admin" || info.id == "PermissionsVerifierServer")
        {
            return;
        }

        Lock sync(*this);
        //cerr << node << " " << info.id << " " << info.state << " " << info.pid << endl;
        ServerDynamicInfoSeq& servers = this->nodes[node].servers;
        ServerDynamicInfoSeq::iterator p;
        for(p = servers.begin(); p != servers.end(); ++p)
        {
            if(p->id == info.id)
            {
                if(info.state == Destroyed)
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
        if(info.state != Destroyed && p == servers.end())
        {
            servers.push_back(info);
        }

        ostringstream os;
        os << "server `" << info.id << "' on node `" << node << "' state updated: " << info.state
           << " (pid = " << info.pid << ")";
        updated(os.str());
    }

    virtual void
    updateAdapter(const string& node, const AdapterDynamicInfo& info, const Ice::Current&)
    {
        if(info.id == "PermissionsVerifierServer.Server")
        {
            return;
        }

        Lock sync(*this);
        //cerr << "update adapter: " << info.id << " " << (info.proxy ? "active" : "inactive") << endl;
        AdapterDynamicInfoSeq& adapters = this->nodes[node].adapters;
        AdapterDynamicInfoSeq::iterator p;
        for(p = adapters.begin(); p != adapters.end(); ++p)
        {
            if(p->id == info.id)
            {
                if(info.proxy)
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
        if(info.proxy && p == adapters.end())
        {
            adapters.push_back(info);
        }

        ostringstream os;
        os << "adapter `" << info.id << " on node `" << node << "' state updated: "
           << (info.proxy ? "active" : "inactive");
        updated(os.str());
    }

    NodeDynamicInfo
    filter(const NodeDynamicInfo& info)
    {
        if(info.info.name != "localnode")
        {
            return info;
        }

        NodeDynamicInfo filtered;
        filtered.info = info.info;

        for(ServerDynamicInfoSeq::const_iterator p = info.servers.begin(); p != info.servers.end(); ++p)
        {
            if(p->id == "Glacier2" || p->id == "Glacier2Admin" || p->id == "PermissionsVerifierServer")
            {
                continue;
            }
            filtered.servers.push_back(*p);
        }

        for(AdapterDynamicInfoSeq::const_iterator a = info.adapters.begin(); a != info.adapters.end(); ++a)
        {
            if(a->id == "PermissionsVerifierServer.Server")
            {
                continue;
            }
            filtered.adapters.push_back(*a);
        }

        return filtered;
    }

    map<string, NodeDynamicInfo> nodes;
};
typedef IceUtil::Handle<NodeObserverI> NodeObserverIPtr;

class RegistryObserverI : public RegistryObserver, public ObserverBase
{
public:

    RegistryObserverI(const string& name) : ObserverBase(name)
    {
    }

    virtual void
    registryInit(const RegistryInfoSeq& info, const Ice::Current&)
    {
        Lock sync(*this);
        for(RegistryInfoSeq::const_iterator p = info.begin(); p != info.end(); ++p)
        {
            this->registries[p->name] = *p;
        }
        updated("init");
    }

    virtual void
    registryUp(const RegistryInfo& info, const Ice::Current&)
    {
        Lock sync(*this);
        this->registries[info.name] = info;
        updated("registry `" + info.name + "' up");
    }

    virtual void
    registryDown(const string& name, const Ice::Current&)
        {
            Lock sync(*this);
            this->registries.erase(name);
            updated("registry `" + name + "' down");
        }

    map<string, RegistryInfo> registries;
};
typedef IceUtil::Handle<RegistryObserverI> RegistryObserverIPtr;

void
testFailedAndPrintObservers(const char* expr, const char* file, unsigned int line)
{
    ObserverBase::printStack();
    Test::testFailed(expr, file, line);
}

#undef test
#define test(ex) ((ex) ? ((void)0) : testFailedAndPrintObservers(#ex, __FILE__, __LINE__))

#if defined(_AIX) && defined(__GNUC__) && !defined(__ibmxl__)
// Strange optimization bug with catching ExtendedPermissionDeniedException with GCC 8.1 on AIX
__attribute__((optimize("O0")))
#endif
void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    bool encoding10 = communicator->getProperties()->getProperty("Ice.Default.EncodingVersion") == "1.0";

    IceGrid::RegistryPrx registry = IceGrid::RegistryPrx::checkedCast(
        communicator->stringToProxy(communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));

    AdminSessionPrx session = registry->createAdminSession("admin3", "test3");
    session->ice_getConnection()->setACM(registry->getACMTimeout(),
                                         IceUtil::None,
                                         Ice::ICE_ENUM(ACMHeartbeat, HeartbeatAlways));

    AdminPrx admin = session->getAdmin();
    test(admin);

    cout << "starting router... " << flush;
    try
    {
        admin->startServer("Glacier2");
    }
    catch(const ServerStartException& ex)
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
    catch(const ServerStartException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }
    cout << "ok" << endl;

    Ice::PropertiesPtr properties = communicator->getProperties();

    IceGrid::RegistryPrx registry1 = IceGrid::RegistryPrx::uncheckedCast(registry->ice_connectionId("reg1"));
    IceGrid::RegistryPrx registry2 = IceGrid::RegistryPrx::uncheckedCast(registry->ice_connectionId("reg2"));

    Glacier2::RouterPrx router = Glacier2::RouterPrx::uncheckedCast(
        communicator->stringToProxy("Glacier2/router:default -p 12347 -h 127.0.0.1"));
    Glacier2::RouterPrx adminRouter = Glacier2::RouterPrx::uncheckedCast(
        communicator->stringToProxy("Glacier2/router:default -p 12348 -h 127.0.0.1"));

    Glacier2::RouterPrx router1 = Glacier2::RouterPrx::uncheckedCast(router->ice_connectionId("router1"));
    Glacier2::RouterPrx router2 = Glacier2::RouterPrx::uncheckedCast(router->ice_connectionId("router2"));

    Glacier2::RouterPrx adminRouter1 = Glacier2::RouterPrx::uncheckedCast(adminRouter->ice_connectionId("admRouter1"));
    Glacier2::RouterPrx adminRouter2 = Glacier2::RouterPrx::uncheckedCast(adminRouter->ice_connectionId("admRouter2"));

    //
    // TODO: Find a better way to wait for the Glacier2 router to be
    // fully started...
    //
    while(true)
    {
        try
        {
            router1->ice_ping();
            adminRouter1->ice_ping();
            break;
        }
        catch(const Ice::LocalException&)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));
        }
    }

    {
        cout << "testing username/password sessions... " << flush;

        SessionPrx session1, session2;

        session1 = SessionPrx::uncheckedCast(registry1->createSession("client1", "test1")->ice_connectionId("reg1"));
        session2 = SessionPrx::uncheckedCast(registry2->createSession("client2", "test2")->ice_connectionId("reg2"));
        try
        {
            registry1->createSession("client3", "test1");
            test(false);
        }
        catch(const IceGrid::PermissionDeniedException&)
        {
        }
        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            registry1->createSession("client3", "test1", ctx);
            test(false);
        }
        catch(const IceGrid::PermissionDeniedException& ex)
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
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            session2->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        try
        {
            session1->ice_connectionId("reg2")->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            session2->ice_connectionId("reg1")->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        session1->destroy();
        session2->destroy();

        AdminSessionPrx adminSession1, adminSession2;

        adminSession1 = AdminSessionPrx::uncheckedCast(
            registry1->createAdminSession("admin1", "test1")->ice_connectionId("reg1"));
        adminSession2 = AdminSessionPrx::uncheckedCast(
            registry2->createAdminSession("admin2", "test2")->ice_connectionId("reg2"));
        try
        {
            registry1->createAdminSession("admin3", "test1");
            test(false);
        }
        catch(const IceGrid::PermissionDeniedException&)
        {
        }
        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            registry1->createSession("admin3", "test1", ctx);
            test(false);
        }
        catch(const IceGrid::PermissionDeniedException& ex)
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
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            adminSession2->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        adminSession1->getAdmin()->ice_connectionId("reg1")->ice_ping();
        adminSession2->getAdmin()->ice_connectionId("reg2")->ice_ping();

        try
        {
            adminSession1->getAdmin()->ice_connectionId("reg2")->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            adminSession2->getAdmin()->ice_connectionId("reg1")->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        adminSession1->destroy();
        adminSession2->destroy();

        cout << "ok" << endl;
    }

    if(properties->getProperty("Ice.Default.Protocol") == "ssl")
    {
        cout << "testing sessions from secure connection... " << flush;

        SessionPrx session1, session2;

        session1 = SessionPrx::uncheckedCast(registry1->createSessionFromSecureConnection()->ice_connectionId("reg1"));
        session2 = SessionPrx::uncheckedCast(registry2->createSessionFromSecureConnection()->ice_connectionId("reg2"));

        session1->ice_ping();
        session2->ice_ping();

        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            registry1->createSessionFromSecureConnection(ctx);
            test(false);
        }
        catch(const IceGrid::PermissionDeniedException& ex)
        {
            test(ex.reason == "reason");
        }

        try
        {
            session1->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            session2->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        session1->destroy();
        session2->destroy();

        AdminSessionPrx adminSession1, adminSession2;

        adminSession1 = AdminSessionPrx::uncheckedCast(
            registry1->createAdminSessionFromSecureConnection()->ice_connectionId("reg1"));
        adminSession2 = AdminSessionPrx::uncheckedCast(
            registry2->createAdminSessionFromSecureConnection()->ice_connectionId("reg2"));

        adminSession1->ice_ping();
        adminSession2->ice_ping();

        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            registry1->createAdminSessionFromSecureConnection(ctx);
            test(false);
        }
        catch(const IceGrid::PermissionDeniedException& ex)
        {
            test(ex.reason == "reason");
        }

        try
        {
            adminSession1->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            adminSession2->ice_connectionId("")->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
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
        catch(const IceGrid::PermissionDeniedException&)
        {
        }
        try
        {
            registry1->createAdminSessionFromSecureConnection();
            test(false);
        }
        catch(const IceGrid::PermissionDeniedException&)
        {
        }
        cout << "ok" << endl;
    }

    {
        cout << "testing Glacier2 username/password sessions... " << flush;

        SessionPrx session1, session2;

        Glacier2::SessionPrx base;

        base = router1->createSession("client1", "test1");
        test(base);
        session1 = SessionPrx::uncheckedCast(base->ice_connectionId("router1")->ice_router(router1));

        base = router2->createSession("client2", "test2");
        test(base);
        session2 = SessionPrx::uncheckedCast(base->ice_connectionId("router2")->ice_router(router2));

        try
        {
            router1->createSession("client3", "test1");
            test(false);
        }
        catch(const Glacier2::CannotCreateSessionException&)
        {
        }
        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            router->ice_connectionId("routerex")->createSession("client3", "test1", ctx);
            test(false);
        }
        catch(const Test::ExtendedPermissionDeniedException& ex)
        {
            test(!encoding10 && ex.reason == "reason");
        }
        catch(const Glacier2::PermissionDeniedException& ex)
        {
            test(encoding10 && ex.reason == "reason");
        }

        session1->ice_ping();
        session2->ice_ping();

        try
        {
            session1->ice_connectionId("router2")->ice_router(router2)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            session2->ice_connectionId("router1")->ice_router(router1)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        Ice::ObjectPrx obj = communicator->stringToProxy("TestIceGrid/Query");
        obj->ice_connectionId("router1")->ice_router(router1)->ice_ping();
        obj->ice_connectionId("router2")->ice_router(router2)->ice_ping();

        obj = communicator->stringToProxy("TestIceGrid/Registry");
        try
        {
            obj->ice_connectionId("router1")->ice_router(router1)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            obj->ice_connectionId("router2")->ice_router(router2)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        router1->destroySession();
        router2->destroySession();

        AdminSessionPrx admSession1, admSession2;

        base = adminRouter1->createSession("admin1", "test1");
        admSession1 = AdminSessionPrx::uncheckedCast(base->ice_connectionId("admRouter1")->ice_router(adminRouter1));

        base = adminRouter2->createSession("admin2", "test2");
        admSession2 = AdminSessionPrx::uncheckedCast(base->ice_connectionId("admRouter2")->ice_router(adminRouter2));

        try
        {
            adminRouter1->createSession("admin3", "test1");
            test(false);
        }
        catch(const Glacier2::CannotCreateSessionException&)
        {
        }
        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            adminRouter->ice_connectionId("routerex")->createSession("admin3", "test1", ctx);
            test(false);
        }
        catch(const Test::ExtendedPermissionDeniedException& ex)
        {
            test(!encoding10 && ex.reason == "reason");
        }
        catch(const Glacier2::PermissionDeniedException& ex)
        {
            test(encoding10 && ex.reason == "reason");
        }

        admSession1->ice_ping();
        admSession2->ice_ping();

        Ice::ObjectPrx admin1 = admSession1->getAdmin()->ice_router(adminRouter1)->ice_connectionId("admRouter1");
        Ice::ObjectPrx admin2 = admSession2->getAdmin()->ice_router(adminRouter2)->ice_connectionId("admRouter2");

        admin1->ice_ping();
        admin2->ice_ping();

        obj = communicator->stringToProxy("TestIceGrid/Query");
        obj->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
        obj->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();

        try
        {
            admSession1->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            admSession2->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        try
        {
            admin1->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            admin2->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        adminRouter1->destroySession();
        adminRouter2->destroySession();

        cout << "ok" << endl;
    }

    if(properties->getProperty("Ice.Default.Protocol") == "ssl")
    {
        cout << "testing Glacier2 sessions from secure connection... " << flush;

        SessionPrx session1, session2;

        Glacier2::SessionPrx base;

        //
        // BUGFIX: We can't re-use the same router proxies because of bug 1034.
        //
        router1 = Glacier2::RouterPrx::uncheckedCast(router1->ice_connectionId("router11"));
        router2 = Glacier2::RouterPrx::uncheckedCast(router2->ice_connectionId("router21"));

        base = router1->createSessionFromSecureConnection();
        session1 = SessionPrx::uncheckedCast(base->ice_connectionId("router11")->ice_router(router1));

        base = router2->createSessionFromSecureConnection();
        session2 = SessionPrx::uncheckedCast(base->ice_connectionId("router21")->ice_router(router2));

        session1->ice_ping();
        session2->ice_ping();

        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            router->ice_connectionId("routerex")->createSessionFromSecureConnection(ctx);
            test(false);
        }
        catch(const Test::ExtendedPermissionDeniedException& ex)
        {
            test(!encoding10 && ex.reason == "reason");
        }
        catch(const Glacier2::PermissionDeniedException& ex)
        {
            test(encoding10 && ex.reason == "reason");
        }

        try
        {
            session1->ice_connectionId("router21")->ice_router(router2)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            session2->ice_connectionId("router11")->ice_router(router1)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        Ice::ObjectPrx obj = communicator->stringToProxy("TestIceGrid/Query");
        obj->ice_connectionId("router11")->ice_router(router1)->ice_ping();
        obj->ice_connectionId("router21")->ice_router(router2)->ice_ping();

        obj = communicator->stringToProxy("TestIceGrid/Registry");
        try
        {
            obj->ice_connectionId("router11")->ice_router(router1)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            obj->ice_connectionId("router21")->ice_router(router2)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        router1->destroySession();
        router2->destroySession();

        AdminSessionPrx admSession1, admSession2;

        //
        // BUGFIX: We can't re-use the same router proxies because of bug 1034.
        //
        adminRouter1 = Glacier2::RouterPrx::uncheckedCast(adminRouter->ice_connectionId("admRouter11"));
        adminRouter2 = Glacier2::RouterPrx::uncheckedCast(adminRouter->ice_connectionId("admRouter21"));

        base = adminRouter1->createSessionFromSecureConnection();
        admSession1 = AdminSessionPrx::uncheckedCast(base->ice_connectionId("admRouter11")->ice_router(adminRouter1));

        base = adminRouter2->createSessionFromSecureConnection();
        admSession2 = AdminSessionPrx::uncheckedCast(base->ice_connectionId("admRouter21")->ice_router(adminRouter2));

        admSession1->ice_ping();
        admSession2->ice_ping();

        try
        {
            Ice::Context ctx;
            ctx["throw"] = "1";
            adminRouter->ice_connectionId("routerex")->createSessionFromSecureConnection(ctx);
            test(false);
        }
        catch(const Test::ExtendedPermissionDeniedException& ex)
        {
            test(!encoding10 && ex.reason == "reason");
        }
        catch(const Glacier2::PermissionDeniedException& ex)
        {
            test(encoding10 && ex.reason == "reason");
        }

        Ice::ObjectPrx admin1 = admSession1->getAdmin()->ice_router(adminRouter1)->ice_connectionId("admRouter11");
        Ice::ObjectPrx admin2 = admSession2->getAdmin()->ice_router(adminRouter2)->ice_connectionId("admRouter21");

        admin1->ice_ping();
        admin2->ice_ping();

        obj = communicator->stringToProxy("TestIceGrid/Query");
        obj->ice_connectionId("admRouter11")->ice_router(adminRouter1)->ice_ping();
        obj->ice_connectionId("admRouter21")->ice_router(adminRouter2)->ice_ping();

        try
        {
            admSession1->ice_connectionId("admRouter21")->ice_router(adminRouter2)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            admSession2->ice_connectionId("admRouter11")->ice_router(adminRouter1)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }

        try
        {
            admin1->ice_connectionId("admRouter21")->ice_router(adminRouter2)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
        {
        }
        try
        {
            admin2->ice_connectionId("admRouter11")->ice_router(adminRouter1)->ice_ping();
            test(false);
        }
        catch(const Ice::ObjectNotExistException&)
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
        catch(const Glacier2::PermissionDeniedException&)
        {
        }
        try
        {
            adminRouter1->createSessionFromSecureConnection();
            test(false);
        }
        catch(const Glacier2::PermissionDeniedException&)
        {
        }
        cout << "ok" << endl;
    }

    {
        cout << "testing updates with admin sessions... " << flush;
        AdminSessionPrx session1 = registry->createAdminSession("admin1", "test1");
        AdminSessionPrx session2 = registry->createAdminSession("admin2", "test2");

        session1->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::HeartbeatOnIdle);
        session2->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::HeartbeatOnIdle);

        AdminPrx admin1 = session1->getAdmin();
        AdminPrx admin2 = session2->getAdmin();

        Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapter("");
        ApplicationObserverIPtr appObs1 = new ApplicationObserverI("appObs1.1");
        Ice::ObjectPrx app1 = adpt1->addWithUUID(appObs1);
        NodeObserverIPtr nodeObs1 = new NodeObserverI("nodeObs1");
        Ice::ObjectPrx no1 = adpt1->addWithUUID(nodeObs1);
        adpt1->activate();
        registry->ice_getConnection()->setAdapter(adpt1);
        session1->setObserversByIdentity(Ice::Identity(),
                                         no1->ice_getIdentity(),
                                         app1->ice_getIdentity(),
                                         Ice::Identity(),
                                         Ice::Identity());

        Ice::ObjectAdapterPtr adpt2 = communicator->createObjectAdapterWithEndpoints("Observer2", "default");
        ApplicationObserverIPtr appObs2 = new ApplicationObserverI("appObs2");
        Ice::ObjectPrx app2 = adpt2->addWithUUID(appObs2);
        NodeObserverIPtr nodeObs2 = new NodeObserverI("nodeObs1");
        Ice::ObjectPrx no2 = adpt2->addWithUUID(nodeObs2);
        adpt2->activate();
        session2->setObservers(0,
                               NodeObserverPrx::uncheckedCast(no2),
                               ApplicationObserverPrx::uncheckedCast(app2),
                               0,
                               0);

        appObs1->waitForUpdate(__FILE__, __LINE__);
        appObs2->waitForUpdate(__FILE__, __LINE__);

        int serial = appObs1->serial;
        test(serial == appObs2->serial);

        try
        {
            session1->getAdmin()->ice_ping();
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }

        try
        {
            int s = session1->startUpdate();
            test(s != serial + 1);
        }
        catch(const AccessDeniedException&)
        {
            test(false);
        }

        try
        {
            int s = session1->startUpdate();
            test(s == serial);
        }
        catch(const Ice::UserException&)
        {
            test(false);
        }

        try
        {
            session2->startUpdate();
            test(false);
        }
        catch(const AccessDeniedException& ex)
        {
            test(ex.lockUserId == "admin1");
        }

        try
        {
            session1->finishUpdate();
        }
        catch(const Ice::UserException&)
        {
            test(false);
        }

        try
        {
            int s = session2->startUpdate();
            test(s == appObs2->serial);
        }
        catch(const Ice::UserException&)
        {
            test(false);
        }

        try
        {
            ApplicationDescriptor app;
            app.name = "Application";
            admin2->addApplication(app);
        }
        catch(const Ice::UserException&)
        {
            test(false);
        }

        try
        {
            admin1->addApplication(ApplicationDescriptor());
            test(false);
        }
        catch(const AccessDeniedException&)
        {
        }

        try
        {
            session2->finishUpdate();
        }
        catch(const Ice::UserException&)
        {
            test(false);
        }

        appObs1->waitForUpdate(__FILE__, __LINE__);
        appObs2->waitForUpdate(__FILE__, __LINE__);

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
        catch(const Ice::UserException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        appObs1->waitForUpdate(__FILE__, __LINE__);
        appObs2->waitForUpdate(__FILE__, __LINE__);

        test(serial + 1 == appObs1->serial);
        test(serial + 1 == appObs2->serial);
        ++serial;

        //
        // We now allow modifying the database without holding the
        // exclusive lock.
        //
//      try
//      {
//          ApplicationUpdateDescriptor update;
//          update.name = "Application";
//          admin1->updateApplication(update);
//          test(false);
//      }
//      catch(const AccessDeniedException&)
//      {
//      }

        try
        {
            int s = session2->startUpdate();
            test(s == serial);
            admin2->removeApplication("Application");
            session2->finishUpdate();
        }
        catch(const Ice::UserException&)
        {
            test(false);
        }

        appObs1->waitForUpdate(__FILE__, __LINE__);
        appObs2->waitForUpdate(__FILE__, __LINE__);

        test(serial + 1 == appObs1->serial);
        test(serial + 1 == appObs2->serial);
        ++serial;

        try
        {
            int s = session1->startUpdate();
            test(s == serial);
        }
        catch(const Ice::UserException&)
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
        catch(const Ice::UserException&)
        {
            test(false);
        }
        session2->destroy();

        adpt1->destroy();
        adpt2->destroy();

        //
        // TODO: test session reaping?
        //

        cout << "ok" << endl;
    }

    {
        cout << "testing invalid configuration... " << flush;
        AdminSessionPrx session1 = registry->createAdminSession("admin1", "test1");
        AdminPrx admin1 = session1->getAdmin();

        Ice::LocatorRegistryPrx locatorRegistry = communicator->getDefaultLocator()->getRegistry();

        try
        {
            ApplicationDescriptor app;
            app.name = string(512, 'A');
            admin1->addApplication(app);
            test(false);
        }
        catch(const DeploymentException&)
        {
        }

        Ice::ObjectPrx obj = communicator->stringToProxy("dummy:tcp -p 10000");
        try
        {
            locatorRegistry->setAdapterDirectProxy(string(512, 'A'), obj);
            test(false);
        }
        catch(const Ice::UnknownException&)
        {
        }

        try
        {
            locatorRegistry->setReplicatedAdapterDirectProxy("Adapter", string(512, 'A'), obj);
            test(false);
        }
        catch(const Ice::UnknownException&)
        {
        }

        try
        {
            admin1->addObjectWithType(obj, string(512, 'T'));
            test(false);
        }
        catch(const DeploymentException&)
        {
        }

        cout << "ok" << endl;
    }

    {
        cout << "testing application observer... " << flush;
        AdminSessionPrx session1 = registry->createAdminSession("admin1", "test1");
        AdminPrx admin1 = session1->getAdmin();

        session1->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::HeartbeatOnIdle);

        Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapter("");
        ApplicationObserverIPtr appObs1 = new ApplicationObserverI("appObs1.2");
        Ice::ObjectPrx app1 = adpt1->addWithUUID(appObs1);
        adpt1->activate();
        registry->ice_getConnection()->setAdapter(adpt1);
        session1->setObserversByIdentity(Ice::Identity(),
                                         Ice::Identity(),
                                         app1->ice_getIdentity(),
                                         Ice::Identity(),
                                         Ice::Identity());

        appObs1->waitForUpdate(__FILE__, __LINE__);

        int serial = appObs1->serial;
        test(appObs1->applications.empty());

        try
        {
            ApplicationDescriptor app;
            app.name = "Application";
            int s = session1->startUpdate();
            test(s == serial);
            admin1->addApplication(app);
            appObs1->waitForUpdate(__FILE__, __LINE__);
            test(appObs1->applications.find("Application") != appObs1->applications.end());
            test(++serial == appObs1->serial);
        }
        catch(const Ice::UserException& ex)
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
            appObs1->waitForUpdate(__FILE__, __LINE__);
            test(appObs1->applications.find("Application") != appObs1->applications.end());
            test(appObs1->applications["Application"].descriptor.variables["test"] == "test");
            test(++serial == appObs1->serial);
        }
        catch(const Ice::UserException& ex)
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
            appObs1->waitForUpdate(__FILE__, __LINE__);
            test(appObs1->applications.find("Application") != appObs1->applications.end());
            test(appObs1->applications["Application"].descriptor.variables.size() == 1);
            test(appObs1->applications["Application"].descriptor.variables["test1"] == "test");
            test(++serial == appObs1->serial);
        }
        catch(const Ice::UserException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
            admin1->removeApplication("Application");
            appObs1->waitForUpdate(__FILE__, __LINE__);
            test(appObs1->applications.empty());
            test(++serial == appObs1->serial);
        }
        catch(const Ice::UserException& ex)
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

        AdminSessionPrx session1 = AdminSessionPrx::uncheckedCast(registry->createAdminSession("admin1", "test1"));
        AdminPrx admin1 = session1->getAdmin();

        session1->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::HeartbeatOnIdle);

        Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapter("");
        AdapterObserverIPtr adptObs1 = new AdapterObserverI("adptObs1");
        Ice::ObjectPrx adapter1 = adpt1->addWithUUID(adptObs1);
        adpt1->activate();
        registry->ice_getConnection()->setAdapter(adpt1);
        session1->setObserversByIdentity(Ice::Identity(),
                                         Ice::Identity(),
                                         Ice::Identity(),
                                         adapter1->ice_getIdentity(),
                                         Ice::Identity());

        adptObs1->waitForUpdate(__FILE__, __LINE__); // init

        try
        {
            Ice::ObjectPrx obj = communicator->stringToProxy("dummy:tcp -p 10000");

            Ice::LocatorRegistryPrx locatorRegistry = communicator->getDefaultLocator()->getRegistry();
            locatorRegistry->setAdapterDirectProxy("DummyAdapter", obj);
            adptObs1->waitForUpdate(__FILE__, __LINE__);
            test(adptObs1->adapters.find("DummyAdapter") != adptObs1->adapters.end());
            test(adptObs1->adapters["DummyAdapter"].proxy == obj);

            obj = communicator->stringToProxy("dummy:tcp -p 10000 -h localhost");
            locatorRegistry->setAdapterDirectProxy("DummyAdapter", obj);
            adptObs1->waitForUpdate(__FILE__, __LINE__);
            test(adptObs1->adapters.find("DummyAdapter") != adptObs1->adapters.end());
            test(adptObs1->adapters["DummyAdapter"].proxy == obj);

            obj = communicator->stringToProxy("dummy:tcp -p 10000 -h localhost");
            locatorRegistry->setReplicatedAdapterDirectProxy("DummyAdapter", "DummyReplicaGroup", obj);
            adptObs1->waitForUpdate(__FILE__, __LINE__);
            test(adptObs1->adapters.find("DummyAdapter") != adptObs1->adapters.end());
            test(adptObs1->adapters["DummyAdapter"].proxy == obj);
            test(adptObs1->adapters["DummyAdapter"].replicaGroupId == "DummyReplicaGroup");

            obj = communicator->stringToProxy("dummy:tcp -p 10000 -h localhost");
            locatorRegistry->setReplicatedAdapterDirectProxy("DummyAdapter1", "DummyReplicaGroup", obj);
            adptObs1->waitForUpdate(__FILE__, __LINE__);
            test(adptObs1->adapters.find("DummyAdapter1") != adptObs1->adapters.end());
            test(adptObs1->adapters["DummyAdapter1"].proxy == obj);
            test(adptObs1->adapters["DummyAdapter1"].replicaGroupId == "DummyReplicaGroup");

            obj = communicator->stringToProxy("dummy:tcp -p 10000 -h localhost");
            locatorRegistry->setReplicatedAdapterDirectProxy("DummyAdapter2", "DummyReplicaGroup", obj);
            adptObs1->waitForUpdate(__FILE__, __LINE__);
            test(adptObs1->adapters.find("DummyAdapter2") != adptObs1->adapters.end());
            test(adptObs1->adapters["DummyAdapter2"].proxy == obj);
            test(adptObs1->adapters["DummyAdapter2"].replicaGroupId == "DummyReplicaGroup");

            admin->removeAdapter("DummyAdapter2");
            adptObs1->waitForUpdate(__FILE__, __LINE__);
            test(adptObs1->adapters.find("DummyAdapter2") == adptObs1->adapters.end());

            admin->removeAdapter("DummyReplicaGroup");
            adptObs1->waitForUpdate(__FILE__, __LINE__);
            adptObs1->waitForUpdate(__FILE__, __LINE__);
            test(adptObs1->adapters["DummyAdapter"].replicaGroupId == "");
            test(adptObs1->adapters["DummyAdapter1"].replicaGroupId == "");

            locatorRegistry->setAdapterDirectProxy("DummyAdapter", 0);
            adptObs1->waitForUpdate(__FILE__, __LINE__);
            test(adptObs1->adapters.find("DummyAdapter") == adptObs1->adapters.end());
        }
        catch(const Ice::UserException& ex)
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

        AdminSessionPrx session1 = AdminSessionPrx::uncheckedCast(registry->createAdminSession("admin1", "test1"));
        AdminPrx admin1 = session1->getAdmin();

        session1->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::HeartbeatOnIdle);

        Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapter("");
        ObjectObserverIPtr objectObs1 = new ObjectObserverI("objectObs1");
        Ice::ObjectPrx object1 = adpt1->addWithUUID(objectObs1);
        adpt1->activate();
        registry->ice_getConnection()->setAdapter(adpt1);
        session1->setObserversByIdentity(Ice::Identity(),
                                         Ice::Identity(),
                                         Ice::Identity(),
                                         Ice::Identity(),
                                         object1->ice_getIdentity());

        objectObs1->waitForUpdate(__FILE__, __LINE__); // init

        try
        {
            Ice::ObjectPrx obj = communicator->stringToProxy("dummy:tcp -p 10000");

            admin->addObjectWithType(obj, "::Dummy");
            objectObs1->waitForUpdate(__FILE__, __LINE__);
            test(objectObs1->objects.find(Ice::stringToIdentity("dummy")) != objectObs1->objects.end());
            test(objectObs1->objects[Ice::stringToIdentity("dummy")].type == "::Dummy");
            test(objectObs1->objects[Ice::stringToIdentity("dummy")].proxy == obj);

            obj = communicator->stringToProxy("dummy:tcp -p 10000 -h localhost");
            admin->updateObject(obj);
            objectObs1->waitForUpdate(__FILE__, __LINE__);
            test(objectObs1->objects.find(Ice::stringToIdentity("dummy")) != objectObs1->objects.end());
            test(objectObs1->objects[Ice::stringToIdentity("dummy")].type == "::Dummy");
            test(objectObs1->objects[Ice::stringToIdentity("dummy")].proxy == obj);

            admin->removeObject(obj->ice_getIdentity());
            objectObs1->waitForUpdate(__FILE__, __LINE__);
            test(objectObs1->objects.find(Ice::stringToIdentity("dummy")) == objectObs1->objects.end());
        }
        catch(const Ice::UserException& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        session1->destroy();
        adpt1->destroy();

        cout << "ok" << endl;
    }

    {
//      cout << "???" << endl;

//      //
//      // Setup a descriptor to deploy a node on the node.
//      //
//      ApplicationDescriptor nodeApp;
//      nodeApp.name = "NodeApp";
//      ServerDescriptorPtr server = new ServerDescriptor();
//      server->id = "node-1";
//      server->exe = properties->getProperty("IceGridNodeExe");
//      server->options.push_back("--nowarn");
//      server->pwd = ".";
//      addProperty(server, "IceGrid.Node.Name", "node-1");
//      addProperty(server, "IceGrid.Node.Data", properties->getProperty("TestDir") + "/db/node-1");
//      addProperty(server, "IceGrid.Node.Endpoints", "default");
//      NodeDescriptor node;
//      node.servers.push_back(server);
//      nodeApp.nodes["localnode"] = node;

//      try
//      {
//          int s = session1->startUpdate();
//          test(s == serial);
//          admin1->addApplication(nodeApp);
//          appObs1->waitForUpdate(__FILE__, __LINE__); // application added
//          test(appObs1->applications.find("NodeApp") != appObs1->applications.end());
//          test(++serial == appObs1->serial);
//      }
//      catch(const DeploymentException& ex)
//      {
//          cerr << ex.reason << endl;
//          test(false);
//      }
//      catch(const Ice::UserException& ex)
//      {
//          cerr << ex << endl;
//          test(false);
//      }

//      try
//      {
//          admin->startServer("node-1");
//      }
//      catch(const NodeUnreachableException& ex)
//      {
//          cerr << ex << ":\n";
//          cerr << "node = " << ex.name << endl;
//          cerr << "reason = " << ex.reason << endl;
//      }
//      appObs1->waitForUpdate(__FILE__, __LINE__); // object added (for node well-known proxy)
//      test(++serial == appObs1->serial);

//      nodeObs1->waitForUpdate(__FILE__, __LINE__); // updateServer
//      nodeObs1->waitForUpdate(__FILE__, __LINE__); // updateServer
//      do
//      {
//          nodeObs1->waitForUpdate(__FILE__, __LINE__); // nodeUp
//      }
//      while(nodeObs1->nodes.find("node-1") == nodeObs1->nodes.end());

//      try
//      {
//          admin->stopServer("node-1");
//      }
//      catch(const NodeUnreachableException& ex)
//      {
//          cerr << ex << ":\n";
//          cerr << "node = " << ex.name << endl;
//          cerr << "reason = " << ex.reason << endl;
//      }
//      appObs1->waitForUpdate(__FILE__, __LINE__); // object removed (for node well-known proxy)
//      test(++serial == appObs1->serial);

//      nodeObs1->waitForUpdate(__FILE__, __LINE__); // updateServer
//      nodeObs1->waitForUpdate(__FILE__, __LINE__); // updateServer
//      nodeObs1->waitForUpdate(__FILE__, __LINE__); // nodeDown
//      test(nodeObs1->nodes.find("node-1") == nodeObs1->nodes.end());

//      try
//      {
//          admin1->removeApplication("NodeApp");
//          appObs1->waitForUpdate(__FILE__, __LINE__); // application removed
//          test(appObs1->applications.empty());
//          test(++serial == appObs1->serial);
//      }
//      catch(const DeploymentException& ex)
//      {
//          cerr << ex.reason << endl;
//          test(false);
//      }
//      catch(const Ice::UserException& ex)
//      {
//          cerr << ex << endl;
//          test(false);
//      }

//      nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Destroying)
//      nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Destroyed)

//      session1->destroy();
//      adpt1->destroy();

//      cout << "ok" << endl;
    }

    {
        cout << "testing node observer... " << flush;
        AdminSessionPrx session1 = registry->createAdminSession("admin1", "test1");

        session1->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::HeartbeatOnIdle);

        Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapter("");
        ApplicationObserverIPtr appObs1 = new ApplicationObserverI("appObs1.3");
        Ice::ObjectPrx app1 = adpt1->addWithUUID(appObs1);
        NodeObserverIPtr nodeObs1 = new NodeObserverI("nodeObs1");
        Ice::ObjectPrx no1 = adpt1->addWithUUID(nodeObs1);
        adpt1->activate();
        registry->ice_getConnection()->setAdapter(adpt1);
        session1->setObserversByIdentity(Ice::Identity(),
                                         no1->ice_getIdentity(),
                                         app1->ice_getIdentity(),
                                         Ice::Identity(),
                                         Ice::Identity());

        appObs1->waitForUpdate(__FILE__, __LINE__);
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // init

        test(nodeObs1->nodes.find("localnode") != nodeObs1->nodes.end());
        test(appObs1->applications.empty());

        ApplicationDescriptor nodeApp;
        nodeApp.name = "NodeApp";
        ServerDescriptorPtr server = new ServerDescriptor();
        server->id = "node-1";
        server->exe = properties->getProperty("IceGridNodeExe");
        server->options.push_back("--nowarn");
        server->pwd = ".";
        server->allocatable = false;
        addProperty(server, "IceGrid.Node.Name", "node-1");
        addProperty(server, "IceGrid.Node.Data", properties->getProperty("TestDir") + "/db/node-1");
        addProperty(server, "IceGrid.Node.Endpoints", "default");
        addProperty(server, "Ice.Admin.Endpoints", "tcp -h 127.0.0.1");

        NodeDescriptor node;
        node.servers.push_back(server);
        nodeApp.nodes["localnode"] = node;

        session->startUpdate();
        admin->addApplication(nodeApp);
        session->finishUpdate();
        appObs1->waitForUpdate(__FILE__, __LINE__);

        admin->startServer("node-1");

        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
        do
        {
            nodeObs1->waitForUpdate(__FILE__, __LINE__); // nodeUp
        }
        while(nodeObs1->nodes.find("node-1") == nodeObs1->nodes.end());

        test(nodeObs1->nodes["localnode"].servers.size() == 1);
        test(nodeObs1->nodes["localnode"].servers[0].state == Active);
        admin->stopServer("node-1");

        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Deactivating)
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Inactive)
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // nodeDown
        test(nodeObs1->nodes["localnode"].servers[0].state == Inactive);

        session->startUpdate();
        admin->removeApplication("NodeApp");
        session->finishUpdate();
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Destroying)
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Destroyed)

        appObs1->waitForUpdate(__FILE__, __LINE__);
        test(nodeObs1->nodes.find("node-1") == nodeObs1->nodes.end());

        ApplicationDescriptor testApp;
        testApp.name = "TestApp";

        server = new ServerDescriptor();
        server->id = "Server";
        server->exe = properties->getProperty("ServerDir") + "/server";
        server->pwd = properties->getProperty("TestDir");

        server->allocatable = false;
        AdapterDescriptor adapter;
        adapter.name = "Server";
        adapter.id = "ServerAdapter";
        adapter.registerProcess = false;
        adapter.serverLifetime = true;
        server->adapters.push_back(adapter);
        addProperty(server, "Server.Endpoints", "default");
        addProperty(server, "Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        node = NodeDescriptor();
        node.servers.push_back(server);
        testApp.nodes["localnode"] = node;

        session->startUpdate();
        admin->addApplication(testApp);
        session->finishUpdate();
        appObs1->waitForUpdate(__FILE__, __LINE__);

        session->startUpdate();
        admin->startServer("Server");
        session->finishUpdate();

        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // adapterUpdate

        test(nodeObs1->nodes.find("localnode") != nodeObs1->nodes.end());
        test(nodeObs1->nodes["localnode"].servers.size() == 1);
        test(nodeObs1->nodes["localnode"].servers[0].state == Active);
        test(nodeObs1->nodes["localnode"].adapters.size() == 1);
        test(nodeObs1->nodes["localnode"].adapters[0].proxy);

        test(nodeObs1->nodes["localnode"].servers[0].enabled);
        admin->enableServer("Server", false);
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
        test(!nodeObs1->nodes["localnode"].servers[0].enabled);
        admin->enableServer("Server", true);
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
        test(nodeObs1->nodes["localnode"].servers[0].enabled);

        admin->stopServer("Server");

        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // adapterUpdate

        test(nodeObs1->nodes.find("localnode") != nodeObs1->nodes.end());
        test(nodeObs1->nodes["localnode"].servers.size() == 1);
        test(nodeObs1->nodes["localnode"].servers[0].state == Inactive);
        test(nodeObs1->nodes["localnode"].adapters.empty());

        session->startUpdate();
        admin->removeApplication("TestApp");
        session->finishUpdate();

        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Destroying)
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Destroyed)
        test(nodeObs1->nodes["localnode"].servers.empty());

        appObs1->waitForUpdate(__FILE__, __LINE__);

        cout << "ok" << endl;
    }

    {
        cout << "testing registry observer... " << flush;
        AdminSessionPrx session1 = registry->createAdminSession("admin1", "test1");

        session1->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::HeartbeatOnIdle);

        Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapter("");
        ApplicationObserverIPtr appObs1 = new ApplicationObserverI("appObs1.4");
        Ice::ObjectPrx app1 = adpt1->addWithUUID(appObs1);
        RegistryObserverIPtr registryObs1 = new RegistryObserverI("registryObs1");
        Ice::ObjectPrx ro1 = adpt1->addWithUUID(registryObs1);
        adpt1->activate();
        registry->ice_getConnection()->setAdapter(adpt1);
        session1->setObserversByIdentity(ro1->ice_getIdentity(),
                                         Ice::Identity(),
                                         app1->ice_getIdentity(),
                                         Ice::Identity(),
                                         Ice::Identity());

        appObs1->waitForUpdate(__FILE__, __LINE__);
        registryObs1->waitForUpdate(__FILE__, __LINE__); // init

        test(registryObs1->registries.find("Master") != registryObs1->registries.end());
        test(appObs1->applications.empty());

        QueryPrx query = QueryPrx::uncheckedCast(communicator->stringToProxy("TestIceGrid/Query"));
        Ice::ObjectProxySeq registries = query->findAllObjectsByType("::IceGrid::Registry");
        const string prefix("Registry-");
        for(Ice::ObjectProxySeq::const_iterator p = registries.begin(); p != registries.end(); ++p)
        {
            string name = (*p)->ice_getIdentity().name;
            string::size_type pos = name.find(prefix);
            if(pos != string::npos)
            {
                name = name.substr(prefix.size());
                test(registryObs1->registries.find(name) != registryObs1->registries.end());
            }
        }
        cout << "ok" << endl;
    }

    {
        cout << "testing observer with direct proxy... " << flush;
        AdminSessionPrx session1 = registry->createAdminSession("admin1", "test1");

        session1->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::HeartbeatOnIdle);

        Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapterWithEndpoints("", "default");
        NodeObserverIPtr nodeObs1 = new NodeObserverI("nodeObs1");
        Ice::ObjectPrx no1 = adpt1->addWithUUID(nodeObs1);
        adpt1->activate();

        session1->setObservers(0, NodeObserverPrx::uncheckedCast(no1), 0, 0, 0);
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // init

        session1->destroy();
        adpt1->destroy();

        cout << "ok" << endl;
    }

    {
        cout << "testing observer with indirect proxy... " << flush;
        AdminSessionPrx session1 = registry->createAdminSession("admin1", "test1");
        communicator->getProperties()->setProperty("IndirectAdpt1.Endpoints", "default");
        communicator->getProperties()->setProperty("IndirectAdpt1.AdapterId", "adapter1");
        Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapter("IndirectAdpt1");
        test(communicator->getDefaultLocator());
        NodeObserverIPtr nodeObs1 = new NodeObserverI("nodeObs1");
        Ice::ObjectPrx no1 = adpt1->addWithUUID(nodeObs1);
        assert(no1->ice_getAdapterId() == "adapter1");
        adpt1->activate();

        session1->setObservers(0, NodeObserverPrx::uncheckedCast(no1), 0, 0, 0);
        nodeObs1->waitForUpdate(__FILE__, __LINE__); // init

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
