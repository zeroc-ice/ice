// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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

-(void) callDispatch:(ICEServant*)servant
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
    [servant iceDispatch:current is:is os:os];
}

-(void) dealloc
{
    [current release];
    [is release];
    [os release];
    [super dealloc];
}
@end
