// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include "ViewController.xaml.h"

#include <Ice/Ice.h>
#include <Controller.h>
#include <TestHelper.h>

#include <string>
#include <iostream>
#include <memory>
#include <condition_variable>
#include <mutex>

using namespace std;
using namespace Controller;
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

typedef int(*MAIN_ENTRY_POINT)(int, char**, Test::MainHelper*);
typedef int(*SHUTDOWN_ENTRY_POINT)();

class MainHelperI : public Test::MainHelper
{
public:

    MainHelperI(ViewController^, const string&, const StringSeq&);

    virtual void serverReady();
    virtual void shutdown();
    virtual void waitForCompleted()
    {
    }
    virtual bool redirect();
    virtual void print(const std::string&);

    virtual void run();
    void join();
    void completed(int);
    void waitReady(int) const;
    int waitSuccess(int) const;
    string getOutput() const;

private:

    ViewController^ _controller;
    string _dll;
    StringSeq _args;
    FARPROC _dllTestShutdown;
    bool _ready;
    bool _completed;
    int _status;
    ostringstream _out;
    thread _thread;
    mutable mutex _mutex;
    mutable condition_variable _condition;
};

class ProcessI : public Process
{
public:

    ProcessI(ViewController^, shared_ptr<MainHelperI>);

    void waitReady(int, const Ice::Current&);
    int waitSuccess(int, const Ice::Current&);
    string terminate(const Ice::Current&);

private:

    ViewController^ _controller;
    shared_ptr<MainHelperI> _helper;
};

class ProcessControllerI : public ProcessController
{
public:

    ProcessControllerI(ViewController^);
    shared_ptr<ProcessPrx> start(string, string, StringSeq, const Ice::Current&);
    virtual string getHost(string, bool, const Ice::Current&);

private:

    ViewController^ _controller;
    string _host;
};

class ControllerHelper
{
public:

    ControllerHelper(ViewController^);
    virtual ~ControllerHelper();

    void
    registerProcessController(ViewController^,
                              const Ice::ObjectAdapterPtr&,
                              const shared_ptr<ProcessControllerRegistryPrx>&,
                              const shared_ptr<ProcessControllerPrx>&);

private:

    shared_ptr<Ice::Communicator> _communicator;
    shared_ptr<Test::Common::ProcessControllerRegistryPrx> _registry;
};

}

MainHelperI::MainHelperI(ViewController^ controller, const string& dll, const StringSeq& args) :
    _controller(controller),
    _dll(dll),
    _args(args),
    _ready(false),
    _completed(false),
    _status(0)
{
}

void
MainHelperI::serverReady()
{
    unique_lock<mutex> lock(_mutex);
    _ready = true;
    _condition.notify_all();
}

void
MainHelperI::shutdown()
{
    unique_lock<mutex> lock(_mutex);
    if(_completed)
    {
        return;
    }

    if(_dllTestShutdown)
    {
        _dllTestShutdown();
    }
}

bool
MainHelperI::redirect()
{
    return _dll.find("client") != string::npos || _dll.find("collocated") != string::npos;
}

void

MainHelperI::print(const std::string& msg)
{
    _out << msg;
    //_controller->println(msg);
}

void
MainHelperI::run()
{
    thread t([this]()
    {
        HINSTANCE hnd = _controller->loadDll(_dll);
        if(hnd == 0)
        {
            ostringstream os;
            os << "failed to load `" << _dll << "': error code: " << GetLastError();
            print(os.str());
            completed(EXIT_FAILURE);
            return;
        }

        _dllTestShutdown = GetProcAddress(hnd, "dllTestShutdown");
        if(_dllTestShutdown == 0)
        {
            print("failed to find dllTestShutdown function from `" + _dll + "'");
            _controller->unloadDll(_dll);
            completed(EXIT_FAILURE);
            return;
        }

        FARPROC sym = GetProcAddress(hnd, "dllMain");
        if(sym == 0)
        {
            print("failed to find dllMain function from `" + _dll + "'");
            _controller->unloadDll(_dll);
            completed(EXIT_FAILURE);
            return;
        }

        MAIN_ENTRY_POINT dllMain = reinterpret_cast<MAIN_ENTRY_POINT>(sym);
        char** argv = new char*[_args.size() + 1];
        for(unsigned int i = 0; i < _args.size(); ++i)
        {
            argv[i] = const_cast<char*>(_args[i].c_str());
        }
        argv[_args.size()] = 0;
        try
        {
            completed(dllMain(static_cast<int>(_args.size()), argv, this));
        }
        catch(const std::exception& ex)
        {
            print("unexpected exception while running `" + _args[0] + "':\n" + ex.what());
            completed(EXIT_FAILURE);
        }
        catch(...)
        {
            print("unexpected unknown exception while running `" + _args[0] + "'");
            completed(EXIT_FAILURE);
        }
        delete[] argv;
        _controller->unloadDll(_dll);
    });
    _thread = move(t);
}

