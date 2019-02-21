// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "Connection.h"
#import "Endpoint.h"
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

//-(BOOL) setAdapter:(ObjectAdapterI*)oa error:(NSError**)error
//{
//
// }

//-(ObjectAdapterI*) getAdapter
// {

// }

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

-(BOOL) setHeartbeatCallback:(void (^)(ICEConnection*))callback error:(NSError**)error
{
    try
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
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
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

-(BOOL) setACM:(id _Nonnull)timeout close:(id _Nonnull)close heartbeat:(id _Nonnull)heartbeat error:(NSError**)error
{
    try
    {
        Ice::optional<int> opTimeout;
        Ice::optional<Ice::ACMClose> opClose;
        Ice::optional<Ice::ACMHeartbeat> opHeartbeat;

        if(timeout != [NSNull null])
        {
            NSNumber* value = timeout;
            assert(value);
            opTimeout = [value intValue];
        }

        if(close != [NSNull null])
        {
            NSNumber* value = close;
            assert(value);
            opClose = Ice::ACMClose([value unsignedCharValue]);
        }

        if(heartbeat != [NSNull null])
        {
            NSNumber* value = heartbeat;
            assert(value);
            opHeartbeat = Ice::ACMHeartbeat([value unsignedCharValue]);
        }

        _connection->setACM(opTimeout, opClose, opHeartbeat);
        return YES;
    }
    catch(const std::exception& ex)
    {
        *error = convertException(ex);
        return NO;
    }
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

    return [NSNull null];
}
@end
