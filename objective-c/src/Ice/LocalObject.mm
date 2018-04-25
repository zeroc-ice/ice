// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <LocalObjectI.h>

#include <map>

#include <IceUtil/Shared.h>
#include <Foundation/Foundation.h>

#define CXXOBJECT ((IceUtil::Shared*)cxxObject_)

namespace
{

std::map<IceUtil::Shared*, ICELocalObject*> cachedObjects;

}

@implementation ICELocalObject

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    cxxObject_ = 0;
    return self;
}
-(id) initWithCxxObject:(IceUtil::Shared*)arg
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    cxxObject_ = arg;
    CXXOBJECT->__incRef();

    //
    // No synchronization because initWithCxxObject is always called with the wrapper class object locked
    //
    assert(cachedObjects.find(CXXOBJECT) == cachedObjects.end());
    cachedObjects.insert(std::make_pair(CXXOBJECT, self));
    return self;
}

-(IceUtil::Shared*) cxxObject
{
    return CXXOBJECT;
}

-(void) dealloc
{
    if(cxxObject_)
    {
        //
        // No synchronization because dealloc is always called with the wrapper class object locked
        //
        cachedObjects.erase(CXXOBJECT);
        CXXOBJECT->__decRef();
        cxxObject_ = 0;
    }

    [super dealloc];
}

+(id) getLocalObjectWithCxxObjectNoAutoRelease:(IceUtil::Shared*)arg
{
    //
    // Note: the returned object is NOT retained. It must be held
    // some other way by the calling thread.
    //

    if(arg == 0)
    {
        return nil;
    }

    @synchronized([ICELocalObject class])
    {
        std::map<IceUtil::Shared*, ICELocalObject*>::const_iterator p = cachedObjects.find(arg);
        if(p != cachedObjects.end())
        {
            return p->second;
        }
    }
    return nil;
}

+(id) localObjectWithCxxObjectNoAutoRelease:(IceUtil::Shared*)arg
{
    if(arg == 0)
    {
        return nil;
    }

    @synchronized([ICELocalObject class])
    {
        std::map<IceUtil::Shared*, ICELocalObject*>::const_iterator p = cachedObjects.find(arg);
        if(p != cachedObjects.end())
        {
            return [p->second retain];
        }
        else
        {
            return [[self alloc] initWithCxxObject:arg];
        }
    }
    return nil; // Keep the compiler happy.
}

+(id) localObjectWithCxxObjectNoAutoRelease:(IceUtil::Shared*)arg allocator:(SEL)alloc
{
    if(arg == 0)
    {
        return nil;
    }

    @synchronized([ICELocalObject class])
    {
        std::map<IceUtil::Shared*, ICELocalObject*>::const_iterator p = cachedObjects.find(arg);
        if(p != cachedObjects.end())
        {
            return [p->second retain];
        }
        else
        {
            return [self performSelector:alloc withObject:[NSValue valueWithPointer:arg]];
        }
    }
    return nil; // Keep the compiler happy.
}

+(id) localObjectWithCxxObject:(IceUtil::Shared*)arg
{
    return [[self localObjectWithCxxObjectNoAutoRelease:arg] autorelease];
}

+(id) localObjectWithCxxObject:(IceUtil::Shared*)arg allocator:(SEL)alloc
{
    return [[self localObjectWithCxxObjectNoAutoRelease:arg allocator:alloc] autorelease];
}

-(id) retain
{
    NSIncrementExtraRefCount(self);
    return self;
}

-(oneway void) release
{
    @synchronized([ICELocalObject class])
    {
        if(NSDecrementExtraRefCountWasZero(self))
        {
            [self dealloc];
        }
    }
}
@end
