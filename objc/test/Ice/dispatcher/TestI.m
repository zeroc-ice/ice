// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <dispatcher/TestI.h>

@implementation TestDispatcherTestIntfI
-(void) op:(ICECurrent*)current
{
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
