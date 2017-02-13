// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <Request.h>

@implementation ICERequest
-(ICECurrent*) getCurrent
{
    return current;
}

+(id) request:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os
{
    ICERequest* result = [((ICERequest*)[ICERequest alloc]) init:current is:is os:os];
    [result autorelease];
    return result;
}

-(id) init:(ICECurrent*)current_ is:(id<ICEInputStream>)is_ os:(id<ICEOutputStream>)os_
{
    if(![super init])
    {
        return nil;
    }
    current = [current_ retain];
    is = [is_ retain];
    os = [os_ retain];
    needReset = NO;
    return self;
}

-(BOOL) callDispatch:(ICEServant*)servant
{
    if(needReset == NO)
    {
        needReset = YES;
    }
    else
    {
        [is rewind];
        [os reset:NO];
    }
    return [servant dispatch__:current is:is os:os];
}

-(void) dealloc
{
    [current release];
    [is release];
    [os release];
    [super dealloc];
}
@end
