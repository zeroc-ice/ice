// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "MainPage.xaml.h"
#include <TestCommon.h>
#include <string>
#include <iostream>
#include <memory>
#include <Ice/Ice.h>
#include <IceUtil/StringUtil.h>
#include <Controller.h>

using namespace std;
using namespace TestSuite;

using namespace Test::Common;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

using namespace Platform::Collections;

namespace
{

typedef int(*MAIN_ENTRY_POINT)(int, char**, Test::MainHelper*);

typedef int(*SHUTDOWN_ENTRY_POINT)();

enum TestConfigType { TestConfigTypeClient, TestConfigTypeServer, TestConfigTypeColloc };

struct TestConfig
{
    TestConfigType type;
    string protocol;
    bool serialize;
    bool ipv6;

    string server;
    string host;
};

bool
configUseWS(const TestConfig& config)
{
    return config.protocol == "ws" || config.protocol == "wss";
}

bool
configUseSSL(const TestConfig& config)
{
    return config.protocol == "ssl" || config.protocol == "wss";
}

bool
isIn(const string s[], const string& name)
{
    int l = sizeof(s) / sizeof(string*);
    for(int i = 0; i < l; ++i)
    {
        if(s[i] == name)
        {
            return true;
        }
    }
    return false;
}

static const string noSSL[] =
{
    "udp",
    "metrics"
};

static const string noWS[] =
{
    "metrics"
};

static const string noIPv6[] =
{
    "metrics"
};

void
addConfiguration(const TestCasePtr& test, const string& desc, const string& configName, bool localOnly,
                 const vector<string>& options = vector<string>(), const vector<string>& languages = vector<string>())
{
    TestConfigurationPtr configuration(new TestConfiguration(desc, configName, localOnly, options, languages));
    test->configurations.push_back(configuration);
}

vector<TestCasePtr> allTest(bool remoteserver)
{
    vector<TestCasePtr> all;

    TestCasePtr test;
    TestConfigurationPtr configuration;

    test.reset(new TestCase("Ice", "acm", "client.dll", "server.dll"));
    all.push_back(test);

    test.reset(new TestCase("Ice", "adapterDeactivation", "client.dll", "server.dll", "", "collocated.dll"));
    all.push_back(test);

    test.reset(new  TestCase("Ice", "admin", "client.dll", "server.dll"));
    all.push_back(test);

    test.reset(new TestCase("Ice", "ami", "client.dll", "server.dll", "", "collocated.dll"));
    all.push_back(test);

    test.reset(new TestCase("Ice", "binding", "client.dll", "server.dll"));
    all.push_back(test);

    test.reset(new TestCase("Ice", "dispatcher", "client.dll", "server.dll", "", "collocated.dll"));
    all.push_back(test);

    test.reset(new TestCase("Ice", "enums", "client.dll", "server.dll"));
    addConfiguration(test, "1.0 encoding", "1.0", false, { "--Ice.Default.EncodingVersion=1.0" });
    addConfiguration(test, "1.1 encoding", "1.1", false);
    all.push_back(test);

    test.reset(new TestCase("Ice", "exceptions", "client.dll", "server.dll", "serveramd.dll", "collocated.dll"));
    addConfiguration(test, "compact (default) format", "compact", false);
    addConfiguration(test, "sliced format", "sliced", false, { "--Ice.Default.SlicedFormat" });
    addConfiguration(test, "1.0 encoding", "1.0", false, { "--Ice.Default.EncodingVersion=1.0" });
    addConfiguration(test, "compact (default) format and AMD server", "", true);
    addConfiguration(test, "sliced format and AMD server", "", true, { "--Ice.Default.SlicedFormat" });
    addConfiguration(test, "1.0 encoding and AMD server", "", true, { "--Ice.Default.EncodingVersion=1.0" });
    all.push_back(test);

    test.reset(new TestCase("Ice", "facets", "client.dll", "server.dll", "", "collocated.dll"));
    all.push_back(test);

    test.reset(new TestCase("Ice", "hash", "client.dll"));
    all.push_back(test);

    test.reset(new TestCase("Ice", "hold", "client.dll", "server.dll"));
    all.push_back(test);

    test.reset(new TestCase("Ice", "info", "client.dll", "server.dll"));
    all.push_back(test);

    test.reset(new TestCase("Ice", "inheritance", "client.dll", "server.dll", "", "collocated.dll"));
    all.push_back(test);

    test.reset(new TestCase("Ice", "invoke", "client.dll", "server.dll"));
    addConfiguration(test, "Blobject server", "blobject", false, {}, { "cpp", "java", "csharp" });
    addConfiguration(test, "BlobjectArray server", "blobjectArray", false, {}, { "cpp" });
    addConfiguration(test, "BlobjectAsync server", "blobjectAsync", false, {}, { "cpp", "java", "csharp" });
    addConfiguration(test, "BlobjectAsyncArray server", "blobjectAsyncArray", false, {}, { "cpp" });
    all.push_back(test);

    test.reset(new TestCase("Ice", "location", "client.dll", "server.dll"));
    all.push_back(test);

    if (!remoteserver)
    {
        test.reset(new TestCase("Ice", "metrics", "client.dll", "server.dll", "serveramd.dll"));
        all.push_back(test);
    }

    test.reset(new TestCase("Ice", "objects", "client.dll", "server.dll", "", "collocated.dll"));
    addConfiguration(test, "compact (default) format", "compact", false);
    addConfiguration(test, "sliced format", "sliced", false, { "--Ice.Default.SlicedFormat" });
    addConfiguration(test, "1.0 encoding", "1.0", false, { "--Ice.Default.EncodingVersion=1.0" });
    all.push_back(test);

    test.reset(new TestCase("Ice", "operations", "client.dll", "server.dll", "serveramd.dll", "collocated.dll"));
    addConfiguration(test, "regular server", "", false);
    addConfiguration(test, "AMD server", "", true);
    addConfiguration(test, "TIE server", "", true, {}, {"java", "csharp"});
    addConfiguration(test, "AMD TIE server", "", true, {}, { "java", "csharp" });
    all.push_back(test);

    test.reset(new TestCase("Ice", "optional", "client.dll", "server.dll"));
    addConfiguration(test, "compact (default) format", "compact", false);
    addConfiguration(test, "sliced format", "sliced", false, { "--Ice.Default.SlicedFormat" });
    addConfiguration(test, "AMD server", "", true);
    all.push_back(test);

    test.reset(new TestCase("Ice", "proxy", "client.dll", "server.dll", "serveramd.dll", "collocated.dll"));
    addConfiguration(test, "regular server", "", false);
    addConfiguration(test, "AMD server", "", true);
    all.push_back(test);

    test.reset(new TestCase("Ice", "retry", "client.dll", "server.dll"));
    all.push_back(test);

    test.reset(new TestCase("Ice", "stream", "client.dll"));
    all.push_back(test);

    test.reset(new TestCase("Ice", "timeout", "client.dll", "server.dll"));
    all.push_back(test);

    if(!remoteserver)
    {
        test.reset(new TestCase("Ice", "udp", "client.dll", "server.dll"));
        all.push_back(test);
    }

    return all;
}

class TestRunner;
typedef IceUtil::Handle<TestRunner> TestRunnerPtr;

class Runnable : public IceUtil::Thread, public Test::MainHelper
{
public:

