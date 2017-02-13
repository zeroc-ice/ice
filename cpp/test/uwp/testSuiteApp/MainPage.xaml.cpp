// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

enum TestType { TestTypeClient, TestTypeServer };

struct TestConfig
{
    string protocol;
    bool serialize;
    bool ipv6;

    string mapping;
    string host;
};

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
    "Ice/udp",
    "Ice/metrics"
};

static const string noWS[] =
{
    "Ice/metrics"
};

static const string noIPv6[] =
{
    "Ice/metrics"
};

bool
canRun(const TestSuiteDesc& test, const TestConfig& config)
{
    if(isIn(noWS, test.id) && (config.protocol == "ws" || config.protocol == "wss"))
    {
        return false;
    }

    if(isIn(noSSL, test.id) && (config.protocol == "ssl" || config.protocol == "wss"))
    {
        return false;
    }

    if(isIn(noIPv6, test.id) && config.ipv6)
    {
        return false;
    }

    return true;
}

vector<TestSuiteDesc> allTests(string remoteServerLang)
{
    vector<TestCaseDesc> clientServerAndCollocated { { "client/server" }, { "collocated" } };
    vector<TestCaseDesc> clientOnly { { "client" } };

    vector<TestSuiteDesc> all {
        { "Ice/acm" },
        { "Ice/adapterDeactivation", clientServerAndCollocated },
        { "Ice/admin" },
        { "Ice/ami", clientServerAndCollocated },
        { "Ice/binding" },
        { "Ice/dispatcher", clientServerAndCollocated },
        { "Ice/enums", {
                { "client/server with default encoding" },
                { "client/server with 1.0 encoding", "", "", { "--Ice.Default.EncodingVersion=1.0" } },
            }
        },
        { "Ice/exceptions", {
                { "client/server with compact format" },
                { "client/server with sliced format", "", "", { "--Ice.Default.SlicedFormat" } },
                { "client/server with 1.0 encoding", "", "", { "--Ice.Default.EncodingVersion=1.0" } },
                { "client/amd server with compact format" },
                { "client/amd server with sliced format", "", "", { "--Ice.Default.SlicedFormat" } },
                { "client/amd server with 1.0 encoding", "", "", { "--Ice.Default.EncodingVersion=1.0" } },
                { "collocated" }
            }
        },
        { "Ice/facets", clientServerAndCollocated },
        { "Ice/hash", clientOnly },
        { "Ice/hold" },
        { "Ice/info" },
        { "Ice/inheritance", clientServerAndCollocated },
        { "Ice/invoke" },
        { "Ice/location" },
        { "Ice/objects", {
                { "client/server with compact format" },
                { "client/server with sliced format", "", "", { "--Ice.Default.SlicedFormat" } },
                { "client/server with 1.0 encoding", "", "", { "--Ice.Default.EncodingVersion=1.0" } },
                { "collocated" }
            }
        },
        { "Ice/operations", {
                { "client/server" },
                { "client/amd server" },
                { "collocated" }
            }
        },
        { "Ice/optional",  {
                { "client/server with compact format" },
                { "client/server with sliced format", "", "", { "--Ice.Default.SlicedFormat" } },
                { "collocated" }
            }
        },
        { "Ice/proxy", {
                { "client/server" },
                { "client/amd server" },
                { "collocated" }
            }
        },
        { "Ice/retry" },
        { "Ice/stream", clientOnly },
        { "Ice/timeout" },
    };

    if(remoteServerLang == "uwp")
    {
        all.push_back({ "Ice/metrics", {
                { "client/server" },
                { "client/amd server" },
                { "collocated" }
            }
        });
        all.push_back({ "Ice/udp" });
    }
    else if(remoteServerLang == "cpp")
    {
        all.push_back({ "IceSSL/configuration", {
                { "client/server", "", "", { string(TESTSUITE_DIR) + "/cpp/test/IceSSL/configuration" } }
            }
        });
    }
    return all;
}

class TestRunner;
typedef IceUtil::Handle<TestRunner> TestRunnerPtr;

