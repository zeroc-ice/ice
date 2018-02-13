// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <Foundation/Foundation.h>

#include <Ice/Ice.h>
#include <Controller.h>
#include <TestHelper.h>

#include <dlfcn.h>

@protocol ViewController
-(void) print:(NSString*)msg;
-(void) println:(NSString*)msg;
@end

using namespace std;
using namespace Test::Common;

namespace
{

    typedef int (*MAIN_ENTRY_POINT)(int argc, char** argv, Test::MainHelper* helper);
    typedef int (*SHUTDOWN_ENTRY_POINT)();

    class MainHelperI : public Test::MainHelper, private IceUtil::Monitor<IceUtil::Mutex>, public IceUtil::Thread
    {
    public:

        MainHelperI(id<ViewController>, const string&, const StringSeq&);
        virtual ~MainHelperI();

        virtual void serverReady();
        virtual void shutdown();
        virtual void waitForCompleted() {}
        virtual bool redirect();
        virtual void print(const std::string&);

        virtual void run();

        void completed(int);
        void waitReady(int) const;
        int waitSuccess(int) const;
        string getOutput() const;

    private:

        id<ViewController> _controller;
        std::string _dll;
        StringSeq _args;
        CFBundleRef _handle;
        SHUTDOWN_ENTRY_POINT _dllTestShutdown;
        bool _ready;
        bool _completed;
        int _status;
        std::ostringstream _out;
    };

    class ProcessI : public Process
    {
    public:

        ProcessI(id<ViewController>, MainHelperI*);
        virtual ~ProcessI();

        void waitReady(int, const Ice::Current&);
        int waitSuccess(int, const Ice::Current&);
        string terminate(const Ice::Current&);

    private:

        id<ViewController> _controller;
        IceUtil::Handle<MainHelperI> _helper;
    };

    class ProcessControllerI : public ProcessController
    {
    public:

        ProcessControllerI(id<ViewController>, NSString*, NSString*);

#ifdef ICE_CPP11_MAPPING
        virtual shared_ptr<ProcessPrx> start(string, string, StringSeq, const Ice::Current&);
        virtual string getHost(string, bool, const Ice::Current&);
#else
        virtual ProcessPrx start(const string&, const string&, const StringSeq&, const Ice::Current&);
        virtual string getHost(const string&, bool, const Ice::Current&);
#endif

    private:

        id<ViewController> _controller;
        string _ipv4;
        string _ipv6;
    };

    class ControllerHelper
    {
    public:

        ControllerHelper(id<ViewController>, NSString*, NSString*);
        virtual ~ControllerHelper();

    private:

        Ice::CommunicatorPtr _communicator;
    };

}

MainHelperI::MainHelperI(id<ViewController> controller, const string& dll, const StringSeq& args) :
_controller(controller),
_dll(dll),
_args(args),
_ready(false),
_completed(false),
_status(0)
{
}

MainHelperI::~MainHelperI()
{
    if(_handle)
    {
        CFBundleUnloadExecutable(_handle);
        CFRelease(_handle);
    }
}

void
MainHelperI::serverReady()
{
    Lock sync(*this);
    _ready = true;
    notifyAll();
}

void
MainHelperI::shutdown()
{
    Lock sync(*this);
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
    //return _dll.find("client") != string::npos || _dll.find("collocated") != string::npos;
    return true;
}

void
MainHelperI::print(const std::string& msg)
{
    _out << msg;
    //[_controller println:[NSString stringWithUTF8String:msg.c_str()]];
}

void
MainHelperI::run()
{
    NSString* bundlePath = [[NSBundle mainBundle] privateFrameworksPath];

    bundlePath = [bundlePath stringByAppendingPathComponent:[NSString stringWithUTF8String:_dll.c_str()]];

    NSURL* bundleURL = [NSURL fileURLWithPath:bundlePath];
    _handle = CFBundleCreate(NULL, (CFURLRef)bundleURL);
    if(!_handle)
    {
        print([[NSString stringWithFormat:@"Could not find bundle %@", bundlePath] UTF8String]);
        completed(EXIT_FAILURE);
        return;
    }

    CFErrorRef error = nil;
    Boolean loaded = CFBundleLoadExecutableAndReturnError(_handle, &error);
    if(error != nil || !loaded)
    {
        print([[(__bridge NSError *)error description] UTF8String]);
        completed(EXIT_FAILURE);
        return;
    }

    //
    // The first CFBundleGetFunction... does not always succeed, so we make up to 5 attempts
    //
    void* sym = 0;
    int attempts = 0;
    while((sym = CFBundleGetFunctionPointerForName(_handle, CFSTR("dllTestShutdown"))) == 0 && attempts < 5)
    {
        attempts++;
        [NSThread sleepForTimeInterval:0.2];
    }

    if(sym == 0)
    {
        NSString* err = [NSString stringWithFormat:@"Could not get function pointer dllTestShutdown from bundle %@",
                         bundlePath];
        print([err UTF8String]);
        completed(EXIT_FAILURE);
        return;
    }
    /*
    else if(attempts > 0)
    {
        print([[NSString stringWithFormat:@"************ found dllTestShutdown after %d failed attempt(s)", attempts] UTF8String]);
    }
    */

    _dllTestShutdown = (SHUTDOWN_ENTRY_POINT)sym;

    sym = CFBundleGetFunctionPointerForName(_handle, CFSTR("dllMain"));
    if(sym == 0)
    {
        NSString* err = [NSString stringWithFormat:@"Could not get function pointer dllMain from bundle %@",
                         bundlePath];
        print([err UTF8String]);
        completed(EXIT_FAILURE);
        return;
    }

    MAIN_ENTRY_POINT dllMain = (MAIN_ENTRY_POINT)sym;
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
    }
    catch(...)
    {
        print("unexpected unknown exception while running `" + _args[0] + "'");
    }
    delete[] argv;
}

