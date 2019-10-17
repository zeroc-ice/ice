//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Communicator.h"
#import "ObjectAdapter.h"
#import "ObjectPrx.h"
#import "Logger.h"
#import "ImplicitContext.h"
#import "Properties.h"
#import "IceUtil.h"
#import "BlobjectFacade.h"
#import "Process.h"
#import "PropertiesAdmin.h"
#import "UnsupportedAdminFacet.h"

#import "LoggerWrapperI.h"
#import "Convert.h"

#include <Ice/Instance.h>
#include <Ice/DefaultsAndOverrides.h>

@implementation ICECommunicator

-(std::shared_ptr<Ice::Communicator>) communicator
{
    return std::static_pointer_cast<Ice::Communicator>(self.cppObject);
}

-(void) destroy
{
    self.communicator->destroy();
}

-(void) shutdown
{
    self.communicator->shutdown();
}

-(void) waitForShutdown
{
    self.communicator->waitForShutdown();
}

-(bool) isShutdown
{
    return self.communicator->isShutdown();
}

-(id) stringToProxy:(NSString*)str error:(NSError**)error
{
    try
    {
        auto prx = self.communicator->stringToProxy(fromNSString(str));
        if(prx)
        {
            return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
        }
        return [NSNull null];
    }
    catch(const std::exception& e)
    {
        *error = convertException(e);
        return nil;
    }
}

-(nullable id) propertyToProxy:(NSString*)property error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto prx = self.communicator->propertyToProxy(fromNSString(property));
        if(prx)
        {
            return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
        }
        return [NSNull null];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(NSDictionary<NSString*, NSString*>*) proxyToProperty:(ICEObjectPrx*)prx property:(NSString*)property error:(NSError* _Nullable * _Nullable)error
{
    return toNSDictionary(self.communicator->proxyToProperty([prx prx], fromNSString(property)));
}

-(ICEObjectAdapter*) createObjectAdapter:(NSString*)name error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto oa = self.communicator->createObjectAdapter(fromNSString(name));
        return [ICEObjectAdapter getHandle:oa];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(ICEObjectAdapter*) createObjectAdapterWithEndpoints:(NSString*)name endpoints:(NSString*)endpoints error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto oa = self.communicator->createObjectAdapterWithEndpoints(fromNSString(name), fromNSString(endpoints));
        return [ICEObjectAdapter getHandle:oa];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(ICEObjectAdapter*) createObjectAdapterWithRouter:(NSString*)name router:(ICEObjectPrx*)router error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        assert(router);
        auto oa = self.communicator->createObjectAdapterWithRouter(fromNSString(name),
                                                               Ice::uncheckedCast<Ice::RouterPrx>([router prx]));
        return [ICEObjectAdapter getHandle:oa];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(ICEImplicitContext*) getImplicitContext
{
    auto implicitContext = self.communicator->getImplicitContext();
    return [ICEImplicitContext getHandle:implicitContext];
}

// id<ICELoggerProtocol> may be either a Swift logger or a wrapper around a C++ logger
-(id<ICELoggerProtocol>) getLogger
{
    auto logger = self.communicator->getLogger();

    auto swiftLogger = std::dynamic_pointer_cast<LoggerWrapperI>(logger);
    if(swiftLogger)
    {
        return swiftLogger->getLogger();
    }

    return [ICELogger getHandle:logger];
}

-(nullable ICEObjectPrx*) getDefaultRouter
{
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:self.communicator->getDefaultRouter()];
}

