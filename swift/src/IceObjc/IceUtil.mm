//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Logger.h"
#import "Properties.h"
#import "IceUtil.h"
#import "Util.h"
#import "LoggerWrapperI.h"

#import <Ice/Instance.h>
#import <Ice/StringUtil.h>
#import <Ice/Network.h>

namespace
{
    class Init
    {
    public:

        Init()
        {
            // Register plug-ins included in the Ice framework (a single binary file)
            // See also RegisterPluginsInit.cpp in cpp/src/Ice
            //
            Ice::registerIceSSL(false);
            Ice::registerIceDiscovery(false);
            Ice::registerIceLocatorDiscovery(false);
#if defined(__APPLE__) && TARGET_OS_IPHONE != 0
            //  Ice::registerIceIAP(false);
#endif
        }
    };
    Init init;
}

@implementation ICEUtil
static Class<ICEExceptionFactory> _exceptionFactory;
static Class<ICEConnectionInfoFactory> _connectionInfoFactory;
static Class<ICEEndpointInfoFactory> _endpointInfoFactory;
static Class<ICEAdminFacetFactory> _adminFacetFactory;

+(Class<ICEExceptionFactory>) exceptionFactory
{
    return _exceptionFactory;
}

+(Class<ICEConnectionInfoFactory>) connectionInfoFactory
{
    return _connectionInfoFactory;
}

+(Class<ICEEndpointInfoFactory>) endpointInfoFactory
{
    return _endpointInfoFactory;
}

+(Class<ICEAdminFacetFactory>) adminFacetFactory
{
    return _adminFacetFactory;
}

+(BOOL) registerFactories:(Class<ICEExceptionFactory>)exception
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

+(ICECommunicator*) initialize:(NSArray*)swiftArgs
                    properties:(ICEProperties*)properties
                withConfigFile:(BOOL)withConfigFile
                        logger:(id<ICELoggerProtocol>)logger
                       remArgs:(NSArray**)remArgs
                         error:(NSError**)error
{
    Ice::StringSeq args;
    fromNSArray(swiftArgs, args);

    assert(properties);
    assert(withConfigFile || args.empty());

    //
    // Collect InitializationData members.
    //
    Ice::InitializationData initData;
    initData.properties = [properties properties];

    if(logger)
    {
        initData.logger = std::make_shared<LoggerWrapperI>(logger);
    }

    try
    {
        std::shared_ptr<Ice::Communicator> communicator;
        if(withConfigFile)
        {
            communicator = Ice::initialize(args, initData);
            *remArgs = toNSArray(args);
        }
        else
        {
            communicator = Ice::initialize(initData);
        }
        return [[ICECommunicator alloc] initWithCppCommunicator:communicator];
    }
    catch(const std::exception& err)
    {
        *error = convertException(err);
    }
    return nil;
}

+(ICEProperties*) createProperties
{
    return [[ICEProperties alloc] initWithCppProperties:Ice::createProperties()];
}

+(ICEProperties*) createProperties:(NSArray*)swiftArgs
                          defaults:(ICEProperties*)defaults
                           remArgs:(NSArray**)remArgs
                             error:(NSError**)error
{
    try
    {
        std::vector<std::string> a;
        fromNSArray(swiftArgs, a);
        std::shared_ptr<Ice::Properties> def;
        if(defaults)
        {
            def = [defaults properties];
        }
        auto props = Ice::createProperties(a, def);

        // a now contains remaning arguments that were not used by Ice::createProperties
        if(remArgs)
        {
            *remArgs = toNSArray(a);
        }
        return [[ICEProperties alloc] initWithCppProperties:props];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
    }

    return nil;
}

+(BOOL) stringToIdentity:(NSString*)str
                    name:(NSString* __strong _Nonnull *  _Nonnull)name
                category:(NSString* __strong  _Nonnull *  _Nonnull)category
                   error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        auto ident = Ice::stringToIdentity(fromNSString(str));
        *name = toNSString(ident.name);
        *category = toNSString(ident.category);
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

+(NSString*) identityToString:(NSString*)name
                              category:(NSString*)category
                                  mode:(uint8_t)mode
{
    Ice::Identity identity{fromNSString(name), fromNSString(category)};
    return toNSString(Ice::identityToString(identity, static_cast<Ice::ToStringMode>(mode)));
}

+(void) currentEncoding:(UInt8*)major minor:(UInt8*)minor
{
    auto encoding = Ice::currentEncoding;
    *major = encoding.major;
    *minor = encoding.minor;
}

+(NSString*) encodingVersionToString:(UInt8)major minor:(UInt8)minor
{
    Ice::EncodingVersion v {major, minor};
    return toNSString(Ice::encodingVersionToString(v));
}

+(NSString*) escapeString:(NSString *)string
                  special:(NSString *)special
                     communicator:(ICECommunicator*)communicator error:(NSError *__autoreleasing  _Nullable *)error
{
    try
    {

        auto instance = IceInternal::getInstance([communicator communicator]);
        return toNSString(IceInternal::escapeString(fromNSString(string),
                                                    fromNSString(special),
                                                    instance->toStringMode()));
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }

}

+(NSString*) errorToString:(int32_t)error
{
    return toNSString(IceUtilInternal::errorToString(error));
}

+(NSString*) errorToStringDNS:(int32_t)error
{
    return toNSString(IceInternal::errorToStringDNS(error));
}

@end
