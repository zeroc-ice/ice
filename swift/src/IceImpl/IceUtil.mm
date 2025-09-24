// Copyright (c) ZeroC, Inc.

#import "include/IceUtil.h"
#import "Convert.h"
#import "Ice/StringUtil.h"
#import "LoggerWrapperI.h"
#import "include/Logger.h"
#import "include/Properties.h"

#include "IceDiscovery/IceDiscovery.h"
#include "IceLocatorDiscovery/IceLocatorDiscovery.h"

@implementation ICEUtil
static Class<ICELocalExceptionFactory> _exceptionFactory;
static Class<ICEConnectionInfoFactory> _connectionInfoFactory;
static Class<ICEEndpointInfoFactory> _endpointInfoFactory;
static Class<ICEAdminFacetFactory> _adminFacetFactory;

+ (Class<ICELocalExceptionFactory>)localExceptionFactory
{
    return _exceptionFactory;
}

+ (Class<ICEConnectionInfoFactory>)connectionInfoFactory
{
    return _connectionInfoFactory;
}

+ (Class<ICEEndpointInfoFactory>)endpointInfoFactory
{
    return _endpointInfoFactory;
}

+ (Class<ICEAdminFacetFactory>)adminFacetFactory
{
    return _adminFacetFactory;
}

+ (BOOL)registerFactories:(Class<ICELocalExceptionFactory>)exception
           connectionInfo:(Class<ICEConnectionInfoFactory>)connectionInfo
             endpointInfo:(Class<ICEEndpointInfoFactory>)endpointInfo
               adminFacet:(Class<ICEAdminFacetFactory>)adminFacet
{
    _exceptionFactory = exception;
    _connectionInfoFactory = connectionInfo;
    _endpointInfoFactory = endpointInfo;
    _adminFacetFactory = adminFacet;
    return true;
}

+ (ICECommunicator*)initialize:(ICEProperties*)properties logger:(id<ICELoggerProtocol>)logger error:(NSError**)error
{
    assert(properties);

    // Collect InitializationData members.
    Ice::InitializationData initData;
    initData.properties = [properties properties];

    if (logger)
    {
        initData.logger = std::make_shared<LoggerWrapperI>(logger);
    }

    // Add plug-in factories.
    initData.pluginFactories = {
        Ice::udpPluginFactory(),
        Ice::wsPluginFactory(),
#if defined(__APPLE__) && TARGET_OS_IPHONE != 0
        Ice::iapPluginFactory(),
#endif
    };

    // Add IceDiscovery/IceLocatorDiscovery if these plug-ins are configured via Ice.Plugin.name.
    if (!initData.properties->getIceProperty("Ice.Plugin.IceDiscovery").empty())
    {
        initData.pluginFactories.push_back(IceDiscovery::discoveryPluginFactory());
    }

    if (!initData.properties->getIceProperty("Ice.Plugin.IceLocatorDiscovery").empty())
    {
        initData.pluginFactories.push_back(IceLocatorDiscovery::locatorDiscoveryPluginFactory());
    }

    try
    {
        Ice::CommunicatorPtr communicator = Ice::initialize(initData);
        return [ICECommunicator getHandle:communicator];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
    }
    return nil;
}

+ (ICEProperties*)createProperties
{
    return [ICEProperties getHandle:Ice::createProperties()];
}

+ (ICEProperties*)createProperties:(NSArray*)swiftArgs
                          defaults:(ICEProperties*)defaults
                           remArgs:(NSArray**)remArgs
                             error:(NSError**)error
{
    try
    {
        std::vector<std::string> a;
        fromNSArray(swiftArgs, a);
        std::shared_ptr<Ice::Properties> def;
        if (defaults)
        {
            def = [defaults properties];
        }
        auto props = Ice::createProperties(a, def);

        // a now contains remaining arguments that were not used by Ice::createProperties
        if (remArgs)
        {
            *remArgs = toNSArray(a);
        }
        return [ICEProperties getHandle:props];
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
    }

    return nil;
}

+ (BOOL)stringToIdentity:(NSString*)str
                    name:(NSString* __strong _Nonnull* _Nonnull)name
                category:(NSString* __strong _Nonnull* _Nonnull)category
                   error:(NSError* _Nullable* _Nullable)error
{
    try
    {
        auto ident = Ice::stringToIdentity(fromNSString(str));
        *name = toNSString(ident.name);
        *category = toNSString(ident.category);
        return YES;
    }
    catch (...)
    {
        *error = convertException(std::current_exception());
        return NO;
    }
}

+ (NSString*)identityToString:(NSString*)name category:(NSString*)category mode:(std::uint8_t)mode
{
    Ice::Identity identity{fromNSString(name), fromNSString(category)};
    return toNSString(Ice::identityToString(identity, static_cast<Ice::ToStringMode>(mode)));
}

+ (NSString*)encodingVersionToString:(UInt8)major minor:(UInt8)minor
{
    Ice::EncodingVersion v{major, minor};
    return toNSString(Ice::encodingVersionToString(v));
}
@end
