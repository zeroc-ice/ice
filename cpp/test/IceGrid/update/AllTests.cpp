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

	testApp = admin->getApplicationDescriptor("TestApp");
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

	testApp = admin->getApplicationDescriptor("TestApp");
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
	
	testApp = admin->getApplicationDescriptor("TestApp");
	test(testApp.nodes.size() == 1);
	test(testApp.nodes["node2"].variables["nodename"] == "node2updated");
	
	admin->removeApplication("TestApp");

	cout << "ok" << endl;
    }	

    {
	cout << "testing variable update... " << flush;

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

	TemplateDescriptor serviceTempl;

	ServerDescriptorPtr server = new ServerDescriptor();
	server->id = "${name}";
	server->exe = "server";
	server->properties = properties;

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
	catch(const DeploymentException& ex)
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
	catch(const DeploymentException& ex)
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
	catch(const DeploymentException& ex)
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
	ApplicationDescriptor origApp = admin->getApplicationDescriptor("TestApp");

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
	test(serverBefore.descriptor->properties.size() == serverAfter.descriptor->properties.size());
	test(serverBefore.descriptor->properties == serverAfter.descriptor->properties);
	
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
	PropertyDescriptorSeq newProps = serverAfter.descriptor->properties;
	for(PropertyDescriptorSeq::const_iterator p = newProps.begin(); p != newProps.end(); ++p)
	{
	    if(p->name == "ApplicationVar")
	    {
		test(p->value == "nodeoverride");
	    }
	    else if(p->name == "NodeVar")
	    {
		test(p->value == "NodeValue");
	    }	    
	    else if(p->name == "ServerParamVar")
	    {
		test(p->value == "ServerParamValue");
	    }
	    else
	    {
		test(false);
	    }
	}
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
	testApp = admin->getApplicationDescriptor("TestApp");
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
	testApp = admin->getApplicationDescriptor("TestApp");
	test(testApp.description == "Description");

	update.description = new BoxedDescription("updatedDescription");
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
	test(testApp.description == "updatedDescription");
	    
	update.description = new BoxedDescription("");
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
	server->exe = properties->getProperty("IceDir") + "/bin/icegridnode";
	AdapterDescriptor adapter;
	adapter.name = "IceGrid.Node";
	adapter.id = "IceGrid.Node.node-${index}";
	adapter.registerProcess = true;
	adapter.waitForActivation = false;
	server->adapters.push_back(adapter);
	PropertyDescriptor prop;
	prop.name = "IceGrid.Node.Name";
	prop.value = "node-${index}";
	server->properties.push_back(prop);
	prop.name = "IceGrid.Node.Data";
	prop.value = properties->getProperty("TestDir") + "/db/node-${index}";
	server->properties.push_back(prop);
	prop.name = "IceGrid.Node.Endpoints";
	prop.value = "default";
	server->properties.push_back(prop);
	prop.name = "IceGrid.Node.PropertiesOverride";
	prop.value = "Ice.Default.Host=127.0.0.1";
	server->properties.push_back(prop);
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
	IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(3));
	test(admin->pingNode("node-1"));
	test(admin->pingNode("node-2"));

	ApplicationDescriptor testApp;
	testApp.name = "TestApp";
	server = new ServerDescriptor();
	server->id = "Server";
	server->exe = properties->getProperty("TestDir") + "/server";
 	adapter.name = "Server";
	adapter.id = "ServerAdapter";
	adapter.registerProcess = true;
	adapter.waitForActivation = true;
	server->adapters.push_back(adapter);
	prop.name = "Server.Endpoints";
	prop.value = "default";
	server->properties.push_back(prop);
	testApp.nodes["node-1"].servers.push_back(server);

	admin->addApplication(testApp);
	try
	{
	    admin->startServer("Server");
	    test(admin->getServerState("Server") == Active);
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
	update.nodes.push_back(nodeUpdate);

	admin->updateApplication(update);	
	test(admin->getServerInfo("Server").node == "node-2" && admin->getServerState("Server") == Inactive);

	admin->startServer("Server");
	test(admin->getServerState("Server") == Active);
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

	admin->removeApplication("TestApp");

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
}
