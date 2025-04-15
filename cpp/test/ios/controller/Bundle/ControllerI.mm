// Copyright (c) ZeroC, Inc.

#import <Foundation/Foundation.h>

#include "Controller.h"
#include "Ice/Ice.h"
#include "IceDiscovery/IceDiscovery.h"
#include "TestHelper.h"

#include <dlfcn.h>

#include <condition_variable>
#include <mutex>

@protocol ControllerView
- (void)print:(NSString*)msg;
- (void)println:(NSString*)msg;
@end

using namespace std;
using namespace Test::Common;

namespace
{
    typedef Test::TestHelper* (*CREATE_HELPER_ENTRY_POINT)();

    class ControllerHelperI : public Test::ControllerHelper
    {
    public:
        ControllerHelperI(id<ControllerView>, const string&, const StringSeq&);
        virtual ~ControllerHelperI();

        virtual string loggerPrefix() const;
        virtual void print(const std::string&);
        virtual void serverReady();
        virtual void communicatorInitialized(const Ice::CommunicatorPtr&);

        void shutdown();
        string getOutput() const;
        void waitReady(int) const;
        int waitSuccess(int) const;

        virtual void run();
        void completed(int);

    private:
        id<ControllerView> _controller;
        std::string _dll;
        StringSeq _args;
        bool _ready;
        bool _completed;
        int _status;
        std::ostringstream _out;
        Ice::CommunicatorPtr _communicator;
        mutable std::mutex _mutex;
        mutable std::condition_variable _condition;
    };

    class ProcessI : public Process
    {
    public:
        ProcessI(id<ControllerView>, std::shared_ptr<ControllerHelperI>);
        virtual ~ProcessI();

        void waitReady(int, const Ice::Current&);
        int waitSuccess(int, const Ice::Current&);
        string terminate(const Ice::Current&);

    private:
        id<ControllerView> _controller;
        std::shared_ptr<ControllerHelperI> _helper;
        std::thread _helperThread;
    };

    class ProcessControllerI : public ProcessController
    {
    public:
        ProcessControllerI(id<ControllerView>, NSString*, NSString*);

        virtual optional<ProcessPrx> start(string, string, StringSeq, const Ice::Current&);
        virtual string getHost(string, bool, const Ice::Current&);

    private:
        id<ControllerView> _controller;
        string _ipv4;
        string _ipv6;
    };

    class ControllerI
    {
    public:
        ControllerI(id<ControllerView>, NSString*, NSString*);
        virtual ~ControllerI();

    private:
        Ice::CommunicatorPtr _communicator;
    };

    Test::StreamHelper streamRedirect;
}

ControllerHelperI::ControllerHelperI(id<ControllerView> controller, const string& dll, const StringSeq& args)
    : _controller(controller),
      _dll(dll),
      _args(args),
      _ready(false),
      _completed(false),
      _status(0)
{
}

ControllerHelperI::~ControllerHelperI() {}

void
ControllerHelperI::serverReady()
{
    lock_guard lock(_mutex);
    _ready = true;
    _condition.notify_all();
}

