//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <ObjectI.h>
#import <StreamI.h>
#import <CurrentI.h>
#import <Util.h>
#import <Request.h>
#import <LocalObjectI.h>

#import <objc/Ice/LocalException.h>

#include <Ice/Object.h>
#include <Ice/IncomingAsync.h>
#include <Ice/Initialize.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/NativePropertiesAdmin.h>

namespace
{

std::map<std::shared_ptr<Ice::Object>, ICEServantWrapper*> cachedObjects;

NSString*
operationModeToString(ICEOperationMode mode)
{
    switch(mode)
    {
    case ICENormal:
        return @"::Ice::Normal";

    case ICENonmutating:
        return @"::Ice::Nonmutating";

    case ICEIdempotent:
        return @"::Ice::Idempotent";

    default:
        return [NSString stringWithFormat:@"unknown value(%d)", mode];
    }
}

class ObjectI : public IceObjC::ServantWrapper, public Ice::BlobjectArrayAsync
{
public:

    ObjectI(ICEServant* servant) : _object(servant)
    {
        // Make sure the ObjcC object is not released while the C++ object is still alive.
        CFRetain(_object);
    }

    virtual ~ObjectI()
    {
        CFRelease(_object);
    }

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&,
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);

    virtual ICEObject* getServant()
    {
        return _object;
    }

private:

    ICEServant* _object;
};

class BlobjectI : public IceObjC::ServantWrapper, public Ice::BlobjectArrayAsync
{
public:

    BlobjectI(ICEBlobject* servant) : _blobject(servant)
    {
        // Make sure the ObjcC object is not released while the C++ object is still alive.
        CFRetain(_blobject);
    }

    virtual ~BlobjectI()
    {
        CFRelease(_blobject);
    }

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&,
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);

    virtual ICEObject* getServant()
    {
        return _blobject;
    }

private:

    ICEBlobject* _blobject;
};

void
ObjectI::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& cb,
                          const std::pair<const Ice::Byte*, const Ice::Byte*>& inParams,
                          const Ice::Current& current)
{
    Ice::Communicator* communicator = current.adapter->getCommunicator().get();
    ICEInputStream* is = [[ICEInputStream alloc] initWithCxxCommunicator:communicator data:inParams];
    ICEOutputStream* os = [[ICEOutputStream alloc] initWithCxxCommunicator:communicator];

    NSException* exception = nil;
    BOOL ok = YES; // Keep the compiler happy
    @autoreleasepool
    {
        ICECurrent* c = [[ICECurrent alloc] initWithCurrent:current];
        @try
        {
            [_object iceDispatch:c is:is os:os];
        }
        @catch(ICEUserException* ex)
        {
            // The generated code started the encapsulation with the appropriate format.
            ok = NO;
            [os writeException:ex];
            [os endEncapsulation];
        }
        @catch(id ex)
        {
            exception = [ex retain];
        }
        @finally
        {
            [c release];
            [is release];
        }
    }

    if(exception != nil)
    {
        [os release];
        rethrowCxxException(exception, true); // True = release the exception.
    }

    cb->ice_response(ok, [os os]->finished());
    [os release];
}

