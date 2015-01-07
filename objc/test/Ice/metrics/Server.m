// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <metrics/TestI.h>
#import <TestCommon.h>
#ifdef ICE_OBJC_GC
#   import <Foundation/NSGarbageCollector.h>
#endif

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -p 12010"];
    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];

    ICEObject* object = [[MetricsI alloc] init];
#if defined(__clang__) && !__has_feature(objc_arc)
    [object autorelease];
#endif
    [adapter add:object identity:[communicator stringToIdentity:@"metrics"]];
    [adapter activate];

    [[communicator getProperties] setProperty:@"ControllerAdapter.Endpoints" value:@"default -p 12011"];
    id<ICEObjectAdapter> controllerAdapter = [communicator createObjectAdapter:@"ControllerAdapter"];

    ICEObject* controller = [[ControllerI alloc] init:adapter];
#if defined(__clang__) && !__has_feature(objc_arc)
    [controller autorelease];
#endif
    [controllerAdapter add:controller identity:[communicator stringToIdentity:@"controller"]];
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
#if TARGET_OS_IPHONE
            initData.prefixTable__ = [NSDictionary dictionaryWithObjectsAndKeys:
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
