// Copyright (c) ZeroC, Inc.
#import "LocalObject.h"

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @interface ICEEndpoint : ICELocalObject
- (NSString*)toString;
- (id)getInfo;
- (bool)isEqual:(ICEEndpoint* _Nullable)endpoint;
@end

// TODO: revise function signatures to be proper ObjC.

ICEIMPL_API @protocol ICEEndpointInfoFactory
+ (id)createTCPEndpointInfo:(BOOL)compress
                       host:(NSString*)host
                       port:(int32_t)port
              sourceAddress:(NSString*)sourceAddress
                       type:(int16_t)type
                     secure:(BOOL)secure;

+ (id)createUDPEndpointInfo:(BOOL)compress
                       host:(NSString*)host
                       port:(int32_t)port
              sourceAddress:(NSString*)sourceAddress
             mcastInterface:(NSString*)mcastInterface
                   mcastTtl:(int32_t)mcastTtl;

+ (id)createWSEndpointInfo:(id)underlying resource:(NSString*)resource;

+ (id)createSSLEndpointInfo:(id)underlying;

+ (id)createIAPEndpointInfo:(BOOL)compress
               manufacturer:(NSString*)manufacturer
                modelNumber:(NSString*)modelNumber
                       name:(NSString*)name
                   protocol:(NSString*)protocol
                       type:(int16_t)type
                     secure:(BOOL)secure;

+ (id)createOpaqueEndpointInfo:(int16_t)type
                 encodingMajor:(UInt8)encodingMajor
                 encodingMinor:(UInt8)encodingMinor
                      rawBytes:(NSData*)rawBytes;
@end

#ifdef __cplusplus

@interface ICEEndpoint ()
@property(nonatomic, readonly) std::shared_ptr<Ice::Endpoint> endpoint;
+ (id)createEndpointInfo:(std::shared_ptr<Ice::EndpointInfo>)infoPtr NS_RETURNS_RETAINED;
@end

#endif

NS_ASSUME_NONNULL_END
