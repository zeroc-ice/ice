// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <AMITest.h>

static int
run(id<ICECommunicator> communicator)
{
    void amiAllTests(id<ICECommunicator>, BOOL);
    amiAllTests(communicator, false);
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main amiClient
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
#if TARGET_OS_IPHONE
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
            //
            // In this test, we need at least two threads in the
            // client side thread pool for nested AMI.
            //
            [initData.properties setProperty:@"Ice.ThreadPool.Client.Size" value:@"2"];
            [initData.properties setProperty:@"Ice.ThreadPool.Client.SizeWarn" value:@"0"];
            [initData.properties setProperty:@"Ice.Warn.AMICallback" value:@"0"];

            //
            // We must set MessageSizeMax to an explicit values, because
            // we run tests to check whether Ice.MemoryLimitException is
            // raised as expected.
            //
            [initData.properties setProperty:@"Ice.MessageSizeMax" value:@"100"];

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
    return status;
}
