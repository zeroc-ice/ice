// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcLogger.h"
#import "IceObjcProperties.h"
#import "IceObjcIceUtil.h"
#import "IceObjcUtil.h"
#import "LoggerWrapperI.h"

#import <Ice/Instance.h>
#import <Ice/StringUtil.h>

namespace
{
    class Init
    {
    public:

        Init()
        {
            Ice::registerIceWS(true);
            Ice::registerIceSSL(false);
            Ice::registerIceDiscovery(false);
            Ice::registerIceLocatorDiscovery(false);
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

//TODO update swiftArgs to remove args removed by initialize
+(ICECommunicator*) initialize:(NSArray*)swiftArgs
                    properties:(ICEProperties*)properties
                    logger:(id<ICELoggerProtocol>)logger
                         error:(NSError**)error
{
    Ice::StringSeq args;
    fromNSArray(swiftArgs, args);

    assert(properties);

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
        auto communicator = Ice::initialize(args, initData);
        return [[ICECommunicator alloc] initWithCppCommunicator:communicator];
    }
    catch(const std::exception& err)
    {
        *error = convertException(err);
    }
    return nil;
}

+(id) createProperties:(NSArray*)swiftArgs
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
        *remArgs = toNSArray(a);
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

+(nullable NSString*) identityToString:(NSString*)name
                              category:(NSString*)category
                                  mode:(uint8_t)mode
                                 error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        Ice::Identity identity{fromNSString(name), fromNSString(category)};
        auto s = Ice::identityToString(std::move(identity), static_cast<Ice::ToStringMode>(mode));
        return toNSString(s);
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
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
@end
