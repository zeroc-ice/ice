// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Stream.h>
#import <Wrapper.h>

#include <IceCpp/Stream.h>

@interface ICEInputStream : ICEInternalWrapper<ICEInputStream>
{
    Ice::InputStream* is_;
}
+(Ice::Object*)createObjectReader:(ICEObject*)obj;
-(Ice::InputStream*) is;
@end

@interface ICEOutputStream : ICEInternalWrapper<ICEOutputStream>
{
    Ice::OutputStream* os_;
    std::map<ICEObject*, Ice::ObjectPtr>* objectWriters_;
}
-(Ice::OutputStream*) os;
@end
