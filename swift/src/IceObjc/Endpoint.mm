//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Endpoint.h"
#import "IceUtil.h"
#import "Convert.h"

@implementation ICEEndpointInfo

-(std::shared_ptr<Ice::EndpointInfo>) info
{
    return std::static_pointer_cast<Ice::EndpointInfo>(self.cppObject);
}

-(int16_t) getType
{
    return self.info->type();
}

-(BOOL) getDatagram
{
    return self.info->datagram();
}

-(BOOL) getSecure
{
    return self.info->secure();
}

@end

@implementation ICEEndpoint

-(std::shared_ptr<Ice::Endpoint>) endpoint
{
    return std::static_pointer_cast<Ice::Endpoint>(self.cppObject);
}

-(NSString*) toString
{
    return toNSString(self.endpoint->toString());
}

-(id) getInfo
{
    auto info = self.endpoint->getInfo();
    return [ICEEndpoint createEndpointInfo:info];
}

-(bool) isEqual:(ICEEndpoint*)other
{
    return Ice::targetEqualTo(self.endpoint, other.endpoint);
}

+(id) createEndpointInfo:(std::shared_ptr<Ice::EndpointInfo>)infoPtr
{
    ICEEndpointInfo* handle = [ICEEndpointInfo getHandle:infoPtr];
    id underlying = infoPtr->underlying ? [self createEndpointInfo:infoPtr->underlying] : [NSNull null];

    Class<ICEEndpointInfoFactory> factory = [ICEUtil endpointInfoFactory];

    //
    // Don't use info->type() to determine the type of the EndpointInfo object. When an endpoint is the
    // underlying endpoint of a parent, the child's value for type() is the same as its parent. We have
    // to use type casts instead.
    //
    auto ipInfo = std::dynamic_pointer_cast<Ice::IPEndpointInfo>(infoPtr);

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
                                      timeout:infoPtr->timeout
                                     compress:infoPtr->compress
                                     resource:toNSString(wsInfo->resource)];
    }

    if(std::dynamic_pointer_cast<IceSSL::EndpointInfo>(infoPtr))
    {
        return [factory createSSLEndpointInfo:handle
                                   underlying:underlying
                                      timeout:infoPtr->timeout
                                     compress:infoPtr->compress];
    }

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
