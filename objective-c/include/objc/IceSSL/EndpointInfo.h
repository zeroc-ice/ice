//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `EndpointInfo.ice'

#import <objc/Ice/Config.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>
#import <objc/Ice/Exception.h>
#import <objc/Ice/Endpoint.h>

#ifndef ICESSL_API
#   if defined(ICE_STATIC_LIBS)
#       define ICESSL_API /**/
#   elif defined(ICESSL_API_EXPORTS)
#       define ICESSL_API ICE_DECLSPEC_EXPORT
#   else
#       define ICESSL_API ICE_DECLSPEC_IMPORT
#   endif
#endif

@class ICESSLEndpointInfo;
@protocol ICESSLEndpointInfo;

ICESSL_API @protocol ICESSLEndpointInfo <ICEEndpointInfo>
@end

ICESSL_API @interface ICESSLEndpointInfo : ICEEndpointInfo
+(id) endpointInfo;
+(id) endpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress;
// This class also overrides copyWithZone:
@end