    Runnable(const TestRunnerPtr&, const string&, DllCache&, const TestConfig&,
             const vector<string>& options = vector<string>());

    virtual ~Runnable();
    virtual void run();
    void waitForStart();
    virtual void waitForCompleted();
    virtual void serverReady();
    virtual void shutdown();
    virtual bool redirect();
    virtual void print(const string& message);
    int status();

private:

    void completed(int);

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    TestRunnerPtr _runner;
    string _test;
    TestConfig _config;
    bool _started;
    bool _completed;
    int _status;
    Ice::CommunicatorPtr _communicator;
    FARPROC _dllTestShutdown;
    DllCache& _dlls;
    vector<string> _options;
};
typedef IceUtil::Handle<Runnable> RunnablePtr;

class TestRunner : public IceUtil::Thread
{
public:

    TestRunner(const std::shared_ptr<TestCase>&, const TestConfig&, MainPage^,
               const Ice::CommunicatorPtr&, DllCache&);
    virtual void run();
    void runClientServerTest(const string&, const string&);
    void runClientServerTestWithRemoteServer(const string&);
    void runClientTest(const string&, bool);

    void printToConsoleOutput(const std::string&);
    void printLineToConsoleOutput(const std::string&);

private:

    std::shared_ptr<TestCase> _test;
    TestConfig _config;
    MainPage^ _page;
    Ice::CommunicatorPtr _communicator;
    DllCache& _dlls;
};

Runnable::Runnable(const TestRunnerPtr& runner, const string& test, DllCache& dlls, const TestConfig& config,
                   const vector<string>& options) :
    _runner(runner),
    _test(test),
    _config(config),
    _started(false),
    _completed(false),
    _status(0),
    _dlls(dlls),
    _options(options)
{
}

Runnable::~Runnable()
{
}

void
Runnable::run()
{
    HINSTANCE hnd = _dlls.loadDll(_test);
    if(hnd == 0)
    {
        ostringstream os;
        os << "failed to load `" << _test << "': error code: " << GetLastError();
        _runner->printLineToConsoleOutput(os.str());
        completed(-1);
        return;
    }

    _dllTestShutdown = GetProcAddress(hnd, "dllTestShutdown");
    if(_dllTestShutdown == 0)
    {
        _runner->printLineToConsoleOutput("failed to find dllTestShutdown function from `" + _test + "'");
        completed(-1);
        return;
    }

    FARPROC dllMain = GetProcAddress(hnd, "dllMain");
    if(dllMain == 0)
    {
        _runner->printLineToConsoleOutput("failed to find dllMain function from `" + _test + "'");
        completed(-1);
        return;
    }

    vector<string> args;
    args.push_back("--Ice.NullHandleAbort=1");
    args.push_back("--Ice.Warn.Connections=1");
    args.push_back("--Ice.ProgramName=" + _test);
    if(_config.serialize)
    {
        args.push_back("--Ice.ThreadPool.Server.Serialize=1");
    }

    if(_config.ipv6)
    {
        args.push_back("--Ice.Default.Host=0:0:0:0:0:0:0:1");
        args.push_back("--Ice.IPv4=1");
        args.push_back("--Ice.IPv6=1");
        args.push_back("--Ice.PreferIPv6Address=1");
    }
    else
    {
        args.push_back("--Ice.Default.Host=127.0.0.1");
        args.push_back("--Ice.IPv4=1");
        args.push_back("--Ice.IPv6=0");
    }

    if(_config.type != TestConfigTypeClient)
    {
        args.push_back("--Ice.ThreadPool.Server.Size=1");
        args.push_back("--Ice.ThreadPool.Server.SizeMax=3");
        args.push_back("--Ice.ThreadPool.Server.SizeWarn=0");
    }

    args.push_back("--Ice.Default.Host=" + _config.host);
    args.push_back("--Ice.Default.Protocol=" + _config.protocol);

    for(vector<string>::const_iterator i = _options.begin(); i != _options.end(); i++)
    {
        args.push_back(*i);
    }

    char** argv = new char*[args.size() + 1];
    for(unsigned int i = 0; i < args.size(); ++i)
    {
        argv[i] = const_cast<char*>(args[i].c_str());
    }
    argv[args.size()] = 0;
    int status = EXIT_FAILURE;
    try
    {
        status = reinterpret_cast<MAIN_ENTRY_POINT>(dllMain)(static_cast<int>(args.size()), argv, this);
    }
    catch(const std::exception& ex)
    {
        print("unexpected exception while running `" + _test + "':\n" + ex.what());
    }
    catch(...)
    {
        print("unexpected unknown exception while running `" + _test + "'");
    }

    completed(status);
    delete[] argv;
}

void
Runnable::waitForStart()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    while(!_started && !_completed)
    {
        _monitor.wait();
    }
    if(_completed && _status != 0)
    {
        ostringstream os;
        os << "failed with status = " << _status;
        throw os.str();
    }
}

