// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "MainPage.xaml.h"
#include <TestCommon.h>
#include <string>
#include <iostream>
#include <Ice/Ice.h>
using namespace std;
using namespace TestSuite;

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

TextBlock^ output = nullptr;
ScrollViewer^ scroller = nullptr;
MainPage^ page = nullptr;

namespace
{

void
printToConsoleOutput(const std::string& message)
{
    assert(output != nullptr);
    String^ msg = ref new String(IceUtil::stringToWstring(message).c_str());
    output->Dispatcher->RunAsync(CoreDispatcherPriority::Normal,
                                 ref new DispatchedHandler(
                                     [msg] ()
                                     {
                                         output->Text += msg;
                                         output->UpdateLayout();
#if (_WIN32_WINNT > 0x0602)
                                         scroller->ChangeView(nullptr, scroller->ScrollableHeight, nullptr);
#else
                                         scroller->ScrollToVerticalOffset(scroller->ScrollableHeight);
#endif
                                     }, CallbackContext::Any));
}

void
printLineToConsoleOutput(const std::string& msg)
{
    printToConsoleOutput(msg + '\n');
}

typedef int (*MAIN_ENTRY_POINT)(int, char**, Test::MainHelper*);

typedef int (*SHUTDOWN_ENTRY_POINT)();

enum TestConfigType { TestConfigTypeClient, TestConfigTypeServer, TestConfigTypeColloc };

struct TestConfig
{
    TestConfigType type;
    string protocol;
    bool serialize;
    bool ipv6;
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

class Runnable : public IceUtil::Thread, public Test::MainHelper
{
public:

    Runnable(const string& test, const TestConfig& config) :
        _test(test),
        _config(config),
        _started(false),
        _completed(false),
        _status(0)
    {
    }

    virtual ~Runnable()
    {
        if(_hnd != 0)
        {
            FreeLibrary(_hnd);
        }
    }
    
    virtual void 
    run()
    {
        _hnd = LoadPackagedLibrary(IceUtil::stringToWstring(_test).c_str(), 0);
        if(_hnd == 0)
        {
            ostringstream os;
            os << "failed to load `" << _test + "': error code: " << GetLastError();
            printLineToConsoleOutput(os.str());
            completed(-1);
            return;
        }

        _dllTestShutdown = GetProcAddress(_hnd, "dllTestShutdown");
        if(_dllTestShutdown == 0)
        {
            printLineToConsoleOutput("failed to find dllTestShutdown function from `" + _test + "'");
            completed(-1);
            return;
        }

        FARPROC dllMain = GetProcAddress(_hnd, "dllMain");
        if(dllMain == 0)
        {
            printLineToConsoleOutput("failed to find dllMain function from `" + _test + "'");
            completed(-1);
            return;
        }

        vector<string> args;
        //
        // NOTE: When testing for SSL on a WinRT device, this needs to
        // be set to the IP addresse of the host running the SSL
        // server.
        //
        args.push_back("--Ice.NullHandleAbort=1");
        args.push_back("--Ice.Warn.Connections=1");
        //args.push_back("--Ice.Trace.Network=2");
        //args.push_back("--Ice.Trace.Protocol=2");
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

        args.push_back("--Ice.Default.Protocol=" + _config.protocol);

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
    waitForStart()
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
    
    virtual void 
    waitForCompleted()
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

    //
    // MainHelper implementation
    //

    virtual void 
    serverReady()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
        _started = true;
        _monitor.notify();
    }

    virtual void 
    shutdown()
    {
        if(_dllTestShutdown)
        {
            reinterpret_cast<SHUTDOWN_ENTRY_POINT>(_dllTestShutdown)();
        }
    }
    
    virtual bool
    redirect()
    {
        return _config.type == TestConfigTypeClient || _config.type == TestConfigTypeColloc;
    }

    virtual void
    print(const string& message)
    {
        printToConsoleOutput(message);
    }

    int
    status()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
        return _status;
    }

private:

    void 
    completed(int status)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
        _completed = true;
        _status = status;
        _monitor.notify();
    }

    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    string _test;
    TestConfig _config;
    bool _started;
    bool _completed;
    int _status;
    Ice::CommunicatorPtr _communicator;
    FARPROC _dllTestShutdown;
    HINSTANCE _hnd;
};
typedef IceUtil::Handle<Runnable> RunnablePtr;

