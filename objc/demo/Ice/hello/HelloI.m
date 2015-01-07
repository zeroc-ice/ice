// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <HelloI.h>

#import <Foundation/NSThread.h>

#import <stdio.h>

@implementation HelloI
+(id) helloI
{
    id instance = [[HelloI alloc] init];
#if defined(__clang__) && !__has_feature(objc_arc)
    [instance autorelease];
#endif
    return instance;
}

-(void) sayHello:(int)delay current:(ICECurrent*)c
{
    if(delay != 0)
    {
        [NSThread sleepForTimeInterval:delay / 1000.f];
    }
    printf("Hello World!\n");
    fflush(stdout);
}

-(void) shutdown:(ICECurrent*)c
{
    printf("Shutting down...\n");
    fflush(stdout);
    [[c.adapter getCommunicator] shutdown];
}

@end
