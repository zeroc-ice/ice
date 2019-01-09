// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <timeout/TestI.h>
#import <objc/Ice.h>

#import <Foundation/NSThread.h>

@interface ActivateAdapterThread : NSThread
{
    id<ICEObjectAdapter> adapter_;
    int timeout_;
}
-(id) init:(id<ICEObjectAdapter>)adapter timeout:(int)timeout;
+(id) activateAdapterThread:(id<ICEObjectAdapter>)adapter timeout:(int)timeout;
-(void) main;
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc;
#endif
@end

@implementation ActivateAdapterThread
-(id) init:(id<ICEObjectAdapter>)adapter timeout:(int)timeout
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    adapter_ = ICE_RETAIN(adapter);
    timeout_ = timeout;
    return self;
}

+(id) activateAdapterThread:(id<ICEObjectAdapter>)adapter timeout:(int)timeout
{
    return ICE_AUTORELEASE([[self alloc] init:adapter timeout:timeout]);
}
-(void) main
{
    [NSThread sleepForTimeInterval:timeout_ / 1000.0];
    [adapter_ activate];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [adapter_ release];
    [super dealloc];
}
#endif

@end

@implementation TimeoutI
-(void) op:(ICECurrent*)__unused current
{
}

-(void) sendData:(TestTimeoutMutableByteSeq*)__unused seq current:(ICECurrent*)__unused current
{
}

-(void) sleep:(ICEInt)to current:(ICECurrent*)__unused current
{
    [NSThread sleepForTimeInterval:to / 1000.0];
}

-(void) holdAdapter:(ICEInt)to current:(ICECurrent*)current
{
    [current.adapter hold];
    ActivateAdapterThread* thread = [ActivateAdapterThread activateAdapterThread:current.adapter timeout:to];
    [thread start];
}

-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
@end

@implementation TimeoutControllerI
-(id) init:(id<ICEObjectAdapter>)adapter
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    adapter_ = ICE_RETAIN(adapter);
    return self;
}
+(id) controller:(id<ICEObjectAdapter>)adapter
{
    return ICE_AUTORELEASE([[self alloc] init:adapter]);
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [adapter_ release];
    [super dealloc];
}
#endif

-(void) holdAdapter:(ICEInt)to current:(ICECurrent*)__unused current
{
    [adapter_ hold];
    if(to >= 0)
    {
        ActivateAdapterThread* thread = [ActivateAdapterThread activateAdapterThread:adapter_ timeout:to];
        [thread start];
    }
}

-(void) resumeAdapter:(ICECurrent*)__unused current
{
    [adapter_ activate];
}

-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
@end