void
BlobjectI::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& cb,
                            const std::pair<const Ice::Byte*, const Ice::Byte*>& inEncaps,
                            const Ice::Current& current)
{
    NSException* exception = nil;
    BOOL ok = YES; // Keep the compiler happy.
    NSMutableData* outE = nil;

    @autoreleasepool
    {
        ICECurrent* c = [[ICECurrent alloc] initWithCurrent:current];
        NSData* inE = [NSData dataWithBytesNoCopy:const_cast<Ice::Byte*>(inEncaps.first)
                                           length:static_cast<NSUInteger>(inEncaps.second - inEncaps.first)
                                     freeWhenDone:NO];
        @try
        {
            // The application-provided implementation of class ICEBlobject must implement the ICEBlobject protocol.
            ok = [(id)_blobject ice_invoke:inE outEncaps:&outE current:c];
            [outE retain];
        }
        @catch(ICEUserException* ex)
        {
            ok = NO;
            Ice::Communicator* communicator = current.adapter->getCommunicator().get();
            ICEOutputStream* os = [[ICEOutputStream alloc] initWithCxxCommunicator:communicator];
            [os startEncapsulation:c.encoding format:ICEDefaultFormat];
            [os writeException:ex];
            [os endEncapsulation];
            outE = [[os finished] retain];
            [os release];
        }
        @catch(id ex)
        {
            exception = [ex retain];
        }
        @finally
        {
            [c release];
        }
    }

    if(exception != nil)
    {
        rethrowCxxException(exception, true); // True = release the exception.
    }

    cb->ice_response(ok, std::make_pair((ICEByte*)[outE bytes], (ICEByte*)[outE bytes] + [outE length]));
    [outE release];
}

}

int
ICEInternalLookupString(NSString* const array[], size_t count, NSString* __unsafe_unretained str)
{
    size_t low = 0;
    size_t high = static_cast<size_t>(count - 1);
    while(low <= high)
    {
        size_t mid = (low + high) / 2;
        switch([array[mid] compare:str])
        {
        case NSOrderedDescending:
            if(mid == 0)
            {
                return -1;
            }
            high = mid - 1;
            break;
        case NSOrderedAscending:
            low = mid + 1;
            break;
        case NSOrderedSame:
            return static_cast<int>(mid);
        default:
            return -1; // Can't be reached
        }
    }
    return -1;
}

void
ICEInternalCheckModeAndSelector(id target, ICEOperationMode expected, SEL sel, ICECurrent* current)
{
    ICEOperationMode received = current.mode;
    if(expected != received)
    {
        assert(expected != ICENonmutating); // We never expect Nonmutating
        if(expected == ICEIdempotent && received == ICENonmutating)
        {
            //
            // Fine: typically an old client still using the deprecated nonmutating keyword
            //
        }
        else
        {
            ICEMarshalException* ex = [ICEMarshalException marshalException:__FILE__ line:__LINE__];
            [ex setReason_:[NSString stringWithFormat:@"unexpected operation mode. expected = %@ received=%@",
                                     operationModeToString(expected), operationModeToString(received)]];
            @throw ex;
        }
    }

    if(![target respondsToSelector:sel])
    {
        @throw [ICEOperationNotExistException operationNotExistException:__FILE__
                                              line:__LINE__
                                              id:current.id_
                                              facet:current.facet
                                              operation:current.operation];
    }
}

@implementation ICEObject (ICEInternal)
-(std::shared_ptr<Ice::Object>) iceObject
{
    NSAssert(NO, @"iceObject requires override");
    return 0;
}
@end

@implementation ICEObject
static NSString* ICEObject_ids[1] =
{
    @"::Ice::Object"
};

-(id)init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    return self;
}

-(void) dealloc
{
    [super dealloc];
}
-(BOOL) ice_isA:(NSString*)__unused typeId current:(ICECurrent*)__unused current
{
    NSAssert(NO, @"ice_isA requires override");
    return NO;
}
-(void) ice_ping:(ICECurrent*)__unused current
{
    NSAssert(NO, @"ice_ping requires override");
}
-(NSString*) ice_id:(ICECurrent*)__unused current
{
    NSAssert(NO, @"ice_id requires override");
    return nil;
}
-(NSArray*) ice_ids:(ICECurrent*)__unused current
{
    NSAssert(NO, @"ice_ids requires override");
    return nil;
}
-(void) ice_dispatch:(id<ICERequest>)__unused request
{
    NSAssert(NO, @"ice_dispatch requires override");
}
+(NSString*) ice_staticId
{
    int count, index;
    NSString*const* staticIds = [self iceStaticIds:&count idIndex:&index];
    return staticIds[index];
}
+(NSString*const*) iceStaticIds:(int*)count idIndex:(int*)idx
{
    *count = sizeof(ICEObject_ids) / sizeof(NSString*);
    *idx = 0;
    return ICEObject_ids;
}
-(id) copyWithZone:(NSZone*)zone
{
    return [[[self class] allocWithZone:zone] init];
}
@end

