// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include "ControllerView.xaml.h"

#include <Ice/Ice.h>
#include <Controller.h>
#include <TestHelper.h>

#include <string>
#include <iostream>
#include <memory>
#include <condition_variable>

using namespace std;
using namespace Test;
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

using namespace Windows::Networking::Connectivity;

using namespace Platform::Collections;

namespace
{

typedef Test::TestHelper* (*CREATE_HELPER_ENTRY_POINT)();

class ControllerHelperI : public Test::ControllerHelper,
                          public enable_shared_from_this<Test::ControllerHelper>
{
public:

    ControllerHelperI(const string&, const StringSeq&);

    virtual std::string loggerPrefix() const;
    virtual void print(const std::string&);
    virtual void serverReady();
    virtual void communicatorInitialized(const shared_ptr<Ice::Communicator>&);

    void run();
    void completed(int);

    int waitSuccess(int) const;
    void waitReady(int) const;
    std::string getOutput() const;
    void join();
    void shutdown();

private:

    string _dll;
    StringSeq _args;
    bool _ready;
    bool _completed;
    int _status;
    ostringstream _out;
    thread _thread;
    mutable mutex _mutex;
    mutable condition_variable _condition;
    //
    // DLL cache
    //
    static std::map<std::string, HINSTANCE> _dlls;
    HINSTANCE loadDll(const string&);

    shared_ptr<Ice::Communicator> _communicator;
};

std::map<std::string, HINSTANCE> ControllerHelperI::_dlls;

class ProcessI : public Process
{
public:

    ProcessI(const shared_ptr<ControllerHelperI>&);

    void waitReady(int, const Ice::Current&);
    int waitSuccess(int, const Ice::Current&);
    string terminate(const Ice::Current&);

private:

    shared_ptr<ControllerHelperI> _controllerHelper;
};

class ProcessControllerI : public ProcessController
{
public:

    ProcessControllerI(ControllerView^);
    shared_ptr<ProcessPrx> start(string, string, StringSeq, const Ice::Current&);
    virtual string getHost(string, bool, const Ice::Current&);

private:

    ControllerView^ _controller;
    string _host;
};

class ControllerI
{
public:

    ControllerI(ControllerView^);
    virtual ~ControllerI();

    void
    registerProcessController(ControllerView^,
                              const shared_ptr<Ice::ObjectAdapter>&,
                              const shared_ptr<ProcessControllerRegistryPrx>&,
                              const shared_ptr<ProcessControllerPrx>&);

private:

    shared_ptr<Ice::Communicator> _communicator;
    shared_ptr<Test::Common::ProcessControllerRegistryPrx> _registry;
};

Test::StreamHelper streamRedirect;

}

ControllerHelperI::ControllerHelperI(const string& dll, const StringSeq& args) :
    _dll(dll),
    _args(args),
    _ready(false),
    _completed(false),
    _status(0)
{
}

string
ControllerHelperI::loggerPrefix() const
{
    return _dll;
}

void
ControllerHelperI::print(const std::string& msg)
{
    _out << msg;
}

void
ControllerHelperI::serverReady()
{
    unique_lock<mutex> lock(_mutex);
    _ready = true;
    _condition.notify_all();
}

void
ControllerHelperI::communicatorInitialized(const shared_ptr<Ice::Communicator>& communicator)
{
    unique_lock<mutex> lock(_mutex);
    _communicator = communicator;
}

void
ControllerHelperI::run()
{
    thread t([this]()
    {
        HINSTANCE hnd = loadDll(_dll);
        if(hnd == 0)
        {
            ostringstream os;
            os << "failed to load `" << _dll << "': error code: " << GetLastError();
            print(os.str());
            completed(1);
            return;
        }

        FARPROC sym = GetProcAddress(hnd, "createHelper");
        if(sym == 0)
        {
            print("failed to find createHelper function from `" + _dll + "'");
            completed(1);
            return;
        }

        CREATE_HELPER_ENTRY_POINT createHelper = reinterpret_cast<CREATE_HELPER_ENTRY_POINT>(sym);
        char** argv = new char*[_args.size() + 1];
        for(unsigned int i = 0; i < _args.size(); ++i)
        {
            argv[i] = const_cast<char*>(_args[i].c_str());
        }
        argv[_args.size()] = 0;

        if(_dll.find("client") != string::npos || _dll.find("collocated") != string::npos)
        {
            streamRedirect.setControllerHelper(this);
        }
        try
        {
            auto helper = unique_ptr<Test::TestHelper>(createHelper());
            helper->setControllerHelper(this);
            helper->run(static_cast<int>(_args.size()), argv);
            completed(0);
        }
        catch(const std::exception& ex)
        {
            print("unexpected exception while running `" + _args[0] + "':\n" + ex.what());
            completed(1);
        }
        catch(...)
        {
            print("unexpected unknown exception while running `" + _args[0] + "'");
            completed(1);
        }
        if(_dll.find("client") != string::npos || _dll.find("collocated") != string::npos)
        {
            streamRedirect.setControllerHelper(0);
        }
        delete[] argv;
    });
    _thread = move(t);
}

