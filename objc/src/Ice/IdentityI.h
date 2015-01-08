// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Identity.h>

#include <IceCpp/Identity.h>

@interface ICEIdentity (ICEInternal)
-(ICEIdentity*)initWithIdentity:(const Ice::Identity&)arg;
-(Ice::Identity)identity;
+(ICEIdentity*)identityWithIdentity:(const Ice::Identity&)arg;
@end