void
Runnable::waitForCompleted()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    while(!_completed)
    {
        _monitor.wait();
    }
    if(_status != 0)
    {
        ostringstream os;
        os << "failed with status = " << _status;
        throw os.str();
    }
}

void
Runnable::serverReady()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    _started = true;
    _monitor.notify();
}

void
Runnable::shutdown()
{
    if(_dllTestShutdown)
    {
        reinterpret_cast<SHUTDOWN_ENTRY_POINT>(_dllTestShutdown)();
    }
}

bool
Runnable::redirect()
{
    return _config.type == TestConfigTypeClient || _config.type == TestConfigTypeColloc;
}

void
Runnable::print(const string& message)
{
    _runner->printToConsoleOutput(message);
}

int
Runnable::status()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    return _status;
}

void
Runnable::completed(int status)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    _completed = true;
    _status = status;
    _monitor.notify();
}

TestRunner::TestRunner(const TestCasePtr& test, const TestConfig& config, MainPage^ page,
                       const Ice::CommunicatorPtr& communicator, DllCache& dlls) :
    _test(test),
    _config(config),
    _page(page),
    _communicator(communicator),
    _dlls(dlls)
{
}

void
TestRunner::printToConsoleOutput(const std::string& message)
{
    vector<string> lines;
    string::size_type pos = 0;
    string data = message;
    while((pos = data.find("\n")) != string::npos)
    {
        lines.push_back(data.substr(0, pos));
        data = data.substr(pos + 1);
    }
    lines.push_back(data);

    for(vector<string>::const_iterator i = lines.begin(); i != lines.end(); ++i)
    {
        _page->printToConsoleOutput(ref new String(IceUtil::stringToWstring(*i).c_str()), i != lines.begin());
    }
}

