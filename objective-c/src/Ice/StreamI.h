// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>

#include <Ice/Stream.h>

@interface ICEInputStream : ICELocalObject<ICEInputStream>
{
    Ice::InputStream* is_;
}
+(Ice::Object*)createObjectReader:(ICEObject*)obj;
-(Ice::InputStream*) is;
@end

@interface ICEOutputStream : ICELocalObject<ICEOutputStream>
{
    Ice::OutputStream* os_;
    std::map<ICEObject*, Ice::ObjectPtr>* objectWriters_;
}
-(Ice::OutputStream*) os;
@end
