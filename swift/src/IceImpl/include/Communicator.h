// Copyright (c) ZeroC, Inc.
#import "LocalObject.h"

@class ICEObjectPrx;
@class ICEImplicitContext;
@class ICEProperties;
@class ICEObjectAdapter;
@protocol ICELoggerProtocol;
@protocol ICEDispatchAdapter;

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @interface ICECommunicator : ICELocalObject
- (void)destroy;
- (void)shutdown;
- (void)waitForShutdown;
- (void)waitForShutdownAsync:(void (^)(void))completed NS_SWIFT_NAME(waitForShutdownAsync(_:));
- (bool)isShutdown;
- (nullable id)stringToProxy:(NSString*)str error:(NSError* _Nullable* _Nonnull)error NS_SWIFT_NAME(stringToProxy(str:));
;
- (nullable id)propertyToProxy:(NSString*)property error:(NSError* _Nullable* _Nonnull)error NS_SWIFT_NAME(propertyToProxy(property:));
- (nullable NSDictionary<NSString*, NSString*>*)proxyToProperty:(ICEObjectPrx*)prx
                                                       property:(NSString*)property
                                                          error:(NSError* _Nullable* _Nonnull)error
    NS_SWIFT_NAME(proxyToProperty(prx:property:));
;
- (nullable ICEObjectAdapter*)createObjectAdapter:(NSString*)name error:(NSError* _Nullable* _Nonnull)error;
- (nullable ICEObjectAdapter*)createObjectAdapterWithEndpoints:(NSString*)name
                                                     endpoints:(NSString*)endpoints
                                                         error:(NSError* _Nullable* _Nonnull)error
    NS_SWIFT_NAME(createObjectAdapterWithEndpoints(name:endpoints:));
;
- (nullable ICEObjectAdapter*)createObjectAdapterWithRouter:(NSString*)name
                                                     router:(ICEObjectPrx*)router
                                                      error:(NSError* _Nullable* _Nonnull)error
    NS_SWIFT_NAME(createObjectAdapterWithRouter(name:router:));
- (nullable ICEObjectAdapter*)getDefaultObjectAdapter;
- (void)setDefaultObjectAdapter:(ICEObjectAdapter* _Nullable)adapter;
- (nullable ICEImplicitContext*)getImplicitContext;
- (id<ICELoggerProtocol>)getLogger;
- (nullable ICEObjectPrx*)getDefaultRouter;
- (BOOL)setDefaultRouter:(ICEObjectPrx* _Nullable)router error:(NSError* _Nullable* _Nonnull)error;
- (nullable ICEObjectPrx*)getDefaultLocator;
- (BOOL)setDefaultLocator:(ICEObjectPrx* _Nullable)locator error:(NSError* _Nullable* _Nonnull)error;
- (void)flushBatchRequests:(uint8_t)compress exception:(void (^)(NSError*))exception sent:(void (^)(bool))sent;
- (nullable ICEObjectPrx*)createAdmin:(ICEObjectAdapter* _Nullable)adminAdapter
                                 name:(NSString*)name
                             category:(NSString*)category
                                error:(NSError* _Nullable* _Nonnull)error;
- (nullable id)getAdmin:(NSError* _Nullable* _Nonnull)error;
- (BOOL)addAdminFacet:(id<ICEDispatchAdapter>)servant facet:(NSString*)facet error:(NSError* _Nullable* _Nonnull)error;
- (nullable id<ICEDispatchAdapter>)removeAdminFacet:(NSString*)facet error:(NSError* _Nullable* _Nonnull)error;
- (nullable id)findAdminFacet:(NSString*)facet error:(NSError* _Nullable* _Nonnull)error;
- (nullable NSDictionary<NSString*, id<ICEDispatchAdapter>>*)findAllAdminFacets:(NSError* _Nullable* _Nonnull)error;
- (ICEProperties*)getProperties;

- (BOOL)initializePlugins:(NSError* _Nullable* _Nonnull)error;
@end

#ifdef __cplusplus

@interface ICECommunicator ()
@property(nonatomic, readonly) std::shared_ptr<Ice::Communicator> communicator;
@end

#endif

NS_ASSUME_NONNULL_END
