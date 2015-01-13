// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in
// the ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <EndpointI.h>
#import <Util.h>
#import <VersionI.h>

#import <objc/Ice/BuiltinSequences.h>

#import <Ice/Endpoint.h>


const int ICETCPEndpointType = 1;
const int ICESSLEndpointType = 2;
const int ICEUDPEndpointType = 3;
const int ICEWSEndpointType = 4;
const int ICEWSSEndpointType = 5;

#define ENDPOINT dynamic_cast<Ice::Endpoint*>(static_cast<IceUtil::Shared*>(cxxObject_))

@implementation ICEEndpoint

-(Ice::Endpoint*) endpoint
{
    return ENDPOINT;
}

-(ICEEndpointInfo*) getInfo
{
    Ice::EndpointInfoPtr info = ENDPOINT->getInfo();
    return [ICEEndpointInfo enpointInfoWithEndpointInfo:info.get()];
}

-(NSString*) toString
{
    return [toNSString(ENDPOINT->toString()) autorelease];
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
    ICEEndpoint* obj_ = (ICEEndpoint*)o_;
    return *ENDPOINT == *(dynamic_cast<Ice::Endpoint*>(static_cast<IceUtil::Shared*>(obj_->cxxObject_)));
}
@end

@implementation ICEEndpointInfo

@synthesize timeout;
@synthesize compress;

-(id) init
{
    self = [super init];
    return self;
}

-(id) initWithEndpointInfo:(Ice::EndpointInfo*)endpointInfo;
{
    self = [super init];
    if(self)
    {
        self->timeout = endpointInfo->timeout;
        self->compress = endpointInfo->compress;
        self->type_ = endpointInfo->type();
        self->datagram_ = endpointInfo->datagram();
        self->secure_ = endpointInfo->secure();
    }
    return self;
}

+(id) enpointInfoWithEndpointInfo:(Ice::EndpointInfo*)endpointInfo
{
    if(!endpointInfo)
    {
        return nil;
    }
    else if(dynamic_cast<Ice::TCPEndpointInfo*>(endpointInfo))
    {
        return [[[ICETCPEndpointInfo alloc]
                                initWithTCPEndpointInfo:dynamic_cast<Ice::TCPEndpointInfo*>(endpointInfo)] autorelease];
    }
    else if(dynamic_cast<Ice::UDPEndpointInfo*>(endpointInfo))
    {
        return [[[ICEUDPEndpointInfo alloc]
                                initWithUDPEndpointInfo:dynamic_cast<Ice::UDPEndpointInfo*>(endpointInfo)] autorelease];
    }
    else if(dynamic_cast<Ice::WSEndpointInfo*>(endpointInfo))
    {
        return [[[ICEWSEndpointInfo alloc]
                                initWithWSEndpointInfo:dynamic_cast<Ice::WSEndpointInfo*>(endpointInfo)] autorelease];
    }
    else if(dynamic_cast<Ice::OpaqueEndpointInfo*>(endpointInfo))
    {
        return [[[ICEOpaqueEndpointInfo alloc]
                     initWithOpaqueEndpointInfo:dynamic_cast<Ice::OpaqueEndpointInfo*>(endpointInfo)] autorelease];
    }
    else if(dynamic_cast<IceSSL::EndpointInfo*>(endpointInfo))
    {
        return [[[ICESSLEndpointInfo alloc]
                                initWithSSLEndpointInfo:dynamic_cast<IceSSL::EndpointInfo*>(endpointInfo)] autorelease];
    }
    return nil;
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
    ICEEndpointInfo* obj_ = (ICEEndpointInfo*)o_;
    if(self->timeout != obj_->timeout)
    {
        return NO;
    }
    if(self->compress != obj_->compress)
    {
        return NO;
    }
    if(self->type_ != obj_->type_)
    {
        return NO;
    }
    if(self->datagram_ != obj_->datagram_)
    {
        return NO;
    }
    if(self->secure_ != obj_->secure_)
    {
        return NO;
    }
    return YES;
}

-(ICEShort) type
{
    return type_;
}

-(BOOL) datagram;
{
    return datagram_;
}

-(BOOL) secure;
{
    return secure_;
}
@end

