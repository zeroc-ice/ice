// Copyright (c) ZeroC, Inc.

#import "include/Endpoint.h"
#import "Convert.h"
#import "include/IceUtil.h"

@implementation ICEEndpoint

- (std::shared_ptr<Ice::Endpoint>)endpoint
{
    return std::static_pointer_cast<Ice::Endpoint>(self.cppObject);
}

- (NSString*)toString
{
    return toNSString(self.endpoint->toString());
}

- (id)getInfo
{
    auto info = self.endpoint->getInfo();
    return [ICEEndpoint createEndpointInfo:info];
}

- (bool)isEqual:(ICEEndpoint*)other
{
    return *self.endpoint == *other.endpoint;
}

+ (id)createEndpointInfo:(std::shared_ptr<Ice::EndpointInfo>)infoPtr
{
    id underlying = infoPtr->underlying ? [self createEndpointInfo:infoPtr->underlying] : [NSNull null];

    Class<ICEEndpointInfoFactory> factory = [ICEUtil endpointInfoFactory];

    //
    // Don't use info->type() to determine the type of the EndpointInfo object. When an endpoint is the
    // underlying endpoint of a parent, the child's value for type() is the same as its parent. We have
    // to use type casts instead.
    //

    auto tcpInfo = std::dynamic_pointer_cast<Ice::TCPEndpointInfo>(infoPtr);
    if (tcpInfo)
    {
        return [factory createTCPEndpointInfo:tcpInfo->timeout
                                     compress:tcpInfo->compress
                                         host:toNSString(tcpInfo->host)
                                         port:tcpInfo->port
                                sourceAddress:toNSString(tcpInfo->sourceAddress)
                                         type:tcpInfo->type()
                                       secure:tcpInfo->secure()];
    }

    if (std::dynamic_pointer_cast<Ice::SSL::EndpointInfo>(infoPtr))
    {
        return [factory createSSLEndpointInfo:underlying];
    }

    auto udpInfo = std::dynamic_pointer_cast<Ice::UDPEndpointInfo>(infoPtr);
    if (udpInfo)
    {
        return [factory createUDPEndpointInfo:udpInfo->compress
                                         host:toNSString(udpInfo->host)
                                         port:udpInfo->port
                                sourceAddress:toNSString(udpInfo->sourceAddress)
                               mcastInterface:toNSString(udpInfo->mcastInterface)
                                     mcastTtl:udpInfo->mcastTtl];
    }

    auto wsInfo = std::dynamic_pointer_cast<Ice::WSEndpointInfo>(infoPtr);
    if (wsInfo)
    {
        return [factory createWSEndpointInfo:underlying resource:toNSString(wsInfo->resource)];
    }

    auto iapInfo = std::dynamic_pointer_cast<Ice::IAPEndpointInfo>(infoPtr);
    if (iapInfo)
    {
        return [factory createIAPEndpointInfo:iapInfo->timeout
                                     compress:iapInfo->compress
                                 manufacturer:toNSString(iapInfo->manufacturer)
                                  modelNumber:toNSString(iapInfo->modelNumber)
                                         name:toNSString(iapInfo->name)
                                     protocol:toNSString(iapInfo->protocol)
                                         type:iapInfo->type()
                                       secure:iapInfo->secure()];
    }

    auto opaqueInfo = std::dynamic_pointer_cast<Ice::OpaqueEndpointInfo>(infoPtr);
    if (opaqueInfo)
    {
        NSData* rawBytes = [[NSData alloc] initWithBytes:opaqueInfo->rawBytes.data()
                                                  length:opaqueInfo->rawBytes.size()];

        return [factory createOpaqueEndpointInfo:opaqueInfo->type()
                                   encodingMajor:opaqueInfo->rawEncoding.major
                                   encodingMinor:opaqueInfo->rawEncoding.minor
                                        rawBytes:rawBytes];
    }

    // TODO: we should never return null from this factory function.
    return [NSNull null];
}
@end
