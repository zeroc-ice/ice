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

    class LoggerWrapperI : public Ice::Logger
    {
    public:

        LoggerWrapperI(id<ICELoggerProtocol> logger) : _logger(logger)
        {
        }

        virtual ~LoggerWrapperI()
        {
        }

        virtual void
        print(const std::string& msg)
        {
            [_logger print:toNSString(msg)];
        }

        virtual void
        trace(const std::string& category, const std::string& msg)
        {
            [_logger trace:toNSString(category) message:toNSString(msg)];
        }

        virtual void
        warning(const std::string& msg)
        {
            [_logger warning:toNSString(msg)];
        }

        virtual void
        error(const std::string& msg)
        {
            [_logger error:toNSString(msg)];
        }

        virtual std::shared_ptr<Ice::Logger>
        cloneWithPrefix(const std::string& prefix)
        {
            return std::make_shared<LoggerWrapperI>([_logger cloneWithPrefix:toNSString(prefix)]);
        }

        virtual std::string
        getPrefix()
        {
            return fromNSString([_logger getPrefix]);
        }

        id<ICELoggerProtocol>
        getLogger()
        {
            return _logger;
        }

    private:

        id<ICELoggerProtocol> _logger;
    };
}

@implementation ICEUtil
static Class<ICELocalExceptionFactory> _localExceptionFactory;
static Class<ICEConnectionInfoFactory> _connectionInfoFactory;
static Class<ICEEndpointInfoFactory> _endpointInfoFactory;

+(Class<ICELocalExceptionFactory>) localExceptionFactory
{
    return _localExceptionFactory;

}

+(Class<ICEConnectionInfoFactory>) connectionInfoFactory
{
    return _connectionInfoFactory;
}

+(Class<ICEEndpointInfoFactory>) endpointInfoFactory
{
    return _endpointInfoFactory;
}

+(BOOL) registerFactories:(Class<ICELocalExceptionFactory>)localException
           connectionInfo:(Class<ICEConnectionInfoFactory>)connectionInfo
             endpointInfo:(Class<ICEEndpointInfoFactory>)endpointInfo
{
    _localExceptionFactory = localException;
    _connectionInfoFactory = connectionInfo;
    _endpointInfoFactory = endpointInfo;
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

    //
    // Collect InitializationData members.
    //
    Ice::InitializationData initData;
    if(properties)
    {
        initData.properties = [properties properties];
    }

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
                                 error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        return toNSString(Ice::identityToString(Ice::Identity{fromNSString(name), fromNSString(category)}));
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
@end
