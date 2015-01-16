// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <ami/TestI.h>
#import <TestCommon.h>
#ifdef ICE_OBJC_GC
#   import <Foundation/NSGarbageCollector.h>
#endif

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getProperties] setProperty:@"TestAMIAdapter.Endpoints" value:@"default -p 12010:udp"];
    [[communicator getProperties] setProperty:@"ControllerAdapter.Endpoints" value:@"tcp -p 12011"];
    [[communicator getProperties] setProperty:@"ControllerAdapter.ThreadPool.Size" value:@"1"];

    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAMIAdapter"];
    id<ICEObjectAdapter> adapter2 = [communicator createObjectAdapter:@"ControllerAdapter"];

#if defined(__clang__) && !__has_feature(objc_arc)
    TestAMITestIntfControllerI* testController
        = [[[TestAMITestIntfControllerI alloc] initWithAdapter:adapter] autorelease];

    [adapter add:[[[TestAMITestIntfI alloc] init] autorelease] identity:[communicator stringToIdentity:@"test"]];
#else
    TestAMITestIntfControllerI* testController
        = [[TestAMITestIntfControllerI alloc] initWithAdapter:adapter];

    [adapter add:[[TestAMITestIntfI alloc] init] identity:[communicator stringToIdentity:@"test"]];
#endif
    //[adapter activate]; // Collocated test doesn't need to activate the OA

    [adapter2 add:testController identity:[communicator stringToIdentity:@"testController"]];
    //[adapter2 activate]; // Collocated test doesn't need to activate the OA

    void amiAllTests(id<ICECommunicator>, BOOL);
    amiAllTests(communicator, true);

    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main amiServer
#endif

int
main(int argc, char* argv[])
{
    int status;
    @autoreleasepool
    {
        id<ICECommunicator> communicator = nil;

        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = defaultServerProperties(&argc, argv);
            [initData.properties setProperty:@"Ice.Warn.AMICallback" value:@"0"];
#if TARGET_OS_IPHONE
            initData.prefixTable__ = [NSDictionary dictionaryWithObjectsAndKeys:
                                  @"TestAMI", @"::Test",
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
    }
#ifdef ICE_OBJC_GC
    [[NSGarbageCollector defaultCollector] collectExhaustively];
#endif
    return status;
}