class Runnable : public IceUtil::Thread, public Test::MainHelper
{
public:

    Runnable(const TestRunnerPtr&, const string&, DllCache&, TestType, const TestConfig&,
             const vector<string>& = vector<string>());

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
    string _dll;
    TestType _type;
    TestConfig _config;
    bool _started;
    bool _completed;
    int _status;
    Ice::CommunicatorPtr _communicator;
    FARPROC _dllTestShutdown;
    DllCache& _dlls;
    vector<string> _args;
};
typedef IceUtil::Handle<Runnable> RunnablePtr;

class TestRunner : public IceUtil::Thread
{
public:

    TestRunner(TestSuiteDesc&, const TestConfig&, MainPage^, const Ice::CommunicatorPtr&, DllCache&);

    virtual void run();

    void runClientServerTest(const TestCaseDesc&);
    void runClientServerTestWithRemoteServer(const TestCaseDesc&);
    void runClientTest(const TestCaseDesc&);

    void printToConsoleOutput(const std::string&);
    void printLineToConsoleOutput(const std::string&);

private:

    TestSuiteDesc& _testSuite;
    TestConfig _config;
    MainPage^ _page;
    Ice::CommunicatorPtr _communicator;
    DllCache& _dlls;
};

Runnable::Runnable(const TestRunnerPtr& runner,
                   const string& dll,
                   DllCache& dlls,
                   TestType type,
                   const TestConfig& config,
                   const vector<string>& args) :
    _runner(runner),
    _dll(dll),
    _type(type),
    _config(config),
    _started(false),
    _completed(false),
    _status(0),
    _dlls(dlls),
    _args(args)
{
}

Runnable::~Runnable()
{
}