@implementation ICEIPEndpointInfo

@synthesize host;
@synthesize port;
@synthesize sourceAddress;

-(id) initWithIPEndpointInfo:(Ice::IPEndpointInfo*)ipEndpointInfo;
{
    self = [super initWithEndpointInfo:ipEndpointInfo];
    if(self)
    {
        self->host = [[NSString alloc] initWithUTF8String:ipEndpointInfo->host.c_str()];
        self->port = ipEndpointInfo->port;
        self->sourceAddress = [[NSString alloc] initWithUTF8String:ipEndpointInfo->sourceAddress.c_str()];
    }
    return self;
}

-(void) dealloc
{
    [self->host release];
    [self->sourceAddress release];
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
    ICEIPEndpointInfo* obj_ = (ICEIPEndpointInfo*)o_;
    if(![self->host isEqualToString:obj_->host])
    {
        return NO;
    }
    if(self->port != obj_->port)
    {
        return NO;
    }
    if(![self->sourceAddress isEqualToString:obj_->sourceAddress])
    {
        return NO;
    }
    return YES;
}
@end

@implementation ICETCPEndpointInfo
-(id) initWithTCPEndpointInfo:(Ice::TCPEndpointInfo*)tcpEndpointInfo
{
    self = [super initWithIPEndpointInfo:tcpEndpointInfo];
    return self;
}
@end

@implementation ICEUDPEndpointInfo

@synthesize mcastInterface;
@synthesize mcastTtl;

-(id) initWithUDPEndpointInfo:(Ice::UDPEndpointInfo*)udpEndpointInfo
{
    self = [super initWithIPEndpointInfo:udpEndpointInfo];
    if(self)
    {
        self->mcastInterface = [[NSString alloc] initWithUTF8String:udpEndpointInfo->mcastInterface.c_str()];
        self->mcastTtl = udpEndpointInfo->mcastTtl;
    }
    return self;
}

-(void) dealloc
{
    [self->mcastInterface release];
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
    ICEUDPEndpointInfo* obj_ = (ICEUDPEndpointInfo*)o_;
    if(![self->mcastInterface isEqualToString:obj_->mcastInterface])
    {
        return NO;
    }
    if(self->mcastTtl != obj_->mcastTtl)
    {
        return NO;
    }
    return YES;
}
@end

@implementation ICEWSEndpointInfo

@synthesize resource;

-(id) initWithWSEndpointInfo:(Ice::WSEndpointInfo*)wsEndpointInfo
{
    self = [super initWithIPEndpointInfo:wsEndpointInfo];
    if(self)
    {
        self->resource = [[NSString alloc] initWithUTF8String:wsEndpointInfo->resource.c_str()];
    }
    return self;
}

-(void) dealloc
{
    [self->resource release];
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
    ICEWSEndpointInfo* obj_ = (ICEWSEndpointInfo*)o_;
    if(![self->resource isEqualToString:obj_->resource])
    {
        return NO;
    }
    return YES;
}
@end

@implementation ICEOpaqueEndpointInfo

@synthesize rawEncoding;
@synthesize rawBytes;

-(id) initWithOpaqueEndpointInfo:(Ice::OpaqueEndpointInfo*)opaqueEndpointInfo
{
    self = [super init];
    if(self)
    {
        self->rawEncoding = [[ICEEncodingVersion alloc] initWithEncodingVersion:opaqueEndpointInfo->rawEncoding];
        self->rawBytes = toNSData(opaqueEndpointInfo->rawBytes);
    }
    return self;
}

-(void) dealloc
{
    [self->rawEncoding release];
    [self->rawBytes release];
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
    ICEOpaqueEndpointInfo* obj_ = (ICEOpaqueEndpointInfo*)o_;
    if(![self->rawEncoding isEqual:obj_->rawEncoding])
    {
        return NO;
    }
    if(![self->rawBytes isEqual:obj_->rawBytes])
    {
        return NO;
    }
    return YES;
}
@end

@implementation ICESSLEndpointInfo
-(id) initWithSSLEndpointInfo:(IceSSL::EndpointInfo*)sslEndpointInfo
{
    self = [super initWithIPEndpointInfo:sslEndpointInfo];
    return self;
}
@end
