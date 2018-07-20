// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <TestHelper.h>
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
getProperty(const PropertyDescriptorSeq& properties, const string& name)
{
    for(PropertyDescriptorSeq::const_iterator q = properties.begin(); q != properties.end(); ++q)
    {
        if(q->name == name)
        {
            return q->value;
        }
    }
    return "";
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
allTests(Test::TestHelper* helper)
{
    const Ice::CommunicatorPtr& communicator = helper->communicator();
    IceGrid::RegistryPrx registry = IceGrid::RegistryPrx::checkedCast(
        communicator->stringToProxy(communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));
    test(registry);
    AdminSessionPrx session = registry->createAdminSession("foo", "bar");

    session->ice_getConnection()->setACM(registry->getACMTimeout(),
                                         IceUtil::None,
                                         Ice::ICE_ENUM(ACMHeartbeat, HeartbeatAlways));

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
        addProperty(server, "Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        AdapterDescriptor adapter;
        adapter.name = "Server";
        adapter.id = "ServerAdapter";
        adapter.registerProcess = false;
        adapter.serverLifetime = false;
        addProperty(server, "Server.Endpoints", "default");
        ObjectDescriptor object;
        object.id = Ice::stringToIdentity("test");
        object.type = "::Test::TestIntf";
        adapter.objects.push_back(object);
        server->adapters.push_back(adapter);
        update.nodes[0].servers.push_back(server);
        admin->updateApplication(update);

        update.nodes[0].servers[0]->id = "Server2";
        try
        {
            admin->updateApplication(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Adapter already exists
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        update.nodes[0].servers[0]->adapters[0].id = "ServerAdapter2";
        try
        {
            admin->updateApplication(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Object already exists
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        update.nodes[0].servers[0]->adapters[0].objects[0].id = Ice::stringToIdentity("test2");
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        TemplateDescriptor templ;
        templ.parameters.push_back("name");
        templ.descriptor = new ServerDescriptor();
        server = ServerDescriptorPtr::dynamicCast(templ.descriptor);
        server->id = "${name}";
        server->exe = "${test.dir}/server";
        server->pwd = ".";
        server->applicationDistrib = false;
        server->allocatable = false;
        addProperty(server, "Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        adapter = AdapterDescriptor();
        adapter.name = "Server";
        adapter.id = "${server}";
        adapter.registerProcess = false;
        adapter.serverLifetime = false;
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
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        update = empty;
        ServerInstanceDescriptor instance;
        instance._cpp_template = "ServerTemplate";
        update.nodes[0].serverInstances.push_back(instance);
        try
        {
            admin->updateApplication(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Missing parameter
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        update = empty;
        update.variables["test.dir"] = properties->getProperty("ServerDir");
        update.variables["variable"] = "";
        instance = ServerInstanceDescriptor();
        instance._cpp_template = "ServerTemplate";
        instance.parameterValues["name"] = "Server1";
        update.nodes[0].serverInstances.push_back(instance);
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        cout << "ok" << endl;

        cout << "testing server remove... " << flush;
        update = empty;
        update.nodes[0].removeServers.push_back("Server2");
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
            admin->getServerInfo("Server2");
            test(false);
        }
        catch(const ServerNotExistException&)
        {
        }

        try
        {
            admin->updateApplication(update);
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

        update = empty;
        update.removeServerTemplates.push_back("ServerTemplate");
        try
        {
            admin->updateApplication(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Server without template!
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        update = empty;
        update.nodes[0].removeServers.push_back("Server1");
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        try
        {
            admin->getServerInfo("Server1");
            test(false);
        }
        catch(const ServerNotExistException&)
        {
        }

        update = empty;
        update.removeServerTemplates.push_back("ServerTemplate");
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        cout << "ok" << endl;

        cout << "testing server update... " << flush;

        ServerInfo info = admin->getServerInfo("Server");
        test(info.descriptor);
        addProperty(info.descriptor, "test", "test");
        update = empty;
        update.nodes[0].servers.push_back(info.descriptor);
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        info = admin->getServerInfo("Server");
        test(info.descriptor);
        test(getProperty(info.descriptor->propertySet.properties, "test") == "test");

        update = empty;
        update.serverTemplates["ServerTemplate"] = templ;
        instance = ServerInstanceDescriptor();
        instance._cpp_template = "ServerTemplate";
        instance.parameterValues["name"] = "Server1";
        update.nodes[0].serverInstances.push_back(instance);
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        update = empty;
        addProperty(server, "test", "test");
        assert(templ.descriptor == server);
        update.serverTemplates["ServerTemplate"] = templ;
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        info = admin->getServerInfo("Server1");
        test(info.descriptor);
        test(getProperty(info.descriptor->propertySet.properties, "test") == "test");

        info = admin->getServerInfo("Server");
        test(info.descriptor);
        adapter = AdapterDescriptor();
        adapter.id = "Server1";
        adapter.serverLifetime = false;
        adapter.registerProcess = false;
        info.descriptor->adapters.push_back(adapter);
        update = empty;
        update.nodes[0].servers.push_back(info.descriptor);
        try
        {
            admin->updateApplication(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Adapter already exists
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        info = admin->getServerInfo("Server");
        test(info.descriptor);
        adapter = AdapterDescriptor();
        adapter.id = "ServerX";
        adapter.serverLifetime = false;
        adapter.registerProcess = false;
        object = ObjectDescriptor();
        object.id = Ice::stringToIdentity("test");
        adapter.objects.push_back(object);
        info.descriptor->adapters.push_back(adapter);
        update = empty;
        update.nodes[0].servers.push_back(info.descriptor);
        try
        {
            admin->updateApplication(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Object already exists
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        info = admin->getServerInfo("Server");
        test(info.descriptor);
        object = ObjectDescriptor();
        object.id = Ice::stringToIdentity("test");
        info.descriptor->adapters[0].objects.push_back(object);
        update = empty;
        update.nodes[0].servers.push_back(info.descriptor);
        try
        {
            admin->updateApplication(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Object already exists
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        info = admin->getServerInfo("Server");
        test(info.descriptor);
        object = ObjectDescriptor();
        object.id = Ice::stringToIdentity("test1");
        info.descriptor->adapters[0].allocatables.push_back(object);
        update = empty;
        update.nodes[0].servers.push_back(info.descriptor);
        try
        {
            admin->updateApplication(update);
            test(true);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        info = admin->getServerInfo("Server");
        test(info.descriptor);
        object = ObjectDescriptor();
        object.id = Ice::stringToIdentity("test1");
        info.descriptor->adapters[0].allocatables.push_back(object);
        update = empty;
        update.nodes[0].servers.push_back(info.descriptor);
        try
        {
            admin->updateApplication(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Object already exists
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        info = admin->getServerInfo("Server");
        test(info.descriptor);
        object = ObjectDescriptor();
        object.id = Ice::stringToIdentity("test");
        info.descriptor->adapters[0].allocatables.push_back(object);
        update = empty;
        update.nodes[0].servers.push_back(info.descriptor);
        try
        {
            admin->updateApplication(update);
            test(true);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        info = admin->getServerInfo("Server");
        test(info.descriptor);
        object = ObjectDescriptor();
        object.id = Ice::stringToIdentity("test");
        info.descriptor->adapters[0].allocatables.push_back(object);
        update = empty;
        update.nodes[0].servers.push_back(info.descriptor);
        try
        {
            admin->updateApplication(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Object already exists
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        admin->removeApplication("TestApp");
        cout << "ok" << endl;
    }

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

        cout << "testing icebox server add... " << flush;

        ServiceDescriptorPtr service = new ServiceDescriptor();
        service->name = "Service1";
        service->entry = "TestService:create";
        AdapterDescriptor adapter;
        adapter.name = "${service}";
        adapter.id = "${server}.${service}";
        adapter.registerProcess = false;
        adapter.serverLifetime = false;
        addProperty(service, "${service}.Endpoints", "default");
        service->adapters.push_back(adapter);

        IceBoxDescriptorPtr server = new IceBoxDescriptor();
        server->id = "IceBox";
        server->exe = properties->getProperty("IceBoxExe");

        server->applicationDistrib = false;
        server->allocatable = false;
        addProperty(server, "Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        server->services.resize(3);
        server->services[0].descriptor = ServiceDescriptorPtr::dynamicCast(service->ice_clone());
        service->name = "Service2";
        server->services[1].descriptor = ServiceDescriptorPtr::dynamicCast(service->ice_clone());
        service->name = "Service3";
        server->services[2].descriptor = ServiceDescriptorPtr::dynamicCast(service->ice_clone());

        update.nodes[0].servers.push_back(server);
        try
        {
            admin->updateApplication(update);
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
        cout << "ok" << endl;

        cout << "testing service add... " << flush;
        service->name = "First";
        server->services.resize(4);
        server->services[3].descriptor = service;
        try
        {
            admin->updateApplication(update);
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
        cout << "ok" << endl;

        cout << "testing service remove... " << flush;
        server->services.resize(3);
        try
        {
            admin->updateApplication(update);
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
        cout << "ok" << endl;

        admin->removeApplication("TestApp");
    }

    {
        cout << "testing node add... " << flush;

        ApplicationDescriptor testApp;
        testApp.name = "TestApp";
        NodeDescriptor node;
        node.variables["nodename"] = "node1";
        testApp.nodes["node1"] = node;

        try
        {
            admin->addApplication(testApp);
        }
        catch(const DeploymentException& ex)
        {
            cerr << ex.reason << endl;
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        ApplicationUpdateDescriptor update;
        update.name = "TestApp";
        NodeUpdateDescriptor nodeUpdate;
        nodeUpdate.name = "node2";
        nodeUpdate.variables["nodename"] = "node2";
        update.nodes.push_back(nodeUpdate);

        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        testApp = admin->getApplicationInfo("TestApp").descriptor;
        test(testApp.nodes.size() == 2);
        test(testApp.nodes["node1"].variables["nodename"] == "node1");
        test(testApp.nodes["node2"].variables["nodename"] == "node2");
        cout << "ok" << endl;

        cout << "testing node update... " << flush;

        nodeUpdate.name = "node2";
        nodeUpdate.variables["nodename"] = "node2updated";
        update.nodes.back() = nodeUpdate;
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        testApp = admin->getApplicationInfo("TestApp").descriptor;
        test(testApp.nodes.size() == 2);
        test(testApp.nodes["node1"].variables["nodename"] == "node1");
        test(testApp.nodes["node2"].variables["nodename"] == "node2updated");

        cout << "ok" << endl;

        cout << "testing node remove... " << flush;

        update.nodes.clear();
        update.removeNodes.push_back("node1");
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        testApp = admin->getApplicationInfo("TestApp").descriptor;
        test(testApp.nodes.size() == 1);
        test(testApp.nodes["node2"].variables["nodename"] == "node2updated");

        admin->removeApplication("TestApp");

        cout << "ok" << endl;
    }

    {
        cout << "testing variable update... " << flush;

        ServerDescriptorPtr server = new ServerDescriptor();
        server->id = "${name}";
        server->exe = "server";
        server->pwd = ".";
        server->applicationDistrib = false;
        server->allocatable = false;

        addProperty(server, "ApplicationVar", "${appvar}");
        addProperty(server, "NodeVar", "${nodevar}");
        addProperty(server, "ServerParamVar", "${serverparamvar}");

        TemplateDescriptor templ;
        templ.parameters.push_back("name");
        templ.parameters.push_back("serverparamvar");
        templ.descriptor = server;

        ApplicationDescriptor testApp;
        testApp.name = "TestApp";
        testApp.variables["appvar"] = "AppValue";
        testApp.serverTemplates["ServerTemplate"] = templ;

        NodeDescriptor node;
        node.variables["nodevar"] = "NodeValue";

        ServerInstanceDescriptor serverInstance;
        serverInstance._cpp_template = "ServerTemplate";
        serverInstance.parameterValues["name"] = "Server";
        serverInstance.parameterValues["serverparamvar"] = "ServerParamValue";
        node.serverInstances.push_back(serverInstance);

        testApp.nodes["node1"] = node;

        try
        {
            admin->addApplication(testApp);
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

        ApplicationUpdateDescriptor empty;
        empty.name = "TestApp";
        ApplicationUpdateDescriptor update = empty;
        update.removeVariables.push_back("appvar");
        try
        {
            admin->updateApplication(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Missing app variable
            //cerr << ex.reason << endl;
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        update = empty;
        NodeUpdateDescriptor nodeUpdate;
        nodeUpdate.name = "node1";
        nodeUpdate.removeVariables.push_back("nodevar");
        update.nodes.push_back(nodeUpdate);
        try
        {
            admin->updateApplication(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Missing node variable
            //cerr << ex.reason << endl;
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        update = empty;
        serverInstance = ServerInstanceDescriptor();
        serverInstance._cpp_template = "ServerTemplate";
        serverInstance.parameterValues["name"] = "Server";
        nodeUpdate = NodeUpdateDescriptor();
        nodeUpdate.name = "node1";
        nodeUpdate.serverInstances.push_back(serverInstance);
        update.nodes.push_back(nodeUpdate);
        try
        {
            admin->updateApplication(update);
            test(false);
        }
        catch(const DeploymentException&)
        {
            // Missing parameter
            //cerr << ex.reason << endl;
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        ServerInfo serverBefore = admin->getServerInfo("Server");
        ApplicationDescriptor origApp = admin->getApplicationInfo("TestApp").descriptor;

        update = empty;
        update.variables["nodevar"] = "appoverride";
        nodeUpdate = NodeUpdateDescriptor();
        nodeUpdate.name = "node1";
        nodeUpdate.variables["serverparamvar"] = "nodeoverride";
        update.nodes.push_back(nodeUpdate);
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        ServerInfo serverAfter = admin->getServerInfo("Server");
        test(serverBefore.descriptor->propertySet == serverAfter.descriptor->propertySet);

        update = empty;
        nodeUpdate = NodeUpdateDescriptor();
        nodeUpdate.name = "node1";
        nodeUpdate.variables["appvar"] = "nodeoverride";
        update.nodes.push_back(nodeUpdate);
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        serverAfter = admin->getServerInfo("Server");
        PropertyDescriptorSeq newProps = serverAfter.descriptor->propertySet.properties;
        test(getProperty(serverAfter.descriptor->propertySet.properties, "ApplicationVar") == "nodeoverride");
        test(getProperty(serverAfter.descriptor->propertySet.properties, "NodeVar") == "NodeValue");
        test(getProperty(serverAfter.descriptor->propertySet.properties, "ServerParamVar") == "ServerParamValue");
        admin->removeApplication("TestApp");
        cout << "ok" << endl;
    }

    {
        cout << "testing property set update... " << flush;

        ServiceDescriptorPtr service = new ServiceDescriptor();
        service->name = "${name}";
        service->entry = "dummy";
        addProperty(service, "ServiceProp", "test");

        TemplateDescriptor svcTempl;
        svcTempl.parameters.push_back("name");
        svcTempl.descriptor = service;

        ServiceInstanceDescriptor serviceInstance;
        serviceInstance._cpp_template = "ServiceTemplate";
        serviceInstance.parameterValues["name"] =  "Service";
        serviceInstance.propertySet.properties.push_back(createProperty("ServiceInstanceProp", "test"));

        IceBoxDescriptorPtr server = new IceBoxDescriptor();
        server->id = "${name}";
        server->exe = "server";
        server->pwd = ".";
        server->applicationDistrib = false;
        server->allocatable = false;
        server->propertySet.references.push_back("ApplicationPropertySet");
        server->propertySet.references.push_back("NodePropertySet");
        addProperty(server, "ServerProp", "test");
        server->services.push_back(serviceInstance);

        TemplateDescriptor templ;
        templ.parameters.push_back("name");
        templ.descriptor = server;

        ApplicationDescriptor testApp;
        testApp.name = "TestApp";
        testApp.variables["appvar"] = "AppValue";
        testApp.serverTemplates["ServerTemplate"] = templ;
        testApp.serviceTemplates["ServiceTemplate"] = svcTempl;
        testApp.propertySets["ApplicationPropertySet"].properties.push_back(createProperty("ApplicationProp","test"));
        testApp.propertySets["ApplicationPropertySet1"].properties.push_back(createProperty("ApplicationProp", "d"));

        NodeDescriptor node;
        node.variables["nodevar"] = "NodeValue";
        node.propertySets["NodePropertySet"].properties.push_back(createProperty("NodeProp", "test"));
        node.propertySets["NodePropertySet1"].properties.push_back(createProperty("NodeProp", "test"));

        ServerInstanceDescriptor serverInstance;
        serverInstance._cpp_template = "ServerTemplate";
        serverInstance.parameterValues["name"] = "Server";
        serverInstance.propertySet.properties.push_back(createProperty("ServerInstanceProp", "test"));
        node.serverInstances.push_back(serverInstance);

        testApp.nodes["node1"] = node;

        try
        {
            admin->addApplication(testApp);
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

        ServerInfo info = admin->getServerInfo("Server");
        test(hasProperty(info.descriptor, "ServerProp", "test"));
        test(hasProperty(info.descriptor, "NodeProp", "test"));
        test(hasProperty(info.descriptor, "ApplicationProp", "test"));
        test(hasProperty(info.descriptor, "ServerInstanceProp", "test"));

        ServiceDescriptorPtr svc = IceBoxDescriptorPtr::dynamicCast(info.descriptor)->services[0].descriptor;
        test(hasProperty(svc, "ServiceProp", "test"));

        ApplicationUpdateDescriptor empty;
        empty.name = "TestApp";
        ApplicationUpdateDescriptor update;

        update = empty;
        service->propertySet.properties.clear();
        addProperty(service, "ServiceProp", "updated");
        svcTempl.descriptor = service;
        update.serviceTemplates["ServiceTemplate"] = svcTempl;
        admin->updateApplication(update);
        info = admin->getServerInfo("Server");
        svc = IceBoxDescriptorPtr::dynamicCast(info.descriptor)->services[0].descriptor;
        test(hasProperty(svc, "ServiceProp", "updated"));

        update = empty;
        serviceInstance.propertySet.properties.clear();
        serviceInstance.propertySet.properties.push_back(createProperty("ServiceInstanceProp", "updated"));
        server->services.clear();
        server->services.push_back(serviceInstance);
        templ.descriptor = server;
        update.serverTemplates["ServerTemplate"] = templ;
        admin->updateApplication(update);
        info = admin->getServerInfo("Server");
        svc = IceBoxDescriptorPtr::dynamicCast(info.descriptor)->services[0].descriptor;
        test(hasProperty(svc, "ServiceInstanceProp", "updated"));

        update = empty;
        server->propertySet.properties.clear();
        addProperty(server, "ServerProp", "updated");
        templ.descriptor = server;
        update.serverTemplates["ServerTemplate"] = templ;
        admin->updateApplication(update);
        info = admin->getServerInfo("Server");
        test(hasProperty(info.descriptor, "ServerProp", "updated"));

        update = empty;
        serverInstance.propertySet.properties.clear();
        serverInstance.propertySet.properties.push_back(createProperty("ServerInstanceProp", "updated"));
        NodeUpdateDescriptor nodeUpdate;
        nodeUpdate.name = "node1";
        nodeUpdate.serverInstances.push_back(serverInstance);
        update.nodes.push_back(nodeUpdate);
        admin->updateApplication(update);
        info = admin->getServerInfo("Server");
        test(hasProperty(info.descriptor, "ServerInstanceProp", "updated"));

        update = empty;
        nodeUpdate.name = "node1";
        nodeUpdate.serverInstances.clear();
        nodeUpdate.propertySets["NodePropertySet"].properties.clear();
        nodeUpdate.propertySets["NodePropertySet"].properties.push_back(
            createProperty("NodeProp", "updated"));
        nodeUpdate.removePropertySets.push_back("NodePropertySet1");
        update.nodes.push_back(nodeUpdate);
        admin->updateApplication(update);
        info = admin->getServerInfo("Server");
        test(hasProperty(info.descriptor, "NodeProp", "updated"));
        ApplicationDescriptor updatedApplication = admin->getApplicationInfo("TestApp").descriptor;
        test(updatedApplication.nodes["node1"].propertySets.find("NodePropertySet1") ==
             updatedApplication.nodes["node1"].propertySets.end());

        update = empty;
        update.propertySets["ApplicationPropertySet"].properties.clear();
        update.propertySets["ApplicationPropertySet"].properties.push_back(
            createProperty("ApplicationProp", "updated"));
        update.removePropertySets.push_back("ApplicationPropertySet1");
        admin->updateApplication(update);
        info = admin->getServerInfo("Server");
        test(hasProperty(info.descriptor, "ApplicationProp", "updated"));
        updatedApplication = admin->getApplicationInfo("TestApp").descriptor;
        test(updatedApplication.propertySets.find("ApplicationPropertySet1") ==
             updatedApplication.propertySets.end());

        admin->removeApplication("TestApp");
        cout << "ok" << endl;
    }

    {
        cout << "testing description update... " << flush;

        ApplicationDescriptor testApp;
        testApp.name = "TestApp";
        testApp.description = "Description";
        try
        {
            admin->addApplication(testApp);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        testApp = admin->getApplicationInfo("TestApp").descriptor;
        test(testApp.description == "Description");

        ApplicationUpdateDescriptor update;
        update.name = "TestApp";
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        testApp = admin->getApplicationInfo("TestApp").descriptor;
        test(testApp.description == "Description");

        update.description = new BoxedString("updatedDescription");
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        testApp = admin->getApplicationInfo("TestApp").descriptor;
        test(testApp.description == "updatedDescription");

        update.description = new BoxedString("");
        try
        {
            admin->updateApplication(update);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        testApp = admin->getApplicationInfo("TestApp").descriptor;
        test(testApp.description == "");

        admin->removeApplication("TestApp");

        cout << "ok" << endl;
    }

    {
        cout << "testing server node move... " << flush;

        ApplicationDescriptor nodeApp;
        nodeApp.name = "NodeApp";

        ServerDescriptorPtr server = new ServerDescriptor();
        server->id = "node-${index}";
        server->exe = properties->getProperty("IceGridNodeExe");
        server->pwd = ".";
        server->applicationDistrib = false;
        server->allocatable = false;
        server->options.push_back("--nowarn");

        addProperty(server, "IceGrid.Node.Name", "node-${index}");
        addProperty(server, "IceGrid.Node.Data", properties->getProperty("TestDir") + "/db/node-${index}");
        addProperty(server, "IceGrid.Node.Endpoints", "default");
        addProperty(server, "IceGrid.Node.PropertiesOverride", properties->getProperty("NodePropertiesOverride"));
        addProperty(server, "Ice.Admin.Endpoints", "tcp -h 127.0.0.1");

        nodeApp.serverTemplates["nodeTemplate"].descriptor = server;
        nodeApp.serverTemplates["nodeTemplate"].parameters.push_back("index");

        ServerInstanceDescriptor instance;
        instance._cpp_template = "nodeTemplate";
        instance.parameterValues["index"] = "1";
        nodeApp.nodes["localnode"].serverInstances.push_back(instance);
        instance.parameterValues["index"] = "2";
        nodeApp.nodes["localnode"].serverInstances.push_back(instance);

        try
        {
            admin->addApplication(nodeApp);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        admin->startServer("node-1");
        admin->startServer("node-2");

        //
        // We need to wait because the node might not be fully started
        // here (the node adapter isn't indirect, so we can't use the
        // wait-for-activation feature here.)
        //
        int retry = 0;
        while(retry < 20)
        {
            try
            {
                if(admin->pingNode("node-1") && admin->pingNode("node-2"))
                {
                    break;
                }
            }
            catch(const NodeNotExistException&)
            {
            }
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
            ++retry;
        }
        test(admin->pingNode("node-1"));
        test(admin->pingNode("node-2"));

        ApplicationDescriptor testApp;
        testApp.name = "TestApp";
        server = new ServerDescriptor();
        server->id = "Server";
        server->exe = properties->getProperty("ServerDir") + "/server";
        server->pwd = ".";
        server->applicationDistrib = false;
        server->allocatable = false;
        addProperty(server, "Ice.Admin.Endpoints", "tcp -h 127.0.0.1");
        AdapterDescriptor adapter;
        adapter.name = "Server";
        adapter.id = "ServerAdapter";
        adapter.registerProcess = false;
        adapter.serverLifetime = true;
        server->adapters.push_back(adapter);
        addProperty(server, "Server.Endpoints", "default");
        testApp.nodes["node-1"].servers.push_back(server);

        try
        {
            admin->addApplication(testApp);
        }
        catch(const DeploymentException& ex)
        {
            cerr << ex.reason << endl;
            test(false);
        }

        try
        {
            admin->startServer("Server");
            test(admin->getServerState("Server") == Active);
        }
        catch(const ServerStartException& ex)
        {
            cerr << ex << "\nreason = " << ex.reason << endl;
            test(false);
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        ApplicationUpdateDescriptor update;
        update.name = "TestApp";

        NodeUpdateDescriptor nodeUpdate;
        nodeUpdate.name = "node-1";
        nodeUpdate.removeServers.push_back("Server");
        update.nodes.push_back(nodeUpdate);
        nodeUpdate.name = "node-2";
        nodeUpdate.servers.push_back(server);
        nodeUpdate.removeServers.clear();
        update.nodes.push_back(nodeUpdate);

        try
        {
            admin->updateApplication(update);
        }
        catch(const DeploymentException& ex)
        {
            cerr << ex.reason << endl;
            test(false);
        }
        while(true)
        {
            try
            {
                test(admin->getServerInfo("Server").node == "node-2" && admin->getServerState("Server") == Inactive);

                admin->startServer("Server");
                test(admin->getServerState("Server") == Active);
                break;
            }
            catch(const DeploymentException&)
            {
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(200));
            }
        }

        IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(1));

        update = ApplicationUpdateDescriptor();
        update.name = "TestApp";
        nodeUpdate = NodeUpdateDescriptor();
        nodeUpdate.name = "node-2";
        nodeUpdate.removeServers.push_back("Server");
        update.nodes.push_back(nodeUpdate);
        nodeUpdate = NodeUpdateDescriptor();
        nodeUpdate.name = "unknownNode";
        nodeUpdate.servers.push_back(server);
        update.nodes.push_back(nodeUpdate);

        try
        {
            admin->updateApplication(update);
        }
        catch(const DeploymentException& ex)
        {
            cerr << ex.reason << endl;
            test(false);
        }

        try
        {
            admin->getServerState("Server");
            test(false);
        }
        catch(const NodeUnreachableException&)
        {
        }

        try
        {
            admin->removeApplication("TestApp");
        }
        catch(const DeploymentException& ex)
        {
            cerr << ex.reason << endl;
            test(false);
        }

        admin->stopServer("node-1");
        admin->stopServer("node-2");

        try
        {
            admin->removeApplication("NodeApp");
        }
        catch(const Ice::Exception& ex)
        {
            cerr << ex << endl;
            test(false);
        }

        cout << "ok" << endl;
    }

    session->destroy();
}
