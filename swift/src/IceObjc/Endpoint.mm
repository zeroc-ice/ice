// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcEndpoint.h"
#import "IceObjcIceUtil.h"
#import "IceObjcUtil.h"

@implementation ICEEndpointInfo

-(instancetype) initWithCppEndpointInfo:(std::shared_ptr<Ice::EndpointInfo>)info
{
    self = [super initWithLocalObject:info.get()];
    if(!self)
    {
        return nil;

    }
    self->_info = info;
    return self;
}

-(int16_t) getType
{
    return _info->type();
}

-(BOOL) getDatagram
{
    return _info->datagram();
}

-(BOOL) getSecure
{
    return _info->secure();
}

@end

@implementation ICEEndpoint

-(instancetype) initWithCppEndpoint:(std::shared_ptr<Ice::Endpoint>)endpoint;
{
    self = [super initWithLocalObject:endpoint.get()];
    if(!self)
    {
        return nil;
    }
    self->_endpoint = endpoint;
    return self;
}

-(NSString*) toString
{
    return toNSString(_endpoint->toString());
}

-(id) getInfo
{
    auto info = _endpoint->getInfo();
    return [ICEEndpoint createEndpointInfo:info];
}

+(id) createEndpointInfo:(std::shared_ptr<Ice::EndpointInfo>)infoPtr
{
    ICEEndpointInfo* handle = [[ICEEndpointInfo alloc] initWithCppEndpointInfo:infoPtr];
    id underlying = infoPtr->underlying ? [self createEndpointInfo:infoPtr->underlying] : [NSNull null];

    Class<ICEEndpointInfoFactory> factory = [ICEUtil endpointInfoFactory];

    //
    // Don't use info->type() to determine the type of the EndpointInfo object. When an endpoint is the
    // underlying endpoint of a parent, the child's value for type() is the same as its parent. We have
    // to use type casts instead.
    //
    auto ipInfo = std::dynamic_pointer_cast<Ice::IPEndpointInfo>(infoPtr);
    if(ipInfo)
    {
        return [factory createIPEndpointInfo:handle
                           underlying:underlying
                              timeout:ipInfo->timeout
                             compress:ipInfo->compress
                                 host:toNSString(ipInfo->host)
                                 port:ipInfo->port
                        sourceAddress:toNSString(ipInfo->sourceAddress)];
    }

    auto opaqueInfo = std::dynamic_pointer_cast<Ice::OpaqueEndpointInfo>(infoPtr);
    if(opaqueInfo)
    {
        NSMutableArray<NSNumber*>* rawBytes = [[NSMutableArray alloc] initWithCapacity:opaqueInfo->rawBytes.size()];
        for(Ice::Byte b: opaqueInfo->rawBytes)
        {
            [rawBytes addObject: [NSNumber numberWithUnsignedChar:b]];
        }

        return [factory createOpaqueEndpointInfo:handle
                                      underlying:underlying
                                         timeout:opaqueInfo->timeout
                                        compress:opaqueInfo->compress
                                   encodingMajor:opaqueInfo->rawEncoding.major
                                   encodingMinor:opaqueInfo->rawEncoding.minor
                                        rawBytes:rawBytes];
    }

    auto udpInfo = std::dynamic_pointer_cast<Ice::UDPEndpointInfo>(infoPtr);
    if(udpInfo)
    {
        return [factory createUDPEndpointInfo:handle
                                   underlying:underlying
                                      timeout:udpInfo->timeout
                                     compress:udpInfo->compress
                                         host:toNSString(udpInfo->host)
                                         port:udpInfo->port
                                sourceAddress:toNSString(udpInfo->sourceAddress)
                               mcastInterface:toNSString(udpInfo->mcastInterface)
                                     mcastTtl:udpInfo->mcastTtl];
    }

    if(std::dynamic_pointer_cast<Ice::TCPEndpointInfo>(infoPtr))
    {
        return [factory createTCPEndpointInfo:handle
                                   underlying:underlying
                                      timeout:ipInfo->timeout
                                     compress:ipInfo->compress
                                         host:toNSString(ipInfo->host)
                                         port:ipInfo->port
                                sourceAddress:toNSString(ipInfo->sourceAddress)];
    }

    auto wsInfo = std::dynamic_pointer_cast<Ice::WSEndpointInfo>(infoPtr);
    if(wsInfo)
    {
        return [factory createWSEndpointInfo:handle
                                   underlying:underlying
                                      timeout:udpInfo->timeout
                                     compress:udpInfo->compress
                                     resource:toNSString(wsInfo->resource)];
    }

    if(std::dynamic_pointer_cast<IceSSL::EndpointInfo>(infoPtr))
    {
        return [factory createSSLEndpointInfo:handle
                                   underlying:underlying
                                      timeout:infoPtr->timeout
                                     compress:infoPtr->compress];
    }

#if TARGET_OS_IPHONE

    auto iapInfo = std::dynamic_pointer_cast<IceIAP::EndpointInfo>(infoPtr);
    if(iapInfo)
    {
        return [factory createIAPEndpointInfo:handle
                                   underlying:underlying
                                      timeout:iapInfo->timeout
                                     compress:iapInfo->compress
                                 manufacturer:toNSString(iapInfo->manufacturer)
                                  modelNumber:toNSString(iapInfo->modelNumber)
                                         name:toNSString(iapInfo->name)
                                     protocol:toNSString(iapInfo->protocol)];
    }

#endif

    return [NSNull null];
}
@end
