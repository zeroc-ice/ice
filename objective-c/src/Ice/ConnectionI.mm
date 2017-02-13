// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you  the terms described in the
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

#define CONNECTION dynamic_cast<Ice::Connection*>(static_cast<IceUtil::Shared*>(cxxObject_))

namespace
{

std::vector<Class>* connectionInfoClasses = 0;

}

namespace IceObjC
{

void
registerConnectionInfoClass(Class cl)
{
    if(!connectionInfoClasses)
    {
        connectionInfoClasses = new std::vector<Class>();
    }
    connectionInfoClasses->push_back(cl);
}

}

@implementation ICEConnectionInfo (ICEInternal)

+(id) checkedConnectionInfoWithConnectionInfo:(Ice::ConnectionInfo*)connectionInfo
{
    assert(false);
    return nil;
}

+(id) connectionInfoWithConnectionInfo:(NSValue*)v
{
    Ice::ConnectionInfo* info = dynamic_cast<Ice::ConnectionInfo*>(reinterpret_cast<IceUtil::Shared*>(v.pointerValue));
    if(!info)
    {
        return nil;
    }

    Ice::UDPConnectionInfoPtr udpInfo = Ice::UDPConnectionInfoPtr::dynamicCast(info);
    if(udpInfo)
    {
        return [[ICEUDPConnectionInfo alloc] initWithUDPConnectionInfo:udpInfo.get()];
    }

    Ice::WSConnectionInfoPtr wsInfo = Ice::WSConnectionInfoPtr::dynamicCast(info);
    if(wsInfo)
    {
        return [[ICEWSConnectionInfo alloc] initWithWSConnectionInfo:wsInfo.get()];
    }

    Ice::TCPConnectionInfoPtr tcpInfo = Ice::TCPConnectionInfoPtr::dynamicCast(info);
    if(tcpInfo)
    {
        return [[ICETCPConnectionInfo alloc] initWithTCPConnectionInfo:tcpInfo.get()];
    }

    if(connectionInfoClasses)
    {
        for(std::vector<Class>::const_iterator p = connectionInfoClasses->begin(); p != connectionInfoClasses->end();
            ++p)
        {
            ICEConnectionInfo* r = [*p checkedConnectionInfoWithConnectionInfo:info];
            if(r)
            {
                return r;
            }
        }
    }

    Ice::IPConnectionInfoPtr ipInfo = Ice::IPConnectionInfoPtr::dynamicCast(info);
    if(ipInfo)
    {
        return [[ICEIPConnectionInfo alloc] initWithIPConnectionInfo:ipInfo.get()];
    }

    return [[ICEConnectionInfo alloc] initWithConnectionInfo:info];
}

-(id) initWithConnectionInfo:(Ice::ConnectionInfo*)connectionInfo;
{
    self = [super initWithCxxObject:connectionInfo];
    if(self != nil)
    {
        self->underlying = [ICEConnectionInfo localObjectWithCxxObjectNoAutoRelease:connectionInfo->underlying.get()
                                                            allocator:@selector(connectionInfoWithConnectionInfo:)];
        self->incoming = connectionInfo->incoming;
        self->adapterName = [[NSString alloc] initWithUTF8String:connectionInfo->adapterName.c_str()];
        self->connectionId = [[NSString alloc] initWithUTF8String:connectionInfo->connectionId.c_str()];
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
    if(self)
    {
        self->rcvSize = tcpConnectionInfo->rcvSize;
        self->sndSize = tcpConnectionInfo->sndSize;
    }
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
        self->rcvSize = udpConnectionInfo->rcvSize;
        self->sndSize = udpConnectionInfo->sndSize;
    }
    return self;
}

@end

@implementation ICEWSConnectionInfo (ICEInternal)
-(id) initWithWSConnectionInfo:(Ice::WSConnectionInfo*)wsConnectionInfo
{
    self = [super initWithConnectionInfo:wsConnectionInfo];
    if(self)
    {
        self->headers = toNSDictionary(wsConnectionInfo->headers);
    }
    return self;
}
@end

