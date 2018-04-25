// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef TEST_API_EXPORTS
#   define TEST_API_EXPORTS
#endif

#include <TestCommon.h>

#include <stdarg.h>
#include <stdlib.h>

#import <objc/Ice.h>

@implementation TestFailedException
-(id)init
{
    return [super initWithName:@"TestFailedException" reason:nil userInfo:nil];
}
@end

#if TARGET_OS_IPHONE

#import <Foundation/NSString.h>
#import <Foundation/NSObject.h>
#import <Foundation/NSThread.h>

#include <objc/Ice.h>

static id outputTarget;
static id testRun;
static SEL readySelector;
static SEL outputSelector;
static id<ICECommunicator> communicator = nil;
static NSString* protocol;

static BOOL sliced;
static BOOL encoding10;

#endif

id<ICEProperties>
defaultServerProperties(int *argc, char** argv)
{
    id<ICEProperties> properties = [ICEUtil createProperties];

#if TARGET_OS_IPHONE
    static NSString* defaults[] =
    {
        @"Ice.NullHandleAbort", @"1",
        @"Ice.Warn.Connections", @"1",
        @"Ice.ThreadPool.Server.Size", @"1",
        @"Ice.ThreadPool.Server.SizeMax", @"3",
        @"Ice.ThreadPool.Server.SizeWarn", @"0",
        //@"Ice.PrintAdapterReady", @"1",
        @"Ice.ServerIdleTime", @"30",
        @"Ice.Default.Host", @"127.0.0.1",
        @"Ice.Trace.Network", @"0",
        @"Ice.Trace.Protocol", @"0"
    };

    static NSString* ssldefaults[] =
    {
        @"Ice.Override.ConnectTimeout", @"10000", // COMPILERFIX: Workaround for SSL hang on iOS devices
        @"IceSSL.CAs", @"cacert.der",
        @"IceSSL.CheckCertName", @"0",
        @"IceSSL.CertFile", @"server.p12",
        @"IceSSL.Password", @"password"
    };

    int i;
    for(i = 0; i < sizeof(defaults)/sizeof(defaults[0]); i += 2)
    {
        [properties setProperty:defaults[i] value:defaults[i+1]];
    }
    [properties setProperty:@"Ice.Default.Protocol" value:protocol];
    if([protocol isEqualToString:@"ssl"] || [protocol isEqualToString:@"wss"])
    {
        for(i = 0; i < sizeof(ssldefaults)/sizeof(ssldefaults[0]); i += 2)
        {
            [properties setProperty:ssldefaults[i] value:ssldefaults[i+1]];
        }
    }

    if(sliced)
    {
        [properties setProperty:@"Ice.Default.SlicedFormat" value:@"1"];
    }
    else if(encoding10)
    {
        [properties setProperty:@"Ice.Default.EncodingVersion" value:@"1.0"];
    }
#endif

    NSArray* args = [properties parseIceCommandLineOptions:[ICEUtil argsToStringSeq:*argc argv:argv]];
    [ICEUtil stringSeqToArgs:args argc:argc argv:argv];

    return properties;
}

id<ICEProperties>
defaultClientProperties(int* argc, char** argv)
{
    id<ICEProperties> properties = [ICEUtil createProperties];

#if TARGET_OS_IPHONE
    static NSString* defaults[] =
    {
        @"Ice.NullHandleAbort", @"1",
        @"Ice.Warn.Connections", @"1",
        @"Ice.Default.Host", @"127.0.0.1",
        @"Ice.Trace.Network", @"0",
        @"Ice.Trace.Protocol", @"0"
    };

    static NSString* ssldefaults[] =
    {
        @"Ice.Override.ConnectTimeout", @"10000", // COMPILERFIX: Workaround for SSL hang on iOS devices
        @"IceSSL.CheckCertName", @"0",
        @"IceSSL.CAs", @"cacert.der",
        @"IceSSL.CertFile", @"client.p12",
        @"IceSSL.Password", @"password"
    };

    int i;
    for(i = 0; i < sizeof(defaults)/sizeof(defaults[0]); i += 2)
    {
        [properties setProperty:defaults[i] value:defaults[i+1]];
    }

    [properties setProperty:@"Ice.Default.Protocol" value:protocol];
    if([protocol isEqualToString:@"ssl"] || [protocol isEqualToString:@"wss"])
    {
        for(i = 0; i < sizeof(ssldefaults)/sizeof(ssldefaults[0]); i += 2)
        {
            [properties setProperty:ssldefaults[i] value:ssldefaults[i+1]];
        }
    }
    if(sliced)
    {
        [properties setProperty:@"Ice.Default.SlicedFormat" value:@""];
    }
    else if(encoding10)
    {
        [properties setProperty:@"Ice.Default.EncodingVersion" value:@"1.0"];
    }
#endif

    NSArray* args = [properties parseIceCommandLineOptions:[ICEUtil argsToStringSeq:*argc argv:argv]];
    [ICEUtil stringSeqToArgs:args argc:argc argv:argv];
    return properties;
}

#if TARGET_OS_IPHONE

void
TestCommonInit(id target, SEL output)
{
    outputTarget = target;
    outputSelector = output;
}

void
TestCommonTestInit(id r, SEL ready, NSString* p, BOOL sl, BOOL e10)
{
    testRun = r;
    readySelector = ready;
    protocol = p;
    sliced = sl;
    encoding10 = e10;
}

void
serverReady(id<ICECommunicator> c)
{
    ICE_RELEASE(communicator);
    communicator = ICE_RETAIN(c);
    [testRun performSelectorOnMainThread:readySelector withObject:nil waitUntilDone:NO];
}

void
serverStop()
{
    @try
    {
        [communicator shutdown];
    }
    @catch(ICECommunicatorDestroyedException* ex)
    {
        // Ignore, it's possible the test shutdown the server explicitly already.
    }
}

void
tprintf(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    NSString* s = ICE_AUTORELEASE([[NSString alloc] initWithFormat:[NSString stringWithCString:fmt
                                                                                      encoding:NSUTF8StringEncoding]
                                                         arguments:va]);
    va_end(va);
    [outputTarget performSelectorOnMainThread:outputSelector withObject:s waitUntilDone:NO];
}

void
testFailed(const char* expr, const char* file, unsigned int line)
{
    tprintf("failed!\n");
    tprintf("%s:%u: assertion `%s' failed\n", file, line, expr);
    @throw ICE_AUTORELEASE([[TestFailedException alloc] init]);
}

#else

#include <stdio.h>

void
tprintf(const char* fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    NSString* s = ICE_AUTORELEASE([[NSString alloc] initWithFormat:[NSString stringWithCString:fmt
                                                                                      encoding:NSUTF8StringEncoding]
                                                         arguments:va]);
    va_end(va);
    fputs([s UTF8String], stdout);
    fflush(stdout);
}

void
serverReady(id<ICECommunicator> c)
{
}

void
testFailed(const char* expr, const char* file, unsigned int line)
{
    tprintf("failed!\n");
    tprintf("%s:%u: assertion `%s' failed\n", file, line, expr);
    abort();
}
#endif
