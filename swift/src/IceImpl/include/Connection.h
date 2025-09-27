// Copyright (c) ZeroC, Inc.
#import "LocalObject.h"

@class ICEObjectPrx;
@class ICEEndpoint;
@class ICEObjectAdapter;

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @interface ICEConnection : ICELocalObject
- (void)abort;
- (void)close:(void (^)(NSError* _Nullable error))completionHandler; // auto-mapped to Swift 'func close() async throws'
- (nullable ICEObjectPrx*)createProxy:(NSString*)name
                             category:(NSString*)category
                                error:(NSError* _Nullable* _Nullable)error;
- (BOOL)setAdapter:(ICEObjectAdapter* _Nullable)oa error:(NSError* _Nullable* _Nullable)error;
- (nullable ICEObjectAdapter*)getAdapter;
- (ICEEndpoint*)getEndpoint;
- (void)flushBatchRequests:(uint8_t)compress exception:(void (^)(NSError*))exception sent:(void (^)(bool))sent;
- (BOOL)setCloseCallback:(nullable void (^)(ICEConnection*))callback error:(NSError* _Nullable* _Nullable)error;
- (void)disableInactivityCheck;

- (NSString*)type;
- (NSString*)toString;
- (nullable id)getInfo:(NSError* _Nullable* _Nullable)error;
- (BOOL)setBufferSize:(int32_t)rcvSize sndSize:(int32_t)sndSize error:(NSError* _Nullable* _Nullable)error;
- (BOOL)throwException:(NSError* _Nullable* _Nullable)error;
@end

// TODO: revise function signatures to be proper ObjC.
ICEIMPL_API @protocol ICEConnectionInfoFactory
+ (id)createTCPConnectionInfo:(BOOL)incoming
                  adapterName:(NSString*)adapterName
                 connectionId:(NSString*)connectionId
                 localAddress:(NSString*)localAddress
                    localPort:(int32_t)localPort
                remoteAddress:(NSString*)remoteAddress
                   remotePort:(int32_t)remotePort
                      rcvSize:(int32_t)rcvSize
                      sndSize:(int32_t)sndSize;

+ (id)createUDPConnectionInfo:(BOOL)incoming
                  adapterName:(NSString*)adapterName
                 connectionId:(NSString*)connectionId
                 localAddress:(NSString*)localAddress
                    localPort:(int32_t)localPort
                remoteAddress:(NSString*)remoteAddress
                   remotePort:(int32_t)remotePort
                 mcastAddress:(NSString*)mcastAddress
                    mcastPort:(int32_t)mcastPort
                      rcvSize:(int32_t)rcvSize
                      sndSize:(int32_t)sndSize;

+ (id)createWSConnectionInfo:(id)underlying headers:(NSDictionary<NSString*, NSString*>*)headers;

+ (id)createSSLConnectionInfo:(id)underlying peerCertificate:(SecCertificateRef)peerCertificate;

+ (id)createIAPConnectionInfo:(BOOL)incoming
                  adapterName:(NSString*)adapterName
                 connectionId:(NSString*)connectionId
                         name:(NSString*)name
                 manufacturer:(NSString*)manufacturer
                  modelNumber:(NSString*)modelNumber
             firmwareRevision:(NSString*)firmwareRevision
             hardwareRevision:(NSString*)hardwareRevision
                     protocol:(NSString*)protocol;
@end

#ifdef __cplusplus

id createConnectionInfo(std::shared_ptr<Ice::ConnectionInfo>);

@interface ICEConnection ()
@property(nonatomic, readonly) std::shared_ptr<Ice::Connection> connection;
@end

#endif

NS_ASSUME_NONNULL_END
