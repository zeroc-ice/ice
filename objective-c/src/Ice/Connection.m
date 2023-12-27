//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `Connection.ice'
#import <objc/Ice/LocalException.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/Connection.h>
#import <objc/runtime.h>

#ifndef ICE_API_EXPORTS
#   define ICE_API_EXPORTS
#endif

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wshadow-ivar"
#endif

@implementation ICEInternalPrefixTable(C13D41417_03CF_4850_A8BD_0AD87B338AC8)
-(void)addPrefixes_C13D41417_03CF_4850_A8BD_0AD87B338AC8:(NSMutableDictionary*)prefixTable
{
    [prefixTable setObject:@"ICE" forKey:@"::Ice"];
}
@end

@implementation ICEConnectionInfo

@synthesize underlying;
@synthesize incoming;
@synthesize adapterName;
@synthesize connectionId;

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->adapterName = @"";
    self->connectionId = @"";
    return self;
}

-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->underlying = ICE_RETAIN(iceP_underlying);
    self->incoming = iceP_incoming;
    self->adapterName = ICE_RETAIN(iceP_adapterName);
    self->connectionId = ICE_RETAIN(iceP_connectionId);
    return self;
}

+(id) connectionInfo
{
    return ICE_AUTORELEASE([(ICEConnectionInfo *)[self alloc] init]);
}

+(id) connectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId
{
    return ICE_AUTORELEASE([(ICEConnectionInfo *)[self alloc] init:iceP_underlying incoming:iceP_incoming adapterName:iceP_adapterName connectionId:iceP_connectionId]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEConnectionInfo *)[[self class] allocWithZone:zone] init:underlying incoming:incoming adapterName:adapterName connectionId:connectionId];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->underlying release];
    [self->adapterName release];
    [self->connectionId release];
    [super dealloc];
}
#endif
@end

@implementation ICEACM

@synthesize timeout;
@synthesize close;
@synthesize heartbeat;

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->close = ICECloseOff;
    self->heartbeat = ICEHeartbeatOff;
    return self;
}

-(id) init:(ICEInt)iceP_timeout close:(ICEACMClose)iceP_close heartbeat:(ICEACMHeartbeat)iceP_heartbeat
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->timeout = iceP_timeout;
    self->close = iceP_close;
    self->heartbeat = iceP_heartbeat;
    return self;
}

+(id) acm
{
    return ICE_AUTORELEASE([(ICEACM *)[self alloc] init]);
}

+(id) aCM
{
    return ICE_AUTORELEASE([self acm]);
}

+(id) acm:(ICEInt)iceP_timeout close:(ICEACMClose)iceP_close heartbeat:(ICEACMHeartbeat)iceP_heartbeat
{
    return ICE_AUTORELEASE([(ICEACM *)[self alloc] init:iceP_timeout close:iceP_close heartbeat:iceP_heartbeat]);
}

+(id) aCM:(ICEInt)iceP_timeout close:(ICEACMClose)iceP_close heartbeat:(ICEACMHeartbeat)iceP_heartbeat
{
    return ICE_AUTORELEASE([self acm:iceP_timeout close:iceP_close heartbeat:iceP_heartbeat]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEACM *)[[self class] allocWithZone:zone] init:timeout close:close heartbeat:heartbeat];
}

-(NSUInteger) hash
{
    NSUInteger h_ = 5381;
    h_ = ((h_ << 5) + h_) ^ (2654435761u * (uint)timeout);
    h_ = ((h_ << 5) + h_) ^ (uint)close;
    h_ = ((h_ << 5) + h_) ^ (uint)heartbeat;
    return h_;
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
    ICEACM *obj_ = (ICEACM *)o_;
    if(timeout != obj_->timeout)
    {
        return NO;
    }
    if(close != obj_->close)
    {
        return NO;
    }
    if(heartbeat != obj_->heartbeat)
    {
        return NO;
    }
    return YES;
}
@end

@implementation ICEIPConnectionInfo

