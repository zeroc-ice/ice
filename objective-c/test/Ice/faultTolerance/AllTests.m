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
#import <FaultToleranceTest.h>

#import <Foundation/Foundation.h>

@interface Callback : NSObject
{
    BOOL called;
    NSCondition* cond;
    ICEInt pid;
}
-(void) check;
-(void) called;
-(ICEInt) pid;
@end

@implementation Callback
-(id) init
{
    if(![super init])
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
-(void) pidResponse:(ICEInt)p
{
    pid = p;
    [self called];
}

-(void) pidException:(ICEException*)ex
{
    test(NO);
}
-(ICEInt) pid
{
    return pid;
}
-(void) shutdownResponse
{
    [self called];
}

-(void) shutdownException:(ICEException*)ex
{
    test(NO);
}
-(void) abortResponse
{
    test(NO);
}
-(void) abortException:(ICEException*)ex
{
    @try
    {
        @throw ex;
    }
    @catch(ICEConnectionLostException*)
    {
    }
    @catch(ICEConnectFailedException*)
    {
    }
    @catch(ICEException* ex)
    {
        NSLog(@"%@", ex);
        test(NO);
    }
    [self called];
}
-(void) idempotentAbortResponse
{
    test(NO);
}
-(void) idempotentAbortException:(ICEException*)ex
{
    @try
    {
        @throw ex;
    }
    @catch(ICEConnectionLostException*)
    {
    }
    @catch(ICEConnectFailedException*)
    {
    }
    @catch(ICEException* ex)
    {
        NSLog(@"%@", ex);
        test(NO);
    }
    [self called];
}
@end

void
allTests(id<ICECommunicator> communicator, NSArray* ports)
{
    tprintf("testing stringToProxy... ");
    NSString* ref = @"test";
    for(NSNumber* p in ports)
    {
        ref = [ref stringByAppendingString:@":default -p "];
        ref = [ref stringByAppendingString:[p stringValue]];
    }
    id<ICEObjectPrx> base = [communicator stringToProxy:ref];
    test(base);
    tprintf("ok\n");

    tprintf("testing checked cast... ");
    id<TestTestIntfPrx> obj = [TestTestIntfPrx checkedCast:base];
    test(obj);
    test([obj isEqual:base]);
    tprintf("ok\n");

    int oldPid = 0;
    BOOL ami = NO;
    unsigned int i, j;
    for(i = 1, j = 0; i <= [ports count]; ++i, ++j)
    {
        if(j > 3)
        {
            j = 0;
            ami = !ami;
        }

        if(!ami)
        {
            tprintf("testing server %d... ", i);
            int pid = [obj pid];
            test(pid != oldPid);
            tprintf("ok\n");
            oldPid = pid;
        }
        else
        {
            tprintf("testing server %d with AMI... ", i);
            Callback* cb = [[Callback alloc] init];
            [obj begin_pid:^(ICEInt pid) { [cb pidResponse:pid]; }
                 exception:^(ICEException* ex) { [cb pidException:ex]; } ];
            [cb check];
            int pid = [cb pid];
            test(pid != oldPid);
            oldPid = pid;
            ICE_RELEASE(cb);
            tprintf("ok\n");
        }

        if(j == 0)
        {
            if(!ami)
            {
                tprintf("shutting down server %d... ", i);
                [obj shutdown];
                tprintf("ok\n");
            }
            else
            {
                tprintf("shutting down server %d with AMI... ", i);
                Callback* cb = [[Callback alloc] init];
                [obj begin_shutdown:^{ [cb shutdownResponse]; }
                          exception:^(ICEException* ex) { [cb shutdownException:ex]; }];
                [cb check];
                ICE_RELEASE(cb);
                tprintf("ok\n");
            }
        }
        else if(j == 1 || i + 1 > [ports count])
        {
            if(!ami)
            {
                tprintf("aborting server %d... ", i);
                @try
                {
                    [obj abort];
                    test(NO);
                }
                @catch(ICEConnectionLostException*)
                {
                    tprintf("ok\n");
                }
                @catch(ICEConnectFailedException*)
                {
                    tprintf("ok\n");
                }
            }
            else
            {
                tprintf("aborting server %d with AMI... ", i);
                Callback* cb = [[Callback alloc] init];
                [obj begin_abort:^{ [cb abortResponse]; } exception:^(ICEException* ex) { [cb abortException:ex]; }];
                [cb check];
                ICE_RELEASE(cb);
                tprintf("ok\n");
            }
        }
        else if(j == 2 || j == 3)
        {
            if(!ami)
            {
                tprintf("aborting server %d and #%d with idempotent call... ", i, i + 1);
                @try
                {
                    [obj idempotentAbort];
                    test(NO);
                }
                @catch(ICEConnectionLostException*)
                {
                    tprintf("ok\n");
                }
                @catch(ICEConnectFailedException*)
                {
                    tprintf("ok\n");
                }
            }
            else
            {
                tprintf("aborting server %d and #%d with idempotent AMI call... ", i, i + 1);
                Callback* cb = [[Callback alloc] init];
                [obj begin_idempotentAbort:^{ [cb idempotentAbortResponse]; }
                       exception:^(ICEException* ex) { [cb idempotentAbortException:ex]; }];
                [cb check];
                ICE_RELEASE(cb);
                tprintf("ok\n");
            }

            ++i;
        }
        else
        {
            test(NO);
        }
    }

    tprintf("testing whether all servers are gone... ");
    @try
    {
        [obj ice_ping];
        test(NO);
    }
    @catch(ICELocalException*)
    {
        tprintf("ok\n");
    }
}
