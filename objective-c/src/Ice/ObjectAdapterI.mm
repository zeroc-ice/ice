// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
class DefaultServantLocator : public Ice::ServantLocator
{
public:

    DefaultServantLocator(const Ice::ObjectPtr& s) :
        _servant(s)
    {
    }

    virtual Ice::ObjectPtr
    locate(const Ice::Current&, Ice::LocalObjectPtr&)
    {
        return _servant;
    }

    virtual void
    finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&)
    {
    }

    virtual void
    deactivate(const std::string&)
    {
    }

    const Ice::ObjectPtr&
    servant() const
    {
        return _servant;
    }

private:
    Ice::ObjectPtr _servant;
};
typedef IceUtil::Handle<DefaultServantLocator> DefaultServantLocatorPtr;

class Cookie : public Ice::LocalObject
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
typedef IceUtil::Handle<Cookie> CookiePtr;

class ExceptionWriter : public Ice::UserException
{
public:

    ExceptionWriter(ICEUserException* ex) : _ex(ex)
    {
    }

    ~ExceptionWriter() throw()
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

class ServantLocatorWrapper : public Ice::ServantLocator
{
public:

    ServantLocatorWrapper(id<ICEServantLocator> locator) :
        _locator(locator)
    {
        [_locator retain];
    }

    ~ServantLocatorWrapper()
    {
        [_locator release];
    }

    virtual Ice::ObjectPtr
    locate(const Ice::Current& current, Ice::LocalObjectPtr& cookie)
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
                    cookie = new Cookie(co);
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

    virtual void
    finished(const Ice::Current& current, const Ice::ObjectPtr& servant, const Ice::LocalObjectPtr& cookie)
    {
        NSException* ex = nil;
        @autoreleasepool
        {
            ICECurrent* cu = [[ICECurrent alloc] initWithCurrent:current];
            id co = cookie ? CookiePtr::dynamicCast(cookie)->cookie() : nil;
            @try
            {
                [_locator finished:cu servant:toObjC(servant) cookie:co];
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

    virtual void
    deactivate(const std::string& category)
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

    id<ICEServantLocator>
    locator() const
    {
        return [[_locator retain] autorelease];
    }

private:

    id<ICEServantLocator> _locator;
};
typedef IceUtil::Handle<ServantLocatorWrapper> ServantLocatorWrapperPtr;

}

#define OBJECTADAPTER dynamic_cast<Ice::ObjectAdapter*>(static_cast<IceUtil::Shared*>(cxxObject_))

@implementation ICEObjectAdapter
-(Ice::ObjectAdapter*) adapter
{
    return OBJECTADAPTER;
}
//
// @protocol ICEObjectAdapter methods.
//

-(id<ICECommunicator>) getCommunicator
{
    NSException* nsex = nil;
    try
    {
        return [ICECommunicator localObjectWithCxxObject:OBJECTADAPTER->getCommunicator().get()];
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
        return [toNSMutableString(OBJECTADAPTER->getName()) autorelease];
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
        OBJECTADAPTER->activate();
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
        OBJECTADAPTER->hold();
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
        OBJECTADAPTER->waitForHold();
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
        OBJECTADAPTER->deactivate();
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
        OBJECTADAPTER->waitForDeactivate();
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
        return OBJECTADAPTER->isDeactivated();
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
        OBJECTADAPTER->destroy();
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
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:OBJECTADAPTER->add([servant iceObject], [ident identity])];
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
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:OBJECTADAPTER->addFacet([servant iceObject], [ident identity],
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
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:OBJECTADAPTER->addWithUUID([servant iceObject])];
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
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:OBJECTADAPTER->addFacetWithUUID([servant iceObject],
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
        Ice::ServantLocatorPtr servantLocator = new DefaultServantLocator([servant iceObject]);
        OBJECTADAPTER->addServantLocator(servantLocator, fromNSString(category));
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
        return [toObjC(OBJECTADAPTER->remove([ident identity])) autorelease];
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
        return [toObjC(OBJECTADAPTER->removeFacet([ident identity], fromNSString(facet))) autorelease];
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
        return [toNSDictionary(OBJECTADAPTER->removeAllFacets([ident identity])) autorelease];
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
        Ice::ServantLocatorPtr locator = OBJECTADAPTER->removeServantLocator(fromNSString(category));
        if(!locator)
        {
            return nil;
        }
        DefaultServantLocatorPtr defaultServantLocator = DefaultServantLocatorPtr::dynamicCast(locator);
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
        return [toObjC(OBJECTADAPTER->find([ident identity])) autorelease];
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
        return [toObjC(OBJECTADAPTER->findFacet([ident identity], fromNSString(facet))) autorelease];
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
        return toNSDictionary(OBJECTADAPTER->findAllFacets([ident identity]));
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
        return [toObjC(OBJECTADAPTER->findByProxy([(ICEObjectPrx*)proxy iceObjectPrx])) autorelease];
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
        OBJECTADAPTER->addServantLocator(new ServantLocatorWrapper(locator), fromNSString(category));
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
        Ice::ServantLocatorPtr locator = OBJECTADAPTER->removeServantLocator(fromNSString(category));
        return ServantLocatorWrapperPtr::dynamicCast(locator)->locator();
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
        Ice::ServantLocatorPtr locator = OBJECTADAPTER->removeServantLocator(fromNSString(category));
        return locator ? ServantLocatorWrapperPtr::dynamicCast(locator)->locator() : nil;
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
        Ice::ServantLocatorPtr servantLocator = OBJECTADAPTER->findServantLocator(fromNSString(category));
        if(servantLocator == 0)
        {
            return nil;
        }
        DefaultServantLocatorPtr defaultServantLocator = DefaultServantLocatorPtr::dynamicCast(servantLocator);
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
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:OBJECTADAPTER->createProxy([ident identity])];
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
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:OBJECTADAPTER->createDirectProxy([ident identity])];
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
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:OBJECTADAPTER->createIndirectProxy([ident identity])];
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
        OBJECTADAPTER->setLocator(Ice::LocatorPrx::uncheckedCast(Ice::ObjectPrx([(ICEObjectPrx*)loc iceObjectPrx])));
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
        return (id<ICELocatorPrx>)[ICELocatorPrx iceObjectPrxWithObjectPrx:OBJECTADAPTER->getLocator()];
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
        return [toNSArray(OBJECTADAPTER->getEndpoints()) autorelease];
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
        OBJECTADAPTER->refreshPublishedEndpoints();
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
        return [toNSArray(OBJECTADAPTER->getPublishedEndpoints()) autorelease];
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
        OBJECTADAPTER->setPublishedEndpoints(cxxNewEndpoints);
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
