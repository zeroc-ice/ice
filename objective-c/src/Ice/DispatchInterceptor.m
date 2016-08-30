// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/DispatchInterceptor.h>

#import <Request.h>

#import <Foundation/NSThread.h>
#import <Foundation/NSInvocation.h>

@implementation ICEDispatchInterceptor
-(void) dispatch__:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os
{
    ICERequest* request = [ICERequest request:current is:is os:os];
    id<ICEDispatchInterceptor> dispatchInterceptor = (id<ICEDispatchInterceptor>)self;
    [dispatchInterceptor dispatch:request];
}
@end

@implementation ICEMainThreadDispatch

-(id)init:(ICEObject*)s
{
    servant = [s retain];
    return self;
}

+(id)mainThreadDispatch:(ICEObject*)s
{
    return [[[self alloc] init:s] autorelease];
}

-(void) dispatch:(id<ICERequest>)request
{
    SEL selector = @selector(ice_dispatch:);
    NSMethodSignature* sig = [[servant class] instanceMethodSignatureForSelector:selector];
    NSInvocation* inv = [NSInvocation invocationWithMethodSignature:sig];
    [inv setTarget:servant];
    [inv setSelector:selector];
    [inv setArgument:&request atIndex:2];
    [inv performSelectorOnMainThread:@selector(invokeWithTarget:) withObject:servant waitUntilDone:YES];
}

-(void)dealloc
{
    [servant release];
    [super dealloc];
}
@end
