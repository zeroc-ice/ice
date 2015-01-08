// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <hold/TestI.h>
#import <TestCommon.h>
#ifdef ICE_OBJC_GC
#   import <Foundation/NSGarbageCollector.h>
#endif

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getProperties] setProperty:@"TestAdapter1.Endpoints" value:@"default -p 12010 -t 10000:udp"];
    [[communicator getProperties] setProperty:@"TestAdapter1.ThreadPool.Size" value:@"5"]; 
    [[communicator getProperties] setProperty:@"TestAdapter1.ThreadPool.SizeMax" value:@"5"]; 
    [[communicator getProperties] setProperty:@"TestAdapter1.ThreadPool.SizeWarn" value:@"0"]; 
    [[communicator getProperties] setProperty:@"TestAdapter1.ThreadPool.Serialize" value:@"0"];
    id<ICEObjectAdapter> adapter1 = [communicator createObjectAdapter:@"TestAdapter1"];


    [[communicator getProperties] setProperty:@"TestAdapter2.Endpoints" value:@"default -p 12011 -t 10000:udp"];
    [[communicator getProperties] setProperty:@"TestAdapter2.ThreadPool.Size" value:@"5"]; 
    [[communicator getProperties] setProperty:@"TestAdapter2.ThreadPool.SizeMax" value:@"5"]; 
    [[communicator getProperties] setProperty:@"TestAdapter2.ThreadPool.SizeWarn" value:@"0"]; 
    [[communicator getProperties] setProperty:@"TestAdapter2.ThreadPool.Serialize" value:@"1"];
    id<ICEObjectAdapter> adapter2 = [communicator createObjectAdapter:@"TestAdapter2"];
    
#if defined(__clang__) && !__has_feature(objc_arc)
    [adapter1 add:[[[HoldI alloc] init] autorelease] identity:[communicator stringToIdentity:@"hold"]];
    [adapter2 add:[[[HoldI alloc] init] autorelease] identity:[communicator stringToIdentity:@"hold"]];
#else
    [adapter1 add:[[HoldI alloc] init] identity:[communicator stringToIdentity:@"hold"]];
    [adapter2 add:[[HoldI alloc] init] identity:[communicator stringToIdentity:@"hold"]];
#endif

    [adapter1 activate];
    [adapter2 activate];

    serverReady(communicator);

    [communicator waitForShutdown];

    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main holdServer
#endif

int
main(int argc, char* argv[])
{
    @autoreleasepool
    {
        int status;
        id<ICECommunicator> communicator = nil;

        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = defaultServerProperties(&argc, argv);
#if TARGET_OS_IPHONE
            initData.prefixTable__ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestHold", @"::Test", 
                                      nil];
#endif

            communicator = [ICEUtil createCommunicator:&argc argv:argv initData:initData];
            status = run(communicator);
        }
        @catch(ICEException* ex)
        {
            tprintf("%@\n", ex);
            status = EXIT_FAILURE;
        }

        if(communicator)
        {
            @try
            {
                [communicator destroy];
            }
            @catch(ICEException* ex)
            {
                tprintf("%@\n", ex);
                status = EXIT_FAILURE;
            }
        }
#ifdef ICE_OBJC_GC
        [[NSGarbageCollector defaultCollector] collectExhaustively];
#endif
        return status;
    }
}
