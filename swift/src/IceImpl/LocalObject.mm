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
        std::unordered_map<void*, __weak ICELocalObject*>::const_iterator p = cachedObjects->find(cppObject.get());
        if (p != cachedObjects->end())
        {
            return p->second;
        }
        else
        {
            return [[[self class] alloc] initWithCppObject:std::move(cppObject)];
        }
    }
}

- (void)dealloc
{
    @synchronized([ICELocalObject class])
    {
        assert(_cppObject != nullptr);
        cachedObjects->erase(_cppObject.get());
        _cppObject = nullptr;
    }
}

@end
