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
#import <TimeoutTest.h>

static int
run(id<ICECommunicator> communicator)
{
    id<TestTimeoutTimeoutPrx> timeoutAllTests(id<ICECommunicator>);
    id<TestTimeoutTimeoutPrx> timeout = timeoutAllTests(communicator);
    [timeout shutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main timeoutClient
#endif

int
main(int argc, char* argv[])
{
#ifdef ICE_STATIC_LIBS
    ICEregisterIceSSL(YES);
    ICEregisterIceWS(YES);
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
            // For this test, we want to disable retries.
            //
            [initData.properties setProperty:@"Ice.RetryIntervals" value:@"-1"];

            //
            // COMPILERFIX: Disable connect timeout introduced for
            // workaround to iOS device hangs when using SSL
            //
            [initData.properties setProperty:@"Ice.Override.ConnectTimeout" value:@""];

            //
            // Limit the send buffer size, this test relies on the socket
            // send() blocking after sending a given amount of data.
            //
            [initData.properties setProperty:@"Ice.TCP.SndSize" value:@"50000"];

            //
            // This test kills connections, so we don't want warnings.
            //
            [initData.properties setProperty:@"Ice.Warn.Connections" value:@"0"];
#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestTimeout", @"::Test",
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