bool 
isIn(const string s[], const string& name)
{
    int l = sizeof(s)/sizeof(string*);
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

struct TestCase
{
    TestCase(const string& module, const string& name, const char* client, const char* server, 
             const char* serverAMD = 0, const char* collocated = 0) :
        name(module + "\\" + name),
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

    string name;
    string prefix;
    const char* client;
    const char* server;
    const char* serverAMD;
    const char* collocated;
    bool sslSupport;
    bool ipv6Support;
    bool wsSupport;
};

static const TestCase allTest[] =
{
    TestCase("Ice", "adapterDeactivation", "client.dll", "server.dll", 0, "collocated.dll"),
    TestCase("Ice", "ami", "client.dll", "server.dll", 0, 0),
    TestCase("Ice", "binding", "client.dll", "server.dll", 0, 0),
    TestCase("Ice", "dispatcher", "client.dll", "server.dll", 0, 0),
    TestCase("Ice", "exceptions", "client.dll", "server.dll", "serveramd.dll", "collocated.dll"),
    TestCase("Ice", "facets", "client.dll", "server.dll", 0, "collocated.dll"),
    TestCase("Ice", "hold", "client.dll", "server.dll", 0, 0),
    TestCase("Ice", "info", "client.dll", "server.dll", 0, 0),
    TestCase("Ice", "inheritance", "client.dll", "server.dll", 0, "collocated.dll"),
    TestCase("Ice", "invoke", "client.dll", "server.dll", 0, 0),
    TestCase("Ice", "location", "client.dll", "server.dll", 0, 0),
    TestCase("Ice", "objects", "client.dll", "server.dll", 0, "collocated.dll"),
    TestCase("Ice", "operations", "client.dll", "server.dll", "serveramd.dll", "collocated.dll"),
    TestCase("Ice", "proxy", "client.dll", "server.dll", "serveramd.dll", "collocated.dll"),
    TestCase("Ice", "retry", "client.dll", "server.dll", 0, 0),
    TestCase("Ice", "stream", "client.dll", 0, 0, 0),
    TestCase("Ice", "timeout", "client.dll", "server.dll", 0, 0),
    TestCase("Ice", "acm", "client.dll", "server.dll", 0, 0),
    TestCase("Ice", "udp", "client.dll", "server.dll", 0, 0),
    TestCase("Ice", "hash", "client.dll", 0, 0, 0),
    TestCase("Ice", "metrics", "client.dll", "server.dll", "serveramd.dll", 0),
    TestCase("Ice", "optional", "client.dll", "server.dll", 0, 0),
    TestCase("Ice", "admin", "client.dll", "server.dll", 0, 0),
    TestCase("Ice", "enums", "client.dll", "server.dll", 0, 0),
};

class TestRunner : public IceUtil::Thread
{
public:

    TestRunner(const TestCase& test, const TestConfig& config) : _test(test), _config(config)
    {
    }

    virtual void
    run()
    {
        try
        {
            if(configUseWS(_config) && !_test.wsSupport)
            {
                printLineToConsoleOutput("**** test " + _test.name + " not supported with WS");   
            }
            else if(configUseSSL(_config) && !_test.sslSupport)
            {
                printLineToConsoleOutput("**** test " + _test.name + " not supported with SSL");   
            }
            else if(_config.ipv6 && !_test.ipv6Support)
            {
                printLineToConsoleOutput("**** test " + _test.name + " not supported with IPv6");
            }
            else if(configUseSSL(_config) && _config.ipv6)
            {
                printLineToConsoleOutput("**** test " + _test.name + " not supported with IPv6 SSL");
            }
            else
            {
                if(_test.server)
                {
                    printLineToConsoleOutput("**** running test " + _test.name);
                    runClientServerTest(_test.server, _test.client);        
                    printLineToConsoleOutput("");
                }
                else
                {
                    assert(_test.client);
                    printLineToConsoleOutput("**** running test " + _test.name);
                    runClientTest(_test.client, false);
                    printLineToConsoleOutput("");
                }

                if(_test.serverAMD)
                {
                    printLineToConsoleOutput("*** running test with AMD server " + _test.name);
                    runClientServerTest(_test.server, _test.client);
                    printLineToConsoleOutput("");
                }

                //
                // Don't run collocated tests with SSL as there isn't SSL server side.
                //
                if(_test.collocated && !configUseSSL(_config))
                {
                    printLineToConsoleOutput("*** running collocated test " + _test.name);
                    runClientTest(_test.collocated, true);
                    printLineToConsoleOutput("");
                }
            }

            page->completed();
        }
        catch(Platform::Exception^ ex)
        {
            page->failed(ex->Message);
        }
        catch(const std::exception& ex)
        {
            page->failed(ref new String(IceUtil::stringToWstring(ex.what()).c_str()));
        }
        catch(const string& ex)
        {
            page->failed(ref new String(IceUtil::stringToWstring(ex).c_str()));
        }
        catch(...)
        {
            page->failed("unknown exception");
        }
    }
    
    void 
    runClientServerTest(const string& server, const string& client)
    {
        RunnablePtr serverRunable;
        if(!configUseSSL(_config))
        {
            TestConfig svrConfig = _config;
            svrConfig.type = TestConfigTypeServer;
            serverRunable = new Runnable(_test.prefix + server, svrConfig);
            serverRunable->start();
            serverRunable->getThreadControl().detach();
            serverRunable->waitForStart();
        }

        TestConfig cltConfig = _config;
        cltConfig.type = TestConfigTypeClient;
        RunnablePtr clientRunable = new Runnable(_test.prefix + client, cltConfig);
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
    runClientTest(const string& client, bool collocated)
    {
        TestConfig cltConfig = _config;
        cltConfig.type = collocated ? TestConfigTypeColloc : TestConfigTypeClient;
        RunnablePtr clientRunable = new Runnable(_test.prefix + client, cltConfig);
        clientRunable->start();
        clientRunable->getThreadControl().detach();
        clientRunable->waitForCompleted();
    }
    
private:

    TestCase _test;
    TestConfig _config;
};

typedef IceUtil::Handle<TestRunner> TestRunnerPtr;

}

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

MainPage::MainPage()
{
    _names = ref new Vector<String^>();
    for(int i = 0; i < sizeof(allTest)/sizeof(allTest[0]); ++i)
    {
        _names->Append(ref new String(IceUtil::stringToWstring(allTest[i].name).c_str()));
    }
    InitializeComponent();
    page = this;
    output = Output;
    scroller = Scroller;
    TestList->ItemsSource = _names;
    TestList->SelectedIndex = 0;
}

/// <summary>
/// Invoked when this page is about to be displayed in a Frame.
/// </summary>
/// <param name="e">Event data that describes how this page was reached.  The Parameter
/// property is typically used to configure the page.</param>

void 
MainPage::OnNavigatedTo(NavigationEventArgs^ e)
{
    (void) e;   // Unused parameter
}


void
MainPage::failed(String^ msg)
{
    printLineToConsoleOutput("Test failed");
    printLineToConsoleOutput(IceUtil::wstringToString(msg->Data()));
    completed();
}

void 
MainPage::completed()
{
    page->Dispatcher->RunAsync(
        CoreDispatcherPriority::Normal, 
        ref new DispatchedHandler([=] ()
                                  {
                                      if(!chkLoop->IsChecked->Value)
                                      {
                                          TestList->IsEnabled = true;
                                          btnRun->IsEnabled = true;
                                          return;
                                      }                                      
                                      TestList->IsEnabled = true;
                                      if(TestList->SelectedIndex == (sizeof(allTest)/sizeof(allTest[0])) -1)
                                      {
                                          TestList->SelectedIndex = 0;
                                      }
                                      else
                                      {
                                          TestList->SelectedIndex++;
                                      }
                                      TestList->IsEnabled = false;
                                      runSelectedTest();
                                  }));
}

void 
MainPage::btnRun_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
    if(TestList->SelectedIndex >= 0 && TestList->SelectedIndex < sizeof(allTest)/sizeof(allTest[0]))
    {
        TestList->IsEnabled = false;
        btnRun->IsEnabled = false;
        output->Text = "";
        runSelectedTest();
    }
}

void 
MainPage::runSelectedTest()
{
    static const string protocols[] = { "tcp", "ssl", "ws", "wss" };
    TestConfig config;
    config.protocol = protocols[protocol->SelectedIndex];
    config.serialize = chkSerialize->IsChecked->Value;
    config.ipv6 = chkIPv6->IsChecked->Value;

    TestRunnerPtr t = new TestRunner(allTest[TestList->SelectedIndex], config);
    t->start();
    t->getThreadControl().detach();
}
