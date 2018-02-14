// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/ImplicitContext.h>

#include <Ice/ImplicitContext.h>

@interface ICEImplicitContext : NSObject<ICEImplicitContext>
{
@private

    Ice::ImplicitContext* implicitContext__;
}
-(id) init:(Ice::ImplicitContext*)implicitContext;
+(id) implicitContextWithImplicitContext:(Ice::ImplicitContext*)implicitContext;
@end
