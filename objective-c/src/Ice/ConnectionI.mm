// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <ConnectionI.h>
#import <EndpointI.h>
#import <IdentityI.h>
#import <ObjectAdapterI.h>
#import <ProxyI.h>
#import <Util.h>
#import <LocalObjectI.h>

#import <objc/Ice/LocalException.h>

#import <objc/runtime.h>

@implementation ICEConnectionInfo (ICEInternal)

-(id) initWithConnectionInfo:(Ice::ConnectionInfo*)connectionInfo;
{
    self = [super initWithCxxObject:connectionInfo];
    if(self != nil)
    {
        self->incoming = connectionInfo->incoming;
        self->adapterName = [[NSString alloc] initWithUTF8String:connectionInfo->adapterName.c_str()];
        self->connectionId = [[NSString alloc] initWithUTF8String:connectionInfo->connectionId.c_str()];
        self->rcvSize = connectionInfo->rcvSize;
        self->sndSize = connectionInfo->sndSize;
    }
    return self;
}

@end

@implementation ICEIPConnectionInfo (ICEInternal)

-(id) initWithIPConnectionInfo:(Ice::IPConnectionInfo*)ipConnectionInfo
{
    self = [super initWithConnectionInfo:ipConnectionInfo];
    if(self)
    {
        self->localAddress = [[NSString alloc] initWithUTF8String:ipConnectionInfo->localAddress.c_str()];
        self->localPort = ipConnectionInfo->localPort;
        self->remoteAddress = [[NSString alloc] initWithUTF8String:ipConnectionInfo->remoteAddress.c_str()];
        self->remotePort = ipConnectionInfo->remotePort;
    }
    return self;
}

@end

@implementation ICETCPConnectionInfo (ICEInternal)
-(id) initWithTCPConnectionInfo:(Ice::TCPConnectionInfo*)tcpConnectionInfo
{
    self = [super initWithIPConnectionInfo:tcpConnectionInfo];
    return self;
}
@end

@implementation ICEUDPConnectionInfo (ICEInternal)

-(id) initWithUDPConnectionInfo:(Ice::UDPConnectionInfo*)udpConnectionInfo
{
    self = [super initWithIPConnectionInfo:udpConnectionInfo];
    if(self)
    {
        self->mcastAddress = [[NSString alloc] initWithUTF8String:udpConnectionInfo->mcastAddress.c_str()];
        self->mcastPort = udpConnectionInfo->mcastPort;
    }
    return self;
}

@end

@implementation ICEWSConnectionInfo (ICEInternal)
-(id) initWithWSConnectionInfo:(Ice::WSConnectionInfo*)wsConnectionInfo
{
    self = [super initWithIPConnectionInfo:wsConnectionInfo];
    if(self)
    {
        self->headers = toNSDictionary(wsConnectionInfo->headers);
    }
    return self;
}
@end

namespace
{

class ConnectionCallbackI : public Ice::ConnectionCallback
{
public:

    ConnectionCallbackI(id<ICEConnection> connection, id<ICEConnectionCallback> callback) :
        _connection(connection), _callback(callback)
    {
        [_connection retain];
        [_callback retain];
    }

    ~ConnectionCallbackI()
    {
        [_connection release];
        [_callback release];
    }

    void
    heartbeat(const Ice::ConnectionPtr& connection)
    {
        NSException* ex = nil;
        @autoreleasepool
        {
            @try
            {
                [_callback heartbeat:_connection];
            }
            @catch(id e)
            {
                ex = [e retain];
            }
        }
        if(ex != nil)
        {
            rethrowCxxException(ex, true); // True = release the exception.
        }
    }

    void
    closed(const Ice::ConnectionPtr& connection)
    {
        NSException* ex = nil;
        @autoreleasepool
        {
            @try
            {
                [_callback closed:_connection];
            }
            @catch(id e)
            {
                ex = [e retain];
            }
        }
        if(ex != nil)
        {
            rethrowCxxException(ex, true); // True = release the exception.
        }
    }

private:

    id<ICEConnection> _connection;
    id<ICEConnectionCallback> _callback;
};

}

#define CONNECTION dynamic_cast<Ice::Connection*>(static_cast<IceUtil::Shared*>(cxxObject_))

