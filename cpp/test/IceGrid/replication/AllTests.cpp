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

void
instantiateServer(const AdminPrx& admin, const string& templ, const string& node, const map<string, string>& params)
{
    ServerInstanceDescriptor desc;
    desc._cpp_template = templ;
    desc.parameterValues = params;
    NodeUpdateDescriptor nodeUpdate;
    nodeUpdate.name = node;
    nodeUpdate.serverInstances.push_back(desc);
    ApplicationUpdateDescriptor update;
    update.name = "Test";
    update.nodes.push_back(nodeUpdate);
    try
    {
	admin->updateApplication(update);
    }
    catch(DeploymentException& ex)
    {
	cerr << ex.reason << endl;
	test(false);
    }
}

void
removeServer(const AdminPrx& admin, const string& id)
{
    try
    {
	admin->stopServer(id);
    }
    catch(const ServerStopException&)
    {
    }
    catch(const NodeUnreachableException&)
    {
    }
    catch(const Ice::UserException& ex)
    {
	cerr << ex << endl;
	test(false);
    }

    NodeUpdateDescriptor nodeUpdate;
    nodeUpdate.name = "localnode";
    nodeUpdate.removeServers.push_back(id);
    ApplicationUpdateDescriptor update;
    update.name = "Test";
    update.nodes.push_back(nodeUpdate);
    try
    {
	admin->updateApplication(update);
    }
    catch(DeploymentException& ex)
    {
	cerr << ex.reason << endl;
	test(false);
    }
}

