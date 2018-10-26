// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <TestCommon.h>
#import <RetryTest.h>

#import <Foundation/Foundation.h>

@interface TestRetryCallback : NSObject
{
    BOOL called;
    NSCondition* cond;
}
-(void) check;
-(void) called;
@end

@implementation TestRetryCallback
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
-(void) retryOpResponse
{
    [self called];
}
-(void) retryOpException:(ICEException*)__unused ex
{
    test(NO);
}
-(void) killRetryOpResponse
{
    test(NO);
}

-(void) killRetryOpException:(ICEException*)ex
{
    test([ex isKindOfClass:[ICEConnectionLostException class]] ||
         [ex isKindOfClass:[ICEUnknownLocalException class]]);
    [self called];
};
@end

id<TestRetryRetryPrx>
retryAllTests(id<ICECommunicator> communicator)
{
    tprintf("testing stringToProxy... ");
    NSString* ref = @"retry:default -p 12010";
    id<ICEObjectPrx> base1 = [communicator stringToProxy:ref];
    test(base1);
    id<ICEObjectPrx> base2 = [communicator stringToProxy:ref];
    test(base2);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    id<TestRetryRetryPrx> retry1 = [TestRetryRetryPrx checkedCast:base1];
    test(retry1);
    test([retry1 isEqual:base1]);
    id<TestRetryRetryPrx> retry2 = [TestRetryRetryPrx checkedCast:base2];
    test(retry2);
    test([retry2 isEqual:base2]);
    tprintf("ok\n");

    tprintf("calling regular operation with first proxy... ");
    [retry1 op:NO];
    tprintf("ok\n");

    tprintf("calling operation to kill connection with second proxy... ");
    @try
    {
        [retry2 op:YES];
        test(NO);
    }
    @catch(ICEUnknownLocalException*)
    {
        // Expected with collocation
    }
    @catch(ICEConnectionLostException*)
    {
    }
    tprintf("ok\n");

    tprintf("calling regular operation with first proxy again... ");
    [retry1 op:NO];
    tprintf("ok\n");

    TestRetryCallback* cb1 = ICE_AUTORELEASE([[TestRetryCallback alloc] init]);
    TestRetryCallback* cb2 = ICE_AUTORELEASE([[TestRetryCallback alloc] init]);

    tprintf("calling regular AMI operation with first proxy... ");
    [retry1 begin_op:NO response:^{ [cb1 retryOpResponse]; }
           exception:^(ICEException* ex) { [cb1 retryOpException:ex]; }];
    [cb1 check];
    tprintf("ok\n");

    tprintf("calling AMI operation to kill connection with second proxy... ");
    [retry2 begin_op:YES response:^{ [cb2 killRetryOpResponse]; }
           exception:^(ICEException* ex) { [cb2 killRetryOpException:ex]; }];
    [cb2 check];
    tprintf("ok\n");

    tprintf("calling regular AMI operation with first proxy again... ");
    [retry1 begin_op:NO response:^{ [cb1 retryOpResponse]; }
           exception:^(ICEException* ex) { [cb1 retryOpException:ex]; }];
    [cb1 check];
    tprintf("ok\n");

    return retry1;
}
