// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/BuiltinSequences.h>
#include <Ice/IdentityUtil.h>
#include <IceGrid/Query.h>
#include <IceGrid/Admin.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;
using namespace IceGrid;

struct ProxyIdentityEqual : public std::binary_function<Ice::ObjectPrx,string,bool>
{

public:

    bool 
    operator()(const Ice::ObjectPrx& p1, const string& id) const
    {
	return p1->ice_getIdentity() == Ice::stringToIdentity(id);
    }
};

void
allCommonTests(const Ice::CommunicatorPtr& communicator)
{
    AdminPrx admin = AdminPrx::checkedCast(communicator->stringToProxy("IceGrid/Admin"));
    test(admin);

    cout << "test server registration... "  << flush;
    Ice::StringSeq serverNames = admin->getAllServerNames();
    test(find(serverNames.begin(), serverNames.end(), "Server1") != serverNames.end());
    test(find(serverNames.begin(), serverNames.end(), "Server2") != serverNames.end());
    test(find(serverNames.begin(), serverNames.end(), "IceBox1") != serverNames.end());
    test(find(serverNames.begin(), serverNames.end(), "IceBox2") != serverNames.end());
    cout << "ok" << endl;

    cout << "testing adapter registration... " << flush;
    Ice::StringSeq adapterIds = admin->getAllAdapterIds();
    test(find(adapterIds.begin(), adapterIds.end(), "Server1.Server") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "Server2.Server") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox1.Service1.Service1") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox1Service2Adapter") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox2.Service1.Service1") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox2Service2Adapter") != adapterIds.end());
    cout << "ok" << endl;

    QueryPrx query = QueryPrx::checkedCast(communicator->stringToProxy("IceGrid/Query"));
    test(query);

    cout << "testing object registration... " << flush;
    Ice::ObjectProxySeq objects = query->findAllObjectsWithType("::Test");
    test(find_if(objects.begin(), objects.end(), bind2nd(ProxyIdentityEqual(),"Server1")) != objects.end());
    test(find_if(objects.begin(), objects.end(), bind2nd(ProxyIdentityEqual(),"Server2")) != objects.end());
    test(find_if(objects.begin(), objects.end(), bind2nd(ProxyIdentityEqual(),"IceBox1-Service1")) != objects.end());
    test(find_if(objects.begin(), objects.end(), bind2nd(ProxyIdentityEqual(),"IceBox1-Service2")) != objects.end());
    test(find_if(objects.begin(), objects.end(), bind2nd(ProxyIdentityEqual(),"IceBox2-Service1")) != objects.end());
    test(find_if(objects.begin(), objects.end(), bind2nd(ProxyIdentityEqual(),"IceBox2-Service2")) != objects.end());

    {
	Ice::ObjectPrx obj = query->findObjectByType("::Test");
	string id = Ice::identityToString(obj->ice_getIdentity());
	test(id == "Server1" || id == "Server2" || 
	     id == "IceBox1-Service1" || id == "IceBox1-Service2" ||
	     id == "IceBox2-Service1" || id == "IceBox2-Service2");
    }

    try
    {
	Ice::ObjectPrx obj = query->findObjectByType("::Foo");
    }
    catch(const ObjectNotExistException&)
    {
    }

    cout << "ok" << endl;
}