void
allTests(const Ice::CommunicatorPtr& comm)
{
    AdminPrx admin = AdminPrx::checkedCast(comm->stringToProxy("IceGrid/Admin"));
    test(admin);

    set<string> serverReplicaIds;
    serverReplicaIds.insert("Server1.ReplicatedAdapter");
    serverReplicaIds.insert("Server2.ReplicatedAdapter");
    serverReplicaIds.insert("Server3.ReplicatedAdapter");
    set<string> svcReplicaIds;
    svcReplicaIds.insert("IceBox1.Service1.Service1");
    svcReplicaIds.insert("IceBox1.Service2.Service2");
    svcReplicaIds.insert("IceBox1.Service3.Service3");

    cout << "testing replication without load balancing... " << flush;
    {
	map<string, string> params;
	params["replicaGroup"] = "Default";
	params["id"] = "Server1";
	instantiateServer(admin, "Server", "localnode", params);
	params["id"] = "Server2";
	instantiateServer(admin, "Server", "localnode", params);
	params["id"] = "Server3";
	instantiateServer(admin, "Server", "localnode", params);
	set<string> replicaIds = serverReplicaIds;
	TestIntfPrx obj = TestIntfPrx::uncheckedCast(comm->stringToProxy("Default"));
	try
	{
	    test(replicaIds.erase(obj->getReplicaIdAndShutdown()) == 1);
	    test(replicaIds.erase(obj->getReplicaIdAndShutdown()) == 1);
	    test(replicaIds.erase(obj->getReplicaIdAndShutdown()) == 1);
	}
	catch(const Ice::LocalException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}
	removeServer(admin, "Server1");
	removeServer(admin, "Server2");
	removeServer(admin, "Server3");
    }
    {
	map<string, string> params;
	params["replicaGroup"] = "Default";
	params["id"] = "IceBox1";
	instantiateServer(admin, "IceBox", "localnode", params);
	set<string> replicaIds = svcReplicaIds;
	TestIntfPrx obj = TestIntfPrx::uncheckedCast(comm->stringToProxy("Default"));
	try
	{
	    test(replicaIds.erase(obj->getReplicaIdAndShutdown()) == 1);
	    test(replicaIds.erase(obj->getReplicaIdAndShutdown()) == 1);
	    test(replicaIds.erase(obj->getReplicaIdAndShutdown()) == 1);
	}
	catch(const Ice::LocalException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}
	removeServer(admin, "IceBox1");
    }
    cout << "ok" << endl;

    cout << "testing replication with round-robin load balancing... " << flush;
    {
	map<string, string> params;
	params["replicaGroup"] = "RoundRobin";
	params["id"] = "Server1";
	instantiateServer(admin, "Server", "localnode", params);
	params["id"] = "Server2";
	instantiateServer(admin, "Server", "localnode", params);
	params["id"] = "Server3";
	instantiateServer(admin, "Server", "localnode", params);
	TestIntfPrx obj = TestIntfPrx::uncheckedCast(comm->stringToProxy("RoundRobin"));
	try
	{
	    test(obj->getReplicaIdAndShutdown() == "Server1.ReplicatedAdapter");
	    test(obj->getReplicaIdAndShutdown() == "Server2.ReplicatedAdapter");
	    test(obj->getReplicaIdAndShutdown() == "Server3.ReplicatedAdapter");	
	}
	catch(const Ice::LocalException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}
	removeServer(admin, "Server1");
	removeServer(admin, "Server2");
	removeServer(admin, "Server3");
    }
    {
	map<string, string> params;
	params["replicaGroup"] = "RoundRobin";
	params["id"] = "IceBox1";
	instantiateServer(admin, "IceBox", "localnode", params);
	TestIntfPrx obj = TestIntfPrx::uncheckedCast(comm->stringToProxy("RoundRobin"));
	try
	{
	    test(obj->getReplicaIdAndShutdown() == "IceBox1.Service1.Service1");
	    test(obj->getReplicaIdAndShutdown() == "IceBox1.Service2.Service2");
	    test(obj->getReplicaIdAndShutdown() == "IceBox1.Service3.Service3");
	}
	catch(const Ice::LocalException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}
	removeServer(admin, "IceBox1");
    }
    cout << "ok" << endl;

    cout << "testing replication with random load balancing... " << flush;
    {
	map<string, string> params;
	params["replicaGroup"] = "Random";
	params["id"] = "Server1";
	instantiateServer(admin, "Server", "localnode", params);
	params["id"] = "Server2";
	instantiateServer(admin, "Server", "localnode", params);
	params["id"] = "Server3";
	instantiateServer(admin, "Server", "localnode", params);
	TestIntfPrx obj = TestIntfPrx::uncheckedCast(comm->stringToProxy("Random"));
	set<string> replicaIds = serverReplicaIds;
	while(!replicaIds.empty())
	{
	    try
	    {
		replicaIds.erase(obj->getReplicaIdAndShutdown());
	    }
	    catch(const Ice::LocalException& ex)
	    {
		cerr << ex << endl;
		test(false);
	    }
	}
	removeServer(admin, "Server1");
	removeServer(admin, "Server2");
	removeServer(admin, "Server3");
    }
    {
	map<string, string> params;
	params["replicaGroup"] = "Random";
	params["id"] = "IceBox1";
	instantiateServer(admin, "IceBox", "localnode", params);
	TestIntfPrx obj = TestIntfPrx::uncheckedCast(comm->stringToProxy("Random"));
	set<string> replicaIds = svcReplicaIds;
	while(!replicaIds.empty())
	{
	    try
	    {
		replicaIds.erase(obj->getReplicaIdAndShutdown());
	    }
	    catch(const Ice::ConnectionRefusedException&)
	    {
	    }
	    catch(const Ice::LocalException& ex)
	    {
		cerr << ex << endl;
		test(false);
	    }
	}
	removeServer(admin, "IceBox1");
    }
    cout << "ok" << endl;

    cout << "testing replication with adaptive load balancing... " << flush;
    {
	map<string, string> params;
	params["replicaGroup"] = "Adaptive";
	params["id"] = "Server1";
	instantiateServer(admin, "Server", "localnode", params);
	params["id"] = "Server2";
	instantiateServer(admin, "Server", "localnode", params);
	params["id"] = "Server3";
	instantiateServer(admin, "Server", "localnode", params);
	TestIntfPrx obj = TestIntfPrx::uncheckedCast(comm->stringToProxy("Adaptive"));
	set<string> replicaIds = serverReplicaIds;
	while(!replicaIds.empty())
	{
	    try
	    {
		replicaIds.erase(obj->getReplicaIdAndShutdown());
	    }
	    catch(const Ice::LocalException& ex)
	    {
		cerr << ex << endl;
		test(false);
	    }
	}
	removeServer(admin, "Server1");
	removeServer(admin, "Server2");
	removeServer(admin, "Server3");
    }
    {
	map<string, string> params;
	params["replicaGroup"] = "Adaptive";
	params["id"] = "IceBox1";
	instantiateServer(admin, "IceBox", "localnode", params);
	TestIntfPrx obj = TestIntfPrx::uncheckedCast(comm->stringToProxy("Adaptive"));
	set<string> replicaIds = svcReplicaIds;
	while(!replicaIds.empty())
	{
	    try
	    {
		replicaIds.erase(obj->getReplicaIdAndShutdown());
	    }
	    catch(const Ice::ConnectionRefusedException&)
	    {
	    }
	    catch(const Ice::LocalException& ex)
	    {
		cerr << ex << endl;
		test(false);
	    }
	}
	removeServer(admin, "IceBox1");
    }
    cout << "ok" << endl;

    cout << "testing replication with inactive nodes... " << flush;
    {
	map<string, string> params;

	params["replicaGroup"] = "Random";
	params["id"] = "Server1";
	instantiateServer(admin, "Server", "inactivenode", params);
	params["id"] = "Server2";
	instantiateServer(admin, "Server", "localnode", params);
	TestIntfPrx obj = TestIntfPrx::uncheckedCast(comm->stringToProxy("Random"));
	test(obj->getReplicaIdAndShutdown() == "Server2.ReplicatedAdapter");
	removeServer(admin, "Server1");
	removeServer(admin, "Server2");

	params["replicaGroup"] = "RoundRobin";
	params["id"] = "Server1";
	instantiateServer(admin, "Server", "inactivenode", params);
	params["id"] = "Server2";
	instantiateServer(admin, "Server", "localnode", params);
	obj = TestIntfPrx::uncheckedCast(comm->stringToProxy("RoundRobin"));
	test(obj->getReplicaIdAndShutdown() == "Server2.ReplicatedAdapter");
	removeServer(admin, "Server1");
	removeServer(admin, "Server2");

	params["replicaGroup"] = "Adaptive";
	params["id"] = "Server1";
	instantiateServer(admin, "Server", "inactivenode", params);
	params["id"] = "Server2";
	instantiateServer(admin, "Server", "localnode", params);
	obj = TestIntfPrx::uncheckedCast(comm->stringToProxy("Adaptive"));
	test(obj->getReplicaIdAndShutdown() == "Server2.ReplicatedAdapter");
	removeServer(admin, "Server1");
	removeServer(admin, "Server2");

	params["replicaGroup"] = "Random";
	params["id"] = "IceBox1";
	instantiateServer(admin, "IceBox", "localnode", params);
	params["id"] = "Server1";
	instantiateServer(admin, "Server", "inactivenode", params);
	obj = TestIntfPrx::uncheckedCast(comm->stringToProxy("Random"));
	test(svcReplicaIds.find(obj->getReplicaIdAndShutdown()) != svcReplicaIds.end());
	removeServer(admin, "IceBox1");
	removeServer(admin, "Server1");
    };
    cout << "ok" << endl;
}