void
Runnable::run()
{
    HINSTANCE hnd = _dlls.loadDll(_dll);
    if(hnd == 0)
    {
        ostringstream os;
        os << "failed to load `" << _dll << "': error code: " << GetLastError();
        _runner->printLineToConsoleOutput(os.str());
        completed(-1);
        return;
    }

    _dllTestShutdown = GetProcAddress(hnd, "dllTestShutdown");
    if(_dllTestShutdown == 0)
    {
        _runner->printLineToConsoleOutput("failed to find dllTestShutdown function from `" + _dll + "'");
        completed(-1);
        return;
    }

    FARPROC dllMain = GetProcAddress(hnd, "dllMain");
    if(dllMain == 0)
    {
        _runner->printLineToConsoleOutput("failed to find dllMain function from `" + _dll + "'");
        completed(-1);
        return;
    }

    vector<string> args;
    args.push_back (_dll);
    args.push_back("--Ice.NullHandleAbort=1");
    args.push_back("--Ice.Warn.Connections=1");
    args.push_back("--Ice.ProgramName=" + _dll);
    if(_config.serialize)
    {
        args.push_back("--Ice.ThreadPool.Server.Serialize=1");
    }

    if(_config.ipv6)
    {
        args.push_back("--Ice.IPv4=1");
        args.push_back("--Ice.IPv6=1");
        args.push_back("--Ice.PreferIPv6Address=1");
    }
    else
    {
        args.push_back("--Ice.IPv4=1");
        args.push_back("--Ice.IPv6=0");
    }

    if(_type != TestTypeClient)
    {
        args.push_back("--Ice.ThreadPool.Server.Size=1");
        args.push_back("--Ice.ThreadPool.Server.SizeMax=3");
        args.push_back("--Ice.ThreadPool.Server.SizeWarn=0");
    }

    if(_config.protocol == "ssl" || _config.protocol == "wss")
    {
        args.push_back("--IceSSL.CertFile=ms-appx:///client.p12");
        args.push_back("--IceSSL.Password=password");
    }

    if(_config.host.empty())
    {
        if(_config.ipv6)
        {
            args.push_back("--Ice.Default.Host=0:0:0:0:0:0:0:1");
        }
        else
        {
            args.push_back("--Ice.Default.Host=127.0.0.1");
        }
    }
    else
    {
        args.push_back("--Ice.Default.Host=" + _config.host);
    }
    args.push_back("--Ice.Default.Protocol=" + _config.protocol);

    for(vector<string>::const_iterator i = _args.begin(); i != _args.end(); i++)
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
        print("unexpected exception while running `" + _dll + "':\n" + ex.what());
    }
    catch(...)
    {
        print("unexpected unknown exception while running `" + _dll + "'");
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
        os << "server failed with status = " << _status;
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
        os << "client failed with status = " << _status;
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
    return _type == TestTypeClient;
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

TestRunner::TestRunner(TestSuiteDesc& testSuite,
                       const TestConfig& config,
                       MainPage^ page,
                       const Ice::CommunicatorPtr& communicator,
                       DllCache& dlls) :
    _testSuite(testSuite),
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
        _page->printToConsoleOutput(ref new String(Ice::stringToWstring(*i).c_str()), i != lines.begin());
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
        if(!canRun(_testSuite, _config))
        {
            printLineToConsoleOutput("**** test " + _testSuite.id + " not supported with this config");
        }
        else
        {
            if(_testSuite.testCases.empty())
            {
                _testSuite.testCases.push_back({ "client/server" });
            }

            printLineToConsoleOutput("**** Running " + _testSuite.id + " tests");
            for(auto p = _testSuite.testCases.begin(); p != _testSuite.testCases.end(); ++p)
            {
                if(p->client.empty())
                {
                    if(p->name.find("client/server") == 0)
                    {
                        p->client = "client.dll";
                        p->server = "server.dll";
                    }
                    else if(p->name.find("client/amd server") == 0)
                    {
                        p->client = "client.dll";
                        p->server = "serveramd.dll";
                    }
                    else if(p->name == "collocated")
                    {
                        p->client = "collocated.dll";
                    }
                    else
                    {
                        p->client = "client.dll";
                    }
                }

                printLineToConsoleOutput("[ running " + p->name + " test ]");
                assert(!p->client.empty());
                if(!p->server.empty())
                {
                    if(_config.mapping == "uwp")
                    {
                        runClientServerTest(*p);
                    }
                    else
                    {
                        runClientServerTestWithRemoteServer(*p);
                    }
                }
                else
                {
                    runClientTest(*p);
                }
            }
        }
        _page->completed();
    }
    catch(Platform::Exception^ ex)
    {
        _page->failed(ex->Message);
    }
    catch(const TestCaseFailedException& ex)
    {
        _page->failed(ref new String(Ice::stringToWstring("testcase failed:\n" + ex.output).c_str()));
    }
    catch(const TestCaseNotExistException& ex)
    {
        _page->failed(ref new String(Ice::stringToWstring("testcase not found:\n" + ex.reason).c_str()));
    }
    catch(const std::exception& ex)
    {
        _page->failed(ref new String(Ice::stringToWstring(ex.what()).c_str()));
    }
    catch(const string& ex)
    {
        _page->failed(ref new String(Ice::stringToWstring(ex).c_str()));
    }
    catch(...)
    {
        _page->failed("unknown exception");
    }
}

