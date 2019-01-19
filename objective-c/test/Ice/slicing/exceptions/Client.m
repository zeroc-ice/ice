//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <TestCommon.h>
#import <SlicingExceptionsTestClient.h>

static int
run(id<ICECommunicator> communicator)
{
    id<TestSlicingExceptionsClientTestIntfPrx> slicingExceptionsAllTests(id<ICECommunicator>);
    id<TestSlicingExceptionsClientTestIntfPrx> thrower = slicingExceptionsAllTests(communicator);
    [thrower shutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main slicingExceptionsClient
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
#if TARGET_OS_IPHONE
        initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                  @"TestSlicingExceptionsClient", @"::Test",
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
