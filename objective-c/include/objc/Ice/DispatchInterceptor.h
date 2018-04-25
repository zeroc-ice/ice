// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Object.h>

ICE_API @protocol ICEDispatchInterceptor <ICEObject>
-(void) dispatch:(id<ICERequest>)request;
@end

ICE_API @interface ICEDispatchInterceptor : ICEServant
@end

ICE_API @interface ICEMainThreadDispatch : ICEDispatchInterceptor<ICEDispatchInterceptor>
{
    ICEObject* servant;
}

-(id)init:(ICEObject*)servant;
+(id)mainThreadDispatch:(ICEObject*)servant;
@end
