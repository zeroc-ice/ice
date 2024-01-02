//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `EndpointInfo.ice'
#import <objc/Ice/LocalException.h>
#import <objc/Ice/Stream.h>
#import <objc/IceSSL/EndpointInfo.h>
#import <objc/runtime.h>

#ifndef ICESSL_API_EXPORTS
#   define ICESSL_API_EXPORTS
#endif

#ifdef __clang__
#   pragma clang diagnostic ignored "-Wshadow-ivar"
#endif

@implementation ICEInternalPrefixTable(C2B6C3EA5_E209_4659_9A29_51F299440EF0)
-(void)addPrefixes_C2B6C3EA5_E209_4659_9A29_51F299440EF0:(NSMutableDictionary*)prefixTable
{
    [prefixTable setObject:@"ICESSL" forKey:@"::IceSSL"];
}
@end

@implementation ICESSLEndpointInfo

+(id) endpointInfo
{
    return ICE_AUTORELEASE([(ICESSLEndpointInfo *)[self alloc] init]);
}

+(id) endpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)iceP_underlying timeout:(ICEInt)iceP_timeout compress:(BOOL)iceP_compress
{
    return ICE_AUTORELEASE([(ICESSLEndpointInfo *)[self alloc] init:iceP_underlying timeout:iceP_timeout compress:iceP_compress]);
}

-(id) copyWithZone:(NSZone *)zone
{
    return [(ICESSLEndpointInfo *)[[self class] allocWithZone:zone] init:underlying timeout:timeout compress:compress];
}
@end
