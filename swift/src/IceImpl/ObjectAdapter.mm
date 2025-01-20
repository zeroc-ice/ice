// Copyright (c) ZeroC, Inc.

#import "include/ObjectAdapter.h"
#import "Convert.h"
#import "include/Communicator.h"
#import "include/Config.h"
#import "include/Connection.h"
#import "include/DispatchAdapter.h"
#import "include/ObjectPrx.h"

@implementation ICEObjectAdapter

- (std::shared_ptr<Ice::ObjectAdapter>)objectAdapter
{
    return std::static_pointer_cast<Ice::ObjectAdapter>(self.cppObject);
}

- (NSString*)getName
{
    return toNSString(self.objectAdapter->getName());
}

- (ICECommunicator*)getCommunicator
{
    auto comm = self.objectAdapter->getCommunicator();
    return [ICECommunicator getHandle:comm];
}

- (BOOL)activate:(NSError* _Nullable* _Nullable)error
{
    try
    {
        self.objectAdapter->activate();
        return YES;
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return NO;
    }
}

- (void)hold
{
    try
    {
        self.objectAdapter->hold();
    }
    catch (const Ice::ObjectAdapterDeactivatedException&)
    {
        // ignored
    }
    catch (const std::exception&)
    {
        // unexpected but ignored nevertheless
    }
}

- (void)waitForHold
{
    try
    {
        self.objectAdapter->waitForHold();
    }
    catch (const Ice::ObjectAdapterDeactivatedException&)
    {
        // ignored, returns immediately
    }
    catch (const std::exception&)
    {
        // unexpected but ignored nevertheless
    }
}

- (void)deactivate
{
    self.objectAdapter->deactivate();
}

- (void)waitForDeactivate
{
    self.objectAdapter->waitForDeactivate();
}

- (BOOL)isDeactivated
{
    return self.objectAdapter->isDeactivated();
}

- (void)destroy
{
    self.objectAdapter->destroy();
}

- (nullable ICEObjectPrx*)createProxy:(NSString*)name
                             category:(NSString*)category
                                error:(NSError* _Nullable* _Nullable)error
{
    try
    {
        auto prx = self.objectAdapter->createProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (nullable ICEObjectPrx*)createDirectProxy:(NSString*)name
                                   category:(NSString*)category
                                      error:(NSError* _Nullable* _Nullable)error
{
    try
    {
        auto prx = self.objectAdapter->createDirectProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (nullable ICEObjectPrx*)createIndirectProxy:(NSString*)name
                                     category:(NSString*)category
                                        error:(NSError* _Nullable* _Nullable)error
{
    try
    {
        auto prx = self.objectAdapter->createIndirectProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return nil;
    }
}

- (void)setLocator:(nullable ICEObjectPrx*)locator
{
    try
    {
        std::optional<Ice::ObjectPrx> l;
        if (locator)
        {
            l = [locator prx];
        }
        self.objectAdapter->setLocator(Ice::uncheckedCast<Ice::LocatorPrx>(l));
    }
    catch (const Ice::ObjectAdapterDeactivatedException&)
    {
        // ignored
    }
    catch (const Ice::CommunicatorDestroyedException&)
    {
        // ignored
    }
    catch (const std::exception&)
    {
        // unexpected but ignored nevertheless
    }
}

- (nullable ICEObjectPrx*)getLocator
{
    auto prx = self.objectAdapter->getLocator();
    if (prx)
    {
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx.value()];
    }
    else
    {
        return nil;
    }
}

- (NSArray<ICEEndpoint*>*)getEndpoints
{
    return toNSArray(self.objectAdapter->getEndpoints());
}

- (NSArray<ICEEndpoint*>*)getPublishedEndpoints
{
    return toNSArray(self.objectAdapter->getPublishedEndpoints());
}

- (BOOL)setPublishedEndpoints:(NSArray<ICEEndpoint*>*)newEndpoints error:(NSError* _Nullable* _Nullable)error
{
    try
    {
        Ice::EndpointSeq endpts;
        fromNSArray(newEndpoints, endpts);

        self.objectAdapter->setPublishedEndpoints(endpts);
        return YES;
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return NO;
    }
}

- (void)registerDispatchAdapter:(id<ICEDispatchAdapter>)dispatchAdapter
{
    auto cppDispatcher = std::make_shared<CppDispatcher>(dispatchAdapter);
    self.objectAdapter->addDefaultServant(cppDispatcher, "");
}

@end
