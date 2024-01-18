//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <ObjectAdapterI.h>
#import <CommunicatorI.h>
#import <ProxyI.h>
#import <IdentityI.h>
#import <ObjectI.h>
#import <Util.h>
#import <LocalObjectI.h>
#import <CurrentI.h>
#import <StreamI.h>

#import <objc/Ice/LocalException.h>
#import <objc/Ice/Locator.h>
#import <objc/Ice/ServantLocator.h>

#include <Ice/Locator.h>
#include <Ice/ServantLocator.h>
#include <Ice/OutputStream.h>

namespace
{

std::map<std::shared_ptr<Ice::ObjectAdapter>, ICEObjectAdapter*> cache;

}

namespace
{
class DefaultServantLocator final : public Ice::ServantLocator
{
public:

    DefaultServantLocator(const std::shared_ptr<Ice::Object>& s) :
        _servant(s)
    {
    }

    std::shared_ptr<Ice::Object> locate(const Ice::Current&, std::shared_ptr<void>&) final
    {
        return _servant;
    }

    void finished(const Ice::Current&, const std::shared_ptr<Ice::Object>&, const std::shared_ptr<void>&) final
    {
    }

    void deactivate(const std::string&) final
    {
    }

    const std::shared_ptr<Ice::Object>& servant() const
    {
        return _servant;
    }

private:

    std::shared_ptr<Ice::Object> _servant;
};

using DefaultServantLocatorPtr = std::shared_ptr<DefaultServantLocator>;

class Cookie
{
public:

    Cookie(id cookie) : _cookie(cookie)
    {
        [_cookie retain];
    }

    ~Cookie()
    {
        [_cookie release];
    }

    id cookie()
    {
        return _cookie;
    }

private:

    id _cookie;
};
using CookiePtr = std::shared_ptr<Cookie>;

class ExceptionWriter : public Ice::UserException
{
public:

    ExceptionWriter(ICEUserException* ex) : _ex(ex)
    {
    }

    ExceptionWriter(const ExceptionWriter& other) : _ex(other._ex)
    {
        [_ex retain];
    }

    ~ExceptionWriter()
    {
        [_ex release];
    }

    void
    _write(Ice::OutputStream* s) const
    {
        ICEOutputStream* os = [[ICEOutputStream alloc] initWithCxxStream:s];
        [_ex iceWrite:os];
        [os release];
    }

    void
    _read(Ice::InputStream*)
    {
        assert(false);
    }

    bool
    _usesClasses() const
    {
        return [_ex iceUsesClasses];
    }

    std::string
    ice_id() const
    {
        return [[_ex ice_id] UTF8String];
    }

    Ice::UserException*
    ice_clone() const
    {
        return new ExceptionWriter(*this);
    }

    void
    ice_throw() const
    {
        throw *this;
    }

protected:

    virtual void _writeImpl(Ice::OutputStream*) const {}
    virtual void _readImpl(Ice::InputStream*) {}

private:

    ICEUserException* _ex;
};

class ServantLocatorWrapper final : public Ice::ServantLocator
{
public:

    ServantLocatorWrapper(id<ICEServantLocator> locator) :
        _locator(locator)
    {
        [_locator retain];
    }

    ~ServantLocatorWrapper() final
    {
        [_locator release];
    }

    std::shared_ptr<Ice::Object> locate(const Ice::Current& current, std::shared_ptr<void>& cookie) final
    {
        NSException* ex = nil;
        @autoreleasepool
        {
            ICECurrent* cu = [[ICECurrent alloc] initWithCurrent:current];
            id co = nil;
            @try
            {
                ICEObject* servant = [_locator locate:cu cookie:&co];
                if(co != nil)
                {
                    cookie = std::make_shared<Cookie>(co);
                }
                return [servant iceObject];
            }
            @catch(id e)
            {
                ex = [e retain];
            }
            @finally
            {
                [cu release];
            }
        }
        if(ex != nil)
        {
            if([ex isKindOfClass:[ICEUserException class]])
            {
                throw ExceptionWriter((ICEUserException*)ex);
            }
            rethrowCxxException(ex, true); // True = release the exception.
        }
    }

    void finished(const Ice::Current& current, const std::shared_ptr<Ice::Object>& servant, const std::shared_ptr<void>& cookie) final
    {
        NSException* ex = nil;
        @autoreleasepool
        {
            ICECurrent* cu = [[ICECurrent alloc] initWithCurrent:current];
            id co = cookie ? std::static_pointer_cast<Cookie>(cookie)->cookie() : nil;
            @try
            {
                [_locator finished:cu servant:[toObjC(servant) autorelease] cookie:co];
            }
            @catch(id e)
            {
                ex = [e retain];
            }
            @finally
            {
                [cu release];
            }
        }
        if(ex != nil)
        {
            if([ex isKindOfClass:[ICEUserException class]])
            {
                throw ExceptionWriter((ICEUserException*)ex);
            }
            rethrowCxxException(ex, true); // True = release the exception.
        }
    }

