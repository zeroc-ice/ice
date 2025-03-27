// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceGrid/IceGrid.h"
#include "Test.h"
#include "TestHelper.h"

#include <algorithm>
#include <fstream>

using namespace std;
using namespace Test;
using namespace IceGrid;

namespace
{
    void writeLongLine(ostream& os)
    {
        os << 'a';
        for (int i = 0; i < 2400; i++)
        {
            os << 'b';
        }
        os << 'c';
    }

    bool isLongLineStart(const string& line)
    {
        test(line.size() < 1024);
        return line.size() > 1 && line[0] == 'a' && line[1] == 'b';
    }

    bool isLongLineContent(const string& line)
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

function<bool(const optional<Ice::ObjectPrx>&)>
proxyIdentityEqual(const string& strId)
{
    return [id = Ice::stringToIdentity(strId)](const optional<Ice::ObjectPrx>& obj)
    { return obj->ice_getIdentity() == id; };
}

void
logTests(const shared_ptr<Ice::Communicator>& comm, const optional<AdminSessionPrx>& session)
{
    cout << "testing stderr/stdout/log files... " << flush;
    string testDir = comm->getProperties()->getProperty("TestDir");
    assert(!testDir.empty());
    try
    {
        session->openServerStdErr("LogServer", -1);
        test(false);
    }
    catch (const FileNotAvailableException&)
    {
    }
    try
    {
        session->openServerStdOut("LogServer", -1);
        test(false);
    }
    catch (const FileNotAvailableException&)
    {
    }
    try
    {
        session->openServerLog("LogServer", "unknown.txt", -1);
        test(false);
    }
    catch (const FileNotAvailableException&)
    {
    }

    TestIntfPrx(comm, "LogServer")->ice_ping();
    try
    {
        session->openServerStdErr("LogServer", -1)->destroy();
        session->openServerStdOut("LogServer", -1)->destroy();
    }
    catch (const FileNotAvailableException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }

    optional<FileIteratorPrx> it;
    Ice::StringSeq lines;
    try
    {
        //
        // Test with empty file.
        //
        string path = testDir + "/log1.txt";
        ofstream os(path.c_str());
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
    catch (const FileNotAvailableException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }

    try
    {
        //
        // Test with log file with one line with no EOL on last line.
        //
        string path = testDir + "/log2.txt";
        ofstream os(path.c_str());
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
    catch (const FileNotAvailableException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }

    try
    {
        //
        // Test with log file with one line with EOL on last line.
        //
        string path = testDir + "/log3.txt";
        ofstream os(path.c_str());
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
    catch (const FileNotAvailableException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }

    try
    {
        //
        // Test with log file with multiple lines
        //
        string path = testDir + "/log4.txt";
        ofstream os(path.c_str());
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
    catch (const FileNotAvailableException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }

    try
    {
        string path = testDir + "/log1.txt";
        ofstream os(path.c_str(), ios_base::out | ios_base::trunc);
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
        test(!it->read(1024, lines) && lines.size() == 1 && isLongLineStart(lines[0]));
        test(!it->read(1024, lines) && lines.size() == 1 && isLongLineContent(lines[0]));
        test(!it->read(1024, lines) && lines.size() == 2 && isLongLineEnd(lines[0]) && isLongLineStart(lines[1]));
        test(!it->read(1024, lines) && lines.size() == 1 && isLongLineContent(lines[0]));
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
    catch (const FileNotAvailableException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }

    cout << "ok" << endl;
}

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr comm = helper->communicator();
    IceGrid::RegistryPrx registry(comm, comm->getDefaultLocator()->ice_getIdentity().category + "/Registry");
    IceGrid::QueryPrx query(comm, comm->getDefaultLocator()->ice_getIdentity().category + "/Query");

    optional<AdminSessionPrx> session = registry->createAdminSession("foo", "bar");

    optional<AdminPrx> admin = session->getAdmin();
    test(admin);

    cout << "testing server registration... " << flush;
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
    test(find(adapterIds.begin(), adapterIds.end(), "ReplicatedAdapter 2") != adapterIds.end());
    cout << "ok" << endl;

    cout << "testing object registration... " << flush;

    auto objs = query->findAllObjectsByType("::Test");
    test(find_if(objs.begin(), objs.end(), proxyIdentityEqual("Server1")) != objs.end());
    test(find_if(objs.begin(), objs.end(), proxyIdentityEqual("Server2")) != objs.end());
    test(find_if(objs.begin(), objs.end(), proxyIdentityEqual("SimpleServer")) != objs.end());
    test(find_if(objs.begin(), objs.end(), proxyIdentityEqual("IceBox1-Service1")) != objs.end());
    test(find_if(objs.begin(), objs.end(), proxyIdentityEqual("IceBox1-Service2")) != objs.end());
    test(find_if(objs.begin(), objs.end(), proxyIdentityEqual("IceBox2-Service1")) != objs.end());
    test(find_if(objs.begin(), objs.end(), proxyIdentityEqual("IceBox2-Service2")) != objs.end());
    test(find_if(objs.begin(), objs.end(), proxyIdentityEqual("SimpleIceBox-SimpleService")) != objs.end());
    test(find_if(objs.begin(), objs.end(), proxyIdentityEqual("ReplicatedObject")) != objs.end());

    {
        test(comm->identityToString(query->findObjectByType("::TestId1")->ice_getIdentity()) == "cat/name1");
        test(comm->identityToString(query->findObjectByType("::TestId2")->ice_getIdentity()) == "cat1/name1");
        test(comm->identityToString(query->findObjectByType("::TestId3")->ice_getIdentity()) == "cat1/name1-bis");
        test(comm->identityToString(query->findObjectByType("::TestId4")->ice_getIdentity()) == "c2\\/c2/n2\\/n2");
        test(comm->identityToString(query->findObjectByType("::TestId5")->ice_getIdentity()) == "n2\\/n2");
    }

    {
        optional<Ice::ObjectPrx> obj = query->findObjectByType("::Test");
        string id = comm->identityToString(obj->ice_getIdentity());
        test(
            id.find("Server") == 0 || id.find("IceBox") == 0 || id == "SimpleServer" ||
            id == "SimpleIceBox-SimpleService" || id == "ReplicatedObject");
    }

    {
        optional<Ice::ObjectPrx> obj = query->findObjectByTypeOnLeastLoadedNode("::Test", LoadSample::LoadSample5);
        string id = comm->identityToString(obj->ice_getIdentity());
        test(
            id.find("Server") == 0 || id.find("IceBox") == 0 || id == "SimpleServer" ||
            id == "SimpleIceBox-SimpleService" || id == "ReplicatedObject");
    }

    {
        optional<Ice::ObjectPrx> obj = query->findObjectByType("::Foo");
        test(!obj);

        obj = query->findObjectByTypeOnLeastLoadedNode("::Foo", LoadSample::LoadSample15);
        test(!obj);
    }

    Ice::Identity encoding10_oneway;
    encoding10_oneway.name = "encoding10-oneway";
    test(query->findObjectById(encoding10_oneway)->ice_getEncodingVersion() == Ice::Encoding_1_0);
    test(query->findObjectById(encoding10_oneway)->ice_isOneway());
    Ice::Identity encoding10_secure;
    encoding10_secure.name = "encoding10-secure";
    test(query->findObjectById(encoding10_secure)->ice_getEncodingVersion() == Ice::Encoding_1_0);
    test(query->findObjectById(encoding10_secure)->ice_isSecure());
    Ice::Identity oaoptions;
    oaoptions.name = "oaoptions";
    test(query->findObjectById(oaoptions)->ice_getEncodingVersion() == Ice::stringToEncodingVersion("1.2"));
    test(query->findObjectById(oaoptions)->ice_isTwoway());
    Ice::Identity comoptions;
    comoptions.name = "communicatoroptions";
    test(query->findObjectById(comoptions)->ice_getEncodingVersion() == Ice::stringToEncodingVersion("1.3"));
    test(query->findObjectById(comoptions)->ice_isTwoway());
    Ice::Identity options34;
    options34.name = "34options";
    test(query->findObjectById(options34)->ice_getEncodingVersion() == Ice::Encoding_1_0);
    Ice::Identity simpleServer;
    simpleServer.name = "SimpleServer";
    test(query->findObjectById(simpleServer)->ice_getEncodingVersion() == Ice::Encoding_1_1);
    Ice::Identity replicated15;
    replicated15.name = "ReplicatedObject15";
    test(query->findObjectById(replicated15)->ice_getEncodingVersion() == Ice::stringToEncodingVersion("1.5"));
    Ice::Identity replicated14;
    replicated14.name = "ReplicatedObject14";
    test(query->findObjectById(replicated14)->ice_getEncodingVersion() == Ice::stringToEncodingVersion("1.4"));

    optional<Ice::LocatorPrx> locator = comm->getDefaultLocator();
    test(locator);
    test(query->findObjectById(encoding10_oneway) == locator->findObjectById(encoding10_oneway));
    test(query->findObjectById(encoding10_secure) == locator->findObjectById(encoding10_secure));
    test(query->findObjectById(oaoptions) == locator->findObjectById(oaoptions));
    test(query->findObjectById(comoptions) == locator->findObjectById(comoptions));
    test(query->findObjectById(options34) == locator->findObjectById(options34));
    test(query->findObjectById(simpleServer) == locator->findObjectById(simpleServer));
    test(query->findObjectById(replicated15) == locator->findObjectById(replicated15));
    test(query->findObjectById(replicated14) == locator->findObjectById(replicated14));

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
    TestIntfPrx(comm, "Server1@Server1.Server")->ice_ping();
    TestIntfPrx(comm, "Server2@Server2.Server")->ice_ping();
    TestIntfPrx(comm, "SimpleServer@SimpleServer.Server")->ice_ping();
    TestIntfPrx(comm, "IceBox1-Service1@IceBox1.Service1.Service1")->ice_ping();
    TestIntfPrx(comm, "IceBox1-Service2@IceBox1Service2Adapter")->ice_ping();
    TestIntfPrx(comm, "IceBox2-Service1@IceBox2.Service1.Service1")->ice_ping();
    TestIntfPrx(comm, "IceBox2-Service2@IceBox2Service2Adapter")->ice_ping();
    TestIntfPrx(comm, "SimpleIceBox-SimpleService@SimpleIceBox.SimpleService.SimpleService")->ice_ping();
    cout << "ok" << endl;

    cout << "testing server configuration... " << flush;
    TestIntfPrx obj(comm, "Server1@Server1.Server");
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
    test(obj->getProperty("PropertyWithSpaces") == "   test   ");
    // \ is escaped in C++ string literals
    test(obj->getProperty("WindowsPath") == "C:\\Program Files (x86)\\ZeroC\\");
    test(obj->getProperty("UNCPath") == "\\\\server\\foo bar\\file");
    test(obj->getProperty("PropertyWith=") == "foo=bar");
    test(obj->getProperty("PropertyWithHash") == "foo#bar");
    test(obj->getProperty("PropertyWithTab") == "foo\tbar");
    test(obj->getProperty("PropertyWithEscapeSpace") == "foo\\ ");
    test(
        obj->getProperty("PropertyWithProperty") ==
        "Plugin.EntryPoint=foo:bar --Ice.Config=\\\\\\server\\foo bar\\file.cfg");
    cout << "ok" << endl;

    cout << "testing service configuration... " << flush;
    obj = TestIntfPrx(comm, "IceBox1-Service1@IceBox1.Service1.Service1");
    test(obj->getProperty("Service1Data.Type") == "standard");
    test(obj->getProperty("Service1Data.ServiceName") == "Service1");
    test(obj->getProperty("TestService1Identity") == "IceBox1-Service1");
    test(obj->getProperty("LogFilePath") == "test-Service1.log");
    test(obj->getProperty("LogFilePath-Service1") == "test.log");
    test(obj->getProperty("PropertyWithSpaces") == "   test   ");
    // \ is escaped in C++ string literals
    test(obj->getProperty("WindowsPath") == "C:\\Program Files (x86)\\ZeroC\\");
    test(obj->getProperty("UNCPath") == "\\\\server\\foo bar\\file");
    test(obj->getProperty("PropertyWith=") == "foo=bar");
    test(obj->getProperty("PropertyWithHash") == "foo#bar");
    test(obj->getProperty("PropertyWithTab") == "foo\tbar");
    test(obj->getProperty("PropertyWithEscapeSpace") == "foo\\ ");
    test(
        obj->getProperty("PropertyWithProperty") ==
        "Plugin.EntryPoint=foo:bar --Ice.Config=\\\\\\server\\foo bar\\file.cfg");

    obj = TestIntfPrx(comm, "IceBox2-Service2@IceBox2Service2Adapter");
    test(obj->getProperty("Service2Data.Type") == "nonstandard");
    test(obj->getProperty("Service2Data.ServiceName") == "Service2");
    test(obj->getProperty("Service2Data.DebugProperty") == "");
    test(obj->getProperty("Service1Data.DebugProperty") == "");
    cout << "ok" << endl;

    cout << "testing server options... " << flush;
    obj = TestIntfPrx(comm, "Server1@Server1.Server");
    test(obj->getProperty("Test.Test") == "2");
    test(obj->getProperty("Test.Test1") == "0");
    cout << "ok" << endl;

    cout << "testing variables... " << flush;
    vector<optional<TestIntfPrx>> proxies;
    obj = TestIntfPrx(comm, "Server1@Server1.Server");
    proxies.emplace_back(obj);
    obj = TestIntfPrx(comm, "IceBox1-Service1@IceBox1.Service1.Service1");
    proxies.emplace_back(obj);
    obj = TestIntfPrx(comm, "SimpleServer@SimpleServer.Server");
    proxies.emplace_back(obj);
    obj = TestIntfPrx(comm, "SimpleIceBox-SimpleService@SimpleIceBox.SimpleService.SimpleService");
    proxies.emplace_back(obj);

    for (const auto& p : proxies)
    {
        test(p->getProperty("AppVarProp") == "AppVar");
        test(p->getProperty("NodeVarProp") == "NodeVar");
        test(p->getProperty("RecursiveAppVarProp") == "Test");
        test(p->getProperty("AppVarOverridedProp") == "OverridedInNode");
        test(p->getProperty("AppVarDefinedInNodeProp") == "localnode");
        test(p->getProperty("EscapedAppVarProp") == "${escaped}");
        test(p->getProperty("RecursiveEscapedAppVarProp") == "${escaped}");
        test(p->getProperty("Recursive2EscapedAppVarProp") == "${escaped}");
        test(p->getProperty("RecursiveNodeVarProp") == "localnode");
        test(p->getProperty("TestDirProp") != "NotThisValue");
    }
    cout << "ok" << endl;

    cout << "testing parameters... " << flush;

    obj = TestIntfPrx(comm, "Server1@Server1.Server");
    test(obj->getProperty("Param1Prop") == "Param1");
    test(obj->getProperty("Param2Prop") == "AppVar");
    test(obj->getProperty("ParamEscapedProp") == "${escaped}");
    test(obj->getProperty("ParamDoubleEscapedProp") == "$escapedvalue");
    test(obj->getProperty("AppVarOverridedByParamProp") == "Overrided");
    test(obj->getProperty("NodeVarOverridedByParamProp") == "Test");
    test(obj->getProperty("DefaultParamProp") == "VALUE");
    obj = TestIntfPrx(comm, "Server2@Server2.Server");
    test(obj->getProperty("Param1Prop") == "Param12");
    test(obj->getProperty("Param2Prop") == "OverridedInNode");
    test(obj->getProperty("ParamEscapedProp") == "${escaped}");
    test(obj->getProperty("ParamDoubleEscapedProp") == "$escapedvalue");
    test(obj->getProperty("AppVarOverridedByParamProp") == "Overrided");
    test(obj->getProperty("NodeVarOverridedByParamProp") == "Test");
    test(obj->getProperty("DefaultParamProp") == "OTHERVALUE");

    obj = TestIntfPrx(comm, "IceBox1-Service1@IceBox1.Service1.Service1");
    test(obj->getProperty("AppVarOverridedByParamProp") == "Test");
    test(obj->getProperty("NodeVarOverridedByParamProp") == "Overrided");
    obj = TestIntfPrx(comm, "IceBox1-Service2@IceBox1Service2Adapter");
    test(obj->getProperty("AppVarOverridedByParamProp") == "Test");
    test(obj->getProperty("NodeVarOverridedByParamProp") == "Test");
    obj = TestIntfPrx(comm, "IceBox2-Service1@IceBox2.Service1.Service1");
    test(obj->getProperty("AppVarOverridedByParamProp") == "Test");
    test(obj->getProperty("NodeVarOverridedByParamProp") == "Overrided");
    obj = TestIntfPrx(comm, "IceBox2-Service2@IceBox2Service2Adapter");
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
    cout << "ok" << endl;

    cout << "testing property sets..." << flush;
    obj = TestIntfPrx(comm, "Server1@Server1.Server");
    test(obj->getProperty("AppProperty") == "AppVar");
    test(obj->getProperty("AppProperty2") == "OverrideMe");
    test(obj->getProperty("AppProperty21") == "Override");
    test(obj->getProperty("NodeProperty") == "NodeVar");

    obj = TestIntfPrx(comm, "Server2@Server2.Server");
    test(obj->getProperty("AppProperty") == "AppVar");
    test(obj->getProperty("AppProperty2") == "OverrideMe");
    test(obj->getProperty("AppProperty21") == "Override");
    test(obj->getProperty("NodeProperty") == "NodeVar");
    test(obj->getProperty("ServerInstanceProperty") == "Server2");

    obj = TestIntfPrx(comm, "IceBox1-Service1@IceBox1.Service1.Service1");
    test(obj->getProperty("AppProperty") == ""); // IceBox server properties aren't inherited for IceBox1
    test(obj->getProperty("AppProperty2") == "");
    test(obj->getProperty("AppProperty21") == "");
    test(obj->getProperty("NodeProperty") == "");

    obj = TestIntfPrx(comm, "IceBox2-Service1@IceBox2.Service1.Service1");
    test(obj->getProperty("AppProperty") == "AppVar");
    test(obj->getProperty("AppProperty2") == "OverrideMe");
    test(obj->getProperty("AppProperty21") == "Override");
    test(obj->getProperty("NodeProperty") == "NodeVar");
    test(obj->getProperty("IceBoxInstanceProperty") == "IceBox2");

    obj = TestIntfPrx(comm, "IceBox2-Service2@IceBox2Service2Adapter");
    test(obj->getProperty("AppProperty") == "AppVar");
    test(obj->getProperty("AppProperty2") == "OverrideMe");
    test(obj->getProperty("AppProperty21") == "Override");
    test(obj->getProperty("NodeProperty") == "NodeVar");
    test(obj->getProperty("IceBoxInstanceProperty") == "IceBox2");
    test(obj->getProperty("ServiceInstanceProperty") == "Service2");

    obj = TestIntfPrx(comm, "SimpleServer@SimpleServer.Server");
    test(obj->getProperty("AppProperty") == "AppVar");
    test(obj->getProperty("AppProperty2") == "OverrideMe");
    test(obj->getProperty("AppProperty21") == "Override");
    test(obj->getProperty("NodeProperty") == "NodeVar");

    obj = TestIntfPrx(comm, "IceBox1-Service1@IceBox1.Service1.Service1");
    test(obj->getProperty("ServerInstanceServiceProperty") == "service1");
    obj = TestIntfPrx(comm, "IceBox1-Service4@IceBox1.Service4.Service4");
    test(obj->getProperty("ServerInstanceServiceProperty") == "service4");
    obj = TestIntfPrx(comm, "IceBox2-Service4@IceBox2.Service4.Service4");
    test(obj->getProperty("IceBoxInstanceProperty") == "overridden");

    cout << "ok" << endl;

    cout << "testing validation... " << flush;
    TemplateDescriptor templ;
    templ.parameters.emplace_back("name");
    templ.parameters.emplace_back("nam3");
    templ.parameters.emplace_back("nam2");
    templ.parameters.emplace_back("nam3");
    templ.descriptor = make_shared<ServerDescriptor>();
    ServerDescriptorPtr server = dynamic_pointer_cast<ServerDescriptor>(templ.descriptor);
    server->id = "test";
    server->exe = "${test.dir}/server";
    server->allocatable = false;
    ApplicationDescriptor desc;
    desc.name = "App";
    desc.serverTemplates["ServerTemplate"] = templ;
    try
    {
        admin->addApplication(desc);
        test(false);
    }
    catch (const DeploymentException& ex)
    {
        test(ex.reason.find("duplicate parameters") != string::npos);
    }
    catch (const Ice::Exception& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    logTests(comm, session);

    session->destroy();
}

void
allTestsWithTarget(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr comm = helper->communicator();
    RegistryPrx registry(comm, comm->getDefaultLocator()->ice_getIdentity().category + "/Registry");
    optional<AdminSessionPrx> session = registry->createAdminSession("foo", "bar");

    optional<AdminPrx> admin = session->getAdmin();
    test(admin);

    cout << "testing targets... " << flush;

    TestIntfPrx obj(comm, "Server3@Server3.Server");
    obj->ice_ping();

    obj = TestIntfPrx(comm, "IceBox3-Service1@IceBox3.Service1.Service1");
    obj->ice_ping();

    obj = TestIntfPrx(comm, "IceBox3-Service3@IceBox3.Service3.Service3");
    obj->ice_ping();

    obj = TestIntfPrx(comm, "IceBox3-Service4@IceBox3.Service4.Service4");
    obj->ice_ping();

    obj = TestIntfPrx(comm, "Server3@Server3.Server");
    obj->ice_ping();

    test(obj->getProperty("TargetProp") == "1");

    cout << "ok" << endl;

    session->destroy();
}
