// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <DispatcherTest.h>
#include <dispatch/dispatch.h>

static int
run(id<ICECommunicator> communicator)
{
    TestDispatcherTestIntfPrx* dispatcherAllTests(id<ICECommunicator>);
    TestDispatcherTestIntfPrx* dispatcher = dispatcherAllTests(communicator);
    [dispatcher shutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main dispatcherClient
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
            initData.properties = defaultClientProperties(&argc, argv);
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
#if defined(__clang__) && !__has_feature(objc_arc)
            dispatch_release(queue);
#endif
        }
        @catch(ICEException* ex)
        {
            tprintf("%@\n", ex);
            status = EXIT_FAILURE;
        }
        @catch(TestFailedException* ex)
        {
            status = EXIT_FAILURE;
        }

        if(communicator)
        {
            [communicator destroy];
        }
    }
    return status;
}