void
ControllerHelperI::join()
{
    _thread.join();
}

void
ControllerHelperI::completed(int status)
{
    unique_lock<mutex> lock(_mutex);
    _completed = true;
    _status = status;
    _communicator = nullptr;
    _condition.notify_all();
}

void
ControllerHelperI::waitReady(int timeout) const
{
    unique_lock<mutex> lock(_mutex);
    while(!_ready && !_completed)
    {
        if(_condition.wait_for(lock, chrono::seconds(timeout)) == cv_status::timeout)
        {
            throw ProcessFailedException("timed out waiting for the process to be ready");
        }
    }
    if(_completed && _status == EXIT_FAILURE)
    {
        throw ProcessFailedException(_out.str());
    }
}

int
ControllerHelperI::waitSuccess(int timeout) const
{
    unique_lock<mutex> lock(_mutex);
    while(!_completed)
    {
        if(_condition.wait_for(lock, chrono::seconds(timeout)) == cv_status::timeout)
        {
            throw ProcessFailedException("timed out waiting for the process to succeed");
        }
    }
    return _status;
}

string
ControllerHelperI::getOutput() const
{
    assert(_completed);
    return _out.str();
}

void
ControllerHelperI::shutdown()
{
    unique_lock<mutex> lock(_mutex);
    if(_communicator)
    {
        _communicator->shutdown();
    }
}

HINSTANCE
ControllerHelperI::loadDll(const string& name)
{
    unique_lock<mutex> lock(_mutex);
    map<string, HINSTANCE>::iterator p = _dlls.find(name);
    if (p == _dlls.end())
    {
        HINSTANCE hnd = LoadPackagedLibrary(Ice::stringToWstring(name).c_str(), 0);
        p = _dlls.insert(make_pair(name, hnd)).first;
    }
    return p->second;
}

ProcessI::ProcessI(const shared_ptr<ControllerHelperI>& controllerHelper) :
    _controllerHelper(controllerHelper)
{
}

void
ProcessI::waitReady(int timeout, const Ice::Current&)
{
    _controllerHelper->waitReady(timeout);
}

int
ProcessI::waitSuccess(int timeout, const Ice::Current&)
{
    return _controllerHelper->waitSuccess(timeout);
}

string
ProcessI::terminate(const Ice::Current& current)
{
    _controllerHelper->shutdown();
    current.adapter->remove(current.id);
    _controllerHelper->join();
    return _controllerHelper->getOutput();
}

ProcessControllerI::ProcessControllerI(ControllerView^ controller) :
    _controller(controller),
    _host(_controller->getHost())
{
}

shared_ptr<ProcessPrx>
ProcessControllerI::start(string testSuite, string exe, StringSeq args, const Ice::Current& c)
{
    ostringstream os;
    os << "starting " << testSuite << " " << exe << "... ";
    _controller->println(os.str());
    replace(testSuite.begin(), testSuite.end(), '/', '_');
    args.insert(args.begin(), testSuite + '_' + exe + ".dll");
    auto helper = make_shared<ControllerHelperI>(testSuite + '_' + exe + ".dll", args);
    helper->run();
    return Ice::uncheckedCast<ProcessPrx>(c.adapter->addWithUUID(make_shared<ProcessI>(helper)));
}

string
ProcessControllerI::getHost(string, bool, const Ice::Current&)
{
    return _host;
}

