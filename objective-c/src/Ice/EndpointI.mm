// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in
// the ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <EndpointI.h>
#import <Util.h>
#import <VersionI.h>
#import <LocalObjectI.h>

#import <objc/Ice/BuiltinSequences.h>

#import <Ice/Endpoint.h>

#include <objc/runtime.h>

#define ENDPOINT dynamic_cast<Ice::Endpoint*>(static_cast<IceUtil::Shared*>(cxxObject_))
#define ENDPOINTINFO dynamic_cast<Ice::EndpointInfo*>(static_cast<IceUtil::Shared*>(cxxObject_))

@implementation ICEEndpoint

-(Ice::Endpoint*) endpoint
{
    return ENDPOINT;
}

-(ICEEndpointInfo*) getInfo
{
    NSException* nsex = nil;
    try
    {
        Ice::EndpointInfoPtr info = ENDPOINT->getInfo();
        if(!info)
        {
            return nil;
        }
        
        Ice::UDPEndpointInfoPtr udpInfo = Ice::UDPEndpointInfoPtr::dynamicCast(info);
        if(udpInfo)
        {
            return [[[ICEUDPEndpointInfo alloc] initWithUDPEndpointInfo:udpInfo.get()] autorelease];
        }

        Ice::WSEndpointInfoPtr wsInfo = Ice::WSEndpointInfoPtr::dynamicCast(info);
        if(wsInfo)
        {
            return [[[ICEWSEndpointInfo alloc] initWithWSEndpointInfo:wsInfo.get()] autorelease];
        }

        Ice::TCPEndpointInfoPtr tcpInfo = Ice::TCPEndpointInfoPtr::dynamicCast(info);
        if(tcpInfo)
        {
            return [[[ICETCPEndpointInfo alloc] initWithTCPEndpointInfo:tcpInfo.get()] autorelease];
        }

        Ice::OpaqueEndpointInfoPtr opaqueInfo = Ice::OpaqueEndpointInfoPtr::dynamicCast(info);
        if(opaqueInfo)
        {
            return [[[ICEOpaqueEndpointInfo alloc] initWithOpaqueEndpointInfo:opaqueInfo.get()] autorelease];
        }
        
        std::ostringstream os;
        os << "endpointInfoWithType_" << info->type() << ":";
        SEL selector = sel_registerName(os.str().c_str());
        if([ICEEndpointInfo respondsToSelector:selector])
        {
            IceUtil::Shared* shared = info.get();
            return [ICEEndpointInfo performSelector:selector withObject:[NSValue valueWithPointer:shared]];
        }

        Ice::IPEndpointInfoPtr ipInfo = Ice::IPEndpointInfoPtr::dynamicCast(info);
        if(ipInfo)
        {
            return [[[ICEIPEndpointInfo alloc] initWithIPEndpointInfo:ipInfo.get()] autorelease];
        }

        return [[[ICEEndpointInfo alloc] initWithEndpointInfo:info.get()] autorelease];
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

-(NSMutableString*) toString
{
    return [toNSMutableString(ENDPOINT->toString()) autorelease];
}

@end

@implementation ICEEndpointInfo(ICEInternal)

-(id) initWithEndpointInfo:(Ice::EndpointInfo*)endpointInfo;
{
    self = [super initWithCxxObject:endpointInfo];
    if(self)
    {
        self->timeout = endpointInfo->timeout;
        self->compress = endpointInfo->compress;
    }
    return self;
}

-(ICEShort) type
{
    return ENDPOINTINFO->type();
}

-(BOOL) datagram;
{
    return ENDPOINTINFO->datagram();
}

-(BOOL) secure;
{
    return ENDPOINTINFO->secure();
}
@end

@implementation ICEIPEndpointInfo(ICEInternal)

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

@end

@implementation ICETCPEndpointInfo(ICEInternal)
-(id) initWithTCPEndpointInfo:(Ice::TCPEndpointInfo*)tcpEndpointInfo
{
    self = [super initWithIPEndpointInfo:tcpEndpointInfo];
    return self;
}
@end

@implementation ICEUDPEndpointInfo(ICEInternal)

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

@end

@implementation ICEWSEndpointInfo(ICEInternal)

-(id) initWithWSEndpointInfo:(Ice::WSEndpointInfo*)wsEndpointInfo
{
    self = [super initWithIPEndpointInfo:wsEndpointInfo];
    if(self)
    {
        self->resource = [[NSString alloc] initWithUTF8String:wsEndpointInfo->resource.c_str()];
    }
    return self;
}

@end

@implementation ICEOpaqueEndpointInfo(ICEInternal)

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

@end
