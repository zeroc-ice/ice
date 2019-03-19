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

BlobjectI::BlobjectI(ICEBlobject* blobject) : _blobject(blobject), _target([blobject iceTarget])
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

            assert(expected != Nonmutating);
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
-(Ice::Object*) iceObject
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
-(id<ICESlicedData>) ice_getSlicedData
{
    return nil;
}
-(BOOL) ice_isA:(NSString*)__unused typeId current:(ICECurrent*)__unused current
{
    NSAssert(NO, @"ice_isA requires override");
    return nil;
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
-(void) iceWrite:(id<ICEOutputStream>)__unused os
{
    NSAssert(NO, @"iceWrite requires override");
}
-(void) iceRead:(id<ICEInputStream>)__unused is
{
    NSAssert(NO, @"iceRead requires override");
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
    iceObject_ = 0;
    iceDelegate_ = 0;
    return self;
}

-(id)initWithDelegate:(id)delegate
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    iceObject_ = 0;
    iceDelegate_ = [delegate retain];
    return self;
}

-(void) dealloc
{
    if(iceObject_)
    {
        delete static_cast<IceObjC::ServantWrapper*>(iceObject_);
        iceObject_ = 0;
    }
    [iceDelegate_ release];
    [super dealloc];
}

+(id)objectWithDelegate:(id)delegate
{
    return [[[self alloc] initWithDelegate:delegate] autorelease];
}

-(BOOL) ice_isA:(NSString*)typeId current:(ICECurrent*)__unused current
{
    int count, index;
    NSString*const* staticIds = [[self class] iceStaticIds:&count idIndex:&index];
    return ICEInternalLookupString(staticIds, count, typeId) >= 0;
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
    return [NSArray arrayWithObjects:staticIds count:count];
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

-(void) iceWrite:(id<ICEOutputStream>)os
{
    [os startValue:nil];
    [self iceWriteImpl:os];
    [os endValue];
}

-(void) iceRead:(id<ICEInputStream>)is
{
    [is startValue];
    [self iceReadImpl:is];
    [is endValue:NO];
}

-(void) iceWriteImpl:(id<ICEOutputStream>)__unused os
{
    NSAssert(NO, @"iceWriteImpl requires override");
}

-(void) iceReadImpl:(id<ICEInputStream>)__unused is
{
    NSAssert(NO, @"iceReadImpl requires override");
}

-(id)iceTarget
{
    return (iceDelegate_ == 0) ? self : iceDelegate_;
}

-(Ice::Object*) iceObject
{
    @synchronized([self class])
    {
        if(iceObject_ == 0)
        {
            //
            // NOTE: IceObjC::ObjectI implements it own reference counting and there's no need
            // to call __incRef/__decRef here. The C++ object and Objective-C object are sharing
            // the same reference count (the one of the Objective-C object). This is necessary
            // to properly release both objects when there's either no more C++ handle/ObjC
            // reference to the object (without this, servants added to the object adapter
            // couldn't be retained or released easily).
            //
            iceObject_ = static_cast<IceObjC::ServantWrapper*>(new ObjectI(self));
        }
    }
    return static_cast<IceObjC::ServantWrapper*>(iceObject_);
}
@end

@implementation ICEBlobject
-(Ice::Object*) iceObject
{
    @synchronized([self class])
    {
        if(iceObject_ == 0)
        {
            //
            // NOTE: IceObjC::ObjectI implements it own reference counting and there's no need
            // to call __incRef/__decRef here. The C++ object and Objective-C object are sharing
            // the same reference count (the one of the Objective-C object). This is necessary
            // to properly release both objects when there's either no more C++ handle/ObjC
            // reference to the object (without this, servants added to the object adapter
            // couldn't be retained or released easily).
            //
            iceObject_ = static_cast<IceObjC::ServantWrapper*>(new BlobjectI(self));
        }
    }
    return static_cast<IceObjC::ServantWrapper*>(iceObject_);
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

    object_ = arg;
    object_->__incRef();
    assert(cachedObjects.find(object_) == cachedObjects.end());
    cachedObjects.insert(std::make_pair(object_, self));
    return self;
}
-(void) dealloc
{
    cachedObjects.erase(object_);
    object_->__decRef();
    [super dealloc];
}
+(id) servantWrapperWithCxxObjectNoAutoRelease:(Ice::Object*)arg
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
-(void) iceWrite:(id<ICEOutputStream>)os
{
    NSException* nsex = nil;
    try
    {
        object_->_iceWrite([(ICEOutputStream*)os os]);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(void) iceRead:(id<ICEInputStream>)is
{
    NSException* nsex = nil;
    try
    {
        object_->_iceRead([(ICEInputStream*)is is]);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(Ice::Object*) iceObject
{
    return object_;
}
@end
