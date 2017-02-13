// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice.h>

@interface InterceptorI : ICEDispatchInterceptor <ICEDispatchInterceptor>
{
    ICEObject* servant;
    NSString* lastOperation;
    BOOL lastStatus;
}

-(id) init:(ICEObject*)servant;
-(BOOL) getLastStatus;
-(NSString*) getLastOperation;
-(void) clear;

@end