void
TestRunner::runClientServerTest(const TestCaseDesc& testCase)
{
    string prefix = _testSuite.id + '_';
    replace(prefix.begin(), prefix.end(), '/', '_');

    RunnablePtr serverRunable = new Runnable(this, prefix + testCase.server, _dlls, TestTypeServer, _config);
    serverRunable->start();
    serverRunable->getThreadControl().detach();
    serverRunable->waitForStart();

    RunnablePtr clientRunable = new Runnable(this, prefix + testCase.client, _dlls, TestTypeClient, _config);
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

void
TestRunner::runClientServerTestWithRemoteServer(const TestCaseDesc& testCase)
{
    ostringstream os;
    os << "controller: tcp -t 10000 -h " << _config.host << " -p 15000";

    shared_ptr<ControllerPrx> controller = Ice::uncheckedCast<ControllerPrx>(_communicator->stringToProxy(os.str()));
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

    //
    // Get the remote test case
    //
    shared_ptr<TestCasePrx> tc = controller->runTestCase("cpp", _testSuite.id, testCase.name, _config.mapping);

    // Create fixed proxy, don't rely on the endpoints published by the controller
    tc = Ice::uncheckedCast<TestCasePrx>(controller->ice_getConnection()->createProxy(tc->ice_getIdentity()));

    shared_ptr<Config> config = make_shared<Config>();
    config->protocol = _config.protocol;
    config->serialize = _config.serialize;
    config->ipv6 = _config.ipv6;

    //
    // Start the server side
    //
    TestConfig clientCfg = _config;
    clientCfg.host = tc->startServerSide(config);

    string dll = _testSuite.id + '_' + testCase.client;
    replace(dll.begin(), dll.end(), '/', '_');

    RunnablePtr runnable = new Runnable(this, dll, _dlls, TestTypeClient, clientCfg, testCase.args);
    runnable->start();
    runnable->getThreadControl().detach();
    try
    {
        runnable->waitForCompleted();
    }
    catch(...)
    {
        try
        {
            printLineToConsoleOutput(tc->stopServerSide(false));
        }
        catch(const Ice::LocalException&)
        {
        }
        throw;
    }
}

void
TestRunner::runClientTest(const TestCaseDesc& testCase)
{
    string dll = _testSuite.id + '_' + testCase.client;
    replace(dll.begin(), dll.end(), '/', '_');

    RunnablePtr clientRunable = new Runnable(this, dll, _dlls, TestTypeClient, _config);
    clientRunable->start();
    clientRunable->getThreadControl().detach();
    clientRunable->waitForCompleted();
}

}

HINSTANCE
DllCache::loadDll(const std::string& name)
{
    map<string, HINSTANCE>::const_iterator p = _dlls.find(name);
    if(p != _dlls.end())
    {
        return p->second;
    }
    HINSTANCE hnd = LoadPackagedLibrary(Ice::stringToWstring(name).c_str(), 0);
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
    _testSuites = findChild<ListBox>(this, "ListBox");
    _testSuites->ItemsSource = _names;
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
                _testSuites->IsEnabled = true;
                _protocol->IsEnabled = true;
                if(selectedLanguage() != "uwp")
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
            _testSuites->IsEnabled = true;
            if(_testSuites->SelectedIndex == _allTests.size() - 1)
            {
                _testSuites->SelectedIndex = 0;
            }
            else
            {
                _testSuites->SelectedIndex++;
            }
            _testSuites->IsEnabled = false;
            runSelectedTest();
        }));
}

void
MainPage::btnRun_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    if(_testSuites->SelectedIndex >= 0 && _testSuites->SelectedIndex < static_cast<int>(_allTests.size()))
    {
        _testSuites->IsEnabled = false;
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

    config.mapping = selectedLanguage();
    config.host = Ice::wstringToString(_host->Text->Data());

    TestRunnerPtr t = new TestRunner(_allTests[_testSuites->SelectedIndex], config, this, communicator(), _dlls);
    t->start();
    t->getThreadControl().detach();
}

void
TestSuite::MainPage::initializeSupportedTests()
{
    _allTests = allTests(selectedLanguage());
    _names->Clear();
    for(auto i = _allTests.begin(); i != _allTests.end(); ++i)
    {
        _names->Append(ref new String(Ice::stringToWstring(i->id).c_str()));
    }
    _testSuites->SelectedIndex = 0;
}

void
TestSuite::MainPage::initializeSupportedProtocols()
{
    _protocols->Clear();
    if(selectedLanguage() == "uwp")
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
    if(selectedLanguage() == "uwp")
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
    static const char* languages[] = {"uwp", "cpp", "csharp", "java"};
    assert(_language->SelectedIndex < sizeof(languages) / sizeof(const char*));
    return languages[_language->SelectedIndex];
}

void
TestSuite::MainPage::Language_SelectionChanged(Platform::Object^ sender, SelectionChangedEventArgs^ e)
{
    if(_language)
    {
        _host->IsEnabled = selectedLanguage() != "uwp";
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
