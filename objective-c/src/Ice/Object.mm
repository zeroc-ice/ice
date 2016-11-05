// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

std::map<Ice::Object*, ICEServantWrapper*> cachedObjects;

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

    ObjectI(ICEServant*);

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&,
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);

    virtual ICEObject* getServant()
    {
        return _object;
    }

    // We must explicitely CFRetain/CFRelease so that the garbage
    // collector does not trash the _object.
    virtual void __incRef()
    {
        CFRetain(_object);
    }

    virtual void __decRef()
    {
        CFRelease(_object);
    }

private:

    ICEServant* _object;
};

class BlobjectI : public IceObjC::ServantWrapper, public Ice::BlobjectArrayAsync
{
public:

    BlobjectI(ICEBlobject*);

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&,
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);

    virtual ICEObject* getServant()
    {
        return _blobject;
    }

    // We must explicitely CFRetain/CFRelease so that the garbage
    // collector does not trash the _blobject.
    virtual void __incRef()
    {
        CFRetain(_blobject);
    }

    virtual void __decRef()
    {
        CFRelease(_blobject);
    }

private:

    ICEBlobject* _blobject;
    id _target;
};

ObjectI::ObjectI(ICEServant* object) : _object(object)
{
}

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
            [_object dispatch__:c is:is os:os];
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

BlobjectI::BlobjectI(ICEBlobject* blobject) : _blobject(blobject), _target([blobject target__])
{
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
                                           length:(inEncaps.second - inEncaps.first)
                                     freeWhenDone:NO];
        @try
        {
            ok = [_target ice_invoke:inE outEncaps:&outE current:c];
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
    size_t high = count - 1;
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
            return mid;
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
        if(expected == ICEIdempotent && received == ICENonmutating)
        {
            //
            // Fine: typically an old client still using the deprecated nonmutating keyword
            //

            //
            // Note that expected == Nonmutating and received == Idempotent is not ok:
            // the server may still use the deprecated nonmutating keyword to detect updates
            // and the client should not break this (deprecated) feature.
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
-(Ice::Object*) object__
{
    NSAssert(NO, @"object__ requires override");
    return 0;
}
@end

@implementation ICEObject
static NSString* ICEObject_ids__[1] =
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
-(BOOL) ice_isA:(NSString*)typeId
{
    return [self ice_isA:typeId current:nil];
}
-(void) ice_ping
{
    [self ice_ping:nil];
}
-(NSString*) ice_id
{
    return [self ice_id:nil];
}
-(NSArray*) ice_ids
{
    return [self ice_ids:nil];
}
-(void) ice_preMarshal
{
}
-(void) ice_postUnmarshal
{
}
-(BOOL) ice_isA:(NSString*)typeId current:(ICECurrent*)current
{
    NSAssert(NO, @"ice_isA requires override");
    return nil;
}
-(void) ice_ping:(ICECurrent*)current
{
    NSAssert(NO, @"ice_ping requires override");
}
-(NSString*) ice_id:(ICECurrent*)current
{
    NSAssert(NO, @"ice_id requires override");
    return nil;
}
-(NSArray*) ice_ids:(ICECurrent*)current
{
    NSAssert(NO, @"ice_ids requires override");
    return nil;
}
-(void) ice_dispatch:(id<ICERequest>)request;
{
    NSAssert(NO, @"ice_dispatch requires override");
}
+(NSString*) ice_staticId
{
    int count, index;
    NSString*const* staticIds = [self staticIds__:&count idIndex:&index];
    return staticIds[index];
}
+(NSString*const*) staticIds__:(int*)count idIndex:(int*)idx
{
    *count = sizeof(ICEObject_ids__) / sizeof(NSString*);
    *idx = 0;
    return ICEObject_ids__;
}
-(void) write__:(id<ICEOutputStream>)os
{
    NSAssert(NO, @"write__ requires override");
}
-(void) read__:(id<ICEInputStream>)is
{
    NSAssert(NO, @"read__ requires override");
}
-(id) copyWithZone:(NSZone*)zone
{
    return [[[self class] allocWithZone:zone] init];
}
@end

@implementation ICEServant
static NSString* ICEObject_all__[4] =
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
    object__ = 0;
    delegate__ = 0;
    return self;
}

-(id)initWithDelegate:(id)delegate
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    object__ = 0;
    delegate__ = [delegate retain];
    return self;
}

