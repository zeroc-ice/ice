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
	server->exe = properties->getProperty("TestDir") + "/server";
	AdapterDescriptor adapter;
	adapter.name = "Server";
	//adapter.endpoints = "default";
	adapter.id = "ServerAdapter";
	adapter.registerProcess = true;
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
	adapter = AdapterDescriptor();
	adapter.name = "Server";
//	adapter.endpoints = "default";
	adapter.id = "${server}";
	adapter.registerProcess = true;
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
	update.variables["test.dir"] = properties->getProperty("TestDir");
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
	PropertyDescriptor property;
	property.name = "test";
	property.value = "test";
	info.descriptor->properties.push_back(property);
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
 	test(info.descriptor->properties.size() == 1);
 	test(info.descriptor->properties[0].name == "test" && info.descriptor->properties[0].value == "test");

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
	server->properties.push_back(property);
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
 	test(info.descriptor->properties.size() == 1);
 	test(info.descriptor->properties[0].name == "test" && info.descriptor->properties[0].value == "test");

	info = admin->getServerInfo("Server");
	test(info.descriptor);
	adapter = AdapterDescriptor();
	adapter.id = "Server1";
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
	cout << "ok" << endl;

	admin->removeApplication("TestApp");
    }

    {
	cout << "testing node add... " << flush;

	ApplicationDescriptor testApp;
	testApp.name = "TestApp";
	NodeDescriptor node;
	node.variables["node"] = "node1";
	testApp.nodes["node1"] = node;

	try
	{
	    admin->addApplication(testApp);
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
	nodeUpdate.variables["node"] = "node2";
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

	testApp = admin->getApplicationDescriptor("TestApp");
	test(testApp.nodes.size() == 2);
	test(testApp.nodes["node1"].variables["node"] == "node1");
	test(testApp.nodes["node2"].variables["node"] == "node2");
	cout << "ok" << endl;

	cout << "testing node update... " << flush;

	nodeUpdate.name = "node2";
	nodeUpdate.variables["node"] = "node2updated";
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

	testApp = admin->getApplicationDescriptor("TestApp");
	test(testApp.nodes.size() == 2);
	test(testApp.nodes["node1"].variables["node"] == "node1");
	test(testApp.nodes["node2"].variables["node"] == "node2updated");

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
	
	testApp = admin->getApplicationDescriptor("TestApp");
	test(testApp.nodes.size() == 1);
	test(testApp.nodes["node2"].variables["node"] == "node2updated");
	
	admin->removeApplication("TestApp");

	cout << "ok" << endl;
    }	

//     {
// 	cout << "testing variable update... " << flush;

// 	PropertyDescriptorSeq properties;
// 	PropertyDescriptor property;
// 	property.name = "ApplicationVar";
// 	property.value = "${appvar}";
// 	properties.push_back(property);
// 	property.name = "NodeVar";
// 	property.value = "${nodevar}";
// 	properties.push_back(property);
// 	property.name = "ServerParamVar";
// 	property.value = "${serverparamvar}";
// 	properties.push_back(property);
// 	property.name = "ServerVar";
// 	property.value = "${servervar}";
// 	properties.push_back(property);
// 	property.name = "ServiceParamVar";
// 	property.value = "${serviceparamvar}";
// 	properties.push_back(property);
// 	property.name = "ServiceVar";
// 	property.value = "${servicevar}";
// 	properties.push_back(property);

// 	TemplateDescriptor serviceTempl;
// 	serviceTempl.parameters.push_back("serviceparamvar");
// 	serviceTempl.descriptor = new ServiceDescriptor();
// 	ServiceDescriptorPtr::dynamicCast(serviceTempl.descriptor)->name = "Service";
// 	serviceTempl.descriptor->properties = properties;

// 	ServiceInstanceDescriptor service;
// 	service._cpp_template = "ServiceTemplate";
// 	service.parameterValues["serviceparamvar"] = "ServiceParamValue";

// 	IceBoxDescriptorPtr icebox = new IceBoxDescriptor();
// 	icebox->services.push_back(service);
// 	icebox->exe = "icebox";

// 	TemplateDescriptor templ;
// 	templ.parameters.push_back("name");
// 	templ.parameters.push_back("serverparamvar");
// 	templ.descriptor = icebox;

// 	ApplicationDescriptor testApp;
// 	testApp.name = "TestApp";
// 	testApp.variables["appvar"] = "AppValue";
// 	testApp.serviceTemplates["ServiceTemplate"] = serviceTempl;
// 	testApp.serverTemplates["IceBoxTemplate"] = templ;

// 	NodeDescriptor node;
// 	node.variables["nodevar"] = "NodeValue";

// 	ServerInstanceDescriptor server;
// 	server._cpp_template = "IceBoxTemplate";
// 	server.parameterValues["name"] = "Server";
// 	server.parameterValues["serverparamvar"] = "ServerParamValue";
// 	node.serverInstances.push_back(server);

// 	testApp.nodes["node1"] = node;

// 	try
// 	{
// 	    admin->addApplication(testApp);
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}

