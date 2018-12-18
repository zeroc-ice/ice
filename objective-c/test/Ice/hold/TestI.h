// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <HoldTest.h>

#include <dispatch/dispatch.h>

@interface Timer : NSObject
{
    dispatch_queue_t queue;
}
-(void) schedule:(void(^)(void))callback timeout:(ICEInt)timeout;
@end

@interface HoldI : TestHoldHold<TestHoldHold>
{
    ICEInt last;
    Timer* timer;
}

@end
