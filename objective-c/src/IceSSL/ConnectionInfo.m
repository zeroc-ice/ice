//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `ConnectionInfo.ice'
#import <objc/Ice/LocalException.h>
#import <objc/Ice/Stream.h>
#import <objc/IceSSL/ConnectionInfo.h>
#import <objc/runtime.h>

#ifndef ICESSL_API_EXPORTS
#   define ICESSL_API_EXPORTS
#endif

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wshadow-ivar"
#endif

@implementation ICEInternalPrefixTable(C65D66F63_EB40_4F6A_962B_977002358EA6)
-(void)addPrefixes_C65D66F63_EB40_4F6A_962B_977002358EA6:(NSMutableDictionary*)prefixTable
{
    [prefixTable setObject:@"ICESSL" forKey:@"::IceSSL"];
}
@end

@implementation ICESSLConnectionInfo

@synthesize cipher;
@synthesize certs;
@synthesize verified;

-(id) init
{
    self = [super init];
    if(!self)
    {
        return nil;
    }
    self->cipher = @"";
    return self;
}

-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId cipher:(NSString*)iceP_cipher certs:(ICEStringSeq*)iceP_certs verified:(BOOL)iceP_verified
{
    self = [super init:iceP_underlying incoming:iceP_incoming adapterName:iceP_adapterName connectionId:iceP_connectionId];
    if(!self)
    {
        return nil;
    }
    self->cipher = ICE_RETAIN(iceP_cipher);
    self->certs = ICE_RETAIN(iceP_certs);
    self->verified = iceP_verified;
    return self;
}

+(id) connectionInfo
{
    return ICE_AUTORELEASE([(ICESSLConnectionInfo *)[self alloc] init]);
}

+(id) connectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)iceP_underlying incoming:(BOOL)iceP_incoming adapterName:(NSString*)iceP_adapterName connectionId:(NSString*)iceP_connectionId cipher:(NSString*)iceP_cipher certs:(ICEStringSeq*)iceP_certs verified:(BOOL)iceP_verified
{
    return ICE_AUTORELEASE([(ICESSLConnectionInfo *)[self alloc] init:iceP_underlying incoming:iceP_incoming adapterName:iceP_adapterName connectionId:iceP_connectionId cipher:iceP_cipher certs:iceP_certs verified:iceP_verified]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICESSLConnectionInfo *)[[self class] allocWithZone:zone] init:underlying incoming:incoming adapterName:adapterName connectionId:connectionId cipher:cipher certs:certs verified:verified];
}

#if defined(__clang__) && !__has_feature(objc_arc)
-(void) dealloc
{
    [self->cipher release];
    [self->certs release];
    [super dealloc];
}
#endif
@end
