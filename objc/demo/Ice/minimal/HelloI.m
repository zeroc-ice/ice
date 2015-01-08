// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <HelloI.h>

#include <stdio.h>

@implementation HelloI
+(id) helloI
{
    id instance = [[HelloI alloc] init];
#if defined(__clang__) && !__has_feature(objc_arc)
    [instance autorelease];
#endif
    return instance;
}

-(void) sayHello:(ICECurrent*)current
{
    printf("Hello World!\n");
    fflush(stdout);
}
@end