ControllerI::ControllerI(ControllerView^ controller)
{
    Ice::InitializationData initData = Ice::InitializationData();
    initData.properties = Ice::createProperties();
    initData.properties->setProperty("Ice.ThreadPool.Client.SizeMax", "10");
    initData.properties->setProperty("Ice.Default.Host", "127.0.0.1");
    initData.properties->setProperty("Ice.Override.ConnectTimeout", "1000");
    //initData.properties->setProperty("Ice.Trace.Network", "3");
    //initData.properties->setProperty("Ice.Trace.Protocol", "1");
    initData.properties->setProperty("ControllerAdapter.AdapterId", Ice::generateUUID());

    _communicator = Ice::initialize(initData);

    auto registry = Ice::uncheckedCast<ProcessControllerRegistryPrx>(
        _communicator->stringToProxy("Util/ProcessControllerRegistry:tcp -h 127.0.0.1 -p 15001"));
    shared_ptr<Ice::ObjectAdapter> adapter = _communicator->createObjectAdapterWithEndpoints("ControllerAdapter", "");
    Ice::Identity ident = { "ProcessController", "UWP"};
    auto processController =
        Ice::uncheckedCast<ProcessControllerPrx>(adapter->add(make_shared<ProcessControllerI>(controller), ident));
    adapter->activate();

    registerProcessController(controller, adapter, registry, processController);
}

void
ControllerI::registerProcessController(ControllerView^ controller,
                                       const shared_ptr<Ice::ObjectAdapter>& adapter,
                                       const shared_ptr<ProcessControllerRegistryPrx>& registry,
                                       const shared_ptr<ProcessControllerPrx>& processController)
{
    registry->ice_pingAsync(
        [this, controller, adapter, registry, processController]()
        {
            auto connection = registry->ice_getCachedConnection();
            connection->setAdapter(adapter);
            connection->setACM(5, Ice::ACMClose::CloseOff, Ice::ACMHeartbeat::HeartbeatAlways);
            connection->setCloseCallback([=](const shared_ptr<Ice::Connection>&)
                {
                    controller->println("connection with process controller registry closed");
                    std::this_thread::sleep_for(2s);
                    registerProcessController(controller, adapter, registry, processController);
                });

            registry->setProcessControllerAsync(processController, nullptr,
                [controller](exception_ptr e)
                {
                    try
                    {
                        rethrow_exception(e);
                    }
                    catch(const Ice::CommunicatorDestroyedException&)
                    {
                    }
                    catch(const std::exception& ex)
                    {
                        ostringstream os;
                        os << "unexpected exception while connecting to process controller registry:\n" << ex.what();
                        controller->println(os.str());
                    }
                });
        },
        [this, controller, adapter, registry, processController](exception_ptr e)
        {
            try
            {
                rethrow_exception(e);
            }
            catch(const Ice::ConnectFailedException&)
            {
                std::this_thread::sleep_for(2s);
                registerProcessController(controller, adapter, registry, processController);
            }
            catch(const Ice::TimeoutException&)
            {
                std::this_thread::sleep_for(2s);
                registerProcessController(controller, adapter, registry, processController);
            }
            catch(const Ice::CommunicatorDestroyedException&)
            {
            }
            catch(const std::exception& ex)
            {
                ostringstream os;
                os << "unexpected exception while connecting to process controller registry:\n" << ex.what();
                controller->println(os.str());
            }
        });
}

ControllerI::~ControllerI()
{
    _communicator->destroy();
}

static ControllerI* controllerI = 0;

ControllerView::ControllerView()
{
    InitializeComponent();
    auto hostnames = NetworkInformation::GetHostNames();
    ipv4Addresses->Items->Append("127.0.0.1");
    for(unsigned int i = 0; i < hostnames->Size; ++i)
    {
        auto hostname = hostnames->GetAt(i);
        if(hostname->Type == Windows::Networking::HostNameType::Ipv4)
        {
            ipv4Addresses->Items->Append(hostname->RawName);
        }
    }
    ipv4Addresses->SelectedIndex = 0;
}

string
ControllerView::getHost() const
{
    return Ice::wstringToString(ipv4Addresses->SelectedItem->ToString()->Data());
}

void
ControllerView::Hostname_SelectionChanged(Platform::Object^, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^)
{
    if(controllerI)
    {
        delete controllerI;
        controllerI = 0;
    }
    controllerI = new ControllerI(this);
}

void
ControllerView::println(const string& s)
{
    if(!s.empty())
    {
        Dispatcher->RunAsync(
            CoreDispatcherPriority::Normal,
            ref new DispatchedHandler([=]()
                                      {
                                          Output->Items->Append(ref new String(Ice::stringToWstring(s).c_str()));
                                          Output->SelectedIndex = Output->Items->Size - 1;
                                          Output->ScrollIntoView(Output->SelectedItem);
                                      }));
    }
}

ControllerView::~ControllerView()
{
    if(controllerI)
    {
        delete controllerI;
        controllerI = 0;
    }
}
