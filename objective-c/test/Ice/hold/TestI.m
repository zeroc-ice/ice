// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <hold/TestI.h>
#import <TestCommon.h>

#import <Foundation/Foundation.h>

@implementation Timer
-(id) init
{
    self = [super init];
    if(self != nil)
    {
        queue = dispatch_queue_create("timer", NULL);
    }
    return self;
}
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    dispatch_release(queue);
    [super dealloc];
}
#endif
-(void) schedule:(void(^)())callback timeout:(ICEInt)t 
{
    dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, queue);
    dispatch_source_set_timer(timer, dispatch_time(DISPATCH_TIME_NOW, t * NSEC_PER_MSEC), DISPATCH_TIME_FOREVER, 0);
    dispatch_source_set_event_handler(timer, ^{
            callback();
            dispatch_source_cancel(timer);
#if defined(__clang__) && !__has_feature(objc_arc)
            dispatch_release(timer);
#endif
        });
    dispatch_resume(timer);
}
@end

@implementation HoldI
-(id) init
{
    self = [super init];
    if(self != nil)
    {
        timer = [Timer new];
    }
    return self;
}
#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [timer release];
    [super dealloc];
}
#endif
-(void) putOnHold:(ICEInt)milliSeconds current:(ICECurrent*)current
{
    if(milliSeconds < 0)
    {
        [current.adapter hold];
    }
    else if(milliSeconds == 0)
    {
        [current.adapter hold];
        [current.adapter activate];
    }
    else
    {
        [timer schedule:^{ 
                @try
                {
                    [current.adapter hold]; 
                    [current.adapter activate]; 
                }
                @catch(ICEObjectAdapterDeactivatedException* ex)
                {
                }
                @catch(id ex)
                {
                    test(NO);
                }
            } timeout:milliSeconds];
    }
}

-(void) waitForHold:(ICECurrent*)current
{
    [timer schedule:^{ 
            @try
            {
                [current.adapter waitForHold]; 
                [current.adapter activate]; 
            }
            @catch(ICEObjectAdapterDeactivatedException* ex)
            {
                test(NO);
            }
            @catch(id ex)
            {
                test(NO);
            }
        } timeout:0];
}

-(ICEInt) set:(ICEInt)value delay:(ICEInt)delay current:(ICECurrent*)current
{
    [NSThread sleepForTimeInterval:delay / 1000.0];
    @synchronized(self)
    {
        ICEInt tmp = last;
        last = value;
        return tmp;
    }
    return 0;
}


-(void) setOneway:(ICEInt)value expected:(ICEInt)expected current:(ICECurrent*)current
{
    @synchronized(self)
    {
        test(last == expected);
        last = value;
    }
}

-(void) shutdown:(ICECurrent*)current
{
    [current.adapter hold];
    [[current.adapter getCommunicator] shutdown];
}

@end
