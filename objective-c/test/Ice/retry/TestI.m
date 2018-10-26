// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <retry/TestI.h>

@implementation TestRetryRetryI
-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    _counter = 0;
    return self;
}

-(void) op:(BOOL)kill current:(ICECurrent*)current
{
   if(kill)
   {
        if([current con])
        {
            [[current con] close:ICEConnectionCloseForcefully];
        }
        else
        {
            @throw [ICEConnectionLostException connectionLostException:__FILE__ line:__LINE__];
        }
   }
}

-(ICEInt) opIdempotent:(ICEInt)nRetry current:(ICECurrent*)__unused current
{
    if(nRetry < 0)
    {
        _counter = 0;
        return 0;
    }

    if(nRetry > _counter)
    {
        ++_counter;
        @throw [ICEConnectionLostException connectionLostException:__FILE__ line:__LINE__];
    }
    int counter = _counter;
    _counter = 0;
    return counter;
}

-(void) opNotIdempotent:(ICECurrent*)__unused current
{
    @throw [ICEConnectionLostException connectionLostException:__FILE__ line:__LINE__];
}

-(void) shutdown:(ICECurrent*)current
{
    [[current.adapter getCommunicator] shutdown];
}
@end