namespace
{

class CloseCallbackI : public Ice::CloseCallback
{
public:

    CloseCallbackI(id<ICEConnection> connection, ICECloseCallback callback) :
        _connection(connection), _callback(Block_copy(callback))
    {
        [_connection retain];
    }

    ~CloseCallbackI()
    {
        [_connection release];
        [_callback release];
    }

    void
    closed(const Ice::ConnectionPtr& connection)
    {
        NSException* ex = nil;
        @autoreleasepool
        {
            @try
            {
                _callback(_connection);
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
    ICECloseCallback _callback;
};

class HeartbeatCallbackI : public Ice::HeartbeatCallback
{
public:

    HeartbeatCallbackI(id<ICEConnection> connection, ICEHeartbeatCallback callback) :
        _connection(connection), _callback(Block_copy(callback))
    {
        [_connection retain];
    }

    ~HeartbeatCallbackI()
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
                _callback(_connection);
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
    ICEHeartbeatCallback _callback;
};

}

@implementation ICEConnection
-(void) close:(ICEConnectionClose)mode
{
    NSException* nsex = nil;
    try
    {
        CONNECTION->close((Ice::ConnectionClose)mode);
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
        return [ICEObjectPrx iceObjectPrxWithObjectPrx:CONNECTION->createProxy([identity identity])];
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
-(void) flushBatchRequests:(ICECompressBatch)compress
{
    NSException* nsex = nil;
    try
    {
        CONNECTION->flushBatchRequests((Ice::CompressBatch)compress);
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
-(id<ICEAsyncResult>) begin_flushBatchRequests:(ICECompressBatch)compress
{
    return beginCppCall(^(Ice::AsyncResultPtr& result)
                        {
                            result = CONNECTION->begin_flushBatchRequests((Ice::CompressBatch)compress);
                        });
}
-(id<ICEAsyncResult>) begin_flushBatchRequests:(ICECompressBatch)compress exception:(void(^)(ICEException*))exception
{
    return [self begin_flushBatchRequests:compress exception:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_flushBatchRequests:(ICECompressBatch)compress
                                     exception:(void(^)(ICEException*))exception
                                          sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::CallbackPtr& cb)
                        {
                            result = CONNECTION->begin_flushBatchRequests((Ice::CompressBatch)compress, cb);
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
-(void) setCloseCallback:(ICECloseCallback)callback;
{
    CONNECTION->setCloseCallback(new CloseCallbackI(self, callback));
}
-(void) setHeartbeatCallback:(ICEHeartbeatCallback)callback
{
    CONNECTION->setHeartbeatCallback(new HeartbeatCallbackI(self, callback));
}
-(void) heartbeat
{
    NSException* nsex = nil;
    try
    {
        CONNECTION->heartbeat();
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
-(id<ICEAsyncResult>) begin_heartbeat
{
    return beginCppCall(^(Ice::AsyncResultPtr& result)
                        {
                            result = CONNECTION->begin_heartbeat();
                        });
}
-(id<ICEAsyncResult>) begin_heartbeat:(void(^)(ICEException*))exception
{
    return [self begin_heartbeat:exception sent:nil];
}
-(id<ICEAsyncResult>) begin_heartbeat:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent
{
    return beginCppCall(^(Ice::AsyncResultPtr& result, const Ice::CallbackPtr& cb)
                        {
                            result = CONNECTION->begin_heartbeat(cb);
                        },
                        ^(const Ice::AsyncResultPtr& result)
                        {
                            CONNECTION->end_heartbeat(result);
                        },
                        exception, sent);
}
-(void) end_heartbeat:(id<ICEAsyncResult>)result
{
    endCppCall(^(const Ice::AsyncResultPtr& r)
               {
                   CONNECTION->end_heartbeat(r);
               }, result);
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
        return [ICEConnectionInfo localObjectWithCxxObject:CONNECTION->getInfo().get()
                                                 allocator:@selector(connectionInfoWithConnectionInfo:)];
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

-(void) throwException
{
    NSException* nsex = nil;
    try
    {
        CONNECTION->throwException();
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
