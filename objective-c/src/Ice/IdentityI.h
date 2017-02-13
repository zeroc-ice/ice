// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Identity.h>

#include <Ice/Identity.h>

@interface ICEIdentity (ICEInternal)
-(ICEIdentity*)initWithIdentity:(const Ice::Identity&)arg;
-(Ice::Identity)identity;
+(ICEIdentity*)identityWithIdentity:(const Ice::Identity&)arg;
@end
