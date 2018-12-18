// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <TimeoutTest.h>

#import <Foundation/Foundation.h>

@interface TestTimeoutCallback : NSObject
{
    BOOL called;
    NSCondition* cond;
}
-(void) check;
-(void) called;
@end

@implementation TestTimeoutCallback
-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    cond = [[NSCondition alloc] init];
    return self;
}

+(id) create
{
    return ICE_AUTORELEASE([[TestTimeoutCallback alloc] init]);
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [cond release];
    [super dealloc];
}
#endif

-(void) check
{
    [cond lock];
    while(!called)
    {
        [cond wait];
    }
    called = NO;
    [cond unlock];
}
-(void) called
{
    [cond lock];
    called = YES;
    [cond signal];
    [cond unlock];
}
-(void) response
{
    [self called];
}
-(void) exception:(ICEException*)__unused ex
{
    test(NO);
}
-(void) responseEx
{
    test(NO);
}
-(void) exceptionEx:(ICEException*)ex
{
    test([ex isKindOfClass:[ICEInvocationTimeoutException class]]);
    [self called];
}
@end

id<ICEConnection>
connectPrx(id<TestTimeoutTimeoutPrx> prx)
{
    int nRetry = 10;
    while(--nRetry > 0)
    {
        @try
        {
            [prx ice_getConnection]; // Establish connection
            break;
        }
        @catch(ICEConnectTimeoutException*)
        {
            // Can sporadically occur with slow machines
        }
    }
    return [prx ice_getConnection];
}