// 	ApplicationUpdateDescriptor empty;
// 	empty.name = "TestApp";
// 	ApplicationUpdateDescriptor update = empty;
// 	update.removeVariables.push_back("appvar");
// 	try
// 	{
// 	    admin->updateApplication(update);
// 	    test(false);
// 	}
// 	catch(const DeploymentException& ex)
// 	{
// 	    // Missing app variable
// 	    //cerr << ex.reason << endl;
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}

// 	update = empty;
// 	node.variables.clear();
// 	update.nodes.push_back(node);
// 	try
// 	{
// 	    admin->updateApplication(update);
// 	    test(false);
// 	}
// 	catch(const DeploymentException& ex)
// 	{
// 	    // Missing node variable
// 	    //cerr << ex.reason << endl;
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}

// 	update = empty;
// 	server = ServerInstanceDescriptor();
// 	server._cpp_template = "IceBoxTemplate";
// 	server.parameterValues["name"] = "Server";
// 	server.node = "node1";
// 	update.nodes[0].serverInstances.push_back(server);
// 	try
// 	{
// 	    admin->updateApplication(update);
// 	    test(false);
// 	}
// 	catch(const DeploymentException& ex)
// 	{
// 	    // Missing parameter
// 	    //cerr << ex.reason << endl;
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}

// 	update = empty;
// 	templ.descriptor->variables.erase("servervar");
// 	update.serverTemplates["IceBoxTemplate"] = templ;
// 	try
// 	{
// 	    admin->updateApplication(update);
// 	    test(false);
// 	}
// 	catch(const DeploymentException& ex)
// 	{
// 	    // Missing server variable
// 	    //cerr << ex.reason << endl;
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}

// 	update = empty;
// 	templ.descriptor->variables["servervar"] = "ServerValue";
// 	IceBoxDescriptorPtr::dynamicCast(templ.descriptor)->services[0].parameterValues.erase("serviceparamvar");
// 	update.serverTemplates["IceBoxTemplate"] = templ;
// 	try
// 	{
// 	    admin->updateApplication(update);
// 	    test(false);
// 	}
// 	catch(const DeploymentException& ex)
// 	{
// 	    // Missing service param variable
// 	    //cerr << ex.reason << endl;
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}

// 	update = empty;
// 	IceBoxDescriptorPtr::dynamicCast(templ.descriptor)->services[0].parameterValues["serviceparamvar"] = 
// 	    "ServiceParamValue";
// 	update.serverTemplates["IceBoxTemplate"] = templ;
// 	serviceTempl.descriptor->variables.erase("servicevar");
// 	update.serviceTemplates["ServiceTemplate"] = serviceTempl;
// 	try
// 	{
// 	    admin->updateApplication(update);
// 	    test(false);
// 	}
// 	catch(const DeploymentException& ex)
// 	{
// 	    // Missing service variable
// 	    //cerr << ex.reason << endl;
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}

// 	testApp = admin->getApplicationDescriptor("TestApp");

