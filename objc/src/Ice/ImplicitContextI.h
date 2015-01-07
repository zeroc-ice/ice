// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/ImplicitContext.h>

#include <IceCpp/ImplicitContext.h>

@interface ICEImplicitContext : NSObject<ICEImplicitContext>
{
@private

    Ice::ImplicitContext* implicitContext__;
}
-(id) init:(Ice::ImplicitContext*)implicitContext;
+(id) implicitContextWithImplicitContext:(Ice::ImplicitContext*)implicitContext;
@end
