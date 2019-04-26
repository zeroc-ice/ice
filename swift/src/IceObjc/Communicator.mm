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
#import "Util.h"

#include <Ice/Instance.h>
#include <Ice/DefaultsAndOverrides.h>

@implementation ICECommunicator

-(instancetype) initWithCppCommunicator:(std::shared_ptr<Ice::Communicator>)communicator
{
    self = [super initWithLocalObject:communicator.get()];
    if(self)
    {
        self->_communicator = communicator;
    }
    return self;
}

-(void) destroy
{
    _communicator->destroy();
}

-(void) shutdown
{
    _communicator->shutdown();
}

-(void) waitForShutdown
{
    _communicator->waitForShutdown();
}

-(bool) isShutdown
{
    return _communicator->isShutdown();
}

-(id) stringToProxy:(NSString*)str error:(NSError**)error
{
    try
    {
        ICEObjectPrx* prx = [[ICEObjectPrx alloc] initWithCppObjectPrx:_communicator->stringToProxy(fromNSString(str))];
        if(prx)
        {
            return prx;
        }
        else
        {
            return [NSNull null];
        }
    }
    catch(const std::exception& e)
    {
        *error = convertException(e);
    }

    return nil;
}

-(nullable id) propertyToProxy:(NSString*)property error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto prx = _communicator->propertyToProxy(fromNSString(property));
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
    return toNSDictionary(_communicator->proxyToProperty([prx prx], fromNSString(property)));
}

-(ICEObjectAdapter*) createObjectAdapter:(NSString*)name error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto oa = _communicator->createObjectAdapter(fromNSString(name));
        return [[ICEObjectAdapter alloc] initWithCppObjectAdapter:oa];
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
        auto oa = _communicator->createObjectAdapterWithEndpoints(fromNSString(name), fromNSString(endpoints));
        return [[ICEObjectAdapter alloc] initWithCppObjectAdapter:oa];
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
        auto oa = _communicator->createObjectAdapterWithRouter(fromNSString(name),
                                                               Ice::uncheckedCast<Ice::RouterPrx>([router prx]));
        return [[ICEObjectAdapter alloc] initWithCppObjectAdapter:oa];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(ICEImplicitContext*) getImplicitContext
{
    auto implicitContext = _communicator->getImplicitContext();
    return createLocalObject(implicitContext, [&implicitContext] () -> id
    {
        return [[ICEImplicitContext alloc] initWithCppImplicitContext:implicitContext];
    });
}

// id<ICELoggerProtocol> may be either a Swift logger or a wrapper around a C++ logger
-(id<ICELoggerProtocol>) getLogger
{
    auto logger = _communicator->getLogger();

    auto swiftLogger = std::dynamic_pointer_cast<LoggerWrapperI>(logger);
    if(swiftLogger)
    {
        return swiftLogger->getLogger();
    }

    return createLocalObject(logger, [&logger] () -> id
    {
        return [[ICELogger alloc] initWithCppLogger:logger];
    });
}

-(nullable ICEObjectPrx*) getDefaultRouter
{
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_communicator->getDefaultRouter()];
}

-(BOOL) setDefaultRouter:(ICEObjectPrx*)router error:(NSError**)error
{
    try
    {
        auto r = router ? [router prx] : nullptr;
        _communicator->setDefaultRouter(Ice::uncheckedCast<Ice::RouterPrx>(r));
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
    return [[ICEObjectPrx alloc] initWithCppObjectPrx:_communicator->getDefaultLocator()];
}

-(BOOL) setDefaultLocator:(ICEObjectPrx*)locator error:(NSError**)error
{
    try
    {
        auto l = locator ? [locator prx] : nullptr;
        _communicator->setDefaultLocator((Ice::uncheckedCast<Ice::LocatorPrx>(l)));
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
        _communicator->flushBatchRequests(Ice::CompressBatch(compress));
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(BOOL) flushBatchRequestsAsync:(uint8_t)compress
                      exception:(void (^)(NSError*))exception
                           sent:(void (^_Nullable)(bool))sent
                          error:(NSError**)error
{
    try
    {
        _communicator->flushBatchRequestsAsync(Ice::CompressBatch(compress),
                                               [exception](std::exception_ptr e)
                                               {
                                                   exception(convertException(e));
                                               },
                                               [sent](bool sentSynchronously)
                                               {
                                                   if(sent)
                                                   {
                                                       sent(sentSynchronously);
                                                   }
                                               });
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
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
        auto prx = _communicator->createAdmin(adapter, ident);
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
        auto adminPrx = _communicator->getAdmin();
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
        _communicator->addAdminFacet(servant, fromNSString(facet));
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
        return [self facetToFacade:_communicator->removeAdminFacet(fromNSString(facet))];
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
        auto servant = _communicator->findAdminFacet(fromNSString(facet));

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

        for(const auto& d : _communicator->findAllAdminFacets())
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
    auto props = _communicator->getProperties();
    return createLocalObject(props, [&props]() -> id
                             {
                                 return [[ICEProperties alloc] initWithCppProperties:props];
                             });
}

-(void) getDefaultEncoding:(nonnull uint8_t*)major minor:(nonnull uint8_t*)minor
{
    auto defaultEncoding = IceInternal::getInstance(_communicator)->defaultsAndOverrides()->defaultEncoding;
    *major = defaultEncoding.major;
    *minor = defaultEncoding.minor;
}

-(uint8_t) getDefaultFormat
{
    return static_cast<uint8_t>(IceInternal::getInstance(_communicator)->defaultsAndOverrides()->defaultFormat);
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
        return [factory createProcess:self
                               handle:[[ICEProcess alloc] initWithCppProcess:process]];
    }

    auto propertiesAdmin = std::dynamic_pointer_cast<Ice::PropertiesAdmin>(servant);
    if(propertiesAdmin)
    {
        return [factory createProperties:self
                                  handle: [[ICEPropertiesAdmin alloc] initWithCppPropertiesAdmin:propertiesAdmin]];
    }

    return [factory createUnsupported:self
                               handle:[[ICEUnsupportedAdminFacet alloc] initWithCppAdminFacet:servant]];
}

@end
