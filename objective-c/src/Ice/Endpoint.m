//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `Endpoint.ice'
#import <objc/Ice/LocalException.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/Endpoint.h>
#import <objc/runtime.h>

#ifndef ICE_API_EXPORTS
#   define ICE_API_EXPORTS
#endif

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wshadow-ivar"
#endif

@implementation ICEInternalPrefixTable(C86077379_8C4D_4A78_9598_01B3D0750AC7)
-(void)addPrefixes_C86077379_8C4D_4A78_9598_01B3D0750AC7:(NSMutableDictionary*)prefixTable
{
    [prefixTable setObject:@"ICE" forKey:@"::Ice"];
}
@end

@implementation ICEEndpointInfo

@synthesize underlying;
@synthesize timeout;
@synthesize compress;

-(id) init:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->underlying = ICE_RETAIN(iceP_underlying);
    self->timeout = iceP_timeout;
    self->compress = iceP_compress;
    return self;
}

+(id) endpointInfo
{
    return ICE_AUTORELEASE([(ICEEndpointInfo *)[self alloc] init]);
}

+(id) endpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress
{
    return ICE_AUTORELEASE([(ICEEndpointInfo *)[self alloc] init:iceP_underlying timeout:iceP_timeout compress:iceP_compress]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEEndpointInfo *)[[self class] allocWithZone:zone] init:underlying timeout:timeout compress:compress];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->underlying release];
    [super dealloc];
}
#endif
@end

@implementation ICEIPEndpointInfo

@synthesize host;
@synthesize port;
@synthesize sourceAddress;

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->host = @"";
    self->sourceAddress = @"";
    return self;
}

-(id) init:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress host:(NSString*)iceP_host port:(ICEInt)iceP_port sourceAddress:(NSString*)iceP_sourceAddress
{
    self = [super init:iceP_underlying timeout:iceP_timeout compress:iceP_compress];
    if(!self)
    {
        return nil;
    }
    self->host = ICE_RETAIN(iceP_host);
    self->port = iceP_port;
    self->sourceAddress = ICE_RETAIN(iceP_sourceAddress);
    return self;
}

+(id) ipEndpointInfo
{
    return ICE_AUTORELEASE([(ICEIPEndpointInfo *)[self alloc] init]);
}

+(id) ipEndpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress host:(NSString*)iceP_host port:(ICEInt)iceP_port sourceAddress:(NSString*)iceP_sourceAddress
{
    return ICE_AUTORELEASE([(ICEIPEndpointInfo *)[self alloc] init:iceP_underlying timeout:iceP_timeout compress:iceP_compress host:iceP_host port:iceP_port sourceAddress:iceP_sourceAddress]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEIPEndpointInfo *)[[self class] allocWithZone:zone] init:underlying timeout:timeout compress:compress host:host port:port sourceAddress:sourceAddress];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->host release];
    [self->sourceAddress release];
    [super dealloc];
}
#endif
@end

@implementation ICETCPEndpointInfo

+(id) tcpEndpointInfo
{
    return ICE_AUTORELEASE([(ICETCPEndpointInfo *)[self alloc] init]);
}

+(id) tcpEndpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress host:(NSString*)iceP_host port:(ICEInt)iceP_port sourceAddress:(NSString*)iceP_sourceAddress
{
    return ICE_AUTORELEASE([(ICETCPEndpointInfo *)[self alloc] init:iceP_underlying timeout:iceP_timeout compress:iceP_compress host:iceP_host port:iceP_port sourceAddress:iceP_sourceAddress]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICETCPEndpointInfo *)[[self class] allocWithZone:zone] init:underlying timeout:timeout compress:compress host:host port:port sourceAddress:sourceAddress];
}
@end

@implementation ICEUDPEndpointInfo

@synthesize mcastInterface;
@synthesize mcastTtl;

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->mcastInterface = @"";
    return self;
}

