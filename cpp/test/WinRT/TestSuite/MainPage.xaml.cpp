// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "MainPage.xaml.h"
#include <IceUtil/StringUtil.h>
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
    bool ssl;
    bool serialize;
    bool ipv6;
};

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
            printLineToConsoleOutput("failed to load `" + _test + "': " + IceUtilInternal::lastErrorToString());
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

        if(_config.ssl)
        {
            args.push_back("--Ice.Default.Protocol=ssl");
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

struct TestCase
{
    string name;
    string prefix;
    const char* client;
    const char* server;
    const char* serverAMD;
    const char* collocated;
    bool sslSupport;
    bool ipv6Support;
};

}
static const TestCase allTest[] =
{
    {"Ice\\adapterDeactivation", "Ice_adapterDeactivation_", "client.dll", "server.dll", 0, "collocated.dll", true, 
     true },
    {"Ice\\ami", "Ice_ami_", "client.dll", "server.dll", 0, 0, true, true },
    {"Ice\\binding", "Ice_binding_", "client.dll", "server.dll", 0, 0, true, true },
    {"Ice\\dispatcher", "Ice_dispatcher_", "client.dll", "server.dll", 0, 0, true, true },
    {"Ice\\exceptions", "Ice_exceptions_", "client.dll", "server.dll", "serveramd.dll", "collocated.dll", true, true },
    {"Ice\\facets", "Ice_facets_", "client.dll", "server.dll", 0, "collocated.dll", true, true },
    {"Ice\\hold", "Ice_hold_", "client.dll", "server.dll", 0, 0, true, true },
    {"Ice\\info", "Ice_info_", "client.dll", "server.dll", 0, 0, true, true },
    {"Ice\\inheritance", "Ice_inheritance_", "client.dll", "server.dll", 0, "collocated.dll", true, true },
    {"Ice\\invoke", "Ice_invoke_", "client.dll", "server.dll", 0, 0, true, true },
    {"Ice\\location", "Ice_location_", "client.dll", "server.dll", 0, 0, true, true },
    {"Ice\\objects", "Ice_objects_", "client.dll", "server.dll", 0, "collocated.dll", true, true },
    {"Ice\\operations", "Ice_operations_", "client.dll", "server.dll", "serveramd.dll", "collocated.dll", true, true },
    {"Ice\\proxy", "Ice_proxy_", "client.dll", "server.dll", "serveramd.dll", "collocated.dll", true, true },
    {"Ice\\retry", "Ice_retry_", "client.dll", "server.dll", 0, 0, true, true },
    {"Ice\\stream", "Ice_stream_", "client.dll", 0, 0, 0, true, true },
    {"Ice\\timeout", "Ice_timeout_", "client.dll", "server.dll", 0, 0, true, true },
    {"Ice\\udp", "Ice_udp_", "client.dll", "server.dll", 0, 0, false, true },
    {"Ice\\hash", "Ice_hash_", "client.dll", 0, 0, 0, true, true },
    {"Ice\\metrics", "Ice_metrics_", "client.dll", "server.dll", "serveramd.dll", 0, false, false },
    {"Ice\\optional", "Ice_optional_", "client.dll", "server.dll", 0, 0, true, true },
    {"Ice\\admin", "Ice_admin_", "client.dll", "server.dll", 0, 0, true, true },
    {"Ice\\enums", "Ice_enums_", "client.dll", "server.dll", 0, 0, true, true }
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
            if(_config.ssl && !_test.sslSupport)
            {
                printLineToConsoleOutput("**** test " + _test.name + " not supported with SSL");   
            }
            else if(_config.ipv6 && !_test.ipv6Support)
            {
                printLineToConsoleOutput("**** test " + _test.name + " not supported with IPv6");
            }
            else if(_config.ssl && _config.ipv6)
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
                if(_test.collocated && !_config.ssl)
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
        if(!_config.ssl)
        {
            TestConfig svrConfig = _config;
            svrConfig.type = TestConfigTypeServer;
            serverRunable = new Runnable(_test.prefix + server, svrConfig);
            serverRunable->start();
            serverRunable->waitForStart();
            serverRunable->getThreadControl().detach();
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
    TestConfig config;
    config.ssl = chkSSL->IsChecked->Value;
    config.serialize = chkSerialize->IsChecked->Value;
    config.ipv6 = chkIPv6->IsChecked->Value;

    TestRunnerPtr t = new TestRunner(allTest[TestList->SelectedIndex], config);
    t->start();
    t->getThreadControl().detach();
}
