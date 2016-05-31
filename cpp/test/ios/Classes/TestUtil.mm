// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
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

@implementation TestCase

@synthesize name;
@synthesize prefix;
@synthesize server;
@synthesize client;
@synthesize serveramd;
@synthesize collocated;
@synthesize sslSupport;
@synthesize wsSupport;
@synthesize runWithSlicedFormat;
@synthesize runWith10Encoding;
@synthesize cpp11Support;

+(id)testWithName:(NSString*)name
              prefix:(NSString*) prefix
              client:(NSString*) client
              server:(NSString*) server
           serveramd:(NSString*) serveramd
          collocated:(NSString*) collocated
          sslSupport:(BOOL) sslSupport
           wsSupport:(BOOL) wsSupport
 runWithSlicedFormat:(BOOL)runWithSlicedFormat
   runWith10Encoding:(BOOL)runWith10Encoding
        cpp11Support:(BOOL)cpp11Support
{
    TestCase* t = [[TestCase alloc] init];
    if(t != nil)
    {
        t->name = name;
        t->prefix = prefix;
        t->client = client;
        t->server = server;
        t->serveramd = serveramd;
        t->collocated = collocated;
        t->sslSupport = sslSupport;
        t->wsSupport = wsSupport;
        t->runWithSlicedFormat = runWithSlicedFormat;
        t->runWith10Encoding = runWith10Encoding;
        t->cpp11Support = cpp11Support;
    }
    return [t autorelease];
}

-(BOOL) hasServer
{
    return server != 0;
}

-(BOOL) hasAMDServer
{
    return serveramd != 0;
}

-(BOOL) hasCollocated
{
    return collocated != 0;
}

-(BOOL)isProtocolSupported:(NSString*)protocol
{
    BOOL supported = YES;
    if(!sslSupport)
    {
        supported &= [protocol isEqualToString:@"tcp"] || [protocol isEqualToString:@"ws"];
    }
    if(!wsSupport)
    {
        supported &= [protocol isEqualToString:@"tcp"] || [protocol isEqualToString:@"ssl"];
    }
    return supported;
}
-(void)dealloc
{
    [name release];
    [prefix release];
    [client release];
    [server release];
    [serveramd release];
    [collocated release];
    [super dealloc];
}

@end

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
    if(_config.type == TestConfigTypeServer)
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
    
    if(_config.type == TestConfigTypeServer)
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
        if(_config.type == TestConfigTypeServer)
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
    
    if(_config.option == TestConfigOptionSliced)
    {
        args.push_back("--Ice.Default.SlicedFormat");
    }
    else if(_config.option == TestConfigOptionEncoding10)
    {
        args.push_back("--Ice.Default.EncodingVersion=1.0");
    }
    
    if(_config.type == TestConfigTypeServer)
    {
        if(_libName.find("serveramd") != std::string::npos)
        {
            print("Running test with AMD server & ");
        }
        else
        {
            print("Running test with ");
        }
    }
    else if(_config.type == TestConfigTypeClient || _config.type == TestConfigTypeColloc)
    {
        if(_config.type == TestConfigTypeColloc)
        {
            print("Running collocated test with ");
        }
        else if(!_config.hasServer)
        {
            print("Running test with ");
        }
        print(_config.protocol + " and ");
        if(_config.option == TestConfigOptionDefault)
        {
            print("default format.\n");
        }
        else if(_config.option == TestConfigOptionSliced)
        {
            print("sliced format.\n");
        }
        else if(_config.option == TestConfigOptionEncoding10)
        {
            print("1.0 encoding.\n");
        }
    }
    
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
    return _config.type == TestConfigTypeClient || _config.type == TestConfigTypeColloc;
}

void
MainHelperI::print(const std::string& msg)
{
    [_target performSelectorOnMainThread:_output withObject:[NSString stringWithUTF8String: msg.c_str()]
             waitUntilDone:NO];
}