void
MainHelperI::join()
{
    _thread.join();
}

void
MainHelperI::completed(int status)
{
    unique_lock<mutex> lock(_mutex);
    _completed = true;
    _status = status;
    _condition.notify_all();
}

void
MainHelperI::waitReady(int timeout) const
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
MainHelperI::waitSuccess(int timeout) const
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
MainHelperI::getOutput() const
{
    assert(_completed);
    return _out.str();
}

ProcessI::ProcessI(ViewController^ controller, shared_ptr<MainHelperI> helper) : _controller(controller), _helper(helper)
{
}

void
ProcessI::waitReady(int timeout, const Ice::Current&)
{
    _helper->waitReady(timeout);
}

int
ProcessI::waitSuccess(int timeout, const Ice::Current&)
{
    return _helper->waitSuccess(timeout);
}

string
ProcessI::terminate(const Ice::Current& current)
{
    _helper->shutdown();
    current.adapter->remove(current.id);
    _helper->join();
    return _helper->getOutput();
}

ProcessControllerI::ProcessControllerI(ViewController^ controller) :
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
    auto helper = make_shared<MainHelperI>(_controller, testSuite + '_' + exe + ".dll", args);
    helper->run();
    return Ice::uncheckedCast<ProcessPrx>(c.adapter->addWithUUID(make_shared<ProcessI>(_controller, helper)));
}

string
ProcessControllerI::getHost(string, bool, const Ice::Current&)
{
    return _host;
}

ControllerHelper::ControllerHelper(ViewController^ controller)
{
    Ice::InitializationData initData = Ice::InitializationData();
    initData.properties = Ice::createProperties();
    initData.properties->setProperty("Ice.ThreadPool.Server.SizeMax", "10");
    initData.properties->setProperty("Ice.Default.Host", "127.0.0.1");
    initData.properties->setProperty("Ice.Override.ConnectTimeout", "1000");
    //initData.properties->setProperty("Ice.Trace.Network", "3");
    //initData.properties->setProperty("Ice.Trace.Protocol", "1");
    initData.properties->setProperty("ControllerAdapter.AdapterId", Ice::generateUUID());

    _communicator = Ice::initialize(initData);

    auto registry = Ice::uncheckedCast<ProcessControllerRegistryPrx>(
        _communicator->stringToProxy("Util/ProcessControllerRegistry:tcp -h 127.0.0.1 -p 15001"));
    Ice::ObjectAdapterPtr adapter = _communicator->createObjectAdapterWithEndpoints("ControllerAdapter", "");
    Ice::Identity ident = { "ProcessController", "UWP"};
    auto processController =
        Ice::uncheckedCast<ProcessControllerPrx>(adapter->add(make_shared<ProcessControllerI>(controller), ident));
    adapter->activate();

    registerProcessController(controller, adapter, registry, processController);
}

void
ControllerHelper::registerProcessController(ViewController^ controller,
                                            const Ice::ObjectAdapterPtr& adapter,
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

ControllerHelper::~ControllerHelper()
{
    _communicator->destroy();
}

static ControllerHelper* controllerHelper = 0;

ViewController::ViewController()
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
ViewController::getHost() const
{
    return Ice::wstringToString(ipv4Addresses->SelectedItem->ToString()->Data());
}

void
ViewController::Hostname_SelectionChanged(Platform::Object^, Windows::UI::Xaml::Controls::SelectionChangedEventArgs^)
{
    if(controllerHelper)
    {
        delete controllerHelper;
        controllerHelper = 0;
    }
    controllerHelper = new ControllerHelper(this);
}

void
ViewController::println(const string& s)
{
    this->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(
        [=]()
        {
            Output->Items->Append(ref new String(Ice::stringToWstring(s).c_str()));
            Output->SelectedIndex = Output->Items->Size - 1;
            Output->ScrollIntoView(Output->SelectedItem);
        }));
}

HINSTANCE
ViewController::loadDll(const string& name)
{
    map<string, pair<HINSTANCE, unsigned int>>::iterator p = _dlls.find(name);
    if(p == _dlls.end())
    {
        HINSTANCE hnd = LoadPackagedLibrary(Ice::stringToWstring(name).c_str(), 0);
        p = _dlls.insert(make_pair(name, make_pair(hnd, 0))).first;
    }
    ++p->second.second;
    return p->second.first;
}

void
ViewController::unloadDll(const string& name)
{
    map<string, pair<HINSTANCE, unsigned int>>::iterator p = _dlls.find(name);
    assert(p != _dlls.end());
    if(--p->second.second == 0)
    {
        FreeLibrary(p->second.first);
        _dlls.erase(p);
    }
}

ViewController::~ViewController()
{
    for(map<string, pair<HINSTANCE, unsigned int>>::const_iterator p = _dlls.begin(); p != _dlls.end(); ++p)
    {
        FreeLibrary(p->second.first);
    }

    if(controllerHelper)
    {
        delete controllerHelper;
        controllerHelper = 0;
    }
}
