//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Connection.h"
#import "Endpoint.h"
#import "ObjectAdapter.h"
#import "ObjectPrx.h"
#import "IceUtil.h"
#import "Convert.h"

@implementation ICEConnection

-(std::shared_ptr<Ice::Connection>) connection
{
    return std::static_pointer_cast<Ice::Connection>(self.cppObject);
}

-(void) close:(uint8_t)mode
{
    self.connection->close(Ice::ConnectionClose(mode));
}

-(nullable ICEObjectPrx*) createProxy:(NSString*)name category:(NSString*)category error:(NSError**)error
{
    try
    {
        auto cppPrx = self.connection->createProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        return [[ICEObjectPrx alloc] initWithCppObjectPrx:cppPrx];
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(BOOL) setAdapter:(ICEObjectAdapter* _Nullable)oa error:(NSError* _Nullable * _Nullable)error;
{
    try
    {
        self.connection->setAdapter(oa == nil ? nullptr : [oa objectAdapter]);
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(nullable ICEObjectAdapter*) getAdapter
{
    auto cppAdapter = self.connection->getAdapter();

    return [ICEObjectAdapter getHandle:cppAdapter];
}

-(ICEEndpoint*) getEndpoint
{
    auto endpoint = self.connection->getEndpoint();
    return [ICEEndpoint getHandle:endpoint];
}

-(BOOL) flushBatchRequests:(uint8_t)compress error:(NSError**)error
{
    try
    {
        self.connection->flushBatchRequests(Ice::CompressBatch(compress));
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
        self.connection->flushBatchRequestsAsync(Ice::CompressBatch(compress),
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

-(BOOL) setCloseCallback:(void (^)(ICEConnection*))callback error:(NSError**)error
{
    try
    {
        if(!callback)
        {
            self.connection->setCloseCallback(nullptr);
        }
        else
        {
            self.connection->setCloseCallback([callback](auto connection)
            {
                ICEConnection* conn = [ICEConnection getHandle:connection];
                assert(conn);
                @autoreleasepool
                {
                    callback(conn);
                }
            });
        }
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(void) setHeartbeatCallback:(void (^)(ICEConnection*))callback
{
    if(!callback)
    {
        self.connection->setHeartbeatCallback(nullptr);
    }
    else
    {
        self.connection->setHeartbeatCallback([callback](auto connection)
        {
            ICEConnection* conn = [ICEConnection getHandle:connection];
            assert(conn);
            @autoreleasepool
            {
                callback(conn);
            }
        });
    }
}

-(BOOL) heartbeat:(NSError**)error
{
    try
    {
        self.connection->heartbeat();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(void) heartbeatAsync:(void (^)(NSError*))exception
                  sent:(void (^_Nullable)(bool))sent
{
    try
    {
        self.connection->heartbeatAsync([exception](std::exception_ptr e)
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

-(void) setACM:(NSNumber* _Nullable)timeout close:(NSNumber* _Nullable)close heartbeat:(NSNumber* _Nullable)heartbeat
{
    Ice::optional<int> opTimeout;
    Ice::optional<Ice::ACMClose> opClose;
    Ice::optional<Ice::ACMHeartbeat> opHeartbeat;

    if(timeout != nil)
    {
        opTimeout = [timeout intValue];
    }

    if(close != nil)
    {
        opClose = Ice::ACMClose([close unsignedCharValue]);
    }

    if(heartbeat != nil)
    {
        opHeartbeat = Ice::ACMHeartbeat([heartbeat unsignedCharValue]);
    }

    self.connection->setACM(opTimeout, opClose, opHeartbeat);
}

-(void) getACM:(int32_t*)timeout close:(uint8_t*)close heartbeat:(uint8_t*)heartbeat
{
    auto acm = self.connection->getACM();
    *timeout = acm.timeout;
    *close = static_cast<uint8_t>(acm.close);
    *heartbeat = static_cast<uint8_t>(acm.heartbeat);
}

-(NSString*) type
{
    return toNSString(self.connection->type());
}

-(int32_t) timeout
{
    return self.connection->timeout();
}

-(NSString*) toString
{
    return toNSString(self.connection->toString());
}

-(id) getInfo:(NSError**)error
{
    try
    {
        auto info = self.connection->getInfo();
        return createConnectionInfo(info);
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return nil;
    }
}

-(BOOL) setBufferSize:(int32_t)rcvSize sndSize:(int32_t)sndSize error:(NSError**)error
{
    try
    {
        self.connection->setBufferSize(rcvSize, sndSize);
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(BOOL) throwException:(NSError**)error
{
    try
    {
        self.connection->throwException();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}
@end

id createConnectionInfo(std::shared_ptr<Ice::ConnectionInfo> infoPtr)
{
    id underlying = infoPtr->underlying ? createConnectionInfo(infoPtr->underlying) : [NSNull null];

    Class<ICEConnectionInfoFactory> factory = [ICEUtil connectionInfoFactory];

    auto ipInfo = std::dynamic_pointer_cast<Ice::IPConnectionInfo>(infoPtr);
    if(ipInfo)
    {
        [factory createIPConnectionInfo:underlying
                               incoming:infoPtr->incoming
                            adapterName:toNSString(infoPtr->adapterName)
                           connectionId:toNSString(ipInfo->connectionId)
                           localAddress:toNSString(ipInfo->localAddress)
                              localPort:ipInfo->localPort
                          remoteAddress:toNSString(ipInfo->remoteAddress)
                             remotePort:ipInfo->remotePort];
    }

    auto tcpInfo = std::dynamic_pointer_cast<Ice::TCPConnectionInfo>(infoPtr);
    if(tcpInfo)
    {
        return [factory createTCPConnectionInfo:underlying
                                       incoming:tcpInfo->incoming
                                    adapterName:toNSString(tcpInfo->adapterName)
                                   connectionId:toNSString(tcpInfo->connectionId)
                                   localAddress:toNSString(tcpInfo->localAddress)
                                      localPort:tcpInfo->localPort
                                  remoteAddress:toNSString(tcpInfo->remoteAddress)
                                     remotePort:tcpInfo->remotePort
                                        rcvSize:tcpInfo->rcvSize
                                        sndSize:tcpInfo->sndSize];
    }

    auto udpInfo = std::dynamic_pointer_cast<Ice::UDPConnectionInfo>(infoPtr);
    if(udpInfo)
    {
        return [factory createUDPConnectionInfo:underlying
                                       incoming:infoPtr->incoming
                                    adapterName:toNSString(infoPtr->adapterName)
                                   connectionId:toNSString(ipInfo->connectionId)
                                   localAddress:toNSString(ipInfo->localAddress)
                                      localPort:ipInfo->localPort
                                  remoteAddress:toNSString(ipInfo->remoteAddress)
                                     remotePort:ipInfo->remotePort
                                   mcastAddress:toNSString(udpInfo->mcastAddress)
                                      mcastPort:udpInfo->mcastPort
                                        rcvSize:udpInfo->rcvSize
                                        sndSize:udpInfo->sndSize];
    }

    auto wsInfo = std::dynamic_pointer_cast<Ice::WSConnectionInfo>(infoPtr);
    if(wsInfo)
    {
        return [factory createWSConnectionInfo:underlying
                                      incoming:wsInfo->incoming
                                   adapterName:toNSString(wsInfo->adapterName)
                                  connectionId:toNSString(wsInfo->adapterName)
                                       headers:toNSDictionary(wsInfo->headers)];
    }

    auto sslInfo = std::dynamic_pointer_cast<IceSSL::ConnectionInfo>(infoPtr);
    if(sslInfo)
    {
        return [factory createSSLConnectionInfo:underlying
                                       incoming:sslInfo->incoming
                                    adapterName:toNSString(sslInfo->adapterName)
                                   connectionId:toNSString(sslInfo->connectionId)
                                         cipher:toNSString(sslInfo->cipher)
                                          certs:toNSArray(sslInfo->certs)
                                       verified:sslInfo->verified];
    }

#if TARGET_OS_IPHONE

    auto iapInfo = std::dynamic_pointer_cast<IceIAP::ConnectionInfo>(infoPtr);
    if(iapInfo)
    {
        return [factory createIAPConnectionInfo:underlying
                                       incoming:iapInfo->incoming
                                    adapterName:toNSString(iapInfo->adapterName)
                                   connectionId:toNSString(iapInfo->connectionId)
                                           name:toNSString(iapInfo->name)
                                   manufacturer:toNSString(iapInfo->manufacturer)
                                    modelNumber:toNSString(iapInfo->modelNumber)
                               firmwareRevision:toNSString(iapInfo->firmwareRevision)
                               hardwareRevision:toNSString(iapInfo->hardwareRevision)
                                       protocol:toNSString(iapInfo->protocol)];
    }

#endif

    return [NSNull null];
}
