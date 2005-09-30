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

void
instantiateServer(const AdminPrx& admin, const string& templ, const map<string, string>& params)
{
    ServerInstanceDescriptor desc;
    desc._cpp_template = templ;
    desc.parameterValues = params;
    NodeUpdateDescriptor nodeUpdate;
    nodeUpdate.name = "localnode";
    nodeUpdate.serverInstances.push_back(desc);
    ApplicationUpdateDescriptor update;
    update.name = "test";
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
    NodeUpdateDescriptor nodeUpdate;
    nodeUpdate.name = "localnode";
    nodeUpdate.removeServers.push_back(id);
    ApplicationUpdateDescriptor update;
    update.name = "test";
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

    TestIntfPrx obj = TestIntfPrx::uncheckedCast(comm->stringToProxy("test"));

    //
    // Test default replica id for a server and service
    //
    cout << "testing default replica id... " << flush;
    {
	map<string, string> params;
	params["id"] = "Server1";
	params["replicaId"] = "";
	instantiateServer(admin, "Server", params);
	test(obj->getReplicaIdAndShutdown() == "Server1");
	removeServer(admin, "Server1");

	params["id"] = "IceBox1";
	params["replicaId"] = "";
	instantiateServer(admin, "IceBox1", params);
	test(obj->getReplicaIdAndShutdown() == "IceBox1.Service1");
	removeServer(admin, "IceBox1");
    }
    cout << "ok" << endl;

    //
    // Test replica id for a server and service
    //
    cout << "testing replica id... " << flush;
    {
	map<string, string> params;
	params["id"] = "Server1";
	params["replicaId"] = "Replica1";
	instantiateServer(admin, "Server", params);
	test(obj->getReplicaIdAndShutdown() == "Replica1");
	removeServer(admin, "Server1");

	params["id"] = "IceBox1";
	params["replicaId"] = "Replica1";
	instantiateServer(admin, "IceBox1", params);
	test(obj->getReplicaIdAndShutdown() == "Replica1");
	removeServer(admin, "IceBox1");
    }
    cout << "ok" << endl;

    cout << "testing replication... " << flush;
    {
	map<string, string> params;
	params["replicaId"] = "";

	params["id"] = "Server1";
	instantiateServer(admin, "Server", params);
	params["id"] = "Server2";
	instantiateServer(admin, "Server", params);
	params["id"] = "Server3";
	instantiateServer(admin, "Server", params);
	params["id"] = "Server4";
	instantiateServer(admin, "Server", params);
	params["id"] = "Server5";
	instantiateServer(admin, "Server", params);

	test(obj->getReplicaIdAndShutdown() == "Server1");
	test(obj->getReplicaIdAndShutdown() == "Server2");
	test(obj->getReplicaIdAndShutdown() == "Server3");
	test(obj->getReplicaIdAndShutdown() == "Server4");
	test(obj->getReplicaIdAndShutdown() == "Server5");
	
	removeServer(admin, "Server1");
	removeServer(admin, "Server2");
	removeServer(admin, "Server3");
	removeServer(admin, "Server4");
	removeServer(admin, "Server5");
    }
    cout << "ok" << endl;

    cout << "testing default replica id with multiple replicas... " << flush;
    {
	map<string, string> params;
	params["id"] = "IceBox1";
	params["replicaId1"] = "";
	params["replicaId2"] = "";
	params["replicaId3"] = "";
	instantiateServer(admin, "IceBox3", params);
	test(obj->getReplicaIdAndShutdown() == "IceBox1.Service1");
	test(obj->getReplicaIdAndShutdown() == "IceBox1.Service2");
	test(obj->getReplicaIdAndShutdown() == "IceBox1.Service3");
	removeServer(admin, "IceBox1");
    }
    cout << "ok" << endl;

    cout << "testing replica id with multiple replicas... " << flush;
    {
	map<string, string> params;
	params["id"] = "IceBox1";
	params["replicaId1"] = "Replica1";
	params["replicaId2"] = "Replica2";
	params["replicaId3"] = "Replica3";
	instantiateServer(admin, "IceBox3", params);
	test(obj->getReplicaIdAndShutdown() == "Replica1");
	test(obj->getReplicaIdAndShutdown() == "Replica2");
	test(obj->getReplicaIdAndShutdown() == "Replica3");
	removeServer(admin, "IceBox1");
    }
    cout << "ok" << endl;
}
