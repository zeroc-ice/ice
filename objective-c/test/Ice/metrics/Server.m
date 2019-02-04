//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice.h>
#import <metrics/TestI.h>
#import <TestCommon.h>

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -p 12010"];
    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];

    ICEObject* object = [MetricsI metrics];
    [adapter add:object identity:[ICEUtil stringToIdentity:@"metrics"]];
    [adapter activate];

    [[communicator getProperties] setProperty:@"ControllerAdapter.Endpoints" value:@"default -p 12011"];
    id<ICEObjectAdapter> controllerAdapter = [communicator createObjectAdapter:@"ControllerAdapter"];

    ICEObject* controller = ICE_AUTORELEASE([[ControllerI alloc] init:adapter]);
    [controllerAdapter add:controller identity:[ICEUtil stringToIdentity:@"controller"]];
    [controllerAdapter activate];

    serverReady(communicator);

    [communicator waitForShutdown];

    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main metricsServer
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
            [initData.properties setProperty:@"Ice.Admin.Endpoints" value:@"tcp"];
            [initData.properties setProperty:@"Ice.Admin.InstanceName" value:@"server"];
            [initData.properties setProperty:@"Ice.Warn.Connections" value:@"0"];
            [initData.properties setProperty:@"Ice.Warn.Dispatch" value:@"0"];
            [initData.properties setProperty:@"Ice.MessageSizeMax" value:@"50000"];

            [initData.properties setProperty:@"IceMX.Metrics.Debug.GroupBy" value:@"id"];
            [initData.properties setProperty:@"IceMX.Metrics.Parent.GroupBy" value:@"parent"];
            [initData.properties setProperty:@"IceMX.Metrics.All.GroupBy" value:@"none"];
            [initData.properties setProperty:@"Ice.Default.Host" value:@"127.0.0.1"];
#if TARGET_OS_IPHONE
            initData.prefixTable_ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestMetrics", @"::Test",
                                      @"ICEMX", @"::IceMX",
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
