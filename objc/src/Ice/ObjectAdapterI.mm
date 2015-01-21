// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
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

#import <objc/Ice/LocalException.h>
#import <objc/Ice/Locator.h>

#include <Ice/Locator.h>
#include <Ice/ServantLocator.h>

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
        return [ICEObjectPrx objectPrxWithObjectPrx__:OBJECTADAPTER->add([servant object__], [ident identity])];
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
        return [ICEObjectPrx objectPrxWithObjectPrx__:OBJECTADAPTER->addFacet([servant object__], [ident identity],
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
        return [ICEObjectPrx objectPrxWithObjectPrx__:OBJECTADAPTER->addWithUUID([servant object__])];
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
        return [ICEObjectPrx objectPrxWithObjectPrx__:OBJECTADAPTER->addFacetWithUUID([servant object__],
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
        Ice::ServantLocatorPtr servantLocator = new DefaultServantLocator([servant object__]);
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
        return toObjC(OBJECTADAPTER->remove([ident identity]));
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
        return toObjC(OBJECTADAPTER->removeFacet([ident identity], fromNSString(facet)));
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
        return toNSDictionary(OBJECTADAPTER->removeAllFacets([ident identity]));
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
        return toObjC(defaultServantLocator->servant());
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
        return toObjC(OBJECTADAPTER->find([ident identity]));
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
        return toObjC(OBJECTADAPTER->findFacet([ident identity], fromNSString(facet)));
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
        return toObjC(OBJECTADAPTER->findByProxy([(ICEObjectPrx*)proxy objectPrx__]));
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
    @throw [ICEFeatureNotSupportedException featureNotSupportedException:__FILE__ line:__LINE__];
}
-(id<ICEServantLocator>) removeServantLocator:(NSString*)category
{
    @throw [ICEFeatureNotSupportedException featureNotSupportedException:__FILE__ line:__LINE__];
}
-(id<ICEServantLocator>) findServantLocator:(NSString*)category
{
    @throw [ICEFeatureNotSupportedException featureNotSupportedException:__FILE__ line:__LINE__];
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
            return toObjC(defaultServantLocator->servant());
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
        return [ICEObjectPrx objectPrxWithObjectPrx__:OBJECTADAPTER->createProxy([ident identity])];
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
        return [ICEObjectPrx objectPrxWithObjectPrx__:OBJECTADAPTER->createDirectProxy([ident identity])];
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
        return [ICEObjectPrx objectPrxWithObjectPrx__:OBJECTADAPTER->createIndirectProxy([ident identity])];
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
        OBJECTADAPTER->setLocator(Ice::LocatorPrx::uncheckedCast(Ice::ObjectPrx([(ICEObjectPrx*)loc objectPrx__])));
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
        return (id<ICELocatorPrx>)[ICELocatorPrx objectPrxWithObjectPrx__:OBJECTADAPTER->getLocator()];
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

-(ICEEndpointSeq*) getEndpoints
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

-(ICEEndpointSeq*) getPublishedEndpoints
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

@end