    void deactivate(const std::string& category) final
    {
        NSException* ex = nil;
        @autoreleasepool
        {
            NSString* cat = toNSString(category);
            @try
            {
                [_locator deactivate:cat];
            }
            @catch(id e)
            {
                ex = [e retain];
            }
            @finally
            {
                [cat release];
            }
        }
        if(ex != nil)
        {
            rethrowCxxException(ex, true); // True = release the exception.
        }
    }

    id<ICEServantLocator> locator() const
    {
        return [[_locator retain] autorelease];
    }

private:

    id<ICEServantLocator> _locator;
};

using ServantLocatorWrapperPtr = std::shared_ptr<ServantLocatorWrapper>;

} // End of anonymous namespace.

@implementation ICEObjectAdapter

-(id) init
{
    self = [super init];
    if (!self)
    {
        return nil;
    }
    adapter_ = nullptr;
    return self;
}

-(id) initWithCxxObject:(const std::shared_ptr<Ice::ObjectAdapter>&)arg
{
    self = [super init];
    if (!self)
    {
        return nil;
    }
    adapter_ = arg;

    //
    // No synchronization because initWithCxxObject is always called with the wrapper class object locked
    //
    assert(cache.find(arg) == cache.end());
    cache.insert(std::make_pair(arg, self));
    return self;
}

-(void) dealloc
{
    if (adapter_)
    {
        //
        // No synchronization because dealloc is always called with the wrapper class object locked
        //
        cache.erase(adapter_);
        adapter_ = nullptr;
    }

    [super dealloc];
}

-(std::shared_ptr<Ice::ObjectAdapter>) cxxObject
{
    return adapter_;
}

+(id) objectAdapterWithCxxObject:(const std::shared_ptr<Ice::ObjectAdapter>&)arg
{
    return [[self objectAdapterWithCxxObjectNoAutoRelease:arg] autorelease];
}

+(id) objectAdapterWithCxxObjectNoAutoRelease:(const std::shared_ptr<Ice::ObjectAdapter>&)arg
{
    if (!arg)
    {
        return nil;
    }

    @synchronized([ICEObjectAdapter class])
    {
        auto p = cache.find(arg);
        if(p != cache.end())
        {
            return [p->second retain];
        }
        else
        {
            return [[ICEObjectAdapter alloc] initWithCxxObject:arg];
        }
    }
}

//
// @protocol ICEObjectAdapter methods.
//

