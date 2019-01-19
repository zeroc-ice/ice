//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <acm/TestI.h>
#import <TestCommon.h>

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -p 12010"];
    [[communicator getProperties] setProperty:@"TestAdapter.ACM.Timeout" value:@"0"];
    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];
    [adapter add:[ACMRemoteCommunicatorI remoteCommunicator]
        identity:[ICEUtil stringToIdentity:@"communicator"]];
    [adapter activate];

    // Disable ready print for further adapters.
    [[communicator getProperties] setProperty:@"Ice.PrintAdapterReady" value:@"0"];

    serverReady(communicator);

    [communicator waitForShutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main acmServer
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

    @autoreleasepool
    {
        int status;
        id<ICECommunicator> communicator = nil;

        @try
        {
            ICEInitializationData* initData = [ICEInitializationData initializationData];
            initData.properties = defaultServerProperties(&argc, argv);
            [initData.properties setProperty:@"Ice.Warn.Connections" value:@"0"];
            [initData.properties setProperty:@"Ice.ACM.Timeout" value:@"1"];

#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestACM", @"::Test",
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
        return status;
    }
}
