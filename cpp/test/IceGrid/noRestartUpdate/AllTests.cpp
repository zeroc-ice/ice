// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;
using namespace IceGrid;

void
addProperty(const CommunicatorDescriptorPtr& communicator, const string& name, const string& value)
{
    PropertyDescriptor prop;
    prop.name = name;
    prop.value = value;
    communicator->propertySet.properties.push_back(prop);
}

string
getProperty(const CommunicatorDescriptorPtr& communicator, const string& name)
{
    PropertyDescriptorSeq& properties = communicator->propertySet.properties;
    for(PropertyDescriptorSeq::const_iterator q = properties.begin(); q != properties.end(); ++q)
    {
        if(q->name == name)
        {
            return q->value;
        }
    }
    return "";
}

void
removeProperty(const CommunicatorDescriptorPtr& communicator, const string& name)
{
    PropertyDescriptorSeq& properties = communicator->propertySet.properties;
    for(PropertyDescriptorSeq::iterator q = properties.begin(); q != properties.end(); ++q)
    {
        if(q->name == name)
        {
            properties.erase(q);
            break;
        }
    }
}

PropertyDescriptor
createProperty(const string& name, const string& value)
{
    PropertyDescriptor prop;
    prop.name = name;
    prop.value = value;
    return prop;
}

bool
hasProperty(const CommunicatorDescriptorPtr& desc, const string& name, const string& value)
{
    for(PropertyDescriptorSeq::const_iterator p = desc->propertySet.properties.begin();
        p != desc->propertySet.properties.end(); ++p)
    {
        if(p->name == name)
        {
            return p->value == value;
        }
    }
    return false;
}

void
updateServerRuntimeProperties(const AdminPrx& admin, const string&, const ServerDescriptorPtr& desc)
{
    ApplicationUpdateDescriptor update;
    update.name = "TestApp";
    NodeUpdateDescriptor node;
    node.name = "localnode";
    node.servers.push_back(desc);
    update.nodes.push_back(node);
    try
    {
        admin->updateApplicationWithoutRestart(update);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }
}

