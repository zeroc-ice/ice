//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `Endpoint.ice'

#import <objc/Ice/Config.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>
#import <objc/Ice/Exception.h>
#import <objc/Ice/Version.h>
#import <objc/Ice/BuiltinSequences.h>
#import <objc/Ice/EndpointF.h>

#ifndef ICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

@class ICEEndpointInfo;
@protocol ICEEndpointInfo;

@protocol ICEEndpoint;

@class ICEIPEndpointInfo;
@protocol ICEIPEndpointInfo;

@class ICETCPEndpointInfo;
@protocol ICETCPEndpointInfo;

@class ICEUDPEndpointInfo;
@protocol ICEUDPEndpointInfo;

@class ICEWSEndpointInfo;
@protocol ICEWSEndpointInfo;

@class ICEOpaqueEndpointInfo;
@protocol ICEOpaqueEndpointInfo;

ICE_API @protocol ICEEndpointInfo <NSObject>
-(ICEShort) type;
-(BOOL) datagram;
-(BOOL) secure;
@end

ICE_API @interface ICEEndpointInfo : ICELocalObject
{
    ICEEndpointInfo<ICEEndpointInfo> *underlying;
    ICEInt timeout;
    BOOL compress;
}

@property(nonatomic, ICE_STRONG_ATTR) ICEEndpointInfo<ICEEndpointInfo> *underlying;
@property(nonatomic, assign) ICEInt timeout;
@property(nonatomic, assign) BOOL compress;

-(id) init:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress;
+(id) endpointInfo;
+(id) endpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @protocol ICEEndpoint <NSObject>
-(NSMutableString*) toString;
-(ICEEndpointInfo<ICEEndpointInfo>*) getInfo;
@end

ICE_API @protocol ICEIPEndpointInfo <ICEEndpointInfo>
@end

ICE_API @interface ICEIPEndpointInfo : ICEEndpointInfo
{
    NSString *host;
    ICEInt port;
    NSString *sourceAddress;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *host;
@property(nonatomic, assign) ICEInt port;
@property(nonatomic, ICE_STRONG_ATTR) NSString *sourceAddress;

-(id) init;
-(id) init:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress host:(NSString*)host port:(ICEInt)port sourceAddress:(NSString*)sourceAddress;
+(id) ipEndpointInfo;
+(id) ipEndpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress host:(NSString*)host port:(ICEInt)port sourceAddress:(NSString*)sourceAddress;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @protocol ICETCPEndpointInfo <ICEIPEndpointInfo>
@end

ICE_API @interface ICETCPEndpointInfo : ICEIPEndpointInfo
+(id) tcpEndpointInfo;
+(id) tcpEndpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress host:(NSString*)host port:(ICEInt)port sourceAddress:(NSString*)sourceAddress;
// This class also overrides copyWithZone:
@end

ICE_API @protocol ICEUDPEndpointInfo <ICEIPEndpointInfo>
@end

ICE_API @interface ICEUDPEndpointInfo : ICEIPEndpointInfo
{
    NSString *mcastInterface;
    ICEInt mcastTtl;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *mcastInterface;
@property(nonatomic, assign) ICEInt mcastTtl;

-(id) init;
-(id) init:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress host:(NSString*)host port:(ICEInt)port sourceAddress:(NSString*)sourceAddress mcastInterface:(NSString*)mcastInterface mcastTtl:(ICEInt)mcastTtl;
+(id) udpEndpointInfo;
+(id) udpEndpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress host:(NSString*)host port:(ICEInt)port sourceAddress:(NSString*)sourceAddress mcastInterface:(NSString*)mcastInterface mcastTtl:(ICEInt)mcastTtl;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @protocol ICEWSEndpointInfo <ICEEndpointInfo>
@end

ICE_API @interface ICEWSEndpointInfo : ICEEndpointInfo
{
    NSString *resource;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *resource;

-(id) init;
-(id) init:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress resource:(NSString*)resource;
+(id) wsEndpointInfo;
+(id) wsEndpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress resource:(NSString*)resource;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @protocol ICEOpaqueEndpointInfo <ICEEndpointInfo>
@end

ICE_API @interface ICEOpaqueEndpointInfo : ICEEndpointInfo
{
    ICEEncodingVersion *rawEncoding;
    ICEByteSeq *rawBytes;
}

@property(nonatomic, ICE_STRONG_ATTR) ICEEncodingVersion *rawEncoding;
@property(nonatomic, ICE_STRONG_ATTR) ICEByteSeq *rawBytes;

-(id) init;
-(id) init:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress rawEncoding:(ICEEncodingVersion*)rawEncoding rawBytes:(ICEByteSeq*)rawBytes;
+(id) opaqueEndpointInfo;
+(id) opaqueEndpointInfo:(ICEEndpointInfo<ICEEndpointInfo>*)underlying timeout:(ICEInt)timeout compress:(BOOL)compress rawEncoding:(ICEEncodingVersion*)rawEncoding rawBytes:(ICEByteSeq*)rawBytes;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end
