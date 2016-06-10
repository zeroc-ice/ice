// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <Test.h>
#include <dlfcn.h>

@implementation Test

@synthesize name;
@synthesize sslSupport;
@synthesize wsSupport;
@synthesize runWithSlicedFormat;
@synthesize runWith10Encoding;

+(id)testWithName:(NSString*)name
           server:(int (*)(int, char**))server
           client:(int (*)(int, char**))client
       sslSupport:(BOOL)sslSupport
        wsSupport:(BOOL)wsSupport
  runWithSlicedFormat:(BOOL)runWithSlicedFormat
    runWith10Encoding:(BOOL)runWith10Encoding
{
    Test* t = [[Test alloc] init];
    if(t != nil)
    {
        t->name = name;
        t->server = server;
        t->client = client;
        t->sslSupport = sslSupport;
        t->wsSupport = wsSupport;
        t->runWithSlicedFormat = runWithSlicedFormat;
        t->runWith10Encoding = runWith10Encoding;
    }
#if defined(__clang__) && !__has_feature(objc_arc)
    return [t autorelease];
#else
    return t;
#endif
}
-(BOOL) hasServer
{
    return server != 0;
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
-(int)server
{
    NSAssert(server != 0, @"server != 0");
    int argc = 0;
    char** argv = 0;
    return (*server)(argc, argv);
}

-(int)client
{
    int argc = 0;
    char** argv = 0;

    NSAssert(client != 0, @"client != 0");
    return (*client)(argc, argv);
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void)dealloc
{
    [name release];
    [super dealloc];
}
#endif

@end