-(id<ICECommunicator>) getCommunicator
{
    NSException* nsex = nil;
    try
    {
        return [ICECommunicator localObjectWithCxxObject:adapter_->getCommunicator().get()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(NSMutableString*) getName
{
    NSException* nsex = nil;
    try
    {
        return [toNSMutableString(adapter_->getName()) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(void) activate
{
    NSException* nsex = nil;
    try
    {
        adapter_->activate();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) hold
{
    NSException* nsex = nil;
    try
    {
        adapter_->hold();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) waitForHold
{
    NSException* nsex = nil;
    try
    {
        adapter_->waitForHold();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) deactivate
{
    NSException* nsex = nil;
    try
    {
        adapter_->deactivate();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(void) waitForDeactivate
{
    NSException* nsex = nil;
    try
    {
        adapter_->waitForDeactivate();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(BOOL) isDeactivated
{
    NSException* nsex = nil;
    try
    {
        return adapter_->isDeactivated();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return NO; // Keep the compiler happy.
}

-(void) destroy
{
    NSException* nsex = nil;
    try
    {
        adapter_->destroy();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(id<ICEObjectPrx>) add:(ICEObject*)servant identity:(ICEIdentity*)ident
{
    NSException* nsex = nil;
    try
    {
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:adapter_->add([servant iceObject], [ident identity])];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(id<ICEObjectPrx>) addFacet:(ICEObject*)servant identity:(ICEIdentity*)ident facet:(NSString*)facet
{
    NSException* nsex = nil;
    try
    {
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:adapter_->addFacet([servant iceObject], [ident identity],
                                                                              fromNSString(facet))];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(id<ICEObjectPrx>) addWithUUID:(ICEObject*)servant
{
    NSException* nsex = nil;
    try
    {
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:adapter_->addWithUUID([servant iceObject])];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(id<ICEObjectPrx>) addFacetWithUUID:(ICEObject*)servant facet:(NSString*)facet
{
    NSException* nsex = nil;
    try
    {
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:adapter_->addFacetWithUUID([servant iceObject],
                                                                                      fromNSString(facet))];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(void) addDefaultServant:(ICEObject*)servant category:(NSString*)category
{
    NSException* nsex = nil;
    try
    {
        Ice::ServantLocatorPtr servantLocator = std::make_shared<DefaultServantLocator>([servant iceObject]);
        adapter_->addServantLocator(servantLocator, fromNSString(category));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(ICEObject*) remove:(ICEIdentity*)ident
{
    NSException* nsex = nil;
    try
    {
        return [toObjC(adapter_->remove([ident identity])) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(ICEObject*) removeFacet:(ICEIdentity*)ident facet:(NSString*)facet
{
    NSException* nsex = nil;
    try
    {
        return [toObjC(adapter_->removeFacet([ident identity], fromNSString(facet))) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(ICEMutableFacetMap*) removeAllFacets:(ICEIdentity*)ident
{
    NSException* nsex = nil;
    try
    {
        return [toNSDictionary(adapter_->removeAllFacets([ident identity])) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(ICEObject*) removeDefaultServant:(NSString*)category
{
    NSException* nsex = nil;
    try
    {
        Ice::ServantLocatorPtr locator = adapter_->removeServantLocator(fromNSString(category));
        if(!locator)
        {
            return nil;
        }
        DefaultServantLocatorPtr defaultServantLocator = std::dynamic_pointer_cast<DefaultServantLocator>(locator);
        if(!defaultServantLocator)
        {
            return nil;
        }
        return [toObjC(defaultServantLocator->servant()) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(ICEObject*) find:(ICEIdentity*)ident
{
    NSException* nsex = nil;
    try
    {
        return [toObjC(adapter_->find([ident identity])) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(ICEObject*) findFacet:(ICEIdentity*)ident facet:(NSString*)facet
{
    NSException* nsex = nil;
    try
    {
        return [toObjC(adapter_->findFacet([ident identity], fromNSString(facet))) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(NSMutableDictionary*) findAllFacets:(ICEIdentity*)ident
{
    NSException* nsex = nil;
    try
    {
        return toNSDictionary(adapter_->findAllFacets([ident identity]));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(ICEObject*) findByProxy:(id<ICEObjectPrx>)proxy
{
    NSException* nsex = nil;
    try
    {
        return [toObjC(adapter_->findByProxy([(ICEObjectPrx*)proxy iceObjectPrx])) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(void) addServantLocator:(id<ICEServantLocator>)locator category:(NSString*)category
{
    NSException* nsex = nil;
    try
    {
        adapter_->addServantLocator(std::make_shared<ServantLocatorWrapper>(locator), fromNSString(category));
        return;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(id<ICEServantLocator>) removeServantLocator:(NSString*)category
{
    NSException* nsex = nil;
    try
    {
        Ice::ServantLocatorPtr locator = adapter_->removeServantLocator(fromNSString(category));
        return std::dynamic_pointer_cast<ServantLocatorWrapper>(locator)->locator();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}
-(id<ICEServantLocator>) findServantLocator:(NSString*)category
{
    NSException* nsex = nil;
    try
    {
        Ice::ServantLocatorPtr locator = adapter_->removeServantLocator(fromNSString(category));
        return locator ? std::dynamic_pointer_cast<ServantLocatorWrapper>(locator)->locator() : nil;
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
}

-(ICEObject*) findDefaultServant:(NSString*)category
{
    NSException* nsex = nil;
    try
    {
        Ice::ServantLocatorPtr servantLocator = adapter_->findServantLocator(fromNSString(category));
        if(servantLocator == 0)
        {
            return nil;
        }
        DefaultServantLocatorPtr defaultServantLocator = std::dynamic_pointer_cast<DefaultServantLocator>(servantLocator);
        if(defaultServantLocator == 0)
        {
            return nil; // should never happen!
        }
        else
        {
            return [toObjC(defaultServantLocator->servant()) autorelease];
        }
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(id<ICEObjectPrx>) createProxy:(ICEIdentity*)ident
{
    NSException* nsex = nil;
    try
    {
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:adapter_->createProxy([ident identity])];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(id<ICEObjectPrx>) createDirectProxy:(ICEIdentity*)ident
{
    NSException* nsex = nil;
    try
    {
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:adapter_->createDirectProxy([ident identity])];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(id<ICEObjectPrx>) createIndirectProxy:(ICEIdentity*)ident
{
    NSException* nsex = nil;
    try
    {
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:adapter_->createIndirectProxy([ident identity])];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}

-(void) setLocator:(id<ICELocatorPrx>)loc
{
    NSException* nsex = nil;
    try
    {
        adapter_->setLocator(Ice::LocatorPrx::uncheckedCast(Ice::ObjectPrx([(ICEObjectPrx*)loc iceObjectPrx])));
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(id<ICELocatorPrx>) getLocator
{
    NSException* nsex = nil;
    try
    {
        return (id<ICELocatorPrx>)[ICELocatorPrx iceObjectPrxWithObjectPrx:adapter_->getLocator()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return nil; // Keep the compiler happy.
}

-(ICEMutableEndpointSeq*) getEndpoints
{
    NSException* nsex = nil;
    try
    {
        return [toNSArray(adapter_->getEndpoints()) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return nil;
}

-(void) refreshPublishedEndpoints
{
    NSException* nsex = nil;
    try
    {
        adapter_->refreshPublishedEndpoints();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(ICEMutableEndpointSeq*) getPublishedEndpoints
{
    NSException* nsex = nil;
    try
    {
        return [toNSArray(adapter_->getPublishedEndpoints()) autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return nil;
}

-(void) setPublishedEndpoints:(ICEEndpointSeq*)newEndpoints
{
    NSException* nsex = nil;
    try
    {
        Ice::EndpointSeq cxxNewEndpoints;
        fromNSArray(newEndpoints, cxxNewEndpoints);
        adapter_->setPublishedEndpoints(cxxNewEndpoints);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

@end