@implementation ICEConnection
-(void) close:(BOOL)force
{
    NSException* nsex = nil;
    try
    {
        CONNECTION->close(force);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}
-(id<ICEObjectPrx>) createProxy:(ICEIdentity*)identity
{
    NSException* nsex = nil;
    try
    {
        return [ICEObjectPrx objectPrxWithObjectPrx__:CONNECTION->createProxy([identity identity])];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(void) setAdapter:(id<ICEObjectAdapter>)adapter
{
    NSException* nsex = nil;
    try
    {
        CONNECTION->setAdapter([(ICEObjectAdapter*)adapter adapter]);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}
-(id<ICEObjectAdapter>) getAdapter
{
    NSException* nsex = nil;
    try
    {
        return [ICEObjectAdapter localObjectWithCxxObject:CONNECTION->getAdapter().get()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    @throw nsex;
    return nil; // Keep the compiler happy.
}
-(void) flushBatchRequests
{
    NSException* nsex = nil;
    try
    {
        CONNECTION->flushBatchRequests();
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}
-(id<ICEAsyncResult>) begin_flushBatchRequests
{
    return beginCppCall(^(Ice::AsyncResultPtr& result)
                        {
                            result = CONNECTION->begin_flushBatchRequests();
                        });
}
-(id<ICEAsyncResult>) begin_flushBatchRequests:(void(^)(ICEException*))exception
{
    return [self begin_flushBatchRequests:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_flushBatchRequests:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::CallbackPtr& cb)
                        {
                            result = CONNECTION->begin_flushBatchRequests(cb);
                        },
                        ^(const Ice::AsyncResultPtr& result) {
                            CONNECTION->end_flushBatchRequests(result);
                        },
                        exception, sent);
}
-(void) end_flushBatchRequests:(id<ICEAsyncResult>)result
{
    endCppCall(^(const Ice::AsyncResultPtr& r)
               {
                   CONNECTION->end_flushBatchRequests(r);
               }, result);
}
-(void) setCallback:(id<ICEConnectionCallback>)callback
{
    CONNECTION->setCallback(new ConnectionCallbackI(self, callback));
}
-(void) setACM:(id)timeout close:(id)close heartbeat:(id)heartbeat
{
    IceUtil::Optional<int> to;
    ICEInt intValue;
    if([ICEOptionalGetter getInt:timeout value:&intValue])
    {
        to = intValue;
    }
    IceUtil::Optional<Ice::ACMClose> c;
    if([ICEOptionalGetter getInt:close value:&intValue])
    {
        c = (Ice::ACMClose)intValue;
    }
    IceUtil::Optional<Ice::ACMHeartbeat> hb;
    if([ICEOptionalGetter getInt:heartbeat value:&intValue])
    {
        hb = (Ice::ACMHeartbeat)intValue;
    }

    CONNECTION->setACM(to, c, hb);
}
-(ICEACM*) getACM
{
    Ice::ACM acm = CONNECTION->getACM();
    return [ICEACM acm:acm.timeout close:(ICEACMClose)acm.close heartbeat:(ICEACMHeartbeat)acm.heartbeat];
}
-(NSMutableString*) type
{
    return [toNSMutableString(CONNECTION->type()) autorelease];
}
-(ICEInt) timeout
{
    return CONNECTION->timeout();
}
-(NSMutableString*) toString
{
    return [toNSMutableString(CONNECTION->toString()) autorelease];
}
-(NSString*) description
{
    return [toNSString(CONNECTION->toString()) autorelease];
}

-(ICEConnectionInfo*) getInfo
{
    NSException* nsex = nil;
    try
    {
        Ice::ConnectionInfoPtr info = CONNECTION->getInfo();
        if(!info)
        {
            return nil;
        }

        Ice::UDPConnectionInfoPtr udpInfo = Ice::UDPConnectionInfoPtr::dynamicCast(info);
        if(udpInfo)
        {
            return [[[ICEUDPConnectionInfo alloc] initWithUDPConnectionInfo:udpInfo.get()] autorelease];
        }

        Ice::WSConnectionInfoPtr wsInfo = Ice::WSConnectionInfoPtr::dynamicCast(info);
        if(wsInfo)
        {
            return [[[ICEWSConnectionInfo alloc] initWithWSConnectionInfo:wsInfo.get()] autorelease];
        }

        Ice::TCPConnectionInfoPtr tcpInfo = Ice::TCPConnectionInfoPtr::dynamicCast(info);
        if(tcpInfo)
        {
            return [[[ICETCPConnectionInfo alloc] initWithTCPConnectionInfo:tcpInfo.get()] autorelease];
        }

        std::ostringstream os;
        os << "connectionInfoWithType_" << CONNECTION->type() << ":";
        SEL selector = sel_registerName(os.str().c_str());
        if([ICEConnectionInfo respondsToSelector:selector])
        {
            IceUtil::Shared* shared = info.get();
            return [ICEConnectionInfo performSelector:selector withObject:[NSValue valueWithPointer:shared]];
        }

        Ice::IPConnectionInfoPtr ipInfo = Ice::IPConnectionInfoPtr::dynamicCast(info);
        if(ipInfo)
        {
            return [[[ICEIPConnectionInfo alloc] initWithIPConnectionInfo:ipInfo.get()] autorelease];
        }

        return [[[ICEConnectionInfo alloc] initWithConnectionInfo:info.get()] autorelease];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return nil;
}

-(void) setBufferSize:(int)rcvSize sndSize:(int)sndSize
{
    NSException* nsex = nil;
    try
    {
        CONNECTION->setBufferSize(rcvSize, sndSize);
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
}

-(ICEEndpoint*) getEndpoint
{
    NSException* nsex = nil;
    try
    {
        return [ICEEndpoint localObjectWithCxxObject:CONNECTION->getEndpoint().get()];
    }
    catch(const std::exception& ex)
    {
        nsex = toObjCException(ex);
    }
    if(nsex != nil)
    {
        @throw nsex;
    }
    return nil;
}
@end
