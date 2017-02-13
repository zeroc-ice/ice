// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestUtil.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdlib.h>

#include <vector>
#include <string>

#import <Foundation/NSString.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSThread.h>

using namespace std;

@implementation TestSuite
@synthesize testSuiteId;
@synthesize testCases;

+(id) testSuite:(NSString*)testSuiteId testCases:(NSArray*)testCases
{
    TestSuite* testSuite = [[TestSuite alloc] init];
    if(testSuite != nil)
    {
        testSuite->testSuiteId = testSuiteId;
        testSuite->testCases = testCases;
    }
    return [testSuite autorelease];
}

-(BOOL) isIn:(NSArray*)tests
{
    for(NSString* t in tests)
    {
        if([t isEqualToString:testSuiteId])
        {
            return TRUE;
        }
    }
    return FALSE;
}
@end

@implementation TestCase
@synthesize name;
@synthesize client;
@synthesize server;
@synthesize args;

+(id) testCase:(NSString*)name client:(NSString*)client server:(NSString*)server args:(NSArray*)args
{
    TestCase* testCase = [[TestCase alloc] init];
    if(testCase != nil)
    {
        testCase->name = name;
        testCase->client = client;
        testCase->server = server;
        testCase->args = args;
    }
    return [testCase autorelease];
}
@end

MainHelperI::MainHelperI(const string& test, const string& libName, TestType type, TestConfig config,
                         id target, SEL out, SEL ready) :
    _name(test),
    _libName(libName),
    _type(type),
    _config(config),
    _completed(false),
    _status(0),
    _target(target),
    _output(out),
    _ready(ready)
{
}

MainHelperI::~MainHelperI()
{
    if(_handle)
    {
        CFBundleUnloadExecutable(_handle);
    }
}

void
MainHelperI::run()
{
    //
    // Compose the path of the Test.bundle resources folder
    //
    NSString* bundlePath = [[NSBundle mainBundle] privateFrameworksPath];

    bundlePath = [bundlePath stringByAppendingPathComponent:[NSString stringWithUTF8String:_libName.c_str()]];

    NSURL* bundleURL = [NSURL fileURLWithPath:bundlePath];
    _handle = CFBundleCreate(NULL, (CFURLRef)bundleURL);

    int status = EXIT_FAILURE;
    NSError* error = nil;
    Boolean loaded = CFBundleLoadExecutableAndReturnError(_handle, (CFErrorRef*)&error);
    if(error != nil || !loaded)
    {
        print([[error description] UTF8String]);
        completed(status);
        return;
    }

    void* sym = dlsym(_handle, "dllTestShutdown");
    sym = CFBundleGetFunctionPointerForName(_handle, CFSTR("dllTestShutdown"));
    if(sym == 0)
    {
        NSString* err = [NSString stringWithFormat:@"Could not get function pointer dllTestShutdown from bundle %@",
                                  bundlePath];
        print([err UTF8String]);
        completed(status);
        return;
    }
    _dllTestShutdown = (SHUTDOWN_ENTRY_POINT)sym;

    sym = CFBundleGetFunctionPointerForName(_handle, CFSTR("dllMain"));
    if(sym == 0)
    {
        NSString* err = [NSString stringWithFormat:@"Could not get function pointer dllMain from bundle %@",
                                  bundlePath];
        print([err UTF8String]);
        completed(status);
        return;
    }

    MAIN_ENTRY_POINT dllMain = (MAIN_ENTRY_POINT)sym;

    std::vector<std::string> args;
    if(_type == TestTypeServer)
    {
        args.push_back("server");
    }
    else
    {
        args.push_back("client");
    }
    args.push_back("--Ice.NullHandleAbort=1");
    args.push_back("--Ice.Warn.Connections=1");
    args.push_back("--Ice.Default.Host=127.0.0.1");
    args.push_back("--Ice.Trace.Network=0");
    args.push_back("--Ice.Trace.Protocol=0");

    if(_type == TestTypeServer)
    {
        args.push_back("--Ice.ThreadPool.Server.Size=1");
        args.push_back("--Ice.ThreadPool.Server.SizeMax=3");
        args.push_back("--Ice.ThreadPool.Server.SizeWarn=0");
        //args.push_back("--Ice.PrintAdapterReady=1");
        args.push_back("--Ice.ServerIdleTime=30");
    }

    args.push_back("--Ice.Default.Protocol=" + _config.protocol);
    if(_config.protocol == "ssl" || _config.protocol == "wss")
    {
        args.push_back("--IceSSL.CAs=cacert.der");
        args.push_back("--IceSSL.CheckCertName=0");
        if(_type == TestTypeServer)
        {
            args.push_back("--IceSSL.CertFile=server.p12");
        }
        else
        {
            args.push_back("--IceSSL.CertFile=client.p12");
        }
        args.push_back("--IceSSL.Password=password");
        args.push_back("--Ice.Override.ConnectTimeout=10000"); // COMPILERFIX: Workaround for SSL hang on iOS devices
    }
    copy(_config.args.begin(), _config.args.end(), back_inserter(args));

    char** argv = new char*[args.size() + 1];
    for(unsigned int i = 0; i < args.size(); ++i)
    {
        argv[i] = const_cast<char*>(args[i].c_str());
    }
    argv[args.size()] = 0;
    try
    {
        status = dllMain(static_cast<int>(args.size()), argv, this);
    }
    catch(const std::exception& ex)
    {
        print("unexpected exception while running `" + args[0] + "':\n" + ex.what());
    }
    catch(...)
    {
        print("unexpected unknown exception while running `" + args[0] + "'");
    }
    completed(status);
    delete[] argv;
}

void
MainHelperI::serverReady()
{
    [_target performSelectorOnMainThread:_ready withObject:nil waitUntilDone:NO];
}

void
MainHelperI::shutdown()
{
    if(_dllTestShutdown)
    {
        _dllTestShutdown();
    }
}

bool
MainHelperI::redirect()
{
    return _type == TestTypeClient;
}

void
MainHelperI::print(const std::string& msg)
{
    [_target performSelectorOnMainThread:_output
             withObject:[NSString stringWithUTF8String: msg.c_str()]
             waitUntilDone:NO];
}