void
TestRunner::printLineToConsoleOutput(const std::string& msg)
{
    printToConsoleOutput(msg + '\n');
}

void
TestRunner::run()
{
    try
    {
        if(configUseWS(_config) && !_test->wsSupport)
        {
            printLineToConsoleOutput("**** test " + _test->name + " not supported with WS");
        }
        else if(configUseSSL(_config) && !_test->sslSupport)
        {
            printLineToConsoleOutput("**** test " + _test->name + " not supported with SSL");
        }
        else if(_config.ipv6 && !_test->ipv6Support)
        {
            printLineToConsoleOutput("**** test " + _test->name + " not supported with IPv6");
        }
        else if(configUseSSL(_config) && _config.ipv6)
        {
            printLineToConsoleOutput("**** test " + _test->name + " not supported with IPv6 SSL");
        }
        else
        {
            if(!_test->server.empty())
            {
                if(_config.server == "winrt")
                {
                    printLineToConsoleOutput("**** running test " + _test->name);
                    runClientServerTest(_test->server, _test->client);
                }
                else
                {
                    printLineToConsoleOutput("**** running test " + _test->name);
                    runClientServerTestWithRemoteServer(_test->client);
                }
            }
            else
            {
                assert(!_test->client.empty());
                printLineToConsoleOutput("**** running test " + _test->name);
                runClientTest(_test->client, false);
            }

            if(!_test->serverAMD.empty() && _config.server == "winrt")
            {
                printLineToConsoleOutput("*** running test with AMD server " + _test->name);
                runClientServerTest(_test->serverAMD, _test->client);
            }

            //
            // Don't run collocated tests with SSL as there isn't SSL server side.
            //
            if(!_test->collocated.empty() && !configUseSSL(_config) && _config.server == "winrt")
            {
                printLineToConsoleOutput("*** running collocated test " + _test->name);
                runClientTest(_test->collocated, true);
            }
        }
        _page->completed();
    }
    catch(Platform::Exception^ ex)
    {
        _page->failed(ex->Message);
    }
    catch (Test::Common::ServerFailedException& ex)
    {
        _page->failed(ref new String(IceUtil::stringToWstring("Server failed to start:\n\n" + ex.reason).c_str()));
    }
    catch(const std::exception& ex)
    {
        _page->failed(ref new String(IceUtil::stringToWstring(ex.what()).c_str()));
    }
    catch(const string& ex)
    {
        _page->failed(ref new String(IceUtil::stringToWstring(ex).c_str()));
    }
    catch(...)
    {
        _page->failed("unknown exception");
    }
}

