//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "Config.h"

@class ICEObjectPrx;
@class ICEImplicitContext;
@class ICEProperties;
@class ICEEndpoint;
@class ICEConnection;
@class ICEInputStream;
@class ICECommunicator;
@protocol ICELoggerProtocol;
@protocol ICEOutputStreamHelper;

NS_ASSUME_NONNULL_BEGIN

@interface ICEObjectPrx : NSObject
-(nonnull ICEObjectPrx*) initWithObjectPrx:(ICEObjectPrx*)prx;
-(nonnull NSString*) ice_toString;
-(nonnull ICECommunicator*) ice_getCommunicator;
-(void) ice_getIdentity:(NSString* __strong _Nonnull * _Nonnull)name
               category:(NSString* __strong _Nonnull * _Nonnull)category;
-(nullable instancetype) ice_identity:(NSString*)name
                             category:(NSString*)category
                                error:(NSError* _Nullable * _Nullable)error;
-(nonnull NSDictionary<NSString*, NSString*>*) ice_getContext;
-(nonnull instancetype) ice_context:(NSDictionary<NSString*, NSString*>*)context;
-(nonnull NSString*) ice_getFacet;
-(nonnull instancetype) ice_facet:(NSString*)facet;
-(nonnull NSString*) ice_getAdapterId;
-(nullable instancetype) ice_adapterId:(NSString*)id error:(NSError* _Nullable * _Nullable)error;
-(nonnull NSArray<ICEEndpoint*>*) ice_getEndpoints;
-(nullable instancetype) ice_endpoints:(NSArray<ICEEndpoint*>*)endpoints error:(NSError* _Nullable * _Nullable)error;
-(int32_t) ice_getLocatorCacheTimeout;
-(nullable instancetype) ice_locatorCacheTimeout:(int32_t)timeout error:(NSError* _Nullable * _Nullable)error;
-(int32_t) ice_getInvocationTimeout;
-(nullable instancetype) ice_invocationTimeout:(int32_t)timeout error:(NSError* _Nullable * _Nullable)error;
-(nonnull NSString*) ice_getConnectionId;
-(nullable instancetype) ice_connectionId:(NSString*)connectionId error:(NSError* _Nullable * _Nullable)error;
-(bool) ice_isConnectionCached;
-(nullable instancetype) ice_connectionCached:(bool)cached error:(NSError* _Nullable * _Nullable)error;
-(uint8_t) ice_getEndpointSelection;
-(nullable instancetype) ice_endpointSelection:(uint8_t)type error:(NSError* _Nullable * _Nullable)error;
-(nonnull instancetype) ice_encodingVersion:(uint8_t)major minor:(uint8_t)minor;
-(void) ice_getEncodingVersion:(uint8_t*)major minor:(uint8_t*)minor;
-(nullable ICEObjectPrx*) ice_getRouter;
-(nullable instancetype) ice_router:(ICEObjectPrx* _Nullable)router error:(NSError* _Nullable * _Nullable)error;
-(nullable ICEObjectPrx*) ice_getLocator;
-(nullable instancetype) ice_locator:(ICEObjectPrx* _Nullable)locator error:(NSError* _Nullable * _Nullable)error;
-(bool) ice_isSecure;
-(nonnull instancetype) ice_secure:(bool)b;
-(bool) ice_isPreferSecure;
-(nullable instancetype) ice_preferSecure:(bool)b error:(NSError* _Nullable * _Nullable)error;
-(bool) ice_isTwoway;
-(nonnull instancetype) ice_twoway;
-(bool) ice_isOneway;
-(nonnull instancetype) ice_oneway;
-(bool) ice_isBatchOneway;
-(nonnull instancetype) ice_batchOneway;
-(bool) ice_isDatagram;
-(nonnull instancetype) ice_datagram;
-(bool) ice_isBatchDatagram;
-(nonnull instancetype) ice_batchDatagram;
// id represents Any in Swift which we use as an Optional int32_t
-(nullable id) ice_getCompress;
-(nonnull instancetype) ice_compress:(bool)compress;
// id represents Any in Swift which we use as an Optional int32_t
-(nullable id) ice_getTimeout;
-(nullable instancetype) ice_timeout:(int32_t)timeout error:(NSError* _Nullable * _Nullable)error;
-(nullable instancetype) ice_fixed:(ICEConnection*)connection error:(NSError* _Nullable * _Nullable)error;
-(bool) ice_isFixed;
-(nullable id) ice_getConnection:(NSError* _Nullable * _Nullable)error; //Either NSNull or ICEConnection
-(void) ice_getConnectionAsync:(void (^)(ICEConnection*)) response
                     exception:(void (^)(NSError*))exception;
-(nullable ICEConnection*) ice_getCachedConnection;
-(BOOL) ice_flushBatchRequests:(NSError* _Nullable * _Nullable)error;
-(BOOL) ice_flushBatchRequestsAsync:(void (^)(NSError*))exception
                               sent:(void (^_Nullable)(bool))sent
                              error:(NSError**)error
    NS_SWIFT_NAME(ice_flushBatchRequestsAsync(exception:sent:));
-(bool) ice_isCollocationOptimized;
-(nullable instancetype) ice_collocationOptimized:(bool)collocated
                                            error:(NSError* _Nullable * _Nullable)error;

// Either ICEObjectPrx or NSNull
+(nullable id) iceRead:(NSData*)data
          communicator:(ICECommunicator*)communicator
         encodingMajor:(uint8_t)major
         encodingMinor:(uint8_t)minor
             bytesRead:(NSInteger*)bytesRead
                 error:(NSError* _Nullable * _Nullable)error;

-(void) iceWrite:(id<ICEOutputStreamHelper>)os
   encodingMajor:(uint8_t)encodingMajor
   encodingMinor:(uint8_t)encodingMinor;


// Sync invocation on oneway proxy
-(BOOL) iceOnewayInvoke:(NSString* _Nonnull)op
                   mode:(uint8_t)mode
               inParams:(NSData*)inParams
                context:(NSDictionary* _Nullable)context
                  error:(NSError* _Nullable * _Nullable)error;

-(BOOL) iceInvokeAsync:(NSString* _Nonnull)op
                  mode:(NSInteger)mode
              inParams:(NSData*)inParams
               context:(NSDictionary* _Nullable)context
              response:(void (^)(bool, const void*, NSInteger))response
             exception:(void (^)(NSError*))exception
                  sent:(void (^_Nullable)(bool))sent
                 error:(NSError* _Nullable * _Nullable)error;

-(bool) isEqual:(ICEObjectPrx* _Nullable)prx;

-(bool) proxyIdentityLess:(ICEObjectPrx* _Nullable)prx;
-(bool) proxyIdentityEqual:(ICEObjectPrx* _Nullable)prx;

-(bool) proxyIdentityAndFacetLess:(ICEObjectPrx* _Nullable)prx;
-(bool) proxyIdentityAndFacetEqual:(ICEObjectPrx* _Nullable)prx;
@end

#ifdef __cplusplus

@interface ICEObjectPrx()
@property (nonatomic, readonly) std::shared_ptr<Ice::ObjectPrx> prx;
-(nullable instancetype) initWithCppObjectPrx:(std::shared_ptr<Ice::ObjectPrx>)prx;
@end

#endif

NS_ASSUME_NONNULL_END