@implementation ICEServant
static NSString* ICEObject_all[4] =
{
    @"ice_id",
    @"ice_ids",
    @"ice_isA",
    @"ice_ping"
};

-(id)init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    iceObject_ = nil;
    return self;
}

-(void) dealloc
{
    if(iceObject_)
    {
        delete static_cast<std::weak_ptr<Ice::Object>*>(iceObject_);
        iceObject_ = nil;
    }
    [super dealloc];
}

-(BOOL) ice_isA:(NSString*)typeId current:(ICECurrent*)__unused current
{
    int count;
    int index;
    NSString*const* staticIds = [[self class] iceStaticIds:&count idIndex:&index];
    return ICEInternalLookupString(staticIds, static_cast<size_t>(count), typeId) >= 0;
}

-(void) ice_ping:(ICECurrent*)__unused current
{
    // Nothing to do.
}

-(NSString*) ice_id:(ICECurrent*)__unused current
{
    return [[self class] ice_staticId];
}

-(NSArray*) ice_ids:(ICECurrent*)__unused current
{
    int count, index;
    NSString*const* staticIds = [[self class] iceStaticIds:&count idIndex:&index];
    return [NSArray arrayWithObjects:staticIds count:static_cast<NSUInteger>(count)];
}

-(void) ice_dispatch:(id<ICERequest>)request
{
    [(ICERequest*)request callDispatch:self];
}

+(void) iceD_ice_isA:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os
{
    ICEEncodingVersion* encoding = [is startEncapsulation];
    NSString* ident = [is readString];
    [is endEncapsulation];
    [os startEncapsulation:encoding format:ICEDefaultFormat];
    BOOL ret = [servant ice_isA:ident current:current];
    [os writeBool:ret];
    [os endEncapsulation];
}

+(void) iceD_ice_ping:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os
{
    ICEEncodingVersion* encoding = [is startEncapsulation];
    [is endEncapsulation];
    [os startEncapsulation:encoding format:ICEDefaultFormat];
    [servant ice_ping:current];
    [os endEncapsulation];
}

+(void) iceD_ice_id:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os
{
    ICEEncodingVersion* encoding = [is startEncapsulation];
    [is endEncapsulation];
    [os startEncapsulation:encoding format:ICEDefaultFormat];
    NSString* ret = [servant ice_id:current];
    [os writeString:ret];
    [os endEncapsulation];
}

+(void) iceD_ice_ids:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os
{
    ICEEncodingVersion* encoding = [is startEncapsulation];
    [is endEncapsulation];
    [os startEncapsulation:encoding format:ICEDefaultFormat];
    NSArray* ret = [servant ice_ids:current];
    [os writeStringSeq:ret];
    [os endEncapsulation];
}

-(void) iceDispatch:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os
{
    switch(ICEInternalLookupString(ICEObject_all, sizeof(ICEObject_all) / sizeof(NSString*), current.operation))
    {
    case 0:
        [ICEServant iceD_ice_id:self current:current is:is os:os];
        return;
    case 1:
        [ICEServant iceD_ice_ids:self current:current is:is os:os];
        return;
    case 2:
        [ICEServant iceD_ice_isA:self current:current is:is os:os];
        return;
    case 3:
        [ICEServant iceD_ice_ping:self current:current is:is os:os];
        return;
    default:
        @throw [ICEOperationNotExistException requestFailedException:__FILE__
                                                                line:__LINE__
                                                                  id:current.id_
                                                               facet:current.facet
                                                           operation:current.operation];
    }
}

