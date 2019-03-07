// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcLocalObject.h"

@class ICEEndpoint;

NS_ASSUME_NONNULL_BEGIN

@interface ICEConnection: ICELocalObject
-(void) close:(uint8_t)mode;
-(nullable id) createProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error;
//-(BOOL) setAdapter:(ObjectAdapterI*)oa error:(NSError* _Nullable * _Nullable)error;
//-(nullable ObjectAdapterI*) getAdapter;
-(ICEEndpoint*) getEndpoint;
-(BOOL) flushBatchRequests:(uint8_t)compress error:(NSError* _Nullable * _Nullable)error;
-(BOOL) setCloseCallback:(nullable void (^)(ICEConnection*))callback  error:(NSError* _Nullable * _Nullable)error;
-(BOOL) setHeartbeatCallback:(nullable void (^)(ICEConnection*))callback error:(NSError* _Nullable * _Nullable)error;
-(BOOL) heartbeat:(NSError* _Nullable * _Nullable)error;
-(BOOL) setACM:(id)timeout close:(id)close heartbeat:(id)heartbeat error:(NSError* _Nullable * _Nullable)error;
-(void) getACM:(int32_t*)timeout close:(uint8_t*)close heartbeat:(uint8_t*)heartbeat;
-(NSString*) type;
-(int32_t) timeout;
-(NSString*) toString;
-(nullable id) getInfo:(NSError* _Nullable * _Nullable)error;
-(BOOL) setBufferSize:(int32_t)rcvSize sndSize:(int32_t)sndSize error:(NSError* _Nullable * _Nullable)error;
-(BOOL) throwException:(NSError* _Nullable * _Nullable)error;
@end

@interface ICEConnectionInfo: NSObject
@end

@protocol ICEConnectionInfoFactory
+(id) createIPConnectionInfo:(id)underlying
                    incoming:(BOOL)incoming
                 adapterName:(NSString*)adapterName
                connectionId:(NSString*)connectionId
                localAddress:(NSString*)localAddress
                   localPort:(int32_t)localPort
               remoteAddress:(NSString*)remoteAddress
                  remotePort:(int32_t)remotePort;

+(id) createTCPConnectionInfo:(id)underlying
                     incoming:(BOOL)incoming
                  adapterName:(NSString*)adapterName
                 connectionId:(NSString*)connectionId
                 localAddress:(NSString*)localAddress
                    localPort:(int32_t)localPort
                remoteAddress:(NSString*)remoteAddress
                   remotePort:(int32_t)remotePort
                      rcvSize:(int32_t)rcvSize
                      sndSize:(int32_t)sndSize;

+(id) createUDPConnectionInfo:(id)underlying
                     incoming:(BOOL)incoming
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

+(id) createWSConnectionInfo:(id)underlying
                    incoming:(BOOL)incoming
                 adapterName:(NSString*)adapterName
                connectionId:(NSString*)connectionId
                     headers:(NSDictionary<NSString*, NSString*>*)headers;

+(id) createSSLConnectionInfo:(id)underlying
                     incoming:(BOOL)incoming
                  adapterName:(NSString*)adapterName
                 connectionId:(NSString*)connectionId
                       cipher:(NSString*)cipher
                        certs:(NSArray<NSString*>*)certs
                     verified:(BOOL)verified;
@end

#ifdef __cplusplus

@interface ICEConnection()
@property (nonatomic, readonly) std::shared_ptr<Ice::Connection> connection;
-(instancetype) initWithCppConnection:(std::shared_ptr<Ice::Connection>) connection;
@end

#endif

NS_ASSUME_NONNULL_END
