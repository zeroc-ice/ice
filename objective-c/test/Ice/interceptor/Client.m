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
#import <InterceptorTest.h>
#import <interceptor/MyObjectI.h>
#import <interceptor/InterceptorI.h>

static int
run(id<ICECommunicator> communicator)
{
    //
    // Create OA and servants
    //
    id<ICEObjectAdapter> oa = [communicator createObjectAdapterWithEndpoints:@"MyOA" endpoints:@"tcp -h localhost"];
    ICEObject* servant = [TestInterceptorMyObjectI myObject];
    InterceptorI* interceptor = ICE_AUTORELEASE([[InterceptorI alloc] init:servant]);

    id<TestInterceptorMyObjectPrx> prx = [TestInterceptorMyObjectPrx uncheckedCast:[oa addWithUUID:interceptor]];

    [oa activate];

    tprintf("testing simple interceptor... ");
    test([[interceptor getLastOperation] length] == 0);
    [prx ice_ping];
    test([[interceptor getLastOperation] isEqualToString:@"ice_ping"]);
    test([interceptor getLastStatus]);
    NSString* typeId = [prx ice_id];
    test([[interceptor getLastOperation] isEqualToString:@"ice_id"]);
    test([interceptor getLastStatus]);
    test([prx ice_isA:typeId]);
    test([[interceptor getLastOperation] isEqualToString:@"ice_isA"]);
    test([interceptor getLastStatus]);
    test([prx add:33 y:12] == 45);
    test([[interceptor getLastOperation] isEqualToString:@"add"]);
    test([interceptor getLastStatus]);
    tprintf("ok\n");

    tprintf("testing retry... ");
    test([prx addWithRetry:33 y:12] == 45);
    test([[interceptor getLastOperation] isEqualToString:@"addWithRetry"]);
    test([interceptor getLastStatus]);
    tprintf("ok\n");

    tprintf("testing user exception... ");
    @try
    {
        [prx badAdd:33 y:12];
        test(NO);
    }
    @catch(TestInterceptorInvalidInputException*)
    {
        // expected
    }
    test([[interceptor getLastOperation] isEqualToString:@"badAdd"]);
    test([interceptor getLastStatus] == NO);
    tprintf("ok\n");
    tprintf("testing ONE... ");

    [interceptor clear];
    @try
    {
        [prx notExistAdd:33 y:12];
        test(NO);
    }
    @catch(ICEObjectNotExistException*)
    {
        // expected
    }
    test([[interceptor getLastOperation] isEqualToString:@"notExistAdd"]);
    tprintf("ok\n");
    tprintf("testing system exception... ");
    [interceptor clear];
    @try
    {
        [prx badSystemAdd:33 y:12];
        test(NO);
    }
    @catch(ICEUnknownLocalException*)
    {
    }
    @catch(NSException*)
    {
        test(NO);
    }
    test([[interceptor getLastOperation] isEqualToString:@"badSystemAdd"]);
    tprintf("ok\n");

    return 0;
}

#if TARGET_OS_IPHONE
#  define main interceptorClient

int
interceptorServer(int argc, char* argv[])
{
    serverReady(nil);
    return 0;
}
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
            initData.properties = defaultClientProperties(&argc, argv);
            [initData.properties setProperty:@"Ice.Warn.Dispatch" value:@"0"];
#if TARGET_OS_IPHONE
            initData.prefixTable__ = [NSDictionary dictionaryWithObjectsAndKeys:
                                      @"TestInterceptor", @"::Test",
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