-(void) dealloc
{
    if(object__)
    {
        delete static_cast<IceObjC::ServantWrapper*>(object__);
        object__ = 0;
    }
    [delegate__ release];
    [super dealloc];
}

+(id)objectWithDelegate:(id)delegate
{
    return [[[self alloc] initWithDelegate:delegate] autorelease];
}

-(BOOL) ice_isA:(NSString*)typeId current:(ICECurrent*)current
{
    int count, index;
    NSString*const* staticIds = [[self class] staticIds__:&count idIndex:&index];
    return ICEInternalLookupString(staticIds, count, typeId) >= 0;
}

-(void) ice_ping:(ICECurrent*)current
{
    // Nothing to do.
}

-(NSString*) ice_id:(ICECurrent*)current
{
    return [[self class] ice_staticId];
}

-(NSArray*) ice_ids:(ICECurrent*)current
{
    int count, index;
    NSString*const* staticIds = [[self class] staticIds__:&count idIndex:&index];
    return [NSArray arrayWithObjects:staticIds count:count];
}

-(void) ice_dispatch:(id<ICERequest>)request
{
    [(ICERequest*)request callDispatch:self];
}

+(void) ice_isA___:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os
{
    ICEEncodingVersion* encoding = [is startEncapsulation];
    NSString* id__ = [is readString];
    [is endEncapsulation];
    [os startEncapsulation:encoding format:ICEDefaultFormat];
    BOOL ret__ = [servant ice_isA:id__ current:current];
    [os writeBool:ret__];
    [os endEncapsulation];
}

+(void) ice_ping___:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os
{
    ICEEncodingVersion* encoding = [is startEncapsulation];
    [is endEncapsulation];
    [os startEncapsulation:encoding format:ICEDefaultFormat];
    [servant ice_ping:current];
    [os endEncapsulation];
}

+(void) ice_id___:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os
{
    ICEEncodingVersion* encoding = [is startEncapsulation];
    [is endEncapsulation];
    [os startEncapsulation:encoding format:ICEDefaultFormat];
    NSString* ret__ = [servant ice_id:current];
    [os writeString:ret__];
    [os endEncapsulation];
}

+(void) ice_ids___:(id)servant current:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os
{
    ICEEncodingVersion* encoding = [is startEncapsulation];
    [is endEncapsulation];
    [os startEncapsulation:encoding format:ICEDefaultFormat];
    NSArray* ret__ = [servant ice_ids:current];
    [os writeStringSeq:ret__];
    [os endEncapsulation];
}

-(void) dispatch__:(ICECurrent*)current is:(id<ICEInputStream>)is os:(id<ICEOutputStream>)os
{
    switch(ICEInternalLookupString(ICEObject_all__, sizeof(ICEObject_all__) / sizeof(NSString*), current.operation))
    {
    case 0:
        [ICEServant ice_id___:self current:current is:is os:os];
        return;
    case 1:
        [ICEServant ice_ids___:self current:current is:is os:os];
        return;
    case 2:
        [ICEServant ice_isA___:self current:current is:is os:os];
        return;
    case 3:
        [ICEServant ice_ping___:self current:current is:is os:os];
        return;
    default:
        @throw [ICEOperationNotExistException requestFailedException:__FILE__
                                              line:__LINE__
                                              id:current.id_
                                              facet:current.facet
                                              operation:current.operation];
    }
}

-(void) write__:(id<ICEOutputStream>)os
{
    [os startValue:nil];
    [self writeImpl__:os];
    [os endValue];
}

-(void) read__:(id<ICEInputStream>)is
{
    [is startValue];
    [self readImpl__:is];
    [is endValue:NO];
}

