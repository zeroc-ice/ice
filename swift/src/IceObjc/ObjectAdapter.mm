//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Config.h"
#import "ObjectAdapter.h"
#import "Util.h"
#import "ObjectPrx.h"
#import "Connection.h"
#import "Communicator.h"
#import "BlobjectFacade.h"

@implementation ICEObjectAdapter

-(instancetype) initWithCppObjectAdapter:(std::shared_ptr<Ice::ObjectAdapter>)objectAdapter
{
    self = [super initWithLocalObject:objectAdapter.get()];
    if(self)
    {
        self->_objectAdapter = objectAdapter;
    }
    return self;
}

-(NSString*) getName
{
    return toNSString(_objectAdapter->getName());
}

-(ICECommunicator*) getCommunicator
{
    auto comm = _objectAdapter->getCommunicator().get();
    return [ICECommunicator fromLocalObject:comm];
}

-(BOOL) activate:(NSError* _Nullable * _Nullable)error
{
    try
    {
        _objectAdapter->activate();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(void) hold
{
    try
    {
        _objectAdapter->hold();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        // ignored
    }
    catch(const std::exception&)
    {
        // unexpected but ignored nevertheless
    }
}

-(void) waitForHold
{
    try
    {
        _objectAdapter->waitForHold();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        // ignored, returns immediately
    }
    catch(const std::exception&)
    {
        // unexpected but ignored nevertheless
    }
}

-(void) deactivate
{
    _objectAdapter->deactivate();
}

-(void) waitForDeactivate
{
    _objectAdapter->waitForDeactivate();
}

-(BOOL) isDeactivated
{
    return _objectAdapter->isDeactivated();
}

-(void) destroy
{
    _objectAdapter->destroy();
}

-(nullable ICEObjectPrx*) createProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto prx = _objectAdapter->createProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(nullable ICEObjectPrx*) createDirectProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto prx = _objectAdapter->createDirectProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(nullable ICEObjectPrx*) createIndirectProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto prx = _objectAdapter->createIndirectProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(void) setLocator:(nullable ICEObjectPrx*) locator
{
    try
    {
        auto l = locator ? [locator prx] : nullptr;
        _objectAdapter->setLocator(Ice::uncheckedCast<Ice::LocatorPrx>(l));
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        // ignored
    }
    catch(const Ice::CommunicatorDestroyedException&)
    {
        // ignored
    }
    catch(const std::exception&)
    {
        // unexpected but ignored nevertheless
    }
}

-(nullable ICEObjectPrx*) getLocator
{
    auto prx = _objectAdapter->getLocator();
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
}

-(NSArray<ICEEndpoint*>*) getEndpoints
{
    return toNSArray(_objectAdapter->getEndpoints());
}

-(BOOL) refreshPublishedEndpoints:(NSError* _Nullable * _Nullable)error
{
    try
    {
        _objectAdapter->refreshPublishedEndpoints();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(NSArray<ICEEndpoint*>*) getPublishedEndpoints
{
    return toNSArray(_objectAdapter->getPublishedEndpoints());
}

-(BOOL) setPublishedEndpoints:(NSArray<ICEEndpoint*>*)newEndpoints error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        Ice::EndpointSeq endpts;
        fromNSArray(newEndpoints, endpts);

        _objectAdapter->setPublishedEndpoints(endpts);
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(void) registerDefaultServant:(id<ICEBlobjectFacade>)facade
{
    auto servant = std::make_shared<BlobjectFacade>(facade);
    _objectAdapter->addDefaultServant(servant, "");
}

@end
