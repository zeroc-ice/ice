// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Object.h>

@protocol ICEDispatchInterceptor <ICEObject>
-(BOOL) dispatch:(id<ICERequest>)request;
@end

@interface ICEDispatchInterceptor : ICEServant
@end

@interface ICEMainThreadDispatch : ICEDispatchInterceptor<ICEDispatchInterceptor>
{
    ICEObject* servant;
}

-(id)init:(ICEObject*)servant;
+(id)mainThreadDispatch:(ICEObject*)servant;
@end

