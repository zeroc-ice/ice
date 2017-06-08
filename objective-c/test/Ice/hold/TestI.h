// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <HoldTest.h>

#include <dispatch/dispatch.h>

@interface Timer : NSObject
{
    dispatch_queue_t queue;
}
-(void) schedule:(void(^)())callback timeout:(ICEInt)timeout;
@end

@interface HoldI : TestHoldHold<TestHoldHold>
{
    ICEInt last;
    Timer* timer;
}

@end
