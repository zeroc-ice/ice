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
#import <RetryTest.h>

static int
run(id<ICECommunicator> communicator)
{
    TestRetryRetryPrx* retryAllTests(id<ICECommunicator>);
    TestRetryRetryPrx* retry = retryAllTests(communicator);
    [retry shutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main retryClient
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

            //
            // This test kills connections, so we don't want warnings.
            //
            [initData.properties setProperty:@"Ice.Warn.Connections" value:@"0"];

            [initData.properties setProperty:@"Ice.RetryIntervals" value:@"0 1 10 1"];

#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestRetry", @"::Test",
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
            [communicator destroy];
        }
    }
    return status;
}
