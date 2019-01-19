//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <slicing/objects/TestI.h>
#include <TestCommon.h>

static int
run(id<ICECommunicator> communicator)
{
    id<ICEProperties> properties = [communicator getProperties];
    [properties setProperty:@"Ice.Warn.Dispatch" value:@"0"];
    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -p 12010"];
    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];
    ICEObject* object = [TestSlicingObjectsServerI serverI];
    [adapter add:object identity:[ICEUtil stringToIdentity:@"Test"]];
    [adapter activate];

    serverReady(communicator);

    [communicator waitForShutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main slicingObjectsServer
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
            initData.properties = defaultServerProperties(&argc, argv);
#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestSlicingObjectsServer", @"::Test",
                                      @"TestSlicingObjectsShared", @"::TestShared",
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
