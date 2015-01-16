// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <retry/TestI.h>
#import <TestCommon.h>
#ifdef ICE_OBJC_GC
#   import <Foundation/NSGarbageCollector.h>
#endif

static int
run(id<ICECommunicator> communicator)
{
    [[communicator getProperties] setProperty:@"TestAdapter.Endpoints" value:@"default -p 12010"];
    id<ICEObjectAdapter> adapter = [communicator createObjectAdapter:@"TestAdapter"];
#if defined(__clang__) && !__has_feature(objc_arc)
    ICEObject* object = [[[TestRetryRetryI alloc] init] autorelease];
#else
    ICEObject* object = [[TestRetryRetryI alloc] init];
#endif
    [adapter add:object identity:[communicator stringToIdentity:@"retry"]];
    //[adapter activate]; // Don't activate OA to ensure collocation is used.

    TestRetryRetryPrx* retryAllTests(id<ICECommunicator>);
    TestRetryRetryPrx* retry = retryAllTests(communicator);
    [retry shutdown];

    return EXIT_SUCCESS;
}

#if TARGET_OS_IPHONE
#  define main retryCollocated
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
	        [initData.properties setProperty:@"Ice.Warn.Dispatch" value:@"0"];
            [initData.properties setProperty:@"Ice.RetryIntervals" value:@"0 1 10 1"];
#if TARGET_OS_IPHONE
            initData.prefixTable__ = [NSDictionary dictionaryWithObjectsAndKeys:
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
