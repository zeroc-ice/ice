// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>

#import <ami/TestI.h>
#import <TestCommon.h>

@implementation TestAMITestIntfI
-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    _cond = [[NSCondition alloc] init];
    return self;
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_cond release];
    [super dealloc];
}
#endif

-(void) op:(ICECurrent*)__unused current
{
}
-(void) opWithPayload:(ICEMutableByteSeq*)__unused data current:(ICECurrent*)__unused current
{
}
-(int) opWithResult:(ICECurrent*)__unused current
{
    return 15;
}
-(void) opWithUE:(ICECurrent*)__unused current
{
    @throw [TestAMITestIntfException testIntfException];
}

-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
-(void) opBatch:(ICECurrent *)__unused current
{
    [_cond lock];
    ++_batchCount;
    [_cond signal];
    [_cond unlock];
}
-(ICEInt) opBatchCount:(ICECurrent *)__unused current
{
    [_cond lock];
    @try
    {
        return _batchCount;
    }
    @finally
    {
        [_cond unlock];
    }
    return 0;
}
-(BOOL) waitForBatch:(ICEInt)count current:(ICECurrent *)__unused current
{
    [_cond lock];
    @try
    {
        while(_batchCount < count)
        {
            [_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:500]];
        }
        BOOL result = count == _batchCount;
        _batchCount = 0;
        return result;
    }
    @finally
    {
        [_cond unlock];
    }
    return NO;
}
-(void) close:(TestAMICloseMode)mode current:(ICECurrent *)current
{
    [current.con close:(ICEConnectionClose)mode];
}
-(void) sleep:(ICEInt)delay current:(ICECurrent *)__unused current
{
    [_cond lock];
    @try
    {
        [_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:delay / 1000.0]];
    }
    @finally
    {
        [_cond unlock];
    }
}
-(void) startDispatch:(ICECurrent*)__unused current
{
    [_cond lock];
    _dispatching = YES;
    @try
    {
        while(_dispatching)
        {
            [_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:30.0]];
        }
    }
    @finally
    {
        [_cond unlock];
    }
}
-(void) finishDispatch:(ICECurrent*)__unused current
{
    [_cond lock];
    _dispatching = NO;
    [_cond signal];
    [_cond unlock];
}
-(BOOL) supportsAMD:(ICECurrent *)__unused current
{
    return NO;
}
-(BOOL) supportsFunctionalTests:(ICECurrent *)__unused current
{
    return NO;
}

-(void) pingBiDir:(ICEIdentity*)id_ current:(ICECurrent *)current
{
    [[TestAMIPingReplyPrx uncheckedCast:[current.con createProxy:id_]] reply];
}
@end

@implementation TestAMITestOuterInnerTestIntfI
-(int) op:(ICEInt)i j:(ICEInt*)j current:(ICECurrent*)__unused current
{
    *j = i;
    return i;
}
@end

@implementation TestAMITestIntfControllerI
-(id) initWithAdapter:(id<ICEObjectAdapter>)adapter
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    _adapter = adapter;
    return self;
}
-(void) holdAdapter:(ICECurrent*)__unused current
{
    [_adapter hold];
}
-(void) resumeAdapter:(ICECurrent*)__unused current
{
    [_adapter activate];
}
@end