-(BOOL) setDefaultRouter:(ICEObjectPrx*)router error:(NSError**)error
{
    try
    {
        auto r = router ? [router prx] : nullptr;
        self.communicator->setDefaultRouter(Ice::uncheckedCast<Ice::RouterPrx>(r));
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(nullable ICEObjectPrx*) getDefaultLocator
{
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:self.communicator->getDefaultLocator()];
}

-(BOOL) setDefaultLocator:(ICEObjectPrx*)locator error:(NSError**)error
{
    try
    {
        auto l = locator ? [locator prx] : nullptr;
        self.communicator->setDefaultLocator((Ice::uncheckedCast<Ice::LocatorPrx>(l)));
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(BOOL) flushBatchRequests:(uint8_t)compress error:(NSError**)error
{
    try
    {
        self.communicator->flushBatchRequests(Ice::CompressBatch(compress));
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(void) flushBatchRequestsAsync:(uint8_t)compress
                      exception:(void (^)(NSError*))exception
                           sent:(void (^_Nullable)(bool))sent
{
    try
    {
        self.communicator->flushBatchRequestsAsync(Ice::CompressBatch(compress),
                                               [exception](std::exception_ptr e)
                                               {
                                                   @autoreleasepool
                                                   {
                                                       exception(convertException(e));
                                                   }
                                               },
                                               [sent](bool sentSynchronously)
                                               {
                                                   if(sent)
                                                   {
                                                       sent(sentSynchronously);
                                                   }
                                               });
    }
    catch(const std::exception& ex)
    {
        // Typically CommunicatorDestroyedException. Note that the callback is called on the
        // thread making the invocation, which is fine since we only use it to fulfill the
        // PromiseKit promise.
        exception(convertException(ex));
    }
}

-(nullable ICEObjectPrx*) createAdmin:(ICEObjectAdapter* _Nullable)adminAdapter
                                 name:(NSString*)name
                             category:(NSString*)category
                                error:(NSError**)error
{

    try
    {
        auto ident =  Ice::Identity{fromNSString(name), fromNSString(category)};
        auto adapter = adminAdapter ? [adminAdapter objectAdapter] : nullptr;
        auto prx = self.communicator->createAdmin(adapter, ident);
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:prx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }

}

-(nullable id) getAdmin:(NSError**)error
{
    try
    {
        auto adminPrx = self.communicator->getAdmin();
        return adminPrx ? [[ICEObjectPrx alloc] initWithCppObjectPrx:adminPrx] : [NSNull null];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(BOOL) addAdminFacet:(id<ICEBlobjectFacade>)facade facet:(NSString*)facet error:(NSError**)error
{
    try
    {
        auto servant = std::make_shared<BlobjectFacade>(facade);
        self.communicator->addAdminFacet(servant, fromNSString(facet));
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(id<ICEBlobjectFacade>) removeAdminFacet:(NSString*)facet error:(NSError**)error
{
    try
    {
        // servant can either be a Swift wrapped facet or a builtin admin facet
        return [self facetToFacade:self.communicator->removeAdminFacet(fromNSString(facet))];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(nullable id) findAdminFacet:(NSString*)facet error:(NSError**)error
{
    try
    {
        // servant can either be null, a Swift wrapped facet, or a builtin admin facet
        auto servant = self.communicator->findAdminFacet(fromNSString(facet));

        if(!servant)
        {
            return [NSNull null];
        }

        return [self facetToFacade:servant];

    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(nullable NSDictionary<NSString*, id<ICEBlobjectFacade>>*) findAllAdminFacets:(NSError**)error
{
    try
    {
        NSMutableDictionary<NSString*, id<ICEBlobjectFacade>>* facets = [NSMutableDictionary dictionary];

        for(const auto& d : self.communicator->findAllAdminFacets())
        {
            [facets setObject:[self facetToFacade:d.second] forKey:toNSString(d.first)];
        }

        return facets;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(ICEProperties*) getProperties
{
    auto props = self.communicator->getProperties();
    return [ICEProperties getHandle:props];
}

-(nullable dispatch_queue_t) getClientDispatchQueue:(NSError* _Nullable * _Nullable)error
{
    try
    {
        return self.communicator->getClientDispatchQueue();
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(nullable dispatch_queue_t) getServerDispatchQueue:(NSError* _Nullable * _Nullable)error
{
    try
    {
        return self.communicator->getServerDispatchQueue();
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(void) getDefaultEncoding:(uint8_t*)major minor:(uint8_t*)minor
{
    auto defaultEncoding = IceInternal::getInstance(self.communicator)->defaultsAndOverrides()->defaultEncoding;
    *major = defaultEncoding.major;
    *minor = defaultEncoding.minor;
}

-(uint8_t) getDefaultFormat
{
    return static_cast<uint8_t>(IceInternal::getInstance(self.communicator)->defaultsAndOverrides()->defaultFormat);
}

-(id<ICEBlobjectFacade>) facetToFacade:(const std::shared_ptr<Ice::Object>&) servant
{
    if(!servant)
    {
        return nil;
    }

    auto blobjectFacade = std::dynamic_pointer_cast<BlobjectFacade>(servant);
    if(blobjectFacade)
    {
        return blobjectFacade->getFacade();
    }

    Class<ICEAdminFacetFactory> factory = [ICEUtil adminFacetFactory];

    auto process = std::dynamic_pointer_cast<Ice::Process>(servant);
    if(process)
    {
        return [factory createProcess:self handle:[ICEProcess getHandle:process]];
    }

    auto propertiesAdmin = std::dynamic_pointer_cast<Ice::PropertiesAdmin>(servant);
    if(propertiesAdmin)
    {
        return [factory createProperties:self handle: [ICEPropertiesAdmin getHandle:propertiesAdmin]];
    }

    return [factory createUnsupported:self handle:[ICEUnsupportedAdminFacet getHandle:servant]];
}

-(void) setSslCertificateVerifier:(nullable bool (^)(id))verifier
{
    auto pluginManager = self.communicator->getPluginManager();
    auto plugin = std::dynamic_pointer_cast<IceSSL::Plugin>(pluginManager->getPlugin("IceSSL"));
    assert(plugin);

    plugin->setCertificateVerifier([verifier] (const std::shared_ptr<IceSSL::ConnectionInfo>& info) -> bool {
        return verifier(createConnectionInfo(info));
    });
}

-(void) setSslPasswordPrompt:(nullable NSString* (^)())prompt;
{
    auto pluginManager = self.communicator->getPluginManager();
    auto plugin = std::dynamic_pointer_cast<IceSSL::Plugin>(pluginManager->getPlugin("IceSSL"));
    assert(plugin);
    plugin->setPasswordPrompt([prompt] {
        return fromNSString(prompt());
    });
}

-(BOOL) initializePlugins: (NSError**)error
{
    try
    {
        self.communicator->getPluginManager()->initializePlugins();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}
@end