void
TestRunner::runClientServerTest(const string& server, const string& client)
{
    RunnablePtr serverRunable;
    TestConfig svrConfig = _config;
    svrConfig.type = TestConfigTypeServer;
    serverRunable = new Runnable(this, _test->prefix + server, _dlls, svrConfig);
    serverRunable->start();
    serverRunable->getThreadControl().detach();
    serverRunable->waitForStart();

    TestConfig cltConfig = _config;
    cltConfig.type = TestConfigTypeClient;
    RunnablePtr clientRunable = new Runnable(this, _test->prefix + client, _dlls, cltConfig);
    clientRunable->start();
    clientRunable->getThreadControl().detach();

    try
    {
        clientRunable->waitForCompleted();
    }
    catch(...)
    {
        if(serverRunable)
        {
            serverRunable->shutdown();
            serverRunable->waitForCompleted();
        }
        throw;
    }

    if(serverRunable)
    {
        serverRunable->waitForCompleted();
    }
}

string
createProxy(const string id, TestConfig config)
{
    ostringstream os;
    os << id << ":" << config.protocol << " -t 10000 -h " << config.host << " -p ";
    if(config.protocol == "tcp")
    {
        os << 15000;
    }
    else if(config.protocol == "ssl")
    {
        os << 15001;
    }
    else if(config.protocol == "ws")
    {
        os << 15002;
    }
    else if(config.protocol == "wss")
    {
        os << 15003;
    }
    return os.str();
}

void
TestRunner::runClientServerTestWithRemoteServer(const string& client)
{
    ControllerPrxPtr controller = ICE_UNCHECKED_CAST(ControllerPrx,
        _communicator->stringToProxy(createProxy("controller", _config)));
    StringSeq options;
    if(_config.serialize)
    {
        options.push_back("Ice.ThreadPool.Server.Serialize=1");
    }

    if(_config.ipv6)
    {
        options.push_back("Ice.IPv4=1");
        options.push_back("Ice.IPv6=1");
        options.push_back("Ice.PreferIPv6Address=1");
    }
    else
    {
        options.push_back("Ice.IPv4=1");
        options.push_back("Ice.IPv6=0");
    }

    vector<TestConfigurationPtr> configurations;
    if(_test->configurations.empty())
    {
        TestConfigurationPtr configuration(new TestConfiguration());
        configurations.push_back(configuration);
    }
    else
    {
        configurations = _test->configurations;
    }

    for(vector<TestConfigurationPtr>::const_iterator i = configurations.begin(); i != configurations.end(); ++i)
    {
        TestConfigurationPtr configuration = *i;

        if(!configuration->languages.empty() &&
            find(configuration->languages.begin(),
                 configuration->languages.end(), _config.server) == configuration->languages.end())
        {
            continue;
        }

        const string desc = configuration->desc.empty() ? "default configuration" : configuration->desc;

        if(configuration->localOnly)
        {
            printLineToConsoleOutput("*** Skipping remote test with " + desc);
            continue;
        }

        RunnablePtr serverRunable;
        ServerPrxPtr server = controller->runServer(_config.server, _test->name, _config.protocol, _config.host, true,
                                                 configuration->configName, options);

        server = ICE_UNCHECKED_CAST(ServerPrx,
            _communicator->stringToProxy(createProxy(server->ice_getIdentity().name, _config)));

        printLineToConsoleOutput("*** Running test with " + desc);
        server->waitForServer();

        TestConfig cltConfig = _config;
        cltConfig.type = TestConfigTypeClient;
        RunnablePtr runnable = new Runnable(this, _test->prefix + client, _dlls, cltConfig, configuration->options);
        runnable->start();
        runnable->getThreadControl().detach();

        try
        {
            runnable->waitForCompleted();
        }
        catch(...)
        {
            if(server)
            {
                try
                {
                    server->terminate();
                }
                catch(const Ice::LocalException&)
                {
                }
            }
            throw;
        }
    }
}