-(id) init:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress host:(NSString*)iceP_host port:(ICEInt)iceP_port sourceAddress:(NSString*)iceP_sourceAddress mcastInterface:(NSString*)iceP_mcastInterface mcastTtl:(ICEInt)iceP_mcastTtl
{
    self = [super init:iceP_underlying timeout:iceP_timeout compress:iceP_compress host:iceP_host port:iceP_port sourceAddress:iceP_sourceAddress];
    if(!self)
    {
        return nil;
    }
    self->mcastInterface = ICE_RETAIN(iceP_mcastInterface);
    self->mcastTtl = iceP_mcastTtl;
    return self;
}

+(id) udpEndpointInfo
{
    return ICE_AUTORELEASE([(ICEUDPEndpointInfo *)[self alloc] init]);
}

+(id) udpEndpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress host:(NSString*)iceP_host port:(ICEInt)iceP_port sourceAddress:(NSString*)iceP_sourceAddress mcastInterface:(NSString*)iceP_mcastInterface mcastTtl:(ICEInt)iceP_mcastTtl
{
    return ICE_AUTORELEASE([(ICEUDPEndpointInfo *)[self alloc] init:iceP_underlying timeout:iceP_timeout compress:iceP_compress host:iceP_host port:iceP_port sourceAddress:iceP_sourceAddress mcastInterface:iceP_mcastInterface mcastTtl:iceP_mcastTtl]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEUDPEndpointInfo *)[[self class] allocWithZone:zone] init:underlying timeout:timeout compress:compress host:host port:port sourceAddress:sourceAddress mcastInterface:mcastInterface mcastTtl:mcastTtl];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->mcastInterface release];
    [super dealloc];
}
#endif
@end

@implementation ICEWSEndpointInfo

@synthesize resource;

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->resource = @"";
    return self;
}

-(id) init:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress resource:(NSString*)iceP_resource
{
    self = [super init:iceP_underlying timeout:iceP_timeout compress:iceP_compress];
    if(!self)
    {
        return nil;
    }
    self->resource = ICE_RETAIN(iceP_resource);
    return self;
}

+(id) wsEndpointInfo
{
    return ICE_AUTORELEASE([(ICEWSEndpointInfo *)[self alloc] init]);
}

+(id) wsEndpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress resource:(NSString*)iceP_resource
{
    return ICE_AUTORELEASE([(ICEWSEndpointInfo *)[self alloc] init:iceP_underlying timeout:iceP_timeout compress:iceP_compress resource:iceP_resource]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEWSEndpointInfo *)[[self class] allocWithZone:zone] init:underlying timeout:timeout compress:compress resource:resource];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->resource release];
    [super dealloc];
}
#endif
@end

@implementation ICEOpaqueEndpointInfo

@synthesize rawEncoding;
@synthesize rawBytes;

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->rawEncoding = [[ICEEncodingVersion alloc] init];
    return self;
}

-(id) init:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress rawEncoding:(ICEEncodingVersion*)iceP_rawEncoding rawBytes:(ICEByteSeq*)iceP_rawBytes
{
    self = [super init:iceP_underlying timeout:iceP_timeout compress:iceP_compress];
    if(!self)
    {
        return nil;
    }
    self->rawEncoding = ICE_RETAIN(iceP_rawEncoding);
    self->rawBytes = ICE_RETAIN(iceP_rawBytes);
    return self;
}

+(id) opaqueEndpointInfo
{
    return ICE_AUTORELEASE([(ICEOpaqueEndpointInfo *)[self alloc] init]);
}

+(id) opaqueEndpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress rawEncoding:(ICEEncodingVersion*)iceP_rawEncoding rawBytes:(ICEByteSeq*)iceP_rawBytes
{
    return ICE_AUTORELEASE([(ICEOpaqueEndpointInfo *)[self alloc] init:iceP_underlying timeout:iceP_timeout compress:iceP_compress rawEncoding:iceP_rawEncoding rawBytes:iceP_rawBytes]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEOpaqueEndpointInfo *)[[self class] allocWithZone:zone] init:underlying timeout:timeout compress:compress rawEncoding:rawEncoding rawBytes:rawBytes];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->rawEncoding release];
    [self->rawBytes release];
    [super dealloc];
}
#endif
@end