void
ControllerHelperI::communicatorInitialized(const Ice::CommunicatorPtr& communicator)
{
    lock_guard lock(_mutex);
    _communicator = communicator;
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
ControllerHelperI::run()
{
    NSString* bundlePath = [[NSBundle mainBundle] privateFrameworksPath];

    bundlePath = [bundlePath stringByAppendingPathComponent:[NSString stringWithUTF8String:_dll.c_str()]];

    NSURL* bundleURL = [NSURL fileURLWithPath:bundlePath];
    CFBundleRef handle = CFBundleCreate(NULL, (CFURLRef)bundleURL);
    if (!handle)
    {
        print([[NSString stringWithFormat:@"Could not find bundle %@", bundlePath] UTF8String]);
        completed(EXIT_FAILURE);
        return;
    }

    CFErrorRef error = nil;
    Boolean loaded = CFBundleLoadExecutableAndReturnError(handle, &error);
    if (error != nil || !loaded)
    {
        print([[(__bridge NSError*)error description] UTF8String]);
        completed(EXIT_FAILURE);
        return;
    }

    //
    // The first CFBundleGetFunction... does not always succeed, so we make up to 5 attempts
    //
    void* sym = 0;
    int attempts = 0;
    while ((sym = CFBundleGetFunctionPointerForName(handle, CFSTR("createHelper"))) == 0 && attempts < 5)
    {
        attempts++;
        [NSThread sleepForTimeInterval:0.2];
    }

    if (sym == 0)
    {
        NSString* err =
            [NSString stringWithFormat:@"Could not get function pointer createHelper from bundle %@", bundlePath];
        print([err UTF8String]);
        completed(EXIT_FAILURE);
        return;
    }

    if (_dll.find("client") != string::npos || _dll.find("collocated") != string::npos)
    {
        streamRedirect.setControllerHelper(this);
    }

    CREATE_HELPER_ENTRY_POINT createHelper = (CREATE_HELPER_ENTRY_POINT)sym;
    char** argv = new char*[_args.size() + 1];
    for (unsigned int i = 0; i < _args.size(); ++i)
    {
        argv[i] = const_cast<char*>(_args[i].c_str());
    }
    argv[_args.size()] = 0;
    try
    {
        unique_ptr<Test::TestHelper> helper(createHelper());
        helper->setControllerHelper(this);
        helper->run(static_cast<int>(_args.size()), argv);
        completed(0);
    }
    catch (const std::exception& ex)
    {
        print("unexpected exception while running `" + _args[0] + "':\n" + ex.what());
        completed(1);
    }
    catch (...)
    {
        print("unexpected unknown exception while running `" + _args[0] + "'");
        completed(1);
    }
    delete[] argv;

    if (_dll.find("client") != string::npos || _dll.find("collocated") != string::npos)
    {
        streamRedirect.setControllerHelper(0);
    }

    CFBundleUnloadExecutable(handle);
    CFRelease(handle);
}

void
ControllerHelperI::completed(int status)
{
    lock_guard lock(_mutex);
    _completed = true;
    _status = status;
    _communicator = 0;
    _condition.notify_all();
}

void
ControllerHelperI::shutdown()
{
    lock_guard lock(_mutex);
    if (_communicator)
    {
        _communicator->shutdown();
    }
}

void
ControllerHelperI::waitReady(int timeout) const
{
    unique_lock<mutex> lock(_mutex);
    while (!_ready && !_completed)
    {
        if (_condition.wait_for(lock, chrono::seconds(timeout)) == cv_status::timeout)
        {
            throw ProcessFailedException("timed out waiting for the process to be ready");
        }
    }
    if (_completed && _status == EXIT_FAILURE)
    {
        throw ProcessFailedException(_out.str());
    }
}

int
ControllerHelperI::waitSuccess(int timeout) const
{
    unique_lock lock(_mutex);
    while (!_completed)
    {
        if (_condition.wait_for(lock, chrono::seconds(timeout)) == cv_status::timeout)
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

ProcessI::ProcessI(id<ControllerView> controller, std::shared_ptr<ControllerHelperI> helper)
    : _controller(controller),
      _helper(helper),
      _helperThread([helper] { helper->run(); })
{
}

ProcessI::~ProcessI() {}

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
    _helperThread.join();
    return _helper->getOutput();
}

ProcessControllerI::ProcessControllerI(id<ControllerView> controller, NSString* ipv4, NSString* ipv6)
    : _controller(controller),
      _ipv4([ipv4 UTF8String]),
      _ipv6([ipv6 UTF8String])
{
}

optional<ProcessPrx>
ProcessControllerI::start(string testSuite, string exe, StringSeq args, const Ice::Current& c)
{
    StringSeq newArgs = args;
    std::string prefix = std::string("test/") + testSuite;
    replace(prefix.begin(), prefix.end(), '/', '_');
    newArgs.insert(newArgs.begin(), testSuite + ' ' + exe);
    [_controller println:[NSString stringWithFormat:@"starting %s %s... ", testSuite.c_str(), exe.c_str()]];
    auto helper = make_shared<ControllerHelperI>(_controller, prefix + '/' + exe + ".bundle", newArgs);
    return c.adapter->addWithUUID<ProcessPrx>(make_shared<ProcessI>(_controller, helper));
}

string
ProcessControllerI::getHost(string protocol, bool ipv6, const Ice::Current& c)
{
    return ipv6 ? _ipv6 : _ipv4;
}

ControllerI::ControllerI(id<ControllerView> controller, NSString* ipv4, NSString* ipv6)
{
    Ice::InitializationData initData = Ice::InitializationData();
    initData.properties = Ice::createProperties();
    initData.properties->setProperty("Ice.ThreadPool.Server.SizeMax", "10");
    initData.properties->setProperty("IceDiscovery.DomainId", "TestController");
    initData.properties->setProperty("ControllerAdapter.Endpoints", "tcp");
    // initData.properties->setProperty("Ice.Trace.Network", "2");
    // initData.properties->setProperty("Ice.Trace.Protocol", "2");
    initData.properties->setProperty("ControllerAdapter.AdapterId", Ice::generateUUID());

    initData.pluginFactories = {Ice::udpPluginFactory(), IceDiscovery::discoveryPluginFactory()};

    _communicator = Ice::initialize(initData);

    Ice::ObjectAdapterPtr adapter = _communicator->createObjectAdapter("ControllerAdapter");
    Ice::Identity ident;
#if TARGET_IPHONE_SIMULATOR != 0
    ident.category = "iPhoneSimulator";
#else
    ident.category = "iPhoneOS";
#endif
    ident.name = [[[NSBundle mainBundle] bundleIdentifier] UTF8String];
    adapter->add(make_shared<ProcessControllerI>(controller, ipv4, ipv6), ident);
    adapter->activate();
}

ControllerI::~ControllerI()
{
    _communicator->destroy();
    _communicator = 0;
}

static ControllerI* controllerI = 0;

extern "C"
{
    void startController(id<ControllerView> controller, NSString* ipv4, NSString* ipv6)
    {
        controllerI = new ControllerI(controller, ipv4, ipv6);
    }

    void stopController()
    {
        if (controllerI)
        {
            delete controllerI;
            controllerI = 0;
        }
    }
}