void
timeoutAllTests(id<ICECommunicator> communicator)
{
    NSString* sref = @"timeout:default -p 12010";
    id<ICEObjectPrx> obj = [communicator stringToProxy:sref];
    test(obj);

    id<TestTimeoutTimeoutPrx> timeout = [TestTimeoutTimeoutPrx checkedCast:obj];
    test(timeout);

    id<TestTimeoutControllerPrx> controller =
        [TestTimeoutControllerPrx checkedCast:[communicator stringToProxy:@"controller:default -p 12011"]];
    test(controller);

    tprintf("testing connect timeout... ");
    {
        //
        // Expect ConnectTimeoutException.
        //
        id<TestTimeoutTimeoutPrx> to = [TestTimeoutTimeoutPrx uncheckedCast:[obj ice_timeout:500]];
        [controller holdAdapter:-1];
        @try
        {
            [to op];
            test(NO);
        }
        @catch(ICEConnectTimeoutException*)
        {
            // Expected.
        }
        [controller resumeAdapter];
        [timeout op]; // Ensure adapter is active.
    }
    {
        //
        // Expect success.
        //
        id<TestTimeoutTimeoutPrx> to = [TestTimeoutTimeoutPrx uncheckedCast:[obj ice_timeout:2000]];
        [controller holdAdapter:200];
        [[to ice_getConnection] close:ICEConnectionCloseForcefully]; // Force a reconnect.
        @try
        {
            [to op];
        }
        @catch(ICEConnectTimeoutException*)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    // The sequence needs to be large enough to fill the write/recv buffers
    TestTimeoutByteSeq* seq = [TestTimeoutMutableByteSeq dataWithLength:2000000];

    tprintf("testing connection timeout... ");
    {
        //
        // Expect TimeoutException.
        //
        id<TestTimeoutTimeoutPrx> to = [TestTimeoutTimeoutPrx uncheckedCast:[obj ice_timeout:250]];
        connectPrx(to);
        [controller holdAdapter:-1];
        @try
        {
            [to sendData:seq];
            test(NO);
        }
        @catch(ICETimeoutException*)
        {
            // Expected.
        }
        [controller resumeAdapter];
        [timeout op]; // Ensure adapter is active.
    }
    {
        //
        // Expect success.
        //
        id<TestTimeoutTimeoutPrx> to = [TestTimeoutTimeoutPrx uncheckedCast:[obj ice_timeout:2000]];
        [controller holdAdapter:200];
        @try
        {
            TestTimeoutByteSeq* seq2 = [TestTimeoutMutableByteSeq dataWithLength:1000000];
            [to sendData:seq2];
        }
        @catch(ICETimeoutException*)
        {
            test(NO);
        }
    }
    tprintf("ok\n");

    tprintf("testing invocation timeout... ");
    {
        id<ICEConnection> connection = [obj ice_getConnection];
        id<TestTimeoutTimeoutPrx> to = [TestTimeoutTimeoutPrx uncheckedCast:[obj ice_invocationTimeout:100]];
        test(connection == [to ice_getConnection]);
        @try
        {
            [to sleep:500];
            test(NO);
        }
        @catch(ICEInvocationTimeoutException*)
        {
        }
        [obj ice_ping];
        to = [TestTimeoutTimeoutPrx uncheckedCast:[obj ice_invocationTimeout:500]];
        test(connection == [to ice_getConnection]);
        @try
        {
            [to sleep:100];
        }
        @catch(ICEInvocationTimeoutException*)
        {
            test(NO);
        }
        test(connection == [to ice_getConnection]);
    }
    {
        //
        // Expect InvocationTimeoutException.
        //
        id<TestTimeoutTimeoutPrx> to = [TestTimeoutTimeoutPrx uncheckedCast:[obj ice_invocationTimeout:100]];
        TestTimeoutCallback* cb = [TestTimeoutCallback create];
        [to begin_sleep:500 response:^ { [cb responseEx]; } exception:^(ICEException* ex) { [cb exceptionEx:ex]; }];
        [cb check];
        [obj ice_ping];
    }
    {
        //
        // Expect Success.
        //
        id<TestTimeoutTimeoutPrx> to = [TestTimeoutTimeoutPrx uncheckedCast:[obj ice_invocationTimeout:500]];
        TestTimeoutCallback* cb = [TestTimeoutCallback create];
        [to begin_sleep:100 response:^ { [cb response]; } exception:^(ICEException* ex) { [cb exception:ex]; }];
        [cb check];
    }
    {
        //
        // Backward compatible connection timeouts
        //
        id<TestTimeoutTimeoutPrx> to =
            [TestTimeoutTimeoutPrx uncheckedCast:[[obj ice_invocationTimeout:-2] ice_timeout:250]];
        id<ICEConnection> con = connectPrx(to);
        @try
        {
            [to sleep:750];
            test(NO);
        }
        @catch(ICETimeoutException*)
        {
            @try
            {
                [con getInfo];
                test(NO);
            }
            @catch(ICETimeoutException*)
            {
                // Connection got closed as well.
            }
        }
        [obj ice_ping];

        @try
        {
            con = connectPrx(to);
            [to end_sleep:[to begin_sleep:750]];
            test(NO);
        }
        @catch(ICETimeoutException*)
        {
            @try
            {
                [con getInfo];
                test(NO);
            }
            @catch(ICETimeoutException*)
            {
                // Connection got closed as well.
            }
        }
        [obj ice_ping];
    }
    tprintf("ok\n");

    tprintf("testing timeout overrides... ");
    {
        //
        // TestTimeout Ice.Override.Timeout. This property overrides all
        // endpoint timeouts.
        //
        ICEInitializationData* initData = [ICEInitializationData initializationData];
        [initData setProperties:[[communicator getProperties] clone]];
        [initData.properties setProperty:@"Ice.Override.ConnectTimeout" value:@"250"];
        [initData.properties setProperty:@"Ice.Override.Timeout" value:@"100"];
        id<ICECommunicator> comm = [ICEUtil createCommunicator:initData];
        id<TestTimeoutTimeoutPrx> to = [TestTimeoutTimeoutPrx uncheckedCast:[comm stringToProxy:sref]];
        connectPrx(to);
        [controller holdAdapter:-1];
        @try
        {
            [to sendData:seq];
            test(NO);
        }
        @catch(ICETimeoutException*)
        {
            // Expected.
        }
        [controller resumeAdapter];
        [timeout op]; // Ensure adapter is active.
        //
        // Calling ice_timeout() should have no effect.
        //
        to = [TestTimeoutTimeoutPrx uncheckedCast:[to ice_timeout:1000]];
        connectPrx(to);
        [controller holdAdapter:-1];
        @try
        {
            [to sendData:seq];
            test(NO);
        }
        @catch(ICETimeoutException*)
        {
            // Expected.
        }
        [controller resumeAdapter];
        [timeout op]; // Ensure adapter is active.
        [comm destroy];
    }
    {
        //
        // TestTimeout Ice.Override.ConnectTimeout.
        //
        ICEInitializationData* initData = [ICEInitializationData initializationData];
        [initData setProperties:[[communicator getProperties] clone]];
        [[initData properties] setProperty:@"Ice.Override.ConnectTimeout" value:@"250"];
        id<ICECommunicator> comm = [ICEUtil createCommunicator:initData];
        [controller holdAdapter:-1];
        id<TestTimeoutTimeoutPrx> to = [TestTimeoutTimeoutPrx uncheckedCast:[comm stringToProxy:sref]];
        @try
        {
            [to op];
            test(NO);
        }
        @catch(ICEConnectTimeoutException*)
        {
            // Expected.
        }
        [controller resumeAdapter];
        [timeout op]; // Ensure adapter is active.
        //
        // Calling ice_timeout() should have no effect on the connect timeout.
        //
        [controller holdAdapter:-1];
        to = [TestTimeoutTimeoutPrx uncheckedCast:[to ice_timeout:1000]];
        @try
        {
            [to op];
            test(NO);
        }
        @catch(ICEConnectTimeoutException*)
        {
            // Expected.
        }
        [controller resumeAdapter];
        [timeout op]; // Ensure adapter is active.
        //
        // Verify that timeout set via ice_timeout() is still used for requests.
        //
        to = [TestTimeoutTimeoutPrx uncheckedCast:[to ice_timeout:100]];
        connectPrx(to);
        [to ice_getConnection]; // Establish connection
        [controller holdAdapter:-1];
        @try
        {
            [to sendData:seq];
            test(NO);
        }
        @catch(ICETimeoutException*)
        {
            // Expected.
        }
        [controller resumeAdapter];
        [timeout op]; // Ensure adapter is active.
        [comm destroy];
    }
    tprintf("ok\n");

    [controller shutdown];
}
