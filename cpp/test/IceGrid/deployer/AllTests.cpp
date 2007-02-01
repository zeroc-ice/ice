// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/BuiltinSequences.h>
#include <IceGrid/Query.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Registry.h>
#include <IceUtil/Thread.h>
#include <TestCommon.h>
#include <Test.h>

#include <fstream>

using namespace std;
using namespace Test;
using namespace IceGrid;

namespace 
{

void
writeLongLine(ostream& os)
{
    os << 'a';
    for(int i = 0; i < 2400; i++)
    {
        os << 'b';
    }
    os << 'c';
}

bool
isLongLineStart(const string& line)
{
    test(line.size() < 1024);
    return line.size() > 1 && line[0] == 'a' && line[1] == 'b';
}

bool
isLongLineContent(const string& line)
{
    test(line.size() < 1024);
    return line.size() > 1 && line[0] == 'b' && line[line.size() - 1] == 'b';
}

bool isLongLineEnd(const string& line)
{
    test(line.size() < 1024);
    return line.size() > 1 && line[line.size() - 2] == 'b' && line[line.size() - 1] == 'c';
}

}

struct ProxyIdentityEqual : public std::binary_function<Ice::ObjectPrx,string,bool>
{

public:

    ProxyIdentityEqual(const Ice::CommunicatorPtr& communicator) :
        _communicator(communicator)
    {
    }

    bool 
    operator()(const Ice::ObjectPrx& p1, const string& id) const
    {
        return p1->ice_getIdentity() == _communicator->stringToIdentity(id);
    }

private:

    Ice::CommunicatorPtr _communicator;
};

class SessionKeepAliveThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionKeepAliveThread(const IceGrid::AdminSessionPrx& session, long timeout) :
        _session(session),
        _timeout(IceUtil::Time::seconds(timeout)),
        _destroy(false)
    {
    }

    virtual void
    run()
    {
        Lock sync(*this);
        while(!_destroy)
        {
            timedWait(_timeout);
            if(_destroy)
            {
                break;
            }
            try
            {
                _session->keepAlive();
            }
            catch(const Ice::Exception&)
            {
                break;
            }
        }
    }

    void
    destroy()
    {
        Lock sync(*this);
        _destroy = true;
        notify();
    }

private:

    IceGrid::AdminSessionPrx _session;
    const IceUtil::Time _timeout;
    bool _destroy;
};
typedef IceUtil::Handle<SessionKeepAliveThread> SessionKeepAliveThreadPtr;

