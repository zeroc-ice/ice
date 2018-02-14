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
#import <InvokeTest.h>

#import <Foundation/Foundation.h>

static NSString* testString = @"This is a test string";

@interface TestInvokeCallback : NSObject
{
    BOOL called;
    NSCondition* cond;
    id<ICECommunicator> communicator;
}
-(BOOL) check;
-(void) called;
@end

@implementation TestInvokeCallback
-(id) initWithCommunicator:(id<ICECommunicator>)c
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    cond = [[NSCondition alloc] init];
    communicator = c;
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [cond release];
    [super dealloc];
}
#endif

-(BOOL) check
{
    [cond lock];
    while(!called)
    {
        if(![cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:50]])
        {
            return NO;
        }
    }
    called = NO;
    [cond unlock];
    return YES;
}
-(void) called
{
    [cond lock];
    called = YES;
    [cond signal];
    [cond unlock];
}
-(void) opString:(BOOL)ok outEncaps:(NSMutableData*)outEncaps
{
    if(ok)
    {
        id<ICEInputStream> inS = [ICEUtil createInputStream:communicator data:outEncaps];
        [inS startEncapsulation];
        NSString* s;
        s = [inS readString];
        test([s isEqualToString:testString]);
        s = [inS readString];
        test([s isEqualToString:testString]);
        [inS endEncapsulation];
        [self called];
    }
    else
    {
        test(NO);
    }
}

-(void) opException:(BOOL)ok outEncaps:(NSMutableData*)outEncaps
{
    if(ok)
    {
        test(NO);
    }
    else
    {
        id<ICEInputStream> inS = [ICEUtil createInputStream:communicator data:outEncaps];
        [inS startEncapsulation];
        @try
        {
            [inS throwException];
        }
        @catch(TestInvokeMyException*)
        {
            [inS endEncapsulation];
            [self called];
        }
        @catch(NSException*)
        {
            test(NO);
        }
    }
}
@end

id<TestInvokeMyClassPrx>
invokeAllTests(id<ICECommunicator> communicator)
{
    NSString* ref = @"test:default -p 12010";
    id<ICEObjectPrx> base = [communicator stringToProxy:ref];
    test(base);

    id<TestInvokeMyClassPrx> cl = [TestInvokeMyClassPrx checkedCast:base];
    test(cl);

    id<TestInvokeMyClassPrx> onewayCl = [cl ice_oneway];
    id<TestInvokeMyClassPrx> batchOneway = [cl ice_batchOneway];

    tprintf("testing ice_invoke... ");

    {
        NSData* inEncaps = [NSData data];
        NSMutableData* outEncaps;
        if(![onewayCl ice_invoke:@"opOneway" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps])
        {
            test(NO);
        }

        test([batchOneway ice_invoke:@"opOneway" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps]);
        test([batchOneway ice_invoke:@"opOneway" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps]);
        test([batchOneway ice_invoke:@"opOneway" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps]);
        test([batchOneway ice_invoke:@"opOneway" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps]);
        [batchOneway ice_flushBatchRequests];

        id<ICEOutputStream> outS = [ICEUtil createOutputStream:communicator];
        [outS startEncapsulation];
        [outS writeString:testString];
        [outS endEncapsulation];
        inEncaps = [outS finished];
        // ice_invoke
        if([cl ice_invoke:@"opString" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps])
        {
            id<ICEInputStream> inS = [ICEUtil createInputStream:communicator data:outEncaps];
            [inS startEncapsulation];
            NSString* s;
            s = [inS readString];
            test([s isEqualToString:testString]);
            s = [inS readString];
            test([s isEqualToString:testString]);
            [inS endEncapsulation];
        }
        else
        {
            test(NO);
        }
    }

    {
        NSData* inEncaps = [NSData data];
        NSMutableData* outEncaps;
        if([cl ice_invoke:@"opException" mode:ICENormal inEncaps:inEncaps outEncaps:&outEncaps])
        {
            test(NO);
        }
        else
        {
            id<ICEInputStream> inS = [ICEUtil createInputStream:communicator data:outEncaps];
            [inS startEncapsulation];
            @try
            {
                [inS throwException];
            }
            @catch(TestInvokeMyException*)
            {
            }
            @catch(NSException*)
            {
                test(NO);
            }
            [inS endEncapsulation];
        }
    }

    tprintf("ok\n");

    tprintf("testing asynchronous ice_invoke... ");

    {
        NSData* inEncaps = [NSData data];
        NSMutableData* outEncaps;
        id<ICEAsyncResult> result = [onewayCl begin_ice_invoke:@"opOneway" mode:ICENormal inEncaps:inEncaps];
        if(![onewayCl end_ice_invoke:&outEncaps result:result])
        {
            test(NO);
        }

        id<ICEOutputStream> outS = [ICEUtil createOutputStream:communicator];
        [outS startEncapsulation];
        [outS writeString:testString];
        [outS endEncapsulation];
        inEncaps = [outS finished];

        // begin_ice_invoke with no callback
        result = [cl begin_ice_invoke:@"opString" mode:ICENormal inEncaps:inEncaps];
        if([cl end_ice_invoke:&outEncaps result:result])
        {
            id<ICEInputStream> inS = [ICEUtil createInputStream:communicator data:outEncaps];
            [inS startEncapsulation];
            NSString* s;
            s = [inS readString];
            test([s isEqualToString:testString]);
            s = [inS readString];
            test([s isEqualToString:testString]);
            [inS endEncapsulation];
        }
        else
        {
            test(NO);
        };

        TestInvokeCallback* cb = [[TestInvokeCallback alloc] initWithCommunicator:communicator];
        [cl begin_ice_invoke:@"opString" mode:ICENormal inEncaps:inEncaps
            response:^(BOOL ok, NSMutableData* outEncaps) { [cb opString:ok outEncaps:outEncaps]; }
            exception:^(ICEException* ex) { test(NO); }];
        [cb check];
        ICE_RELEASE(cb);
    }

    {
        NSData* inEncaps = [NSData data];
        NSMutableData* outEncaps;

        // begin_ice_invoke with no callback
        id<ICEAsyncResult> result = [cl begin_ice_invoke:@"opException" mode:ICENormal inEncaps:inEncaps];
        if([cl end_ice_invoke:&outEncaps result:result])
        {
            test(NO);
        }
        else
        {
            id<ICEInputStream> inS = [ICEUtil createInputStream:communicator data:outEncaps];
            [inS startEncapsulation];
            @try
            {
                [inS throwException];
            }
            @catch(TestInvokeMyException*)
            {
            }
            @catch(NSException*)
            {
                test(NO);
            }
            [inS endEncapsulation];
        }

        // begin_ice_invoke with Callback_Object_ice_invoke
        TestInvokeCallback* cb = [[TestInvokeCallback alloc] initWithCommunicator:communicator];
        [cl begin_ice_invoke:@"opException" mode:ICENormal inEncaps:inEncaps
            response:^(BOOL ok, NSMutableData* outP) { [cb opException:ok outEncaps:outP]; }
            exception:^(ICEException* ex) { test(NO); }];
        [cb check];
        ICE_RELEASE(cb);
    }

    tprintf("ok\n");

    return cl;

}
