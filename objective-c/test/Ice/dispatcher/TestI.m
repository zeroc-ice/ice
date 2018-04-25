// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <dispatcher/TestI.h>

#import <Foundation/NSThread.h>

@implementation TestDispatcherTestIntfI
-(void) op:(ICECurrent*)current
{
}
-(void) sleep:(ICEInt)to current:(ICECurrent*)current
{
    [NSThread sleepForTimeInterval:to / 1000.0];
}
-(void) opWithPayload:(ICEMutableByteSeq*)data current:(ICECurrent*)current
{
}
-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
@end

@implementation TestDispatcherTestIntfControllerI
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
-(void) holdAdapter:(ICECurrent*)current
{
    [_adapter hold];
}
-(void) resumeAdapter:(ICECurrent*)current
{
    [_adapter activate];
}
@end