void
TestRunner::runClientTest(const string& client, bool collocated)
{
    TestConfig cltConfig = _config;
    cltConfig.type = collocated ? TestConfigTypeColloc : TestConfigTypeClient;
    RunnablePtr clientRunable = new Runnable(this, _test->prefix + client, _dlls, cltConfig);
    clientRunable->start();
    clientRunable->getThreadControl().detach();
    clientRunable->waitForCompleted();
}

}

TestConfiguration::TestConfiguration(const string& desc, const string& configName, bool localOnly,
                                     const vector<string>& options, const vector<string>& languages) :
    desc(desc),
    configName(configName),
    localOnly(localOnly),
    options(options),
    languages(languages)
{
}

TestCase::TestCase(const string& module, const string& name, const string& client, const string& server,
                   const string& serverAMD, const string& collocated) :
    name(module + "/" + name),
    prefix(module + "_" + name + "_"),
    client(client),
    server(server),
    serverAMD(serverAMD),
    collocated(collocated),
    sslSupport(!isIn(noSSL, name)),
    ipv6Support(!isIn(noIPv6, name)),
    wsSupport(!isIn(noWS, name))
{
}

HINSTANCE
DllCache::loadDll(const std::string& name)
{
    map<string, HINSTANCE>::const_iterator p = _dlls.find(name);
    if(p != _dlls.end())
    {
        return p->second;
    }
    HINSTANCE hnd = LoadPackagedLibrary(IceUtil::stringToWstring(name).c_str(), 0);
    _dlls.insert(make_pair(name, hnd));

    return hnd;
}

DllCache::~DllCache()
{
    for(map<string, HINSTANCE>::const_iterator p = _dlls.begin(); p != _dlls.end(); ++p)
    {
        FreeLibrary(p->second);
    }
}

MainPage::MainPage() :
    _names(ref new Vector<String^>()),
    _protocols(ref new Vector<String^>()),
    _messages(ref new Vector<String^>())
{
    InitializeComponent();
    Ice::registerIceSSL();
}

void
MainPage::Tests_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    _tests = findChild<ListBox>(this, "ListBox");
    _tests->ItemsSource = _names;
}

void
MainPage::Output_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    _output = findChild<ListBox>(this, "Output");
    _output->ItemsSource = _messages;
}

void
MainPage::Configuration_Loaded(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    _language = findChild<ComboBox>(this, "Language");

    _host = findChild<TextBox>(this, "Host");
    _host->IsEnabled = false;

    _protocol = findChild<ComboBox>(this, "Protocol");
    _loop = findChild<CheckBox>(this, "Loop");
    _serialize = findChild<CheckBox>(this, "Serialize");
    _ipv6 = findChild<CheckBox>(this, "IPv6");
    _run = findChild<Button>(this, "Run");
    _stop = findChild<Button>(this, "Stop");
    _stop->IsEnabled = false;

    initializeSupportedProtocols();
    initializeSupportedTests();
}

void
MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
    (void) e;   // Unused parameter
}

void
MainPage::failed(String^ msg)
{
    printToConsoleOutput(ref new String(L"Test failed"), false);
    printToConsoleOutput(msg, true);
    completed();
}

void
MainPage::completed()
{
    Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(
        [=] ()
        {
            if(!_loop->IsChecked->Value)
            {
                _tests->IsEnabled = true;
                _protocol->IsEnabled = true;
                if(selectedLanguage() != "winrt")
                {
                    _host->IsEnabled = true;
                }
                _language->IsEnabled = true;
                _loop->IsEnabled = true;
                _serialize->IsEnabled = true;
                _ipv6->IsEnabled = true;
                _run->IsEnabled = true;
                _stop->Content = "Stop";
                return;
            }
            _tests->IsEnabled = true;
            if(_tests->SelectedIndex == _allTests.size() - 1)
            {
                _tests->SelectedIndex = 0;
            }
            else
            {
                _tests->SelectedIndex++;
            }
            _tests->IsEnabled = false;
            runSelectedTest();
        }));
}

