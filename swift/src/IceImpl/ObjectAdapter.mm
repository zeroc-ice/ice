//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Config.h"
#import "ObjectAdapter.h"
#import "Convert.h"
#import "ObjectPrx.h"
#import "Connection.h"
#import "Communicator.h"
#import "BlobjectFacade.h"

@implementation ICEObjectAdapter

-(std::shared_ptr<Ice::ObjectAdapter>) objectAdapter
{
    return std::static_pointer_cast<Ice::ObjectAdapter>(self.cppObject);
}

-(NSString*) getName
{
    return toNSString(self.objectAdapter->getName());
}

-(ICECommunicator*) getCommunicator
{
    auto comm = self.objectAdapter->getCommunicator();
    return [ICECommunicator getHandle:comm];
}

-(BOOL) activate:(NSError* _Nullable * _Nullable)error
{
    try
    {
        self.objectAdapter->activate();
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
        self.objectAdapter->hold();
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
        self.objectAdapter->waitForHold();
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
    self.objectAdapter->deactivate();
}

-(void) waitForDeactivate
{
    self.objectAdapter->waitForDeactivate();
}

-(BOOL) isDeactivated
{
    return self.objectAdapter->isDeactivated();
}

-(void) destroy
{
    self.objectAdapter->destroy();
}

-(nullable ICEObjectPrx*) createProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto prx = self.objectAdapter->createProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
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
        auto prx = self.objectAdapter->createDirectProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
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
        auto prx = self.objectAdapter->createIndirectProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
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
        self.objectAdapter->setLocator(Ice::uncheckedCast<Ice::LocatorPrx>(l));
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
    auto prx = self.objectAdapter->getLocator();
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
}

-(NSArray<ICEEndpoint*>*) getEndpoints
{
    return toNSArray(self.objectAdapter->getEndpoints());
}

-(BOOL) refreshPublishedEndpoints:(NSError* _Nullable * _Nullable)error
{
    try
    {
        self.objectAdapter->refreshPublishedEndpoints();
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
    return toNSArray(self.objectAdapter->getPublishedEndpoints());
}

-(BOOL) setPublishedEndpoints:(NSArray<ICEEndpoint*>*)newEndpoints error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        Ice::EndpointSeq endpts;
        fromNSArray(newEndpoints, endpts);

        self.objectAdapter->setPublishedEndpoints(endpts);
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(dispatch_queue_t) getDispatchQueue:(NSError* _Nullable * _Nullable)error
{
    try
    {
        return self.objectAdapter->getDispatchQueue();
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(void) registerDefaultServant:(id<ICEBlobjectFacade>)facade
{
    auto servant = std::make_shared<BlobjectFacade>(facade);
    self.objectAdapter->addDefaultServant(servant, "");
}

@end