ServiceDescriptorPtr
getServiceDescriptor(const AdminPrx& admin, const string& service)
{
    ServerInfo info = admin->getServerInfo("IceBox");
    test(info.descriptor);
    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(info.descriptor);
    for(ServiceInstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
    {
        if(p->descriptor->name == service)
        {
            return p->descriptor;
        }
    }
    return 0;
}

void
updateServiceRuntimeProperties(const AdminPrx& admin, const ServiceDescriptorPtr& desc)
{
    ServerInfo info = admin->getServerInfo("IceBox");
    test(info.descriptor);
    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(info.descriptor);
    for(ServiceInstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
    {
        if(p->descriptor->name == desc->name)
        {
            p->descriptor->propertySet.properties = desc->propertySet.properties;
        }
    }
    ApplicationUpdateDescriptor update;
    update.name = "TestApp";
    NodeUpdateDescriptor node;
    node.name = "localnode";
    node.servers.push_back(info.descriptor);
    update.nodes.push_back(node);
    try
    {
        admin->updateApplicationWithoutRestart(update);
    }
    catch(const DeploymentException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }
}

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    IceGrid::RegistryPrx registry = IceGrid::RegistryPrx::checkedCast(
        communicator->stringToProxy(communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));
    test(registry);
    AdminSessionPrx session = registry->createAdminSession("foo", "bar");

    session->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::ICE_ENUM(ACMHeartbeat, HeartbeatAlways));

    AdminPrx admin = session->getAdmin();
    test(admin);

    Ice::PropertiesPtr properties = communicator->getProperties();

    {
        ApplicationDescriptor testApp;
        testApp.name = "TestApp";
        admin->addApplication(testApp);

        ApplicationUpdateDescriptor empty;
        empty.name = "TestApp";
        NodeUpdateDescriptor node;
        node.name = "localnode";
        empty.nodes.push_back(node);

        ApplicationUpdateDescriptor update = empty;

        cout << "testing server add... " << flush;

        ServerDescriptorPtr server = new ServerDescriptor();
        server->id = "Server";
        server->exe = properties->getProperty("ServerDir") + "/server";
        server->pwd = ".";
        server->applicationDistrib = false;
        server->allocatable = false;
        server->activation = "on-demand";
        addProperty(server, "Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        AdapterDescriptor adapter;
        adapter.name = "Server";
        adapter.id = "ServerAdapter";
        adapter.registerProcess = false;
        adapter.serverLifetime = true;
        addProperty(server, "Server.Endpoints", "default");
        ObjectDescriptor object;
        object.id = Ice::stringToIdentity("${server}");
        object.type = "::Test::TestIntf";
        adapter.objects.push_back(object);
        server->adapters.push_back(adapter);
        update.nodes[0].servers.push_back(server);
        admin->updateApplicationWithoutRestart(update);
        admin->startServer("Server");
        int serverPid = admin->getServerPid("Server");

        update.nodes[0].servers[0]->id = "Server2";
        update.nodes[0].servers[0]->adapters[0].id = "ServerAdapter2";
        update.nodes[0].servers[0]->adapters[0].objects[0].id = Ice::stringToIdentity("test2");
        try
        {
            admin->updateApplicationWithoutRestart(update);
        }
        catch(const DeploymentException& ex)
        {
            cerr << ex.reason << endl;
            test(false);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        admin->startServer("Server2");
        int server2Pid = admin->getServerPid("Server2");

        TemplateDescriptor templ;
        templ.parameters.push_back("name");
        templ.descriptor = new ServerDescriptor();
        server = ServerDescriptorPtr::dynamicCast(templ.descriptor);
        server->id = "${name}";
        server->exe = "${server.dir}/server";
        server->pwd = ".";
        server->applicationDistrib = false;
        server->allocatable = false;
        server->activation = "on-demand";
        addProperty(server, "Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        adapter = AdapterDescriptor();
        adapter.name = "Server";
        adapter.id = "${server}";
        adapter.registerProcess = false;
        adapter.serverLifetime = true;
        addProperty(server, "Server.Endpoints", "default");
        object = ObjectDescriptor();
        object.id = Ice::stringToIdentity("${server}");
        object.type = "::Test::TestIntf";
        adapter.objects.push_back(object);
        server->adapters.push_back(adapter);
        update = empty;
        update.serverTemplates["ServerTemplate"] = templ;
        try
        {
            admin->updateApplicationWithoutRestart(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        test(serverPid == admin->getServerPid("Server"));
        test(server2Pid == admin->getServerPid("Server2"));

        ServerInstanceDescriptor instance;
        update = empty;
        update.variables["server.dir"] = properties->getProperty("ServerDir");
        update.variables["variable"] = "";
        instance = ServerInstanceDescriptor();
        instance._cpp_template = "ServerTemplate";
        instance.parameterValues["name"] = "Server1";
        update.nodes[0].serverInstances.push_back(instance);
        try
        {
            admin->updateApplicationWithoutRestart(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        admin->startServer("Server1");
        int server1Pid = admin->getServerPid("Server1");
        test(serverPid == admin->getServerPid("Server"));
        test(server2Pid == admin->getServerPid("Server2"));

        cout << "ok" << endl;

        cout << "testing server remove... " << flush;
        update = empty;
        update.nodes[0].removeServers.push_back("Server2");
        try
        {
            admin->updateApplicationWithoutRestart(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        test(server2Pid == admin->getServerPid("Server2"));

        update = empty;
        update.nodes[0].removeServers.push_back("Server1");
        try
        {
            admin->updateApplicationWithoutRestart(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        test(server1Pid == admin->getServerPid("Server1"));

        admin->stopServer("Server2");
        update.nodes[0].removeServers.clear();
        update.nodes[0].removeServers.push_back("Server2");
        try
        {
            admin->updateApplicationWithoutRestart(update);
        }
        catch(const DeploymentException& ex)
        {
            cerr << ex.reason << endl;
            test(false);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        try
        {
            admin->getServerPid("Server2");
            test(false);
        }
        catch(const ServerNotExistException&)
        {
        }

        cout << "ok" << endl;

        cout << "testing server update... " << flush;

        ServerInfo info = admin->getServerInfo("Server");
        test(info.descriptor);

        addProperty(info.descriptor, "test", "test");
        test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server"))->getProperty("test") == "");
        updateServerRuntimeProperties(admin, "Server", info.descriptor);
        test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server"))->getProperty("test") == "test");
        test(serverPid == admin->getServerPid("Server"));
        admin->stopServer("Server");
        test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server"))->getProperty("test") == "test");
        test((serverPid = admin->getServerPid("Server")) > 0);
        test(hasProperty(admin->getServerInfo("Server").descriptor, "test", "test"));

        addProperty(info.descriptor, "test2", "test2");
        test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server"))->getProperty("test2") == "");
        updateServerRuntimeProperties(admin, "Server", info.descriptor);
        test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server"))->getProperty("test2") == "test2");
        test(serverPid == admin->getServerPid("Server"));
        test(hasProperty(admin->getServerInfo("Server").descriptor, "test2", "test2"));

        removeProperty(info.descriptor, "test2");
        updateServerRuntimeProperties(admin, "Server", info.descriptor);
        test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server"))->getProperty("test2") == "");
        test(serverPid == admin->getServerPid("Server"));
        test(!hasProperty(admin->getServerInfo("Server").descriptor, "test2", "test2"));

        addProperty(info.descriptor, "test3", "test3");
        test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server"))->getProperty("test3") == "");
        updateServerRuntimeProperties(admin, "Server", info.descriptor);
        test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server"))->getProperty("test3") == "test3");
        test(serverPid == admin->getServerPid("Server"));
        test(hasProperty(admin->getServerInfo("Server").descriptor, "test3", "test3"));

        admin->stopServer("Server");
        test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server"))->getProperty("test") == "test");
        test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server"))->getProperty("test2") == "");
        test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server"))->getProperty("test3") == "test3");
        test((serverPid = admin->getServerPid("Server")) > 0);
        test(hasProperty(admin->getServerInfo("Server").descriptor, "test", "test"));
        test(!hasProperty(admin->getServerInfo("Server").descriptor, "test2", ""));
        test(hasProperty(admin->getServerInfo("Server").descriptor, "test3", "test3"));

        try
        {
            update = empty;
            update.serverTemplates["ServerTemplate"] = templ;
            instance = ServerInstanceDescriptor();
            instance._cpp_template = "ServerTemplate";
            instance.parameterValues["name"] = "Server1";
            update.nodes[0].serverInstances.push_back(instance);
            admin->updateApplicationWithoutRestart(update);
            test(server1Pid == admin->getServerPid("Server1"));
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
            update = empty;
            addProperty(server, "test", "test");
            assert(templ.descriptor == server);
            update.serverTemplates["ServerTemplate"] = templ;
            test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server1"))->getProperty("test") == "");
            admin->updateApplicationWithoutRestart(update);
            test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server1"))->getProperty("test") == "test");
            test(server1Pid == admin->getServerPid("Server1"));
            admin->stopServer("Server1");
            test(TestIntfPrx::uncheckedCast(communicator->stringToProxy("Server1"))->getProperty("test") == "test");
            server1Pid = admin->getServerPid("Server1");
            test(hasProperty(admin->getServerInfo("Server1").descriptor, "test", "test"));
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
            info = admin->getServerInfo("Server");
            test(info.descriptor);
            adapter = AdapterDescriptor();
            adapter.id = "ServerFoo";
            adapter.serverLifetime = false;
            adapter.registerProcess = false;
            info.descriptor->adapters.push_back(adapter);
            update = empty;
            update.nodes[0].servers.push_back(info.descriptor);
            admin->updateApplicationWithoutRestart(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Can't update server adapters without restart
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
            info = admin->getServerInfo("Server");
            test(info.descriptor);
            test(info.descriptor->adapters.size() == 1);
            object = ObjectDescriptor();
            object.id = Ice::stringToIdentity("testfoo");
            info.descriptor->adapters[0].objects.push_back(object);
            update = empty;
            update.nodes[0].servers.push_back(info.descriptor);
            admin->updateApplicationWithoutRestart(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Can't update server objects without restart
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
            info = admin->getServerInfo("Server");
            test(info.descriptor);
            object = ObjectDescriptor();
            object.id = Ice::stringToIdentity("test");
            info.descriptor->adapters[0].allocatables.push_back(object);
            update = empty;
            update.nodes[0].servers.push_back(info.descriptor);
            admin->updateApplicationWithoutRestart(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Can't update server allocatables without restart
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        test(serverPid == admin->getServerPid("Server"));

        try
        {
            info = admin->getServerInfo("Server");
            test(info.descriptor);
            DbEnvDescriptor dbEnv = DbEnvDescriptor();
            dbEnv.name = "foo";
            info.descriptor->dbEnvs.push_back(dbEnv);
            update = empty;
            update.nodes[0].servers.push_back(info.descriptor);
            admin->updateApplicationWithoutRestart(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Can't update server dbenvs without restart
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        test(serverPid == admin->getServerPid("Server"));

        cout << "ok" << endl;

        cout << "testing icebox server add... " << flush;

        ServiceDescriptorPtr service = new ServiceDescriptor();
        addProperty(service, "Ice.Warn.UnknownProperties", "0");
        //addProperty(service, "Ice.Trace.Admin.Properties", "1");
        service->name = "Service1";

        service->entry = properties->getProperty("ServiceDir") + "/TestService:create";

        adapter = AdapterDescriptor();
        adapter.name = "${service}";
        adapter.id = "${server}.${service}";
        adapter.registerProcess = false;
        adapter.serverLifetime = false;
        addProperty(service, "${service}.Endpoints", "default");
        object.id = Ice::stringToIdentity("${server}.${service}");
        addProperty(service, "${service}.Identity", "${server}.${service}");
        adapter.objects.push_back(object);
        service->adapters.push_back(adapter);

        IceBoxDescriptorPtr icebox = new IceBoxDescriptor();
        icebox->id = "IceBox";
        icebox->exe = properties->getProperty("IceBoxExe");
        icebox->activation = "on-demand";
        icebox->applicationDistrib = false;
        icebox->allocatable = false;
        addProperty(icebox, "Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        icebox->services.resize(3);
        icebox->services[0].descriptor = ServiceDescriptorPtr::dynamicCast(service->ice_clone());
        service->name = "Service2";
        icebox->services[1].descriptor = ServiceDescriptorPtr::dynamicCast(service->ice_clone());
        service->name = "Service3";
        // Test also with shared communicator because it uses different proxy name
        // and thus different branches in code.
        addProperty(icebox, "IceBox.UseSharedCommunicator.Service3", "1");
        icebox->services[2].descriptor = ServiceDescriptorPtr::dynamicCast(service->ice_clone());

        try
        {
            update = empty;
            update.nodes[0].servers.push_back(icebox);
            admin->updateApplicationWithoutRestart(update);
            test(serverPid == admin->getServerPid("Server"));
            test(server1Pid == admin->getServerPid("Server1"));
            admin->startServer("IceBox");
        }
        catch(const DeploymentException& ex)
        {
            cerr << ex.reason << endl;
            test(false);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        int iceBoxPid = admin->getServerPid("IceBox");

        cout << "ok" << endl;

        cout << "testing service add... " << flush;
        try
        {
            service->name = "First";
            icebox->services.resize(4);
            icebox->services[3].descriptor = service;
            admin->updateApplicationWithoutRestart(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // can't add service without restart
            test(iceBoxPid == admin->getServerPid("IceBox"));
            icebox = IceBoxDescriptorPtr::dynamicCast(admin->getServerInfo("IceBox").descriptor);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        cout << "ok" << endl;

        cout << "testing service remove... " << flush;
        try
        {
            icebox->services.resize(2);
            admin->updateApplicationWithoutRestart(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // can't remove service without restart
            test(iceBoxPid == admin->getServerPid("IceBox"));
            icebox = IceBoxDescriptorPtr::dynamicCast(admin->getServerInfo("IceBox").descriptor);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        cout << "ok" << endl;

        cout << "testing service update... " << flush;
        try
        {
            icebox->services[0].descriptor->entry = properties->getProperty("ServiceDir") + "/TestService:create2";
            admin->updateApplicationWithoutRestart(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // can't update service entry point without restart
            test(iceBoxPid == admin->getServerPid("IceBox"));
            icebox = IceBoxDescriptorPtr::dynamicCast(admin->getServerInfo("IceBox").descriptor);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        ServiceDescriptorPtr svc1 = icebox->services[0].descriptor;
        TestIntfPrx svc1Prx = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox.Service1"));

        ServiceDescriptorPtr svc2 = icebox->services[1].descriptor;
        TestIntfPrx svc2Prx = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox.Service2"));

        ServiceDescriptorPtr svc3 = icebox->services[2].descriptor;
        TestIntfPrx svc3Prx = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox.Service3"));

        addProperty(svc1, "test", "test");
        test(svc1Prx->getProperty("test") == "");
        updateServerRuntimeProperties(admin, "IceBox", icebox);
        test(svc1Prx->getProperty("test") == "test");
        test(iceBoxPid == admin->getServerPid("IceBox"));
        admin->stopServer("IceBox");
        admin->startServer("IceBox");
        test((iceBoxPid = admin->getServerPid("IceBox")) > 0);
        test(svc1Prx->getProperty("test") == "test");
        test(hasProperty(getServiceDescriptor(admin, "Service1"), "test", "test"));

        addProperty(svc1, "test2", "test2");
        test(svc1Prx->getProperty("test2") == "");
        updateServerRuntimeProperties(admin, "IceBox", icebox);
        test(svc1Prx->getProperty("test2") == "test2");
        test(iceBoxPid == admin->getServerPid("IceBox"));
        test(hasProperty(getServiceDescriptor(admin, "Service1"), "test2", "test2"));

        removeProperty(svc1, "test2");
        updateServerRuntimeProperties(admin, "IceBox", icebox);
        test(svc1Prx->getProperty("test2") == "");
        test(iceBoxPid == admin->getServerPid("IceBox"));
        test(!hasProperty(getServiceDescriptor(admin, "Service1"), "test2", "test2"));

        addProperty(svc1, "test3", "test3");
        test(svc1Prx->getProperty("test3") == "");
        updateServerRuntimeProperties(admin, "IceBox", icebox);
        test(svc1Prx->getProperty("test3") == "test3");
        test(iceBoxPid == admin->getServerPid("IceBox"));
        test(hasProperty(getServiceDescriptor(admin, "Service1"), "test3", "test3"));

        admin->stopServer("IceBox");
        test(svc1Prx->getProperty("test") == "test");
        test(svc1Prx->getProperty("test2") == "");
        test(svc1Prx->getProperty("test3") == "test3");
        test((iceBoxPid = admin->getServerPid("IceBox")) > 0);
        // Wait for the server to be active to have the guarantee that
        // the property update will return once the properties are
        // updated.
        while(admin->getServerState("IceBox") != IceGrid::Active)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));
        }
        test(hasProperty(getServiceDescriptor(admin, "Service1"), "test", "test"));
        test(!hasProperty(getServiceDescriptor(admin, "Service1"), "test2", ""));
        test(hasProperty(getServiceDescriptor(admin, "Service1"), "test3", "test3"));

        addProperty(svc2, "test22", "test22");
        addProperty(svc3, "test32", "test32");
        test(svc2Prx->getProperty("test22") == "");
        test(svc3Prx->getProperty("test32") == "");
        updateServerRuntimeProperties(admin, "IceBox", icebox);
        test(svc2Prx->getProperty("test22") == "test22");
        test(svc3Prx->getProperty("test32") == "test32");
        test(iceBoxPid == admin->getServerPid("IceBox"));
        test(hasProperty(getServiceDescriptor(admin, "Service2"), "test22", "test22"));
        test(hasProperty(getServiceDescriptor(admin, "Service3"), "test32", "test32"));

        removeProperty(svc2, "test22");
        addProperty(svc3, "test33", "test33");
        updateServerRuntimeProperties(admin, "IceBox", icebox);
        test(svc2Prx->getProperty("test22") == "");
        test(svc3Prx->getProperty("test33") == "test33");
        test(iceBoxPid == admin->getServerPid("IceBox"));
        test(!hasProperty(getServiceDescriptor(admin, "Service2"), "test22", "test22"));
        test(hasProperty(getServiceDescriptor(admin, "Service3"), "test33", "test33"));

        addProperty(svc2, "test24", "test24");
        removeProperty(svc3, "test33");
        addProperty(svc3, "test34", "test34");
        updateServerRuntimeProperties(admin, "IceBox", icebox);
        test(svc2Prx->getProperty("test24") == "test24");
        test(svc3Prx->getProperty("test33") == "");
        test(svc3Prx->getProperty("test34") == "test34");
        test(iceBoxPid == admin->getServerPid("IceBox"));
        test(hasProperty(getServiceDescriptor(admin, "Service2"), "test24", "test24"));
        test(!hasProperty(getServiceDescriptor(admin, "Service3"), "test33", "test33"));
        test(hasProperty(getServiceDescriptor(admin, "Service3"), "test34", "test34"));

        cout << "ok" << endl;

        admin->removeApplication("TestApp");
    }

    //
    // TODO: Add more tests.
    //

    session->destroy();
}