void
MainPage::btnRun_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    if(_tests->SelectedIndex >= 0 && _tests->SelectedIndex < static_cast<int>(_allTests.size()))
    {
        _tests->IsEnabled = false;
        _protocol->IsEnabled = false;
        _host->IsEnabled = false;
        _language->IsEnabled = false;
        _loop->IsEnabled = false;
        _serialize->IsEnabled = false;
        _ipv6->IsEnabled = false;
        _run->IsEnabled = false;
        _stop->IsEnabled = _loop->IsChecked->Value;
        _messages->Clear();
        runSelectedTest();
    }
}

void
MainPage::btnStop_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    _loop->IsChecked = false;
    _stop->IsEnabled = false;
    _stop->Content = "Waiting for test to finish...";
}

Ice::CommunicatorPtr
MainPage::communicator()
{
    if(!_communicator)
    {
        _communicator = Ice::initialize();
    }
    return _communicator;
}

void
MainPage::runSelectedTest()
{
    TestConfig config;
    config.protocol = selectedProtocol();
    config.serialize = _serialize->IsChecked->Value;
    config.ipv6 = _ipv6->IsChecked->Value;

    config.server = selectedLanguage();
    config.host = IceUtil::wstringToString(_host->Text->Data());

    TestRunnerPtr t = new TestRunner(_allTests[_tests->SelectedIndex], config, this, communicator(), _dlls);
    t->start();
    t->getThreadControl().detach();
}

void
TestSuite::MainPage::initializeSupportedTests()
{
    _allTests = allTest(selectedLanguage() != "winrt");
    _names->Clear();
    for(vector<TestCasePtr>::const_iterator i = _allTests.begin(); i != _allTests.end(); ++i)
    {
        TestCasePtr test = *i;
        _names->Append(ref new String(IceUtil::stringToWstring(test->name).c_str()));
    }
    _tests->SelectedIndex = 0;
}

void
TestSuite::MainPage::initializeSupportedProtocols()
{
    _protocols->Clear();
    if(selectedLanguage() == "winrt")
    {
        _protocols->Append("tcp");
        _protocols->Append("ws");
    }
    else
    {
        _protocols->Append("tcp");
        _protocols->Append("ssl");
        _protocols->Append("ws");
        _protocols->Append("wss");
    }

    _protocol->ItemsSource = _protocols;
    _protocol->SelectedIndex = 0;
}

std::string
TestSuite::MainPage::selectedProtocol()
{
    if(selectedLanguage() == "winrt")
    {
        const char* protocols[] = { "tcp", "ws" };
        assert(_protocol->SelectedIndex < sizeof(protocols) / sizeof(const char*));
        return protocols[_protocol->SelectedIndex];
    }
    else
    {
        const char* protocols[] = { "tcp", "ssl", "ws", "wss" };
        assert(_protocol->SelectedIndex < sizeof(protocols) / sizeof(const char*));
        return protocols[_protocol->SelectedIndex];
    }
}
std::string
TestSuite::MainPage::selectedLanguage()
{
    static const char* languages[] = {"winrt", "cpp", "csharp", "java"};
    assert(_language->SelectedIndex < sizeof(languages) / sizeof(const char*));
    return languages[_language->SelectedIndex];
}

void
TestSuite::MainPage::Language_SelectionChanged(Platform::Object^ sender, SelectionChangedEventArgs^ e)
{
    if(_language)
    {
        _host->IsEnabled = selectedLanguage() != "winrt";
        initializeSupportedTests();
        initializeSupportedProtocols();
    }
}

void
TestSuite::MainPage::printToConsoleOutput(String^ message, bool newline)
{
    _output->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(
        [=]()
    {
        if(newline)
        {
            _messages->Append(ref new String(L""));
        }

        if(_messages->Size > 0)
        {
            String^ item = _messages->GetAt(_messages->Size - 1);
            wstring s(item->Data());
            s += wstring(message->Data());
            _messages->SetAt(_messages->Size - 1, ref new String(s.c_str()));
        }
        else
        {
            _messages->Append(message);
        }

        _output->SelectedIndex = _messages->Size - 1;
        _output->ScrollIntoView(_output->SelectedItem);
    }, CallbackContext::Any));
}
