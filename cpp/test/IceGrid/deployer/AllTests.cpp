// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
allTests(const Ice::CommunicatorPtr& comm)
{
    AdminPrx admin = AdminPrx::checkedCast(comm->stringToProxy("IceGrid/Admin"));
    test(admin);

    cout << "testing server registration... "  << flush;
    Ice::StringSeq serverIds = admin->getAllServerIds();
    test(find(serverIds.begin(), serverIds.end(), "Server1") != serverIds.end());
    test(find(serverIds.begin(), serverIds.end(), "Server2") != serverIds.end());
    test(find(serverIds.begin(), serverIds.end(), "IceBox1") != serverIds.end());
    test(find(serverIds.begin(), serverIds.end(), "IceBox2") != serverIds.end());
    test(find(serverIds.begin(), serverIds.end(), "SimpleServer") != serverIds.end());
    test(find(serverIds.begin(), serverIds.end(), "SimpleIceBox") != serverIds.end());
    cout << "ok" << endl;

    cout << "testing adapter registration... " << flush;
    Ice::StringSeq adapterIds = admin->getAllAdapterIds();
    test(find(adapterIds.begin(), adapterIds.end(), "Server1.Server") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "Server2.Server") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "SimpleServer.Server") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox1.Service1.Service1") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox1Service2Adapter") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox2.Service1.Service1") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "IceBox2Service2Adapter") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "SimpleIceBox.SimpleService.SimpleService") != adapterIds.end());
    test(find(adapterIds.begin(), adapterIds.end(), "ReplicatedAdapter") != adapterIds.end());
    cout << "ok" << endl;

    QueryPrx query = QueryPrx::checkedCast(comm->stringToProxy("IceGrid/Query"));
    test(query);

    cout << "testing object registration... " << flush;
    Ice::ObjectProxySeq objs = query->findAllObjectsByType("::Test");
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(),"Server1")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(),"Server2")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(),"SimpleServer")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(),"IceBox1-Service1")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(),"IceBox1-Service2")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(),"IceBox2-Service1")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(),"IceBox2-Service2")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(),"SimpleIceBox-SimpleService")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(),"ReplicatedObject")) != objs.end());

    {
	test(Ice::identityToString(query->findObjectByType("::TestId1")->ice_getIdentity()) == "cat/name1");
	test(Ice::identityToString(query->findObjectByType("::TestId2")->ice_getIdentity()) == "cat1/name1");
	test(Ice::identityToString(query->findObjectByType("::TestId3")->ice_getIdentity()) == "cat1/name1-bis");
	test(Ice::identityToString(query->findObjectByType("::TestId4")->ice_getIdentity()) == "c2\\/c2/n2\\/n2");
	test(Ice::identityToString(query->findObjectByType("::TestId5")->ice_getIdentity()) == "n2\\/n2");
    }

    {
	Ice::ObjectPrx obj = query->findObjectByType("::Test");
	string id = Ice::identityToString(obj->ice_getIdentity());
	test(id == "Server1" || id == "Server2" || id == "SimpleServer" ||
	     id == "IceBox1-Service1" || id == "IceBox1-Service2" ||
	     id == "IceBox2-Service1" || id == "IceBox2-Service2" ||
	     id == "SimpleIceBox-SimpleService" || "ReplicatedObject");
    }

    {
	Ice::ObjectPrx obj = query->findObjectByTypeOnLeastLoadedNode("::Test", LoadSample5);
	string id = Ice::identityToString(obj->ice_getIdentity());
	test(id == "Server1" || id == "Server2" || id == "SimpleServer" ||
	     id == "IceBox1-Service1" || id == "IceBox1-Service2" ||
	     id == "IceBox2-Service1" || id == "IceBox2-Service2" ||
	     id == "SimpleIceBox-SimpleService" || "ReplicatedObject");
    }

    {
	Ice::ObjectPrx obj = query->findObjectByType("::Foo");
	test(!obj);

	obj = query->findObjectByTypeOnLeastLoadedNode("::Foo", LoadSample15);
	test(!obj);
    }

    cout << "ok" << endl;

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
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("Server1@Server1.Server"));
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("Server2@Server2.Server"));
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("SimpleServer@SimpleServer.Server"));
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox1-Service2@IceBox1Service2Adapter"));
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox2-Service1@IceBox2.Service1.Service1"));
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));
    obj = TestIntfPrx::checkedCast(
	comm->stringToProxy("SimpleIceBox-SimpleService@SimpleIceBox.SimpleService.SimpleService"));
    cout << "ok" << endl;

    cout << "testing server configuration... " << flush;
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("Server1@Server1.Server"));
    test(obj->getProperty("Type") == "Server");
    test(obj->getProperty("Name") == "Server1");
    test(obj->getProperty("NameName") == "Server1Server1");
    test(obj->getProperty("NameEscaped") == "${name}");
    test(obj->getProperty("NameEscapeEscaped") == "$Server1");
    test(obj->getProperty("NameEscapedEscapeEscaped") == "$${name}");
    test(obj->getProperty("ManyEscape") == "$$$${name}");    
    cout << "ok" << endl;

    cout << "testing service configuration... " << flush;
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    test(obj->getProperty("Service1.Type") == "standard");
    test(obj->getProperty("Service1.ServiceName") == "Service1");
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));
    test(obj->getProperty("Service2.Type") == "freeze");
    test(obj->getProperty("Service2.ServiceName") == "Service2");
    test(obj->getProperty("Service2.DebugProperty") == "");
    test(obj->getProperty("Service1.DebugProperty") == "");
    cout << "ok" << endl;

    cout << "testing server options... " << flush;
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("Server1@Server1.Server"));
    test(obj->getProperty("Test.Test") == "2");
    test(obj->getProperty("Test.Test1") == "0");
    cout << "ok" << endl;

    cout << "testing variables... " << flush;
    vector<TestIntfPrx> proxies;
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("Server1@Server1.Server"));
    proxies.push_back(obj);
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    proxies.push_back(obj);
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("SimpleServer@SimpleServer.Server"));
    proxies.push_back(obj);
    obj = TestIntfPrx::checkedCast(
	comm->stringToProxy("SimpleIceBox-SimpleService@SimpleIceBox.SimpleService.SimpleService"));
    proxies.push_back(obj);
    
    for(vector<TestIntfPrx>::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
    {
	test((*p)->getProperty("AppVarProp") == "AppVar");
	test((*p)->getProperty("NodeVarProp") == "NodeVar");
	test((*p)->getProperty("RecursiveAppVarProp") == "test");
	test((*p)->getProperty("AppVarOverridedProp") == "OverridedInNode");
	test((*p)->getProperty("AppVarDefinedInNodeProp") == "localnode");
	test((*p)->getProperty("EscapedAppVarProp") == "${escaped}");
	test((*p)->getProperty("RecursiveEscapedAppVarProp") == "${escaped}");
	test((*p)->getProperty("Recursive2EscapedAppVarProp") == "${escaped}");
	test((*p)->getProperty("RecursiveNodeVarProp") == "localnode");
	test((*p)->getProperty("TestDirProp") != "NotThisValue");
    }
    cout << "ok" << endl;

    cout << "testing parameters... " << flush;

    obj = TestIntfPrx::checkedCast(comm->stringToProxy("Server1@Server1.Server"));
    test(obj->getProperty("Param1Prop") == "Param1");
    test(obj->getProperty("Param2Prop") == "AppVar");
    test(obj->getProperty("ParamEscapedProp") == "${escaped}");
    test(obj->getProperty("ParamDoubleEscapedProp") == "$escapedvalue");
    test(obj->getProperty("AppVarOverridedByParamProp") == "Overrided");
    test(obj->getProperty("NodeVarOverridedByParamProp") == "Test");
    test(obj->getProperty("DefaultParamProp") == "VALUE");
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("Server2@Server2.Server"));
    test(obj->getProperty("Param1Prop") == "Param12");
    test(obj->getProperty("Param2Prop") == "OverridedInNode");
    test(obj->getProperty("ParamEscapedProp") == "${escaped}");
    test(obj->getProperty("ParamDoubleEscapedProp") == "$escapedvalue");
    test(obj->getProperty("AppVarOverridedByParamProp") == "Overrided");
    test(obj->getProperty("NodeVarOverridedByParamProp") == "Test");
    test(obj->getProperty("DefaultParamProp") == "OTHERVALUE");

    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    test(obj->getProperty("AppVarOverridedByParamProp") == "Test");
    test(obj->getProperty("NodeVarOverridedByParamProp") == "Overrided");
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox1-Service2@IceBox1Service2Adapter"));
    test(obj->getProperty("AppVarOverridedByParamProp") == "Test");
    test(obj->getProperty("NodeVarOverridedByParamProp") == "Test");
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox2-Service1@IceBox2.Service1.Service1"));
    test(obj->getProperty("AppVarOverridedByParamProp") == "Test");
    test(obj->getProperty("NodeVarOverridedByParamProp") == "Overrided");
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));
    test(obj->getProperty("AppVarOverridedByParamProp") == "Test");
    test(obj->getProperty("NodeVarOverridedByParamProp") == "Test");
    
    cout << "ok" << endl;

    cout << "testing descriptions... " << flush;
    ApplicationDescriptor desc = admin->getApplicationDescriptor("test");
    test(desc.description == "APP AppVar");
    test(desc.nodes["localnode"].description == "NODE NodeVar");
    test(desc.replicaGroups[0].description == "REPLICA GROUP AppVar");
    test(desc.nodes["localnode"].servers.size() == 2);
    const int idx = desc.nodes["localnode"].servers[0]->id == "SimpleServer" ? 0 : 1;
    test(desc.nodes["localnode"].servers[idx]);
    test(desc.nodes["localnode"].servers[idx]->id == "SimpleServer");
    test(desc.nodes["localnode"].servers[idx]->description == "SERVER NodeVar");
    test(desc.nodes["localnode"].servers[idx]->adapters[0].description == "ADAPTER NodeVar");
    test(desc.nodes["localnode"].servers[idx]->dbEnvs[0].description == "DBENV NodeVar");
    cout << "ok" << endl;
}

void
allTestsWithTarget(const Ice::CommunicatorPtr& comm)
{
    AdminPrx admin = AdminPrx::checkedCast(comm->stringToProxy("IceGrid/Admin"));
    test(admin);

    cout << "testing targets... " << flush;
    
    TestIntfPrx obj = TestIntfPrx::checkedCast(comm->stringToProxy("Server3@Server3.Server"));
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox3-Service1@IceBox3.Service1.Service1"));
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox3-Service3@IceBox3.Service3.Service3"));

    obj = TestIntfPrx::checkedCast(comm->stringToProxy("Server3@Server3.Server"));
    test(obj->getProperty("TargetProp") == "1");

    cout << "ok" << endl;
}
