//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <unordered_map>

#import "LocalObject.h"

namespace
{
    std::unordered_map<void*, __unsafe_unretained ICELocalObject*> cachedObjects;
}

@implementation ICELocalObject

-(instancetype) initWithLocalObject:(void*)object
{
    assert(object);
    self = [super init];
    if(!self)
    {
        return nil;
    }

    _object = object;

    @synchronized([ICELocalObject class])
    {
        assert(cachedObjects.find(_object) == cachedObjects.end());
        cachedObjects.insert(std::make_pair(_object, self));
    }
    return self;
}

+(instancetype) fromLocalObject:(void*)object
{
    if(object == nullptr)
    {
        return nil;
    }

    @synchronized([ICELocalObject class])
    {
        std::unordered_map<void*, __unsafe_unretained ICELocalObject*>::const_iterator p = cachedObjects.find(object);
        if(p != cachedObjects.end())
        {
            return p->second;
        }
        else
        {
            return nil;
        }
    }
    return nil; // Keep the compiler happy.
}

-(void) dealloc {
    if(_object)
    {
        @synchronized([ICELocalObject class])
        {
            cachedObjects.erase(_object);
            _object = nullptr;
        }
    }
}

@end