@synthesize localAddress;
@synthesize localPort;
@synthesize remoteAddress;
@synthesize remotePort;

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->localAddress = @"";
    self->localPort = -1;
    self->remoteAddress = @"";
    self->remotePort = -1;
    return self;
}

-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId localAddress:(NSString*)iceP_localAddress localPort:(ICEInt)iceP_localPort remoteAddress:(NSString*)iceP_remoteAddress remotePort:(ICEInt)iceP_remotePort
{
    self = [super init:iceP_underlying incoming:iceP_incoming adapterName:iceP_adapterName connectionId:iceP_connectionId];
    if(!self)
    {
        return nil;
    }
    self->localAddress = ICE_RETAIN(iceP_localAddress);
    self->localPort = iceP_localPort;
    self->remoteAddress = ICE_RETAIN(iceP_remoteAddress);
    self->remotePort = iceP_remotePort;
    return self;
}

+(id) ipConnectionInfo
{
    return ICE_AUTORELEASE([(ICEIPConnectionInfo *)[self alloc] init]);
}

+(id) ipConnectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId localAddress:(NSString*)iceP_localAddress localPort:(ICEInt)iceP_localPort remoteAddress:(NSString*)iceP_remoteAddress remotePort:(ICEInt)iceP_remotePort
{
    return ICE_AUTORELEASE([(ICEIPConnectionInfo *)[self alloc] init:iceP_underlying incoming:iceP_incoming adapterName:iceP_adapterName connectionId:iceP_connectionId localAddress:iceP_localAddress localPort:iceP_localPort remoteAddress:iceP_remoteAddress remotePort:iceP_remotePort]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEIPConnectionInfo *)[[self class] allocWithZone:zone] init:underlying incoming:incoming adapterName:adapterName connectionId:connectionId localAddress:localAddress localPort:localPort remoteAddress:remoteAddress remotePort:remotePort];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->localAddress release];
    [self->remoteAddress release];
    [super dealloc];
}
#endif
@end

@implementation ICETCPConnectionInfo

@synthesize rcvSize;
@synthesize sndSize;

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->rcvSize = 0;
    self->sndSize = 0;
    return self;
}

-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId localAddress:(NSString*)iceP_localAddress localPort:(ICEInt)iceP_localPort remoteAddress:(NSString*)iceP_remoteAddress remotePort:(ICEInt)iceP_remotePort rcvSize:(ICEInt)iceP_rcvSize sndSize:(ICEInt)iceP_sndSize
{
    self = [super init:iceP_underlying incoming:iceP_incoming adapterName:iceP_adapterName connectionId:iceP_connectionId localAddress:iceP_localAddress localPort:iceP_localPort remoteAddress:iceP_remoteAddress remotePort:iceP_remotePort];
    if(!self)
    {
        return nil;
    }
    self->rcvSize = iceP_rcvSize;
    self->sndSize = iceP_sndSize;
    return self;
}

+(id) tcpConnectionInfo
{
    return ICE_AUTORELEASE([(ICETCPConnectionInfo *)[self alloc] init]);
}

+(id) tcpConnectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId localAddress:(NSString*)iceP_localAddress localPort:(ICEInt)iceP_localPort remoteAddress:(NSString*)iceP_remoteAddress remotePort:(ICEInt)iceP_remotePort rcvSize:(ICEInt)iceP_rcvSize sndSize:(ICEInt)iceP_sndSize
{
    return ICE_AUTORELEASE([(ICETCPConnectionInfo *)[self alloc] init:iceP_underlying incoming:iceP_incoming adapterName:iceP_adapterName connectionId:iceP_connectionId localAddress:iceP_localAddress localPort:iceP_localPort remoteAddress:iceP_remoteAddress remotePort:iceP_remotePort rcvSize:iceP_rcvSize sndSize:iceP_sndSize]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICETCPConnectionInfo *)[[self class] allocWithZone:zone] init:underlying incoming:incoming adapterName:adapterName connectionId:connectionId localAddress:localAddress localPort:localPort remoteAddress:remoteAddress remotePort:remotePort rcvSize:rcvSize sndSize:sndSize];
}
@end