// 	update = empty;
// 	update.variables["nodevar"] = "appoverride";
// 	node = testApp.nodes[0];
// 	node.variables["serverparamvar"] = "nodeoverride";
// 	node.variables["servervar"] = "nodeoverride";
// 	update.nodes.push_back(node);
// 	templ = testApp.serverTemplates["IceBoxTemplate"];
// 	templ.descriptor->variables["serviceparamvar"] = "serveroverride";
// 	templ.descriptor->variables["servicevar"] = "serveroverride";
// 	update.serverTemplates["IceBoxTemplate"] = templ;
// 	try
// 	{
// 	    admin->updateApplication(update);
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}
	
// 	ApplicationDescriptor previousApp = testApp;
// 	icebox = IceBoxDescriptorPtr::dynamicCast(testApp.servers[0].descriptor);	
// 	PropertyDescriptorSeq previousProps = icebox->services[0].descriptor->properties;

// 	testApp = admin->getApplicationDescriptor("TestApp");
// 	icebox = IceBoxDescriptorPtr::dynamicCast(testApp.servers[0].descriptor);	
// 	PropertyDescriptorSeq newProps = icebox->services[0].descriptor->properties;
// 	test(newProps.size() == previousProps.size());
// 	test(newProps == previousProps);
	
// 	update = empty;
// 	node = previousApp->nodes[0];
// 	node.variables["appvar"] = "nodeoverride";
// 	update.nodes.push_back(node);
// 	templ = previousApp->serverTemplates["IceBoxTemplate"];
// 	templ.descriptor->variables["nodevar"] = "serveroverride";
// 	update.serverTemplates["IceBoxTemplate"] = templ;
// 	serviceTempl = previousApp->serviceTemplates["ServiceTemplate"];
// 	serviceTempl.descriptor->variables["servervar"] = "serviceoverride";
// 	update.serviceTemplates["ServiceTemplate"] = serviceTempl;
// 	try
// 	{
// 	    admin->updateApplication(update);
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}

// 	testApp = admin->getApplicationDescriptor("TestApp");
// 	icebox = IceBoxDescriptorPtr::dynamicCast(testApp.servers[0].descriptor);	
// 	newProps = icebox->services[0].descriptor->properties;
// 	for(PropertyDescriptorSeq::const_iterator p = newProps.begin(); p != newProps.end(); ++p)
// 	{
// 	    if(p->name == "ApplicationVar")
// 	    {
// 		test(p->value == "nodeoverride");
// 	    }
// 	    else if(p->name == "NodeVar")
// 	    {
// 		test(p->value == "serveroverride");
// 	    }
// 	    else if(p->name == "ServerVar")
// 	    {
// 		test(p->value == "serviceoverride");
// 	    }
// 	    else if(p->name == "ServiceVar")
// 	    {
// 		test(p->value == "ServiceValue");
// 	    }
// 	    else if(p->name == "ServerParamVar")
// 	    {
// 		test(p->value == "ServerParamValue");
// 	    }
// 	    else if(p->name == "ServiceParamVar")
// 	    {
// 		test(p->value == "ServiceParamValue");
// 	    }
// 	    else
// 	    {
// 		test(false);
// 	    }
// 	}
// 	admin->removeApplication("TestApp");
// 	cout << "ok" << endl;
//     }

//     {
// 	cout << "testing comment update... " << flush;

// 	ApplicationDescriptor testApp;
// 	testApp.name = "TestApp";
// 	testApp.comment = "Comment";
// 	try
// 	{
// 	    admin->addApplication(testApp);
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}
// 	testApp = admin->getApplicationDescriptor("TestApp");
// 	test(testApp.comment == "Comment");
	
// 	ApplicationUpdateDescriptor update;
// 	update.name = "TestApp";
// 	try
// 	{
// 	    admin->updateApplication(update);
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}
// 	testApp = admin->getApplicationDescriptor("TestApp");
// 	test(testApp.comment == "Comment");

// 	update.comment = new BoxedComment("updatedComment");
// 	try
// 	{
// 	    admin->updateApplication(update);
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}
// 	testApp = admin->getApplicationDescriptor("TestApp");
// 	test(testApp.comment == "updatedComment");
	    
// 	update.comment = new BoxedComment("");
// 	try
// 	{
// 	    admin->updateApplication(update);
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}
// 	testApp = admin->getApplicationDescriptor("TestApp");
// 	test(testApp.comment == "");

