// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <acm/TestI.h>

@interface ConnectionCallbackI : NSObject<ICEConnectionCallback>
{
    NSCondition* _cond;
    int _count;
}
-(void) waitForCount:(int)count;
@end


@implementation ConnectionCallbackI
-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    _cond = [[NSCondition alloc] init];
    _count = 0;
    return self;
}
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_cond release];
    [super dealloc];
}
#endif
-(void) heartbeat:(id<ICEConnection>)c
{
    [_cond lock];
    --_count;
    [_cond signal];
    [_cond unlock];
}
-(void) closed:(id<ICEConnection>)c
{
}
-(void) waitForCount:(int)count
{
    [_cond lock];
    _count = count;
    @try
    {
        while(_count > 0)
        {
            [_cond wait];
        }
    }
    @finally
    {
        [_cond unlock];
    }
}
@end

@implementation RemoteCommunicatorI
-(id<TestACMRemoteObjectAdapterPrx>) createObjectAdapter:(ICEInt)timeout close:(ICEInt)close heartbeat:(ICEInt)heartbeat
                                                 current:(ICECurrent*)current
{
    id<ICECommunicator> com = [current.adapter getCommunicator];
    id<ICEProperties> properties = [com getProperties];
    NSString* protocol = [properties getPropertyWithDefault:@"Ice.Default.Protocol" value:@"tcp"];
    NSString* host = [properties getPropertyWithDefault:@"Ice.Default.Host" value:@"127.0.0.1"];

    NSString* name = [ICEUtil generateUUID];
    if(timeout >= 0)
    {
        [properties setProperty:[name stringByAppendingString:@".ACM.Timeout"]
                          value:[NSString stringWithFormat:@"%d", timeout]];
    }
    if(close >= 0)
    {
        [properties setProperty:[name stringByAppendingString:@".ACM.Close"]
                          value:[NSString stringWithFormat:@"%d", close]];
    }
    if(heartbeat >= 0)
    {
        [properties setProperty:[name stringByAppendingString:@".ACM.Heartbeat"]
                          value:[NSString stringWithFormat:@"%d", heartbeat]];
    }
    [properties setProperty:[name stringByAppendingString:@".ThreadPool.Size"] value:@"12"];
    id<ICEObjectAdapter> adapter =
        [com createObjectAdapterWithEndpoints:name
                                    endpoints:[NSString stringWithFormat:@"%@ -h \"%@\"", protocol, host]];

    RemoteObjectAdapterI* remoteAdapter = ICE_AUTORELEASE([[RemoteObjectAdapterI alloc] initWithAdapter:adapter]);
    return [TestACMRemoteObjectAdapterPrx uncheckedCast:[current.adapter addWithUUID:remoteAdapter]];
}
-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
@end

@implementation RemoteObjectAdapterI
-(id) initWithAdapter:(id<ICEObjectAdapter>)adapter
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    _adapter = ICE_RETAIN(adapter);
    _testIntf = ICE_RETAIN([TestACMTestIntfPrx uncheckedCast:[_adapter add:[TestACMTestIntfI testIntf]
                                                    identity:[[_adapter getCommunicator] stringToIdentity:@"test"]]]);
    [_adapter activate];

    return self;
}
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [_adapter release];
    [_testIntf release];
    [super dealloc];
}
#endif

-(id<TestACMTestIntfPrx>) getTestIntf:(ICECurrent*)current
{
    return _testIntf;
}
-(void) activate:(ICECurrent*)current
{
    [_adapter activate];
}
-(void) hold:(ICECurrent*)current
{
    [_adapter hold];
}
-(void) deactivate:(ICECurrent*)current
{
    @try
    {
        [_adapter activate];
    }
    @catch(ICEObjectAdapterDeactivatedException* ex)
    {
    }
}
@end

@implementation TestACMTestIntfI
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
-(void) sleep:(ICEInt)delay current:(ICECurrent*)current
{
    [_cond lock];
    @try
    {
        [_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:delay]];
    }
    @finally
    {
        [_cond unlock];
    }
}
-(void) sleepAndHold:(ICEInt)delay current:(ICECurrent*)current
{
    [_cond lock];
    [current.adapter hold];
    @try
    {
        [_cond waitUntilDate:[NSDate dateWithTimeIntervalSinceNow:delay]];
    }
    @finally
    {
        [_cond unlock];
    }
}
-(void) interruptSleep:(ICECurrent*)current
{
    [_cond lock];
    [_cond signal];
    [_cond unlock];
}
-(void) waitForHeartbeat:(int)count current:(ICECurrent*)current
{
    ConnectionCallbackI* callback = [ConnectionCallbackI new];
    [current.con setCallback:callback];
    [callback waitForCount:count];
    ICE_RELEASE(callback);
}
@end
