// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Exception.h>

#include <Ice/Exception.h>

@interface ICELocalException ()
-(id) initWithLocalException:(const Ice::LocalException&)ex;
-(void) rethrowCxx;
+(id) localExceptionWithLocalException:(const Ice::LocalException&)ex;
@end