@implementation ICEUDPConnectionInfo

@synthesize mcastAddress;
@synthesize mcastPort;
@synthesize rcvSize;
@synthesize sndSize;

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->mcastAddress = @"";
    self->mcastPort = -1;
    self->rcvSize = 0;
    self->sndSize = 0;
    return self;
}

-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId localAddress:(NSString*)iceP_localAddress localPort:(ICEInt)iceP_localPort remoteAddress:(NSString*)iceP_remoteAddress remotePort:(ICEInt)iceP_remotePort mcastAddress:(NSString*)iceP_mcastAddress mcastPort:(ICEInt)iceP_mcastPort rcvSize:(ICEInt)iceP_rcvSize sndSize:(ICEInt)iceP_sndSize
{
    self = [super init:iceP_underlying incoming:iceP_incoming adapterName:iceP_adapterName connectionId:iceP_connectionId localAddress:iceP_localAddress localPort:iceP_localPort remoteAddress:iceP_remoteAddress remotePort:iceP_remotePort];
    if(!self)
    {
        return nil;
    }
    self->mcastAddress = ICE_RETAIN(iceP_mcastAddress);
    self->mcastPort = iceP_mcastPort;
    self->rcvSize = iceP_rcvSize;
    self->sndSize = iceP_sndSize;
    return self;
}

+(id) udpConnectionInfo
{
    return ICE_AUTORELEASE([(ICEUDPConnectionInfo *)[self alloc] init]);
}

+(id) udpConnectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId localAddress:(NSString*)iceP_localAddress localPort:(ICEInt)iceP_localPort remoteAddress:(NSString*)iceP_remoteAddress remotePort:(ICEInt)iceP_remotePort mcastAddress:(NSString*)iceP_mcastAddress mcastPort:(ICEInt)iceP_mcastPort rcvSize:(ICEInt)iceP_rcvSize sndSize:(ICEInt)iceP_sndSize
{
    return ICE_AUTORELEASE([(ICEUDPConnectionInfo *)[self alloc] init:iceP_underlying incoming:iceP_incoming adapterName:iceP_adapterName connectionId:iceP_connectionId localAddress:iceP_localAddress localPort:iceP_localPort remoteAddress:iceP_remoteAddress remotePort:iceP_remotePort mcastAddress:iceP_mcastAddress mcastPort:iceP_mcastPort rcvSize:iceP_rcvSize sndSize:iceP_sndSize]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEUDPConnectionInfo *)[[self class] allocWithZone:zone] init:underlying incoming:incoming adapterName:adapterName connectionId:connectionId localAddress:localAddress localPort:localPort remoteAddress:remoteAddress remotePort:remotePort mcastAddress:mcastAddress mcastPort:mcastPort rcvSize:rcvSize sndSize:sndSize];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->mcastAddress release];
    [super dealloc];
}
#endif
@end

@implementation ICEWSConnectionInfo

@synthesize headers;

-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId headers:(ICEHeaderDict*)iceP_headers
{
    self = [super init:iceP_underlying incoming:iceP_incoming adapterName:iceP_adapterName connectionId:iceP_connectionId];
    if(!self)
    {
        return nil;
    }
    self->headers = ICE_RETAIN(iceP_headers);
    return self;
}

+(id) wsConnectionInfo
{
    return ICE_AUTORELEASE([(ICEWSConnectionInfo *)[self alloc] init]);
}

+(id) wsConnectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId headers:(ICEHeaderDict*)iceP_headers
{
    return ICE_AUTORELEASE([(ICEWSConnectionInfo *)[self alloc] init:iceP_underlying incoming:iceP_incoming adapterName:iceP_adapterName connectionId:iceP_connectionId headers:iceP_headers]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICEWSConnectionInfo *)[[self class] allocWithZone:zone] init:underlying incoming:incoming adapterName:adapterName connectionId:connectionId headers:headers];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->headers release];
    [super dealloc];
}
#endif
@end
