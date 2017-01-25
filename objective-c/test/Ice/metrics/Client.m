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
#import <MetricsTest.h>

static int
run(id<ICECommunicator> communicator)
{
    id<TestMetricsMetricsPrx> metricsAllTests(id<ICECommunicator>);
    id<TestMetricsMetricsPrx> metrics = metricsAllTests(communicator);
    [metrics shutdown];
    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main metricsClient
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
            [initData.properties setProperty:@"Ice.Admin.Endpoints" value:@"tcp"];
            [initData.properties setProperty:@"Ice.Admin.InstanceName" value:@"client"];
            [initData.properties setProperty:@"Ice.Admin.DelayCreation" value:@"1"];
            [initData.properties setProperty:@"Ice.Warn.Connections" value:@"0"];
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
