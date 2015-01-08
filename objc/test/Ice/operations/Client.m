// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <OperationsTest.h>
#ifdef ICE_OBJC_GC
#   import <Foundation/NSGarbageCollector.h>
#endif

static int
run(id<ICECommunicator> communicator)
{
    id<TestOperationsMyClassPrx> operationsAllTests(id<ICECommunicator>, BOOL);
    id<TestOperationsMyClassPrx> myClass = operationsAllTests(communicator, NO);
    tprintf("testing server shutdown... ");
    [myClass shutdown];
    @try
    {
        [myClass opVoid];
        test(false);
    }
    @catch(ICELocalException*)
    {
        tprintf("ok\n");
    }

    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main operationsClient
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
            initData.properties = defaultClientProperties(&argc, argv);
            //
            // In this test, we need at least two threads in the
            // client side thread pool for nested AMI.
            //
            [initData.properties setProperty:@"Ice.ThreadPool.Client.Size" value:@"2"];
            [initData.properties setProperty:@"Ice.ThreadPool.Client.SizeWarn" value:@"0"];

            //
            // We must set MessageSizeMax to an explicit values, because
            // we run tests to check whether Ice.MemoryLimitException is
            // raised as expected.
            //
            [initData.properties setProperty:@"Ice.MessageSizeMax" value:@"100"];

#if TARGET_OS_IPHONE
            initData.prefixTable__ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestOperations", @"::Test", 
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
        @catch(TestFailedException* ex)
        {
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