-(std::shared_ptr<Ice::Object>) iceObject
{
    @synchronized([self class])
    {
        std::shared_ptr<Ice::Object> result;

        if (iceObject_)
        {
            auto weakPtrPtr = static_cast<std::weak_ptr<Ice::Object>*>(iceObject_);

            result = weakPtrPtr->lock();

            // result can be null if the C++ wrapper was released / destroyed
            // This typically occurs when the servant (really its C++ wrapper) is removed from an object adapter.
            if (!result)
            {
                delete weakPtrPtr;
                iceObject_ = nullptr;
            }
        }

        if (!result)
        {
            auto wrapper = std::make_shared<ObjectI>(self);
            iceObject_ = new std::weak_ptr<Ice::Object>(wrapper);
            result = wrapper;
        }

        return result;
    }
}
@end

@implementation ICEBlobject
-(std::shared_ptr<Ice::Object>) iceObject
{
    @synchronized([self class])
    {
        std::shared_ptr<Ice::Object> result;

        if (iceObject_)
        {
            auto weakPtrPtr = static_cast<std::weak_ptr<Ice::Object>*>(iceObject_);

            result = weakPtrPtr->lock();

            // result can be null if the C++ wrapper was released / destroyed.
            // This typically occurs when the servant (really its C++ wrapper) is removed from an object adapter.
            if (!result)
            {
                delete weakPtrPtr;
                iceObject_ = nullptr;
            }
        }

        if (!result)
        {
            auto wrapper = std::make_shared<BlobjectI>(self);
            iceObject_ = new std::weak_ptr<Ice::Object>(wrapper);
            result = wrapper;
        }

        return result;
    }
}
@end

@implementation ICEServantWrapper
-(id) initWithCxxObject:(const std::shared_ptr<Ice::Object>&)arg
{
    self = [super init];
    if(!self)
    {
        return nil;
    }

    object_ = arg;
    assert(cachedObjects.find(object_) == cachedObjects.end());
    cachedObjects.insert(std::make_pair(object_, self));
    return self;
}
-(void) dealloc
{
    cachedObjects.erase(object_);
    [super dealloc];
}
+(id) servantWrapperWithCxxObjectNoAutoRelease:(const std::shared_ptr<Ice::Object>&)arg
{
    @synchronized([ICEServantWrapper class])
    {
        std::map<std::shared_ptr<Ice::Object>, ICEServantWrapper*>::const_iterator p = cachedObjects.find(arg);
        if(p != cachedObjects.end())
        {
            return [p->second retain];
        }
        else
        {
            return [(ICEServantWrapper*)[self alloc] initWithCxxObject:arg];
        }
    }
}
-(id) retain
{
    NSIncrementExtraRefCount(self);
    return self;
}
-(oneway void) release
{
    @synchronized([ICEServantWrapper class])
    {
        if(NSDecrementExtraRefCountWasZero(self))
        {
            [self dealloc];
        }
    }
}
-(BOOL) ice_isA:(NSString*)typeId current:(ICECurrent*)__unused current
{
    NSException* nsex = nil;
    try
    {
        return object_->ice_isA(fromNSString(typeId), Ice::Current());
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(void) ice_ping:(ICECurrent*)__unused current
{
    NSException* nsex = nil;
    try
    {
        return object_->ice_ping(Ice::Current());
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(NSString*) ice_id:(ICECurrent*)__unused current
{
    NSException* nsex = nil;
    try
    {
        return toNSString(object_->ice_id(Ice::Current()));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(NSArray*) ice_ids:(ICECurrent*)__unused current
{
    NSException* nsex = nil;
    try
    {
        return toNSArray(object_->ice_ids(Ice::Current()));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(void) ice_dispatch:(id<ICERequest>)__unused request
{
    @throw [ICEFeatureNotSupportedException featureNotSupportedException:__FILE__ line:__LINE__];
}

-(std::shared_ptr<Ice::Object>) iceObject
{
    return object_;
}
@end
