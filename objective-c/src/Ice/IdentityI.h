// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#import <objc/Ice/Identity.h>

#include <Ice/Identity.h>

@interface ICEIdentity (ICEInternal)
-(ICEIdentity*)initWithIdentity:(const Ice::Identity&)arg;
-(Ice::Identity)identity;
+(ICEIdentity*)identityWithIdentity:(const Ice::Identity&)arg;
@end
