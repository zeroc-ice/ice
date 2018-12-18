// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>
#import <interceptor/MyObjectI.h>
#import <TestCommon.h>

@implementation TestInterceptorMyObjectI
-(int) add:(int)x y:(int)y current:(ICECurrent*)__unused current
{
    return x + y;
}

-(int) addWithRetry:(int)x y:(int)y current:(ICECurrent*)current
{
    id val = [current.ctx objectForKey:@"retry"];

    if(val == nil || ![(NSString*)val isEqualToString:@"no"])
    {
        @throw [TestInterceptorRetryException retryException:__FILE__ line:__LINE__];
    }
    return x + y;
}

-(int) badAdd:(int)__unused x y:(int)__unused y current:(ICECurrent*)__unused current
{
    @throw [TestInterceptorInvalidInputException invalidInputException];
}

-(int) notExistAdd:(int)__unused x y:(int)__unused y current:(ICECurrent*)__unused current
{
    @throw [ICEObjectNotExistException objectNotExistException:__FILE__ line:__LINE__];
}

-(int) badSystemAdd:(int)__unused x y:(int)__unused y current:(ICECurrent*)__unused current
{
    @throw [ICEInitializationException initializationException:__FILE__ line:__LINE__ reason:@"testing"];
}

@end