-(void) writeImpl__:(id<ICEOutputStream>)os
{
    NSAssert(NO, @"writeImpl__ requires override");
}

-(void) readImpl__:(id<ICEInputStream>)is
{
    NSAssert(NO, @"readImpl__ requires override");
}

-(id)target__
{
    return (delegate__ == 0) ? self : delegate__;
}

-(Ice::Object*) object__
{
    @synchronized([self class])
    {
        if(object__ == 0)
        {
            //
            // NOTE: IceObjC::ObjectI implements it own reference counting and there's no need
            // to call __incRef/__decRef here. The C++ object and Objective-C object are sharing
            // the same reference count (the one of the Objective-C object). This is necessary
            // to properly release both objects when there's either no more C++ handle/ObjC
            // reference to the object (without this, servants added to the object adapter
            // couldn't be retained or released easily).
            //
            object__ = static_cast<IceObjC::ServantWrapper*>(new ObjectI(self));
        }
    }
    return static_cast<IceObjC::ServantWrapper*>(object__);
}
@end

@implementation ICEBlobject
-(Ice::Object*) object__
{
    @synchronized([self class])
    {
        if(object__ == 0)
        {
            //
            // NOTE: IceObjC::ObjectI implements it own reference counting and there's no need
            // to call __incRef/__decRef here. The C++ object and Objective-C object are sharing
            // the same reference count (the one of the Objective-C object). This is necessary
            // to properly release both objects when there's either no more C++ handle/ObjC
            // reference to the object (without this, servants added to the object adapter
            // couldn't be retained or released easily).
            //
            object__ = static_cast<IceObjC::ServantWrapper*>(new BlobjectI(self));
        }
    }
    return static_cast<IceObjC::ServantWrapper*>(object__);
}
@end

@implementation ICEServantWrapper
-(id) initWithCxxObject:(Ice::Object*)arg
{
    self = [super init];
    if(!self)
    {
        return nil;
    }

    object__ = arg;
    object__->__incRef();
    assert(cachedObjects.find(object__) == cachedObjects.end());
    cachedObjects.insert(std::make_pair(object__, self));
    return self;
}
-(void) dealloc
{
    cachedObjects.erase(object__);
    object__->__decRef();
    [super dealloc];
}
+(id) servantWrapperWithCxxObject:(Ice::Object*)arg
{
    @synchronized([ICEServantWrapper class])
    {
        std::map<Ice::Object*, ICEServantWrapper*>::const_iterator p = cachedObjects.find(arg);
        if(p != cachedObjects.end())
        {
            return [p->second retain];
        }
        else
        {
            return [[(ICEServantWrapper*)[self alloc] initWithCxxObject:arg] autorelease];
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
-(BOOL) ice_isA:(NSString*)typeId current:(ICECurrent*)current
{
    NSException* nsex = nil;
    try
    {
        return object__->ice_isA(fromNSString(typeId), Ice::Current());
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(void) ice_ping:(ICECurrent*)current
{
    NSException* nsex = nil;
    try
    {
        return object__->ice_ping(Ice::Current());
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(NSString*) ice_id:(ICECurrent*)current
{
    NSException* nsex = nil;
    try
    {
        return toNSString(object__->ice_id(Ice::Current()));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(NSArray*) ice_ids:(ICECurrent*)current
{
    NSException* nsex = nil;
    try
    {
        return toNSArray(object__->ice_ids(Ice::Current()));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(void) ice_dispatch:(id<ICERequest>)request
{
    @throw [ICEFeatureNotSupportedException featureNotSupportedException:__FILE__ line:__LINE__];
}
-(void) write__:(id<ICEOutputStream>)os
{
    NSException* nsex = nil;
    try
    {
        object__->__write([(ICEOutputStream*)os os]);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(void) read__:(id<ICEInputStream>)is
{
    NSException* nsex = nil;
    try
    {
        object__->__read([(ICEInputStream*)is is]);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(Ice::Object*) object__
{
    return object__;
}
@end