void
MainHelperI::completed(int status)
{
    Lock sync(*this);
    _completed = true;
    _status = status;
    notifyAll();
}

void
MainHelperI::waitReady(int timeout) const
{
    Lock sync(*this);
    while(!_ready && !_completed)
    {
        if(!timedWait(IceUtil::Time::seconds(timeout)))
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
    Lock sync(*this);
    while(!_completed)
    {
        if(!timedWait(IceUtil::Time::seconds(timeout)))
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

ProcessI::ProcessI(id<ViewController> controller, MainHelperI* helper) : _controller(controller), _helper(helper)
{
}

ProcessI::~ProcessI()
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
    _helper->getThreadControl().join();
    return _helper->getOutput();
}

ProcessControllerI::ProcessControllerI(id<ViewController> controller, NSString* ipv4, NSString* ipv6) :
    _controller(controller), _ipv4([ipv4 UTF8String]), _ipv6([ipv6 UTF8String])
{
}

#ifdef ICE_CPP11_MAPPING
shared_ptr<ProcessPrx>
ProcessControllerI::start(string testSuite, string exe, StringSeq args, const Ice::Current& c)
#else
ProcessPrx
ProcessControllerI::start(const string& testSuite, const string& exe, const StringSeq& args, const Ice::Current& c)
#endif
{
    std::string prefix = std::string("test/") + testSuite;
    replace(prefix.begin(), prefix.end(), '/', '_');
    [_controller println:[NSString stringWithFormat:@"starting %s %s... ", testSuite.c_str(), exe.c_str()]];
    IceUtil::Handle<MainHelperI> helper = new MainHelperI(_controller, prefix + '/' + exe + ".bundle", args);

    //
    // Use a 768KB thread stack size for the objects test. This is necessary when running the
    // test on arm64 devices with a debug Ice libraries which require lots of stack space.
    //
    helper->start(768 * 1024);
    return ICE_UNCHECKED_CAST(ProcessPrx, c.adapter->addWithUUID(ICE_MAKE_SHARED(ProcessI, _controller, helper.get())));
}

string
#ifdef ICE_CPP11_MAPPING
ProcessControllerI::getHost(string protocol, bool ipv6, const Ice::Current& c)
#else
ProcessControllerI::getHost(const string& protocol, bool ipv6, const Ice::Current& c)
#endif
{
    return ipv6 ? _ipv6 : _ipv4;
}

ControllerHelper::ControllerHelper(id<ViewController> controller, NSString* ipv4, NSString* ipv6)
{
    Ice::registerIceDiscovery();

    Ice::InitializationData initData = Ice::InitializationData();
    initData.properties = Ice::createProperties();
    initData.properties->setProperty("Ice.ThreadPool.Server.SizeMax", "10");
    initData.properties->setProperty("IceDiscovery.DomainId", "TestController");
    initData.properties->setProperty("ControllerAdapter.Endpoints", "tcp");
    //initData.properties->setProperty("Ice.Trace.Network", "2");
    //initData.properties->setProperty("Ice.Trace.Protocol", "2");
    initData.properties->setProperty("ControllerAdapter.AdapterId", Ice::generateUUID());

    _communicator = Ice::initialize(initData);

    Ice::ObjectAdapterPtr adapter = _communicator->createObjectAdapter("ControllerAdapter");
    Ice::Identity ident;
#if TARGET_IPHONE_SIMULATOR != 0
    ident.category = "iPhoneSimulator";
#else
    ident.category = "iPhoneOS";
#endif
    ident.name = [[[NSBundle mainBundle] bundleIdentifier] UTF8String];
    adapter->add(ICE_MAKE_SHARED(ProcessControllerI, controller, ipv4, ipv6), ident);
    adapter->activate();
}

ControllerHelper::~ControllerHelper()
{
    _communicator->destroy();
    _communicator = 0;
}

static ControllerHelper* controllerHelper = 0;

extern "C"
{

void
startController(id<ViewController> controller, NSString* ipv4, NSString* ipv6)
{
    controllerHelper = new ControllerHelper(controller, ipv4, ipv6);
}

void
stopController()
{
    if(controllerHelper)
    {
        delete controllerHelper;
        controllerHelper = 0;
    }
}

}
