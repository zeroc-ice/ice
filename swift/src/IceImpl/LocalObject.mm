// Copyright (c) ZeroC, Inc.
#include <unordered_map>

#import "include/LocalObject.h"

namespace
{
    // We "leak" this map to avoid the destructor being called when the application is terminated.
    auto* cachedObjects = new std::unordered_map<void*, __weak ICELocalObject*>();
}

@implementation ICELocalObject

- (instancetype)initWithCppObject:(std::shared_ptr<void>)cppObject
{
    assert(cppObject);
    self = [super init];
    if (!self)
    {
        return nil;
    }

    _cppObject = std::move(cppObject);

    @synchronized([ICELocalObject class])
    {
        assert(cachedObjects->find(_cppObject.get()) == cachedObjects->end());
        cachedObjects->insert(std::make_pair(_cppObject.get(), self));
    }
    return self;
}

+ (nullable instancetype)getHandle:(std::shared_ptr<void>)cppObject
{
    if (cppObject == nullptr)
    {
        return nil;
    }
    @synchronized([ICELocalObject class])
    {
        auto p = cachedObjects->find(cppObject.get());
        if (p != cachedObjects->end())
        {
            // Get a strong reference to the object. If it's nil, preemptively remove it from the cache,
            // otherwise we'll get an assert when we try to init a new one.
            // This can happen if the object is being deallocated on another thread.
            ICELocalObject* obj = p->second;
            if (obj == nil)
            {
                cachedObjects->erase(p);
            }
            else
            {
                return obj;
            }
        }

        return [[[self class] alloc] initWithCppObject:std::move(cppObject)];
    }
}

- (void)dealloc
{
    @synchronized([ICELocalObject class])
    {
        assert(_cppObject != nullptr);

        // Find the object in the cache. If it's not there, it's likely that another thread already replaced
        // the object with a new one and then that new one was quickly deallocated.
        if (auto p = cachedObjects->find(_cppObject.get()); p != cachedObjects->end())
        {
            // The object in the cache is either nil or NOT the current object. The later can happen if this thread was
            // trying to deallocate the object while another thread was trying to create a new one.
            assert(p->second == nil || p->second != self);

            // When the last reference on this object is released, p->second is nil and we remove the stale entry from
            // the cache.
            if (p->second == nil)
            {
                cachedObjects->erase(p);
            }
        }
    }
}

@end
