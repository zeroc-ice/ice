// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <Ice/Ice.h>
#include <IceGrid/Observer.h>
#include <IceGrid/Admin.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;
using namespace IceGrid;

void 
allTests(const Ice::CommunicatorPtr& communicator)
{
    AdminPrx admin = AdminPrx::checkedCast(communicator->stringToProxy("IceGrid/Admin"));
    test(admin);

    Ice::PropertiesPtr properties = communicator->getProperties();

    {
	ApplicationDescriptorPtr testApp = new ApplicationDescriptor();
	testApp->name = "TestApp";
	admin->addApplication(testApp);

	ApplicationUpdateDescriptor empty;
	empty.name = "TestApp";
	ApplicationUpdateDescriptor update = empty;

	cout << "testing server add... " << flush;

	ServerInstanceDescriptor instance;
	instance.node = "localnode";
	instance.descriptor = new ServerDescriptor();
	instance.descriptor->name = "Server";
	instance.descriptor->exe = properties->getProperty("TestDir") + "/server";
	AdapterDescriptor adapter;
	adapter.name = "Server";
	adapter.endpoints = "default";
	adapter.id = "ServerAdapter";
	adapter.registerProcess = true;
	ObjectDescriptor object;
	object.proxy = communicator->stringToProxy("test@ServerAdapter");
	object.type = "::Test::TestIntf";
	object.adapterId = "ServerAdapter";
	adapter.objects.push_back(object);
	instance.descriptor->adapters.push_back(adapter);
	update.servers.push_back(instance);
	admin->updateApplication(update);

	update.servers[0].descriptor->name = "Server2";
	try
	{
	    admin->updateApplication(update);
	    test(false);
	}
	catch(const DeploymentException&)
	{
	    // Adapter already exists
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}
	
	update.servers[0].descriptor->adapters[0].id = "ServerAdapter2";
	try
	{
	    admin->updateApplication(update);
	    test(false);
	}
	catch(const DeploymentException&)
	{
	    // Object already exists
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	update.servers[0].descriptor->adapters[0].objects[0].proxy = communicator->stringToProxy("test2@ServerAdapter");
	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	TemplateDescriptor templ;
	templ.parameters.push_back("name");
	templ.descriptor = new ServerDescriptor();
	templ.descriptor->name = "${name}";
	ServerDescriptorPtr server = ServerDescriptorPtr::dynamicCast(templ.descriptor);
	server->exe = "${test.dir}/server";
	adapter = AdapterDescriptor();
	adapter.name = "Server";
	adapter.endpoints = "default";
	adapter.id = "${server}";
	adapter.registerProcess = true;
	object = ObjectDescriptor();
	object.proxy = communicator->stringToProxy("${server}@" + adapter.id);
	object.type = "::Test::TestIntf";
	object.adapterId = adapter.id;
	adapter.objects.push_back(object);
	server->adapters.push_back(adapter);
	update = empty;
	update.serverTemplates["ServerTemplate"] = templ;
	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	update = empty;
	instance = ServerInstanceDescriptor();
	instance._cpp_template = "ServerTemplate";
	update.servers.push_back(instance);
	try
	{
	    admin->updateApplication(update);
	    test(false);
	}
	catch(const DeploymentException&)
	{
	    // Missing parameter
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	update = empty;
	update.variables["test.dir"] = properties->getProperty("TestDir");
	update.variables["variable"] = "";
	instance = ServerInstanceDescriptor();
	instance._cpp_template = "ServerTemplate";
	instance.parameterValues["name"] = "Server1";
	update.servers.push_back(instance);
	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}
	
	cout << "ok" << endl;

	cout << "test server remove... " << flush;
	update = empty;
	update.removeServers.push_back("Server2");
	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
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
	catch(const Ice::UserException& ex)
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
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	update = empty;
	update.removeServers.push_back("Server1");
	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}
	
	update = empty;
	update.removeServerTemplates.push_back("ServerTemplate");
	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}
	cout << "ok" << endl;

	cout << "test server update... " << flush;

	instance = admin->getServerDescriptor("Server");
	test(instance.descriptor);
	PropertyDescriptor property;
	property.name = "test";
	property.value = "test";
	instance.descriptor->properties.push_back(property);
	update = empty;
	update.servers.push_back(instance);
	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}
 	instance = admin->getServerDescriptor("Server");
	test(instance.descriptor);
 	test(instance.descriptor->properties.size() == 1);
 	test(instance.descriptor->properties[0].name == "test" && instance.descriptor->properties[0].value == "test");

	update = empty;
	update.serverTemplates["ServerTemplate"] = templ;
	instance = ServerInstanceDescriptor();
	instance._cpp_template = "ServerTemplate";
	instance.parameterValues["name"] = "Server1";
	update.servers.push_back(instance);
	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	update = empty;
	server->properties.push_back(property);
	assert(templ.descriptor == server);
	update.serverTemplates["ServerTemplate"] = templ;
	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

 	instance = admin->getServerDescriptor("Server1");
	test(instance.descriptor);
 	test(instance.descriptor->properties.size() == 1);
 	test(instance.descriptor->properties[0].name == "test" && instance.descriptor->properties[0].value == "test");

	instance = admin->getServerDescriptor("Server");
	test(instance.descriptor);
	adapter = AdapterDescriptor();
	adapter.id = "Server1";
	instance.descriptor->adapters.push_back(adapter);
	update = empty;
	update.servers.push_back(instance);
	try
	{
	    admin->updateApplication(update);
	    test(false);
	}
	catch(const DeploymentException&)
	{
	    // Adapter already exists
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	instance = admin->getServerDescriptor("Server");
	test(instance.descriptor);
	adapter = AdapterDescriptor();
	adapter.id = "ServerX";
	object = ObjectDescriptor();
	object.proxy = communicator->stringToProxy("test");
	adapter.objects.push_back(object);
	instance.descriptor->adapters.push_back(adapter);
	update = empty;
	update.servers.push_back(instance);
	try
	{
	    admin->updateApplication(update);
	    test(false);
	}
	catch(const DeploymentException&)
	{
	    // Object already exists
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}
	cout << "ok" << endl;

	admin->removeApplication("TestApp");
    }

    {
	cout << "test node add..." << flush;

	ApplicationDescriptorPtr testApp = new ApplicationDescriptor();
	testApp->name = "TestApp";
	NodeDescriptor node;
	node.name = "node1";
	node.variables["node"] = "node1";
	testApp->nodes.push_back(node);

	try
	{
	    admin->addApplication(testApp);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	ApplicationUpdateDescriptor update;
	update.name = "TestApp";
	node.name = "node2";
	node.variables["node"] = "node2";
	update.nodes.push_back(node);

	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	testApp = admin->getApplicationDescriptor("TestApp");
	test(testApp->nodes.size() == 2);
	int node1 = testApp->nodes[0].name == "node1" ? 0 : 1;
	int node2 = testApp->nodes[0].name == "node2" ? 0 : 1;
	test(testApp->nodes[node1].name == "node1" && testApp->nodes[node1].variables["node"] == "node1");
	test(testApp->nodes[node2].name == "node2" && testApp->nodes[node2].variables["node"] == "node2");
	cout << "ok" << endl;

	cout << "test node update..." << flush;

	node.name = "node2";
	node.variables["node"] = "node2updated";
	update.nodes.back() = node;
	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	testApp = admin->getApplicationDescriptor("TestApp");
	test(testApp->nodes.size() == 2);
	node1 = testApp->nodes[0].name == "node1" ? 0 : 1;
	node2 = testApp->nodes[0].name == "node2" ? 0 : 1;
	test(testApp->nodes[node1].name == "node1" && testApp->nodes[node1].variables["node"] == "node1");
	test(testApp->nodes[node2].name == "node2" && testApp->nodes[node2].variables["node"] == "node2updated");

	cout << "ok" << endl;

	cout << "test node remove..." << flush;

	update.nodes.clear();
	update.removeNodes.push_back("node1");
	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}
	
	testApp = admin->getApplicationDescriptor("TestApp");
	test(testApp->nodes.size() == 1);
	test(testApp->nodes[0].name == "node2" && testApp->nodes[0].variables["node"] == "node2updated");
	
	admin->removeApplication("TestApp");

	cout << "ok" << endl;
    }	

    {
	cout << "test variable update... " << flush;

	PropertyDescriptorSeq properties;
	PropertyDescriptor property;
	property.name = "ApplicationVar";
	property.value = "${appvar}";
	properties.push_back(property);
	property.name = "NodeVar";
	property.value = "${nodevar}";
	properties.push_back(property);
	property.name = "ServerParamVar";
	property.value = "${serverparamvar}";
	properties.push_back(property);
	property.name = "ServerVar";
	property.value = "${servervar}";
	properties.push_back(property);
	property.name = "ServiceParamVar";
	property.value = "${serviceparamvar}";
	properties.push_back(property);
	property.name = "ServiceVar";
	property.value = "${servicevar}";
	properties.push_back(property);

	TemplateDescriptor serviceTempl;
	serviceTempl.parameters.push_back("serviceparamvar");
	serviceTempl.descriptor = new ServiceDescriptor();
	serviceTempl.descriptor->variables["servicevar"] = "ServiceValue";
	serviceTempl.descriptor->name = "Service";
	serviceTempl.descriptor->properties = properties;

	ServiceInstanceDescriptor service;
	service._cpp_template = "ServiceTemplate";
	service.parameterValues["serviceparamvar"] = "ServiceParamValue";

	IceBoxDescriptorPtr icebox = new IceBoxDescriptor();
	icebox->services.push_back(service);
	icebox->interpreter = "icebox";
	icebox->variables["servervar"] = "ServerValue";

	TemplateDescriptor templ;
	templ.parameters.push_back("name");
	templ.parameters.push_back("serverparamvar");
	templ.descriptor = icebox;

	ApplicationDescriptorPtr testApp = new ApplicationDescriptor();
	testApp->name = "TestApp";
	testApp->variables["appvar"] = "AppValue";
	testApp->serviceTemplates["ServiceTemplate"] = serviceTempl;
	testApp->serverTemplates["IceBoxTemplate"] = templ;

	NodeDescriptor node;
	node.name = "node1";
	node.variables["nodevar"] = "NodeValue";
	testApp->nodes.push_back(node);

	ServerInstanceDescriptor server;
	server._cpp_template = "IceBoxTemplate";
	server.parameterValues["name"] = "Server";
	server.parameterValues["serverparamvar"] = "ServerParamValue";
	server.node = "node1";
	testApp->servers.push_back(server);

	try
	{
	    admin->addApplication(testApp);
	}
	catch(const Ice::UserException& ex)
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
	catch(const DeploymentException& ex)
	{
	    // Missing app variable
	    //cerr << ex.reason << endl;
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	update = empty;
	node.variables.clear();
	update.nodes.push_back(node);
	try
	{
	    admin->updateApplication(update);
	    test(false);
	}
	catch(const DeploymentException& ex)
	{
	    // Missing node variable
	    //cerr << ex.reason << endl;
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	update = empty;
	server = ServerInstanceDescriptor();
	server._cpp_template = "IceBoxTemplate";
	server.parameterValues["name"] = "Server";
	server.node = "node1";
	update.servers.push_back(server);
	try
	{
	    admin->updateApplication(update);
	    test(false);
	}
	catch(const DeploymentException& ex)
	{
	    // Missing parameter
	    //cerr << ex.reason << endl;
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	update = empty;
	templ.descriptor->variables.erase("servervar");
	update.serverTemplates["IceBoxTemplate"] = templ;
	try
	{
	    admin->updateApplication(update);
	    test(false);
	}
	catch(const DeploymentException& ex)
	{
	    // Missing server variable
	    //cerr << ex.reason << endl;
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	update = empty;
	templ.descriptor->variables["servervar"] = "ServerValue";
	IceBoxDescriptorPtr::dynamicCast(templ.descriptor)->services[0].parameterValues.erase("serviceparamvar");
	update.serverTemplates["IceBoxTemplate"] = templ;
	try
	{
	    admin->updateApplication(update);
	    test(false);
	}
	catch(const DeploymentException& ex)
	{
	    // Missing service param variable
	    //cerr << ex.reason << endl;
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	update = empty;
	IceBoxDescriptorPtr::dynamicCast(templ.descriptor)->services[0].parameterValues["serviceparamvar"] = 
	    "ServiceParamValue";
	update.serverTemplates["IceBoxTemplate"] = templ;
	serviceTempl.descriptor->variables.erase("servicevar");
	update.serviceTemplates["ServiceTemplate"] = serviceTempl;
	try
	{
	    admin->updateApplication(update);
	    test(false);
	}
	catch(const DeploymentException& ex)
	{
	    // Missing service variable
	    //cerr << ex.reason << endl;
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	testApp = admin->getApplicationDescriptor("TestApp");

	update = empty;
	update.variables["nodevar"] = "appoverride";
	node = testApp->nodes[0];
	node.variables["serverparamvar"] = "nodeoverride";
	node.variables["servervar"] = "nodeoverride";
	update.nodes.push_back(node);
	templ = testApp->serverTemplates["IceBoxTemplate"];
	templ.descriptor->variables["serviceparamvar"] = "serveroverride";
	templ.descriptor->variables["servicevar"] = "serveroverride";
	update.serverTemplates["IceBoxTemplate"] = templ;
	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}
	
	ApplicationDescriptorPtr previousApp = testApp;
	icebox = IceBoxDescriptorPtr::dynamicCast(testApp->servers[0].descriptor);	
	PropertyDescriptorSeq previousProps = icebox->services[0].descriptor->properties;

	testApp = admin->getApplicationDescriptor("TestApp");
	icebox = IceBoxDescriptorPtr::dynamicCast(testApp->servers[0].descriptor);	
	PropertyDescriptorSeq newProps = icebox->services[0].descriptor->properties;
	test(newProps.size() == previousProps.size());
	test(newProps == previousProps);
	
	update = empty;
	node = previousApp->nodes[0];
	node.variables["appvar"] = "nodeoverride";
	update.nodes.push_back(node);
	templ = previousApp->serverTemplates["IceBoxTemplate"];
	templ.descriptor->variables["nodevar"] = "serveroverride";
	update.serverTemplates["IceBoxTemplate"] = templ;
	serviceTempl = previousApp->serviceTemplates["ServiceTemplate"];
	serviceTempl.descriptor->variables["servervar"] = "serviceoverride";
	update.serviceTemplates["ServiceTemplate"] = serviceTempl;
	try
	{
	    admin->updateApplication(update);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	testApp = admin->getApplicationDescriptor("TestApp");
	icebox = IceBoxDescriptorPtr::dynamicCast(testApp->servers[0].descriptor);	
	newProps = icebox->services[0].descriptor->properties;
	for(PropertyDescriptorSeq::const_iterator p = newProps.begin(); p != newProps.end(); ++p)
	{
	    if(p->name == "ApplicationVar")
	    {
		test(p->value == "nodeoverride");
	    }
	    else if(p->name == "NodeVar")
	    {
		test(p->value == "serveroverride");
	    }
	    else if(p->name == "ServerVar")
	    {
		test(p->value == "serviceoverride");
	    }
	    else if(p->name == "ServiceVar")
	    {
		test(p->value == "ServiceValue");
	    }
	    else if(p->name == "ServerParamVar")
	    {
		test(p->value == "ServerParamValue");
	    }
	    else if(p->name == "ServiceParamVar")
	    {
		test(p->value == "ServiceParamValue");
	    }
	    else
	    {
		test(false);
	    }
	}

	cout << "ok" << endl;
    }
}
