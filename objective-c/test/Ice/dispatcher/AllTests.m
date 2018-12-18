// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <DispatcherTest.h>

#import <Foundation/Foundation.h>

static BOOL isDispatcherThread()
{
    return strcmp(dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL), "Dispatcher") == 0;
}

@interface TestDispatcherCallback : NSObject
{
    BOOL called;
    NSCondition* cond;
}
-(void) check;
-(void) called;
@end

@implementation TestDispatcherCallback
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
    test(isDispatcherThread());
    [self called];
}

-(void) exception:(ICEException*)ex
{
    test([ex isKindOfClass:[ICENoEndpointException class]]);
    test(isDispatcherThread());
    [self called];
}
-(void) responseEx
{
    test(NO);
}

-(void) exceptionEx:(ICEException*)ex
{
    test([ex isKindOfClass:[ICEInvocationTimeoutException class]]);
    test(isDispatcherThread());
    [self called];
}

-(void) payload
{
    test(isDispatcherThread());
}

-(void) ignoreEx:(ICEException*)ex
{
    test([ex isKindOfClass:[ICECommunicatorDestroyedException class]]);
}

-(void) sent:(BOOL)sentSynchronously
{
    test(sentSynchronously || isDispatcherThread());
}

@end

id<TestDispatcherTestIntfPrx>
dispatcherAllTests(id<ICECommunicator> communicator)
{
    NSString* sref = @"test:default -p 12010";
    id<ICEObjectPrx> obj = [communicator stringToProxy:sref];
    test(obj);

    id<TestDispatcherTestIntfPrx> p = [TestDispatcherTestIntfPrx uncheckedCast:obj];

    sref = @"testController:tcp -p 12011";
    obj = [communicator stringToProxy:sref];
    test(obj);

    id<TestDispatcherTestIntfControllerPrx> testController = [TestDispatcherTestIntfControllerPrx uncheckedCast:obj];

    tprintf("testing dispatcher... ");
    {
        [p op];

        TestDispatcherCallback* cb = [[TestDispatcherCallback alloc] init];
        [p begin_op:^ { [cb response]; } exception:^(ICEException* ex) { [cb exception:ex]; }];
        [cb check];

        TestDispatcherTestIntfPrx* i = [p ice_adapterId:@"dummy"];
        [i begin_op:^ { [cb response]; } exception:^(ICEException* ex) { [cb exception:ex]; }];
        [cb check];

        TestDispatcherTestIntfPrx* to = [p ice_invocationTimeout:250];
        [to begin_sleep:500 response:^ { [cb responseEx]; } exception:^(ICEException* ex) { [cb exceptionEx:ex]; }];
        [cb check];

        [testController holdAdapter];

        ICEByte d[1024];
        ICEMutableByteSeq* seq = [ICEMutableByteSeq dataWithBytes:d length:sizeof(d)];

        id<ICEAsyncResult> result;
        @autoreleasepool
        {
            id response = ^{ [cb payload]; };
            id exception = ^(ICEException* ex) { [cb ignoreEx:ex]; };
            id sent = ^(BOOL ss) { [cb sent:ss]; };

            while([(result = [p begin_opWithPayload:seq
                                           response:response
                                          exception:exception
                                               sent:sent]) sentSynchronously]);
            [testController resumeAdapter];
            [result waitForCompleted];
        }
        ICE_RELEASE(cb);
    }
    tprintf("ok\n");

    return p;
}
