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

namespace
{

std::vector<Class>* endpointInfoClasses = 0;

}

namespace IceObjC
{

void
registerEndpointInfoClass(Class cl)
{
    if(!endpointInfoClasses)
    {
        endpointInfoClasses = new std::vector<Class>();
    }
    endpointInfoClasses->push_back(cl);
}

}

@implementation ICEEndpointInfo(ICEInternal)

+(id) checkedEndpointInfoWithEndpointInfo:(Ice::EndpointInfo*)endpointInfo
{
    assert(false);
    return nil;
}

+(id) endpointInfoWithEndpointInfo:(NSValue*)v
{
    Ice::EndpointInfo* info = dynamic_cast<Ice::EndpointInfo*>(reinterpret_cast<IceUtil::Shared*>([v pointerValue]));
    if(!info)
    {
        return nil;
    }

    Ice::UDPEndpointInfoPtr udpInfo = Ice::UDPEndpointInfoPtr::dynamicCast(info);
    if(udpInfo)
    {
        return [[ICEUDPEndpointInfo alloc] initWithUDPEndpointInfo:udpInfo.get()];
    }

    Ice::WSEndpointInfoPtr wsInfo = Ice::WSEndpointInfoPtr::dynamicCast(info);
    if(wsInfo)
    {
        return [[ICEWSEndpointInfo alloc] initWithWSEndpointInfo:wsInfo.get()];
    }

    Ice::TCPEndpointInfoPtr tcpInfo = Ice::TCPEndpointInfoPtr::dynamicCast(info);
    if(tcpInfo)
    {
        return [[ICETCPEndpointInfo alloc] initWithTCPEndpointInfo:tcpInfo.get()];
    }

    Ice::OpaqueEndpointInfoPtr opaqueInfo = Ice::OpaqueEndpointInfoPtr::dynamicCast(info);
    if(opaqueInfo)
    {
        return [[ICEOpaqueEndpointInfo alloc] initWithOpaqueEndpointInfo:opaqueInfo.get()];
    }

    if(endpointInfoClasses)
    {
        for(std::vector<Class>::const_iterator p = endpointInfoClasses->begin(); p != endpointInfoClasses->end(); ++p)
        {
            ICEEndpointInfo* r = [*p checkedEndpointInfoWithEndpointInfo:info];
            if(r)
            {
                return r;
            }
        }
    }

    Ice::IPEndpointInfoPtr ipInfo = Ice::IPEndpointInfoPtr::dynamicCast(info);
    if(ipInfo)
    {
        return [[ICEIPEndpointInfo alloc] initWithIPEndpointInfo:ipInfo.get()];
    }

    return [[ICEEndpointInfo alloc] initWithEndpointInfo:info];
}

-(id) initWithEndpointInfo:(Ice::EndpointInfo*)endpointInfo;
{
    self = [super initWithCxxObject:endpointInfo];
    if(self)
    {
        self->underlying = [ICEEndpointInfo localObjectWithCxxObjectNoAutoRelease:endpointInfo->underlying.get()
                                            allocator:@selector(endpointInfoWithEndpointInfo:)];
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
    self = [super initWithEndpointInfo:wsEndpointInfo];
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
        return [ICEEndpointInfo localObjectWithCxxObject:ENDPOINT->getInfo().get()
                                               allocator:@selector(endpointInfoWithEndpointInfo:)];
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
