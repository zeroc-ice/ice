// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/LocalObject.h>

namespace IceUtil
{
class Shared;
}

@interface ICELocalObject ()
-(id) initWithCxxObject:(IceUtil::Shared*)arg;

//
// Note: the returned object is NOT retained. It must be held
// some other way by the calling thread.
//
+(id) getLocalObjectWithCxxObjectNoAutoRelease:(IceUtil::Shared*)arg;

+(id) localObjectWithCxxObjectNoAutoRelease:(IceUtil::Shared*)arg;
+(id) localObjectWithCxxObjectNoAutoRelease:(IceUtil::Shared*)arg allocator:(SEL)alloc;
+(id) localObjectWithCxxObject:(IceUtil::Shared*)arg;
+(id) localObjectWithCxxObject:(IceUtil::Shared*)arg allocator:(SEL)alloc;

-(IceUtil::Shared*) cxxObject;
@end
