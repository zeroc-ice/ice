// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <Wrapper.h>

//
// Using NSMapTable is necessary for this code to work properly with GC.
// We could also use NSMapTable on MacOS without GC, however,
// it's preferable to use the same code for non-GC on MacOS and other platforms
//
// #if defined(ICE_OBJC_GC) || !TARGET_OS_IPHONE
//
#ifdef ICE_OBJC_GC
   #define ICE_USE_MAP_TABLE 1
#endif

#ifdef ICE_USE_MAP_TABLE
   #import <Foundation/NSMapTable.h>
#else
   #include <map>
#endif

#include <IceUtilCpp/Shared.h>
#include <Foundation/Foundation.h>

#define CXXOBJECT ((IceUtil::Shared*)cxxObject_)

#ifndef ICE_USE_MAP_TABLE
namespace 
{

std::map<IceUtil::Shared*, ICEInternalWrapper*>* cachedObjects = 0;

class Init
{
public:

    Init()
    {
        cachedObjects = new std::map<IceUtil::Shared*, ICEInternalWrapper*>;
    }

    ~Init()
    {
        delete cachedObjects;
    }
};

Init init;
}
#endif


@implementation ICEInternalWrapper

#ifdef ICE_USE_MAP_TABLE
//
// mapTable singleton
//
+(NSMapTable*) mapTable
{
    static NSMapTable* instance;
    @synchronized(self)
    {
        if(instance == 0)
        {
            //
            // Unfortunately the values we use are not very well documented, and small variations
            // (like using the opaque pointer memory personality for keys) introduces strange bugs
            //
            instance = [NSMapTable
                         mapTableWithKeyOptions:NSPointerFunctionsOpaquePersonality
                         valueOptions:NSMapTableZeroingWeakMemory];

            CFRetain(instance); // leak it!
        }
    }
    return instance;
} 
#endif

-(id) initWithCxxObject:(IceUtil::Shared*)arg
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    cxxObject_ = arg;
    CXXOBJECT->__incRef();

#ifdef ICE_USE_MAP_TABLE
    //
    // No synchronization because initWithCxxObject is always called with mapTable locked, see below
    //
    NSMapTable* mapTable = [ICEInternalWrapper mapTable];
    void* result = NSMapInsertIfAbsent(mapTable, arg, self); 
    // COMPILERFIX: NSMapTable bug where the entry sometime doesn't get properly
    // added. Adding it a second time works.
    if(NSMapGet(mapTable, arg) == 0)
    {
        result = NSMapInsertIfAbsent([ICEInternalWrapper mapTable], arg, self);
    }
    assert(result == 0);
#else
    //
    // No synchronization because initWithCxxObject is always called with the wrapper class object locked
    //
    assert(cachedObjects->find(CXXOBJECT) == cachedObjects->end());
    cachedObjects->insert(std::make_pair(CXXOBJECT, self));
#endif
    
    return self;
}

-(IceUtil::Shared*) cxxObject
{
    return CXXOBJECT;
}

-(void) dealloc
{  
   
#ifdef ICE_USE_MAP_TABLE
    //
    // No synchronization because dealloc is always called with 
    // mapTable locked, see below
    //
    NSMapRemove([ICEInternalWrapper mapTable], cxxObject_);
#else
    //
    // No synchronization because dealloc is always called with the wrapper class object locked
    //
    cachedObjects->erase(CXXOBJECT);
#endif

    CXXOBJECT->__decRef();
    cxxObject_ = 0;
    [super dealloc];
}

#ifdef ICE_USE_MAP_TABLE
//
// No GC and therefore finalize without the MapTable!
//
-(void) finalize
{ 
    assert(cxxObject_ != 0);
    CXXOBJECT->__decRef();
    cxxObject_ = 0;
    [super finalize];
}
#endif

+(id) getWrapperWithCxxObjectNoAutoRelease:(IceUtil::Shared*)arg
{
    //
    // Note: the returned object is NOT retained. It must be held
    // some other way by the calling thread.
    //

    if(arg == 0)
    {
        return nil;
    }
    
#ifdef ICE_USE_MAP_TABLE
    NSMapTable* mapTable = [ICEInternalWrapper mapTable];
    @synchronized(mapTable)
    {
        id val = (id)NSMapGet(mapTable, arg);
        return val;
    }
#else
    @synchronized([ICEInternalWrapper class])
    {
        std::map<IceUtil::Shared*, ICEInternalWrapper*>::const_iterator p = cachedObjects->find(arg);
        if(p != cachedObjects->end())
        {
            return p->second;
        }
    }
#endif
    return nil;
}

+(id) wrapperWithCxxObjectNoAutoRelease:(IceUtil::Shared*)arg
{
    if(arg == 0)
    {
        return nil;
    }
    
#ifdef ICE_USE_MAP_TABLE
    NSMapTable* mapTable = [ICEInternalWrapper mapTable];
    @synchronized(mapTable)
    {
        id val = (id)NSMapGet(mapTable, arg);

        if(val == nil)
        {
            return [[self alloc] initWithCxxObject:arg];
        }
        else
        {
            return [val retain];
        }
    }
#else
    @synchronized([ICEInternalWrapper class])
    {
        std::map<IceUtil::Shared*, ICEInternalWrapper*>::const_iterator p = cachedObjects->find(arg);
        if(p != cachedObjects->end())
        {
            return [p->second retain];
        }
        else
        {
            return [[self alloc] initWithCxxObject:arg];
        }
    }
#endif
    return nil; // Keep the compiler happy.
}

+(id) wrapperWithCxxObject:(IceUtil::Shared*)arg
{
    return [[self wrapperWithCxxObjectNoAutoRelease:arg] autorelease];
}


-(id) retain
{
    NSIncrementExtraRefCount(self);
    return self;
}

-(oneway void) release
{
#ifdef ICE_USE_MAP_TABLE
    @synchronized([ICEInternalWrapper mapTable])
#else
    @synchronized([ICEInternalWrapper class])  
#endif
    {
        if(NSDecrementExtraRefCountWasZero(self))
        {
            [self dealloc];
        }
    }
}
@end
