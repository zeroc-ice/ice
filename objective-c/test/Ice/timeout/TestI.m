// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
-(void) op:(ICECurrent*)current
{
}

-(void) sendData:(TestTimeoutMutableByteSeq*)seq current:(ICECurrent*)current
{
}

-(void) sleep:(ICEInt)to current:(ICECurrent*)current
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
