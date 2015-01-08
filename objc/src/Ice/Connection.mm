// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <ConnectionI.h>
#import <EndpointI.h>
#import <IdentityI.h>
#import <ObjectAdapterI.h>
#import <ProxyI.h>
#import <Util.h>

@implementation ICEConnectionInfo

@synthesize incoming;
@synthesize adapterName;
@synthesize connectionId;

-(id) initWithConnectionInfo:(Ice::ConnectionInfo*)connectionInfo;
{
    self = [super init];
    if(self != nil)
    {
        self->incoming = connectionInfo->incoming;
        self->adapterName = [[NSString alloc] initWithUTF8String:connectionInfo->adapterName.c_str()];
        self->connectionId = [[NSString alloc] initWithUTF8String:connectionInfo->connectionId.c_str()];
    }
    return self;
}

-(void) dealloc
{
    [self->adapterName release];
    [self->connectionId release];
    [super dealloc];
}

-(BOOL) isEqual:(id)o_
{   
    if(self == o_)
    {
        return YES;
    }
    if(!o_ || ![o_ isKindOfClass:[self class]])
    {
        return NO;
    }
    ICEConnectionInfo *obj_ = (ICEConnectionInfo *)o_;
    if(self->incoming != obj_->incoming)
    {
        return NO;
    }
    if(![self->adapterName isEqualToString:obj_->adapterName])
    {
        return NO;
    }
    if(![self->connectionId isEqualToString:obj_->connectionId])
    {
        return NO;
    }
    return YES;
}

+(id) connectionInfoWithConnectionInfo:(Ice::ConnectionInfo*)connectionInfo
{
    if(!connectionInfo)
    {
        return nil;
    }
    else if(dynamic_cast<Ice::UDPConnectionInfo*>(connectionInfo))
    {
        return [[[ICEUDPConnectionInfo alloc]
                    initWithUDPConnectionInfo:dynamic_cast<Ice::UDPConnectionInfo*>(connectionInfo)] autorelease];
    }
    else if(dynamic_cast<Ice::TCPConnectionInfo*>(connectionInfo))
    {
        return [[[ICETCPConnectionInfo alloc] 
                    initWithTCPConnectionInfo:dynamic_cast<Ice::TCPConnectionInfo*>(connectionInfo)] autorelease];
    }
    else if(dynamic_cast<IceSSL::ConnectionInfo*>(connectionInfo))
    {
        return [[[ICESSLConnectionInfo alloc] 
                    initWithSSLConnectionInfo:dynamic_cast<IceSSL::ConnectionInfo*>(connectionInfo)] autorelease];
    }
    return nil;
}
@end

@implementation ICEIPConnectionInfo

@synthesize localAddress;
@synthesize localPort;
@synthesize remoteAddress;
@synthesize remotePort;

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

-(void) dealloc
{
    [self->localAddress release];
    [self->remoteAddress release];
    [super dealloc];
}

-(BOOL) isEqual:(id)o_
{   
    if(self == o_)
    {
        return YES;
    }
    if(!o_ || ![o_ isKindOfClass:[self class]])
    {
        return NO;
    }
    if(![super isEqual:o_])
    {
        return NO;
    }
    ICEIPConnectionInfo *obj_ = (ICEIPConnectionInfo *)o_;
    if(![self->localAddress isEqualToString:obj_->localAddress])
    {
        return NO;
    }
    if(self->localPort != obj_->localPort)
    {
        return NO;
    }
    if(![self->remoteAddress isEqualToString:obj_->remoteAddress])
    {
        return NO;
    }
    if(self->remotePort != obj_->remotePort)
    {
        return NO;
    }
    return YES;
}
@end

@implementation ICETCPConnectionInfo

-(id) initWithTCPConnectionInfo:(Ice::TCPConnectionInfo*)tcpConnectionInfo
{
    self = [super initWithIPConnectionInfo:tcpConnectionInfo];
    return self;
}
@end

@implementation ICEUDPConnectionInfo

@synthesize mcastAddress;
@synthesize mcastPort;

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

-(void) dealloc
{
    [self->mcastAddress release];
    [super dealloc];
}

-(BOOL) isEqual:(id)o_
{   
    if(self == o_)
    {
        return YES;
    }
    if(!o_ || ![o_ isKindOfClass:[self class]])
    {
        return NO;
    }
    if(![super isEqual:o_])
    {
        return NO;
    }
    ICEUDPConnectionInfo *obj_ = (ICEUDPConnectionInfo *)o_;
    if(![self->mcastAddress isEqualToString:obj_->mcastAddress])
    {
        return NO;
    }
    if(self->mcastPort != obj_->mcastPort)
    {
        return NO;
    }
    return YES;
}
@end

@implementation ICESSLConnectionInfo

@synthesize cipher;
@synthesize certs;

-(id) initWithSSLConnectionInfo:(IceSSL::ConnectionInfo*)sslConnectionInfo
{
    self = [super initWithIPConnectionInfo:sslConnectionInfo];
    if(self)
    {
        self->cipher = [[NSString alloc] initWithUTF8String:sslConnectionInfo->cipher.c_str()];
        self->certs = toNSArray(sslConnectionInfo->certs);
    }
    return self;
}

-(void) dealloc
{
    [self->cipher release];
    [self->certs release];
    [super dealloc];
}

-(BOOL) isEqual:(id)o_
{   
    if(self == o_)
    {
        return YES;
    }
    if(!o_ || ![o_ isKindOfClass:[self class]])
    {
        return NO;
    }
    if(![super isEqual:o_])
    {
        return NO;
    }
    ICESSLConnectionInfo* obj_ = (ICESSLConnectionInfo*)o_;
    if(![self->cipher isEqualToString:obj_->cipher])
    {
        return NO;
    }
    if(![self->certs isEqual:obj_->certs])
    {
        return NO;
    }
    return YES;
}
@end

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
        return [ICEObjectAdapter wrapperWithCxxObject:CONNECTION->getAdapter().get()];
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
-(NSString*) type
{
    return [toNSString(CONNECTION->type()) autorelease];
}
-(ICEInt) timeout
{
    return CONNECTION->timeout();
}
-(NSString*) toString
{
    return [toNSString(CONNECTION->toString()) autorelease];
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
        return [ICEConnectionInfo connectionInfoWithConnectionInfo:info.get()];
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

-(ICEEndpoint*) getEndpoint
{
    NSException* nsex = nil;
    try
    {
        return [ICEEndpoint wrapperWithCxxObject:CONNECTION->getEndpoint().get()];
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