// 	admin->removeApplication("TestApp");
	
// 	cout << "ok" << endl;
//     }

//     {
// 	cout << "testing server node move... " << flush;
	
// 	ApplicationDescriptor nodeApp;
// 	nodeApp.name = "NodeApp";

// 	ServerDescriptorPtr server = new ServerDescriptor();
// 	server->id = "node-${index}";
// 	server->exe = properties->getProperty("IceDir") + "/bin/icegridnode";
// 	AdapterDescriptor adapter;
// 	adapter.name = "IceGrid.Node";
// 	adapter.endpoints = "default";
// 	adapter.id = "IceGrid.Node.node-${index}";
// 	adapter.registerProcess = true;
// 	adapter.waitForActivation = false;
// 	server->adapters.push_back(adapter);
// 	PropertyDescriptor prop;
// 	prop.name = "IceGrid.Node.Name";
// 	prop.value = "node-${index}";
// 	server->properties.push_back(prop);
// 	prop.name = "IceGrid.Node.Data";
// 	prop.value = properties->getProperty("TestDir") + "/db/node-${index}";
// 	server->properties.push_back(prop);
// 	prop.name = "IceGrid.Node.PropertiesOverride";
// 	prop.value = "Ice.Default.Host=127.0.0.1";
// 	server->properties.push_back(prop);
// 	nodeApp.serverTemplates["nodeTemplate"].descriptor = server;
// 	nodeApp.serverTemplates["nodeTemplate"].parameters.push_back("index");

// 	ServerInstanceDescriptor instance;
// 	instance.node = "localnode";
// 	instance._cpp_template = "nodeTemplate";

// 	instance.parameterValues["index"] = "1";
// 	nodeApp.servers.push_back(instance);

// 	instance.parameterValues["index"] = "2";
// 	nodeApp.servers.push_back(instance);
	
// 	try
// 	{
// 	    admin->addApplication(nodeApp);
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}

// 	admin->startServer("node-1");
// 	admin->startServer("node-2");
// 	IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(3));
// 	test(admin->pingNode("node-1"));
// 	test(admin->pingNode("node-2"));

// 	ApplicationDescriptor testApp;
// 	testApp.name = "TestApp";
// 	instance = ServerInstanceDescriptor();
// 	instance.node = "node-1";
// 	instance.descriptor = new ServerDescriptor();
// 	instance.descriptor->id = "Server";
// 	instance.descriptor->exe = properties->getProperty("TestDir") + "/server";
//  	adapter.name = "Server";
// 	adapter.endpoints = "default";
// 	adapter.id = "ServerAdapter";
// 	adapter.registerProcess = true;
// 	adapter.waitForActivation = true;
// 	instance.descriptor->adapters.push_back(adapter);
// 	testApp.servers.push_back(instance);

// 	admin->addApplication(testApp);
// 	try
// 	{
// 	    admin->startServer("Server");
// 	    test(admin->getServerState("Server") == Active);
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}
	
// 	ApplicationUpdateDescriptor update;
// 	update.name = "TestApp";
// 	instance.node = "node-2";	
// 	update.nodes[0].serverInstances.push_back(instance);
// 	admin->updateApplication(update);	
// 	test(admin->getServerState("Server") == Inactive);

// 	admin->startServer("Server");
// 	test(admin->getServerState("Server") == Active);
// 	IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(1));

// 	update = ApplicationUpdateDescriptor();
// 	update.name = "TestApp";
// 	instance.node = "anUnknownNode";
// 	update.nodes[0].serverInstances.push_back(instance);
// 	admin->updateApplication(update);	
// 	try
// 	{
// 	    admin->getServerState("Server");
// 	    test(false);
// 	}
// 	catch(const NodeUnreachableException&)
// 	{
// 	}

// 	admin->removeApplication("TestApp");

// 	admin->stopServer("node-1");
// 	admin->stopServer("node-2");

// 	try
// 	{
// 	    admin->removeApplication("NodeApp");
// 	}
// 	catch(const Ice::Exception& ex)
// 	{
// 	    cerr << ex << endl;
// 	    test(false);
// 	}

// 	cout << "ok" << endl;
//     }
}
