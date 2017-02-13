// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <dispatcher/TestI.h>
#import <TestCommon.h>
#include <dispatch/dispatch.h>

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -p 12010:udp"];
    [[communicator getProperties] setProperty:@"ControllerAdapter.Endpoints" value:@"tcp -p 12011"];
    [[communicator getProperties] setProperty:@"ControllerAdapter.ThreadPool.Size" value:@"1"];

    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];
    id<ICEObjectAdapter> adapter2 = [communicator createObjectAdapter:@"ControllerAdapter"];

    TestDispatcherTestIntfControllerI* testController
        = ICE_AUTORELEASE([[TestDispatcherTestIntfControllerI alloc] initWithAdapter:adapter]);

    ICEObject* object = [TestDispatcherTestIntfI testIntf];

    [adapter add:object identity:[ICEUtil stringToIdentity:@"test"]];
    [adapter activate];

    [adapter2 add:testController identity:[ICEUtil stringToIdentity:@"testController"]];
    [adapter2 activate];

    serverReady(communicator);

    [communicator waitForShutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main dispatcherServer
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
#if TARGET_OS_IPHONE && !TARGET_IPHONE_SIMULATOR
    ICEregisterIceIAP(YES);
#endif
#endif

    int status;
    @autoreleasepool
    {
        id<ICECommunicator> communicator = nil;
        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = defaultServerProperties(&argc, argv);
            dispatch_queue_t queue = dispatch_queue_create("Dispatcher", DISPATCH_QUEUE_SERIAL);
            initData.dispatcher = ^(id<ICEDispatcherCall> call, id<ICEConnection> con) {
                dispatch_sync(queue, ^ { [call run]; });
            };
#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestDispatcher", @"::Test",
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
            [communicator destroy];
        }
    }
    return status;
}