void 
allTests(const Ice::CommunicatorPtr& communicator, bool withTemplates)
{
    allCommonTests(communicator);

    //
    // Ensure that all server and service objects are reachable.
    //
    // The identity for the test object in deployed server or services
    // is the name of the service or server. The object adapter name
    // is Adapter prefixed with the name of the service or
    // server. Ensure we can reach each object.
    //
    cout << "pinging server objects... " << flush;

    TestIntfPrx obj;

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server1@Server1.Server"));
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server2@Server2.Server"));
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox1-Service2@IceBox1Service2Adapter"));
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox2-Service1@IceBox2.Service1.Service1"));
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));
    
    cout << "ok" << endl;

    cout << "testing server configuration... " << flush;

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server1@Server1.Server"));
    test(obj->getProperty("Type") == "Server");
    test(obj->getProperty("Name") == "Server1");

    test(obj->getProperty("Variable") == "val0prop");

    test(obj->getProperty("NameName") == "Server1Server1");
    test(obj->getProperty("NameEscaped") == "${name}");
    test(obj->getProperty("NameEscapeEscaped") == "$Server1");
    test(obj->getProperty("NameEscapedEscapeEscaped") == "$${name}");
    test(obj->getProperty("ManyEscape") == "$$$${name}");

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server2@Server2.Server"));
    if(!withTemplates)
    {
	test(obj->getProperty("Target1") == "1");
	test(obj->getProperty("Target2") == "1");
    }
    if(!withTemplates)
    {
	test(obj->getProperty("Variable") == "val0target1");
    }
    cout << "ok" << endl;

    cout << "testing service configuration... " << flush;

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    test(obj->getProperty("Service1.Type") == "standard");
    test(obj->getProperty("Service1.ServiceName") == "Service1");
    
    test(obj->getProperty("Service1.InheritedVariable") == "inherited");

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));
    test(obj->getProperty("Service2.Type") == "freeze");
    test(obj->getProperty("Service2.ServiceName") == "Service2");

    test(obj->getProperty("Service2.DebugProperty") == "");
    test(obj->getProperty("Service1.DebugProperty") == "");
    
    cout << "ok" << endl;

    cout << "testing server options... " << flush;

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server1@Server1.Server"));
    test(obj->getProperty("Test.Test") == "2");
    test(obj->getProperty("Test.Test1") == "0");

    cout << "ok" << endl;

    cout << "testing application update... " << flush;
    AdminPrx admin = AdminPrx::checkedCast(communicator->stringToProxy("IceGrid/Admin"));
    test(admin);

    //
    // Make sure the IceBox servers are totally started before to shut it down.
    //
    communicator->stringToProxy("IceBox1/ServiceManager@IceBox1.IceBox.ServiceManager")->ice_ping();
    communicator->stringToProxy("IceBox2/ServiceManager@IceBox2.IceBox.ServiceManager")->ice_ping();

    admin->stopServer("Server1");
    admin->stopServer("Server2");
    admin->stopServer("IceBox1");
    admin->stopServer("IceBox2");

    //
    // Update the application
    //
    ApplicationDescriptorPtr application = admin->getApplicationDescriptor("test");

    InstanceDescriptorSeq::iterator svr1;
    InstanceDescriptor server1;
    InstanceDescriptor server2;
    InstanceDescriptor icebox1;
    InstanceDescriptor icebox2;
    TemplateDescriptor iceBoxTmpl;
    TemplateDescriptor serviceTmpl;

    for(InstanceDescriptorSeq::iterator p = application->servers.begin(); p != application->servers.end(); ++p)
    {
	if(p->descriptor->name == "Server1")
	{
	    svr1 = p;
	    server1 = *p;
	}
	else if(p->descriptor->name == "Server2")
	{
	    server2 = *p;	    
	    ServerDescriptorPtr server;
	    if(withTemplates)
	    {
		TemplateDescriptorDict::iterator q = application->serverTemplates.find(p->_cpp_template);
		assert(q != application->serverTemplates.end());
		server = ServerDescriptorPtr::dynamicCast(q->second.descriptor);
	    }
	    else
	    {
		server = ServerDescriptorPtr::dynamicCast(p->descriptor);
	    }
	    assert(server);
	    for(PropertyDescriptorSeq::iterator r = server->properties.begin(); r != server->properties.end(); ++r)
	    {
		if(r->name == "Type")
		{
		    r->value = "ServerUpdated";
		}
	    }
	}
	else if(p->descriptor->name == "IceBox1")
	{
	    icebox1 = *p;
	    IceBoxDescriptorPtr iceBox;
	    if(withTemplates)
	    {
		TemplateDescriptorDict::iterator q = application->serverTemplates.find(p->_cpp_template);
		assert(q != application->serverTemplates.end());
		iceBoxTmpl = q->second;
		q->second.descriptor = ComponentDescriptorPtr::dynamicCast(q->second.descriptor->ice_clone());
		iceBox = IceBoxDescriptorPtr::dynamicCast(q->second.descriptor);
		for(InstanceDescriptorSeq::iterator r = iceBox->services.begin(); r != iceBox->services.end(); ++r)
		{
		    if(r->parameterValues["name"] == "Service1")
		    {
			iceBox->services.erase(r);
			break;
		    }
		}
	    }
	    else
	    {
		p->descriptor = ComponentDescriptorPtr::dynamicCast(p->descriptor->ice_clone());
		iceBox = IceBoxDescriptorPtr::dynamicCast(p->descriptor);
		for(InstanceDescriptorSeq::iterator r = iceBox->services.begin(); r != iceBox->services.end(); ++r)
		{
		    if(r->descriptor->name == "Service1")
		    {
			iceBox->services.erase(r);
			break;
		    }
		}
	    }
	}
	else if(p->descriptor->name == "IceBox2")
	{
	    icebox2 = *p;
	    ComponentDescriptorPtr service;
	    if(withTemplates)
	    {
		TemplateDescriptorDict::iterator q = application->serviceTemplates.find("FreezeServiceTemplate");
		assert(q != application->serviceTemplates.end());
		serviceTmpl = q->second;
		q->second.descriptor = ComponentDescriptorPtr::dynamicCast(q->second.descriptor->ice_clone());
		service = q->second.descriptor;
	    }
	    else
	    {
		p->descriptor = ComponentDescriptorPtr::dynamicCast(p->descriptor->ice_clone());
		IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(p->descriptor);
		assert(iceBox);
		for(InstanceDescriptorSeq::iterator r = iceBox->services.begin(); r != iceBox->services.end(); ++r)
		{
		    if(r->descriptor->name == "Service2")
		    {
			r->descriptor = ComponentDescriptorPtr::dynamicCast(r->descriptor->ice_clone());
			service = r->descriptor;
		    }
		}
	    }
	    assert(!service->dbEnvs.empty());
	    service->dbEnvs.clear();
	}
    }

    application->servers.erase(svr1);

    try
    {
	admin->syncApplication(application);
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }

    try
    {
	admin->startServer("Server1"); // Ensure Server1 was removed.
	test(false);
    }
    catch(const ServerNotExistException&)
    {
    }

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server2@Server2.Server"));
    test(obj->getProperty("Type") == "ServerUpdated"); // Ensure Server2 configuration was updated.
    
    try
    {
	//
	// Ensure the service 1 of the IceBox1 server is gone.
	//
	obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    }
    catch(const Ice::NotRegisteredException&)
    {
    }

    
    //
    // Make sure the database environment of Service2 of IceBox2 was removed.
    //
    
    if(!withTemplates)
    {
	obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox1-Service2@IceBox1Service2Adapter"));
	test(!obj->getProperty("Freeze.DbEnv.Service2.DbHome").empty());
    }

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));
    test(obj->getProperty("Freeze.DbEnv.Service2.DbHome").empty());

    //
    // Make sure the IceBox servers are totally started before to shut it down.
    //
    communicator->stringToProxy("IceBox1/ServiceManager@IceBox1.IceBox.ServiceManager")->ice_ping();
    communicator->stringToProxy("IceBox2/ServiceManager@IceBox2.IceBox.ServiceManager")->ice_ping();

    admin->stopServer("Server2");
    admin->stopServer("IceBox1");
    admin->stopServer("IceBox2");

    ApplicationUpdateDescriptor update;
    update.name = "test";
    if(withTemplates)
    {
	update.servers.push_back(server1);
	update.removeServers.push_back(server2.descriptor->name);
	update.serverTemplates["IceBoxTemplate"] = iceBoxTmpl;
	update.serviceTemplates["FreezeServiceTemplate"] = serviceTmpl;
    }
    else
    {
	update.servers.push_back(server1);
	update.removeServers.push_back(server2.descriptor->name);
	update.servers.push_back(icebox1);
	update.servers.push_back(icebox2);
    }

    admin->updateApplication(update);

    admin->startServer("Server1"); // Ensure Server1 is back.
    try
    {
	admin->startServer("Server2"); // Ensure Server2 was removed.
	test(false);
    }
    catch(const ServerNotExistException&)
    {
    }

    //
    // Ensure the service 1 of the IceBox1 server is back.
    //
    try
    {
	obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }

    //
    // Make sure the database environment of Service2 of IceBox2 is back.
    //
    try
    {
	obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));
	test(!obj->getProperty("Freeze.DbEnv.Service2.DbHome").empty());
    }
    catch(const Ice::LocalException&)
    {
	test(false);
    }

    //
    // Make sure the IceBox servers are totally started before to shut it down.
    //
    communicator->stringToProxy("IceBox1/ServiceManager@IceBox1.IceBox.ServiceManager")->ice_ping();
    communicator->stringToProxy("IceBox2/ServiceManager@IceBox2.IceBox.ServiceManager")->ice_ping();

    admin->stopServer("Server1");
    admin->stopServer("IceBox1");
    admin->stopServer("IceBox2");

    cout << "ok" << endl;
}

void
allTestsWithTarget(const Ice::CommunicatorPtr& communicator)
{
    allCommonTests(communicator);

    AdminPrx admin = AdminPrx::checkedCast(communicator->stringToProxy("IceGrid/Admin"));
    test(admin);

    cout << "pinging server objects... " << flush;

    TestIntfPrx obj;
    admin->setServerActivation("Server1", Manual);
    try
    {
	obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server1@Server1.Server"));
	test(false);
    }
    catch(const Ice::LocalException&)
    {
    }
    admin->startServer("Server1");
    
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server1@Server1.Server"));
    test(obj->getProperty("Mode") == "manual");
    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("Server2@Server2.Server"));

    cout << "ok" << endl;

    cout << "testing service configuration... " << flush;

    obj = TestIntfPrx::checkedCast(communicator->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    test(obj->getProperty("Service1.DebugProperty") == "debug");

    cout << "ok" << endl;
}
