//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `ConnectionInfo.ice'

#import <objc/Ice/Config.h>
#import <objc/Ice/Proxy.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>
#import <objc/Ice/Exception.h>
#import <objc/Ice/Connection.h>

#ifndef ICESSL_API
#   if defined(ICE_STATIC_LIBS)
#       define ICESSL_API /**/
#   elif defined(ICESSL_API_EXPORTS)
#       define ICESSL_API ICE_DECLSPEC_EXPORT
#   else
#       define ICESSL_API ICE_DECLSPEC_IMPORT
#   endif
#endif

@class ICESSLConnectionInfo;
@protocol ICESSLConnectionInfo;

ICESSL_API @protocol ICESSLConnectionInfo <ICEConnectionInfo>
@end

ICESSL_API @interface ICESSLConnectionInfo : ICEConnectionInfo
{
    NSString *cipher;
    ICEStringSeq *certs;
    BOOL verified;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *cipher;
@property(nonatomic, ICE_STRONG_ATTR) ICEStringSeq *certs;
@property(nonatomic, assign) BOOL verified;

-(id) init;
-(id) init:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId cipher:(NSString*)cipher certs:(ICEStringSeq*)certs verified:(BOOL)verified;
+(id) connectionInfo;
+(id) connectionInfo:(ICEConnectionInfo<ICEConnectionInfo>*)underlying incoming:(BOOL)incoming adapterName:(NSString*)adapterName connectionId:(NSString*)connectionId cipher:(NSString*)cipher certs:(ICEStringSeq*)certs verified:(BOOL)verified;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end
