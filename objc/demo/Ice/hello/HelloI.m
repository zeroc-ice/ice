// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <HelloI.h>

#import <Foundation/NSThread.h>

#import <stdio.h>

@implementation HelloI
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