void
allTests(const Ice::CommunicatorPtr& comm)
{
    RegistryPrx registry = IceGrid::RegistryPrx::checkedCast(comm->stringToProxy("IceGrid/Registry"));
    test(registry);
    AdminSessionPrx session = registry->createAdminSession("foo", "bar");

    SessionKeepAliveThreadPtr keepAlive = new SessionKeepAliveThread(session, registry->getSessionTimeout()/2);
    keepAlive->start();

    AdminPrx admin = session->getAdmin();
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
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(comm),"Server1")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(comm),"Server2")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(comm),"SimpleServer")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(comm),"IceBox1-Service1")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(comm),"IceBox1-Service2")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(comm),"IceBox2-Service1")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(comm),"IceBox2-Service2")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(comm),"SimpleIceBox-SimpleService")) != objs.end());
    test(find_if(objs.begin(), objs.end(), bind2nd(ProxyIdentityEqual(comm),"ReplicatedObject")) != objs.end());

    {
        test(comm->identityToString(query->findObjectByType("::TestId1")->ice_getIdentity()) == "cat/name1");
        test(comm->identityToString(query->findObjectByType("::TestId2")->ice_getIdentity()) == "cat1/name1");
        test(comm->identityToString(query->findObjectByType("::TestId3")->ice_getIdentity()) == "cat1/name1-bis");
        test(comm->identityToString(query->findObjectByType("::TestId4")->ice_getIdentity()) == "c2\\/c2/n2\\/n2");
        test(comm->identityToString(query->findObjectByType("::TestId5")->ice_getIdentity()) == "n2\\/n2");
    }

    {
        Ice::ObjectPrx obj = query->findObjectByType("::Test");
        string id = comm->identityToString(obj->ice_getIdentity());
        test(id == "Server1" || id == "Server2" || id == "SimpleServer" ||
             id == "IceBox1-Service1" || id == "IceBox1-Service2" ||
             id == "IceBox2-Service1" || id == "IceBox2-Service2" ||
             id == "SimpleIceBox-SimpleService" || "ReplicatedObject");
    }

    {
        Ice::ObjectPrx obj = query->findObjectByTypeOnLeastLoadedNode("::Test", LoadSample5);
        string id = comm->identityToString(obj->ice_getIdentity());
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
    test(obj->getProperty("TestServer1Identity") == "Server1");
    test(obj->getProperty("LogFilePath") == "test-Server1.log");
    test(obj->getProperty("LogFilePath-Server1") == "test.log");
    cout << "ok" << endl;

    cout << "testing service configuration... " << flush;
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    test(obj->getProperty("Service1.Type") == "standard");
    test(obj->getProperty("Service1.ServiceName") == "Service1");
    test(obj->getProperty("TestService1Identity") == "IceBox1-Service1");
    test(obj->getProperty("LogFilePath") == "test-Service1.log");
    test(obj->getProperty("LogFilePath-Service1") == "test.log");

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
        test((*p)->getProperty("RecursiveAppVarProp") == "Test");
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
    //
    // NOTE: We can't test the following since
    // getApplicationDescriptor doesn't return the instantiated
    // application descriptor...
    //
//     ApplicationDescriptor desc = admin->getApplicationDescriptor("test");
//     test(desc.description == "APP AppVar");
//     test(desc.nodes["localnode"].description == "NODE NodeVar");
//     test(desc.replicaGroups[0].description == "REPLICA GROUP AppVar");
//     test(desc.nodes["localnode"].servers.size() == 2);
//    const int idx = desc.nodes["localnode"].servers[0]->id == "SimpleServer" ? 0 : 1;
    ServerInfo info = admin->getServerInfo("SimpleServer");
    test(info.descriptor->id == "SimpleServer");
    test(info.descriptor->description == "SERVER NodeVar");
    test(info.descriptor->adapters[0].description == "ADAPTER NodeVar");
    test(info.descriptor->dbEnvs[0].description == "DBENV NodeVar");
    cout << "ok" << endl;
    
    cout << "testing property sets..." << flush;
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("Server1@Server1.Server"));
    test(obj->getProperty("AppProperty") == "AppVar");
    test(obj->getProperty("AppProperty2") == "OverrideMe");
    test(obj->getProperty("AppProperty21") == "Override");
    test(obj->getProperty("NodeProperty") == "NodeVar");

    obj = TestIntfPrx::checkedCast(comm->stringToProxy("Server2@Server2.Server"));
    test(obj->getProperty("AppProperty") == "AppVar");
    test(obj->getProperty("AppProperty2") == "OverrideMe");
    test(obj->getProperty("AppProperty21") == "Override");
    test(obj->getProperty("NodeProperty") == "NodeVar");
    test(obj->getProperty("ServerInstanceProperty") == "Server2");

    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    test(obj->getProperty("AppProperty") == ""); // IceBox server properties aren't inherited for IceBox1
    test(obj->getProperty("AppProperty2") == "");
    test(obj->getProperty("AppProperty21") == "");
    test(obj->getProperty("NodeProperty") == "");

    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox2-Service1@IceBox2.Service1.Service1"));
    test(obj->getProperty("AppProperty") == "AppVar");
    test(obj->getProperty("AppProperty2") == "OverrideMe");
    test(obj->getProperty("AppProperty21") == "Override");
    test(obj->getProperty("NodeProperty") == "NodeVar");
    test(obj->getProperty("IceBoxInstanceProperty") == "IceBox2");

    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));    
    test(obj->getProperty("AppProperty") == "AppVar");
    test(obj->getProperty("AppProperty2") == "OverrideMe");
    test(obj->getProperty("AppProperty21") == "Override");
    test(obj->getProperty("NodeProperty") == "NodeVar");
    test(obj->getProperty("IceBoxInstanceProperty") == "IceBox2");
    test(obj->getProperty("ServiceInstanceProperty") == "Service2");

    obj = TestIntfPrx::checkedCast(comm->stringToProxy("SimpleServer@SimpleServer.Server"));
    test(obj->getProperty("AppProperty") == "AppVar");
    test(obj->getProperty("AppProperty2") == "OverrideMe");
    test(obj->getProperty("AppProperty21") == "Override");
    test(obj->getProperty("NodeProperty") == "NodeVar");
     
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
    test(obj->getProperty("ServerInstanceServiceProperty") == "service1");
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox1-Service4@IceBox1.Service4.Service4"));
    test(obj->getProperty("ServerInstanceServiceProperty") == "service4");
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox2-Service4@IceBox2.Service4.Service4"));
    test(obj->getProperty("IceBoxInstanceProperty") == "overriden");

    cout << "ok" << endl;

    cout << "testing validation... " << flush;
    TemplateDescriptor templ;
    templ.parameters.push_back("name");
    templ.parameters.push_back("nam3");
    templ.parameters.push_back("nam2");
    templ.parameters.push_back("nam3");
    templ.descriptor = new ServerDescriptor();
    ServerDescriptorPtr server = ServerDescriptorPtr::dynamicCast(templ.descriptor);
    server->id = "test";
    server->exe = "${test.dir}/server";
    ApplicationDescriptor desc;
    desc.name = "App";
    desc.serverTemplates["ServerTemplate"] = templ;
    try
    {
        admin->addApplication(desc);
        test(false);
    }
    catch(const DeploymentException& ex)
    {
        test(ex.reason.find("duplicate parameters") != string::npos);
    }
    catch(const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing stderr/stdout/log files... " << flush;
    string testDir = comm->getProperties()->getProperty("TestDir");
    assert(!testDir.empty());
    try
    {
        session->openServerStdErr("LogServer", -1);
        test(false);
    }
    catch(const FileNotAvailableException&)
    {
    }
    try
    {
        session->openServerStdOut("LogServer", -1);
        test(false);
    }
    catch(const FileNotAvailableException&)
    {
    }
    try
    {
        session->openServerLog("LogServer", "unknown.txt", -1);
        test(false);
    }
    catch(const FileNotAvailableException&)
    {
    }

    obj = TestIntfPrx::checkedCast(comm->stringToProxy("LogServer"));
    try
    {
        session->openServerStdErr("LogServer", -1)->destroy();
        session->openServerStdOut("LogServer", -1)->destroy();
    }
    catch(const FileNotAvailableException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }

    FileIteratorPrx it;
    Ice::StringSeq lines;
    try
    {
        //
        // Test with empty file.
        // 
        ofstream os((testDir + "/log1.txt").c_str());
        os.close();

        it = session->openServerLog("LogServer", testDir + "/log1.txt", -1);
        test(it->read(1024, lines) && lines.empty());
        test(it->read(1024, lines) && lines.empty());
        it->destroy();

        it = session->openServerLog("LogServer", testDir + "/log1.txt", 0);
        test(it->read(1024, lines) && lines.empty());
        test(it->read(1024, lines) && lines.empty());
        it->destroy();

        it = session->openServerLog("LogServer", testDir + "/log1.txt", 100);
        test(it->read(1024, lines) && lines.empty());
        test(it->read(1024, lines) && lines.empty());
        it->destroy();
    }
    catch(const FileNotAvailableException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }

    try
    {
        //
        // Test with log file with one line with no EOL on last line.
        // 
        ofstream os((testDir + "/log2.txt").c_str());
        os << "one line file with no EOL on last line";
        os.close();

        it = session->openServerLog("LogServer", testDir + "/log2.txt", -1);
        test(it->read(1024, lines) && lines.size() == 1);
        test(lines[0] == "one line file with no EOL on last line");
        test(it->read(1024, lines) && lines.empty());
        it->destroy();

        it = session->openServerLog("LogServer", testDir + "/log2.txt", 0);
        test(it->read(1024, lines) && lines.empty());
        test(it->read(1024, lines) && lines.empty());
        it->destroy();
        
        it = session->openServerLog("LogServer", testDir + "/log2.txt", 1);
        test(it->read(1024, lines) && lines.size() == 1);
        test(lines[0] == "one line file with no EOL on last line");
        test(it->read(1024, lines) && lines.empty());
        it->destroy();

        it = session->openServerLog("LogServer", testDir + "/log2.txt", 100);
        test(it->read(1024, lines) && lines.size() == 1);
        test(lines[0] == "one line file with no EOL on last line");
        test(it->read(1024, lines) && lines.empty());
        it->destroy();
    }
    catch(const FileNotAvailableException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }

    try
    {
        //
        // Test with log file with one line with EOL on last line.
        // 
        ofstream os((testDir + "/log3.txt").c_str());
        os << "one line file with EOL on last line" << endl;
        os.close();

        it = session->openServerLog("LogServer", testDir + "/log3.txt", -1);
        test(it->read(1024, lines) && lines.size() == 2);
        test(lines[0] == "one line file with EOL on last line");
        test(lines[1].empty());
        test(it->read(1024, lines) && lines.empty());
        it->destroy();

        it = session->openServerLog("LogServer", testDir + "/log3.txt", 0);
        test(it->read(1024, lines) && lines.empty());
        test(it->read(1024, lines) && lines.empty());
        it->destroy();
        
        it = session->openServerLog("LogServer", testDir + "/log3.txt", 1);
        test(it->read(1024, lines) && lines.size() == 2);
        test(lines[0] == "one line file with EOL on last line");
        test(lines[1].empty());
        test(it->read(1024, lines) && lines.empty());
        it->destroy();

        it = session->openServerLog("LogServer", testDir + "/log3.txt", 100);
        test(it->read(1024, lines) && lines.size() == 2);
        test(lines[0] == "one line file with EOL on last line");
        test(lines[1].empty());
        it->destroy();

        it = session->openServerLog("LogServer", testDir + "/log3.txt", 2);
        test(it->read(1024, lines) && lines.size() == 2);
        test(lines[0] == "one line file with EOL on last line");
        test(lines[1].empty());
        it->destroy();
    }
    catch(const FileNotAvailableException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }

    try
    {
        //
        // Test with log file with multiple lines
        // 
        ofstream os((testDir + "/log4.txt").c_str());
        os << "line 1" << endl;
        os << "line 2" << endl;
        os << "line 3" << endl;
        os.close();

        it = session->openServerLog("LogServer", testDir + "/log4.txt", -1);
        test(it->read(1024, lines) && lines.size() == 4);
        test(lines[0] == "line 1");
        test(lines[1] == "line 2");
        test(lines[2] == "line 3");
        test(lines[3].empty());
        test(it->read(1024, lines) && lines.empty());
        it->destroy();

        it = session->openServerLog("LogServer", testDir + "/log4.txt", 0);
        test(it->read(1024, lines) && lines.empty());
        test(it->read(1024, lines) && lines.empty());
        it->destroy();
        
        it = session->openServerLog("LogServer", testDir + "/log4.txt", 1);
        test(it->read(1024, lines) && lines.size() == 2);
        test(lines[0] == "line 3");
        test(lines[1].empty());
        it->destroy();

        it = session->openServerLog("LogServer", testDir + "/log4.txt", 2);
        test(it->read(1024, lines) && lines.size() == 3);
        test(lines[0] == "line 2");
        test(lines[1] == "line 3");
        test(lines[2].empty());
        it->destroy();

        it = session->openServerLog("LogServer", testDir + "/log4.txt", 100);
        test(it->read(1024, lines) && lines.size() == 4);
        test(lines[0] == "line 1");
        test(lines[1] == "line 2");
        test(lines[2] == "line 3");
        test(lines[3].empty());
        it->destroy();
    }
    catch(const FileNotAvailableException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }

    try
    {
        ofstream os((testDir + "/log1.txt").c_str(), ios_base::out | ios_base::trunc);
        os << flush;

        it = session->openServerLog("LogServer", testDir + "/log1.txt", -1);
        test(it->read(1024, lines) && lines.empty());

        os << "started a line" << flush;
        test(it->read(1024, lines) && lines.size() == 1 && lines[0] == "started a line");
        os << ", continuing the line" << flush;
        test(it->read(1024, lines) && lines.size() == 1 && lines[0] == ", continuing the line");
        os << ", finished" << endl;
        test(it->read(1024, lines) && lines.size() == 2);
        test(lines[0] == ", finished");
        test(lines[1].empty());

        os << "started a line" << flush;
        test(it->read(1024, lines) && lines.size() == 1 && lines[0] == "started a line");
        os << endl << flush;
        test(it->read(1024, lines) && lines.size() == 2 && lines[0].empty() && lines[1].empty());
        os << "and another line" << endl;
        test(it->read(1024, lines) && lines.size() == 2 && !lines[0].empty() && lines[1].empty());

        os << "starting a long line now, " << flush;
        test(it->read(1024, lines) && lines.size() == 1 && lines[0] == "starting a long line now, ");
        writeLongLine(os);
        os.flush();
        test(!it->read(1024, lines) && lines.size() == 1 && isLongLineStart(lines[0]));
        test(!it->read(1024, lines) && lines.size() == 1 && isLongLineContent(lines[0]));
        test(it->read(1024, lines) && lines.size() == 1 && isLongLineEnd(lines[0]));
        test(it->read(1024, lines) && lines.empty());
        os << endl;
        test(it->read(1024, lines) && lines.size() == 2 && lines[0].empty() && lines[1].empty());
        
        os << "starting multiple long line now, " << flush;
        test(it->read(1024, lines) && lines.size() == 1 && lines[0] == "starting multiple long line now, ");
        writeLongLine(os);
        os << endl;
        writeLongLine(os);
        os << endl;
        writeLongLine(os);
        os.flush();
        test(!it->read(1024, lines) && lines.size() == 1 &&  isLongLineStart(lines[0]));
        test(!it->read(1024, lines) && lines.size() == 1 &&  isLongLineContent(lines[0]));
        test(!it->read(1024, lines) && lines.size() == 2 && isLongLineEnd(lines[0]) && isLongLineStart(lines[1]));
        test(!it->read(1024, lines) && lines.size() == 1 &&  isLongLineContent(lines[0]));
        test(!it->read(1024, lines) && lines.size() == 2 && isLongLineEnd(lines[0]) && isLongLineStart(lines[1]));
        test(!it->read(1024, lines) && lines.size() == 1 && isLongLineContent(lines[0]));
        test(!it->read(1024, lines) && lines.size() == 1 && isLongLineContent(lines[0]));
        test(it->read(1024, lines) && lines.size() == 1 && isLongLineEnd(lines[0]));
        os << endl;
        test(it->read(1024, lines) && lines.size() == 2 && lines[0].empty() && lines[1].empty());

        it->destroy();

        it = session->openServerLog("LogServer", testDir + "/log1.txt", 0);
        test(it->read(1024, lines) && lines.empty());
        it->destroy();

        it = session->openServerLog("LogServer", testDir + "/log1.txt", 1);
        test(!it->read(1024, lines) && lines.size() == 1 && isLongLineStart(lines[0]));
        test(!it->read(1024, lines) && lines.size() == 1 && isLongLineContent(lines[0]));
        test(it->read(1024, lines) && lines.size() == 2 && isLongLineEnd(lines[0]) && lines[1].empty());
        test(it->read(1024, lines) && lines.empty());
        it->destroy();
        
        it = session->openServerLog("LogServer", testDir + "/log1.txt", 2);
        test(!it->read(1024, lines) && lines.size() == 1 && isLongLineStart(lines[0]));
        test(!it->read(1024, lines) && lines.size() == 1 && isLongLineContent(lines[0]));
        test(!it->read(1024, lines) && lines.size() == 2 && isLongLineEnd(lines[0]) && isLongLineStart(lines[1]));
        test(!it->read(1024, lines) && lines.size() == 1 && isLongLineContent(lines[0]));
        test(it->read(1024, lines) && lines.size() == 2 && isLongLineEnd(lines[0]) && lines[1].empty());
        it->destroy();
    }
    catch(const FileNotAvailableException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }

    cout << "ok" << endl;

    keepAlive->destroy();
    keepAlive->getThreadControl().join();
    keepAlive = 0;

    session->destroy();
}

void
allTestsWithTarget(const Ice::CommunicatorPtr& comm)
{
    RegistryPrx registry = IceGrid::RegistryPrx::checkedCast(
        comm->stringToProxy("IceGrid/Registry"));
    test(registry);
    AdminSessionPrx session = registry->createAdminSession("foo", "bar");

    SessionKeepAliveThreadPtr keepAlive = new SessionKeepAliveThread(session, registry->getSessionTimeout()/2);
    keepAlive->start();

    AdminPrx admin = session->getAdmin();
    test(admin);

    cout << "testing targets... " << flush;
    
    TestIntfPrx obj = TestIntfPrx::checkedCast(comm->stringToProxy("Server3@Server3.Server"));
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox3-Service1@IceBox3.Service1.Service1"));
    obj = TestIntfPrx::checkedCast(comm->stringToProxy("IceBox3-Service3@IceBox3.Service3.Service3"));

    obj = TestIntfPrx::checkedCast(comm->stringToProxy("Server3@Server3.Server"));
    test(obj->getProperty("TargetProp") == "1");

    cout << "ok" << endl;

    keepAlive->destroy();
    keepAlive->getThreadControl().join();
    keepAlive = 0;

    session->destroy();
}
