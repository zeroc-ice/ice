// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Exception.h>

#include <Ice/Exception.h>

@interface ICELocalException ()
-(id) initWithLocalException:(const Ice::LocalException&)ex;
-(void) rethrowCxx;
+(id) localExceptionWithLocalException:(const Ice::LocalException&)ex;
@end
