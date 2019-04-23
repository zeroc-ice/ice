//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Connection.h"
#import "Endpoint.h"
#import "ObjectAdapter.h"
#import "ObjectPrx.h"
#import "IceUtil.h"
#import "Util.h"

@implementation ICEConnection

-(instancetype) initWithCppConnection:(std::shared_ptr<Ice::Connection>) connection
{
    self = [super initWithLocalObject:connection.get()];
    if(!self)
    {
        return nil;
    }
    _connection = connection;
    return self;
}

-(void) close:(uint8_t)mode
{
    _connection->close(Ice::ConnectionClose(mode));
}

-(id) createProxy:(NSString*)name category:(NSString*)category error:(NSError**)error
{
    try
    {
        auto cppPrx = _connection->createProxy(Ice::Identity{fromNSString(name), fromNSString(category)});
        ICEObjectPrx* prx = [[ICEObjectPrx alloc] initWithCppObjectPrx:cppPrx];
        return prx ? prx : [NSNull null];
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
        _connection->setAdapter(oa == nil ? nullptr : [oa objectAdapter]);
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
    auto cppAdapter = _connection->getAdapter();

    auto adapter = [ICEObjectAdapter fromLocalObject:cppAdapter.get()];
    return adapter ? adapter : [[ICEObjectAdapter alloc] initWithCppObjectAdapter:cppAdapter];
}

-(ICEEndpoint*) getEndpoint
{
    auto endpoint = _connection->getEndpoint();
    return [[ICEEndpoint alloc] initWithCppEndpoint:endpoint];
}

-(BOOL) flushBatchRequests:(uint8_t)compress error:(NSError**)error
{
    try
    {
        _connection->flushBatchRequests(Ice::CompressBatch(compress));
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
                          error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        _connection->flushBatchRequestsAsync(Ice::CompressBatch(compress),
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

-(BOOL) setCloseCallback:(void (^)(ICEConnection*))callback error:(NSError**)error
{
    try
    {
        if(!callback)
        {
            _connection->setCloseCallback(nullptr);
        }
        else
        {
            _connection->setCloseCallback([callback](auto connection)
            {
                ICEConnection* conn = [ICEConnection fromLocalObject:connection.get()];
                assert(conn);
                callback(conn);
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
        _connection->setHeartbeatCallback(nullptr);
    }
    else
    {
        _connection->setHeartbeatCallback([callback](auto connection)
        {
            ICEConnection* conn = [ICEConnection fromLocalObject:connection.get()];
            assert(conn);
            callback(conn);
        });
    }
}

-(BOOL) heartbeat:(NSError**)error
{
    try
    {
        _connection->heartbeat();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(BOOL) heartbeatAsync:(void (^)(NSError*))exception
                  sent:(void (^_Nullable)(bool))sent
                 error:(NSError* _Nullable * _Nullable)error
{
    try
    {
        _connection->heartbeatAsync([exception](std::exception_ptr e)
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
        NSNumber* value = heartbeat;
        assert(value);
        opHeartbeat = Ice::ACMHeartbeat([heartbeat unsignedCharValue]);
    }

    _connection->setACM(opTimeout, opClose, opHeartbeat);
}

-(void) getACM:(int32_t*)timeout close:(uint8_t*)close heartbeat:(uint8_t*)heartbeat
{
    auto acm = _connection->getACM();
    *timeout = acm.timeout;
    *close = static_cast<uint8_t>(acm.close);
    *heartbeat = static_cast<uint8_t>(acm.heartbeat);
}

-(NSString*) type
{
    return toNSString(_connection->type());
}

-(int32_t) timeout
{
    return _connection->timeout();
}

-(NSString*) toString
{
    return toNSString(_connection->toString());
}

-(id) getInfo:(NSError**)error
{
    try
    {
        auto info = _connection->getInfo();
        return [self createConnectionInfo:info];
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
        _connection->setBufferSize(rcvSize, sndSize);
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
        _connection->throwException();
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
}

-(id) createConnectionInfo:(std::shared_ptr<Ice::ConnectionInfo>)infoPtr
{
    id underlying = infoPtr->underlying ? [self createConnectionInfo:infoPtr->underlying] : [NSNull null];;

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
@end
