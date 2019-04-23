//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import "LocalObject.h"

@class ICECommunicator;
@class ICEObjectPrx;
@class ICEEndpoint;
@class ICEInputStream;
@class ICEConnection;
@class ICERuntimeException;
@protocol ICEBlobjectFacade;

NS_ASSUME_NONNULL_BEGIN

@interface ICEObjectAdapter: ICELocalObject
-(NSString*) getName;
-(ICECommunicator*) getCommunicator;
-(BOOL) activate:(NSError* _Nullable * _Nullable)error;
-(void) hold;
-(void) waitForHold;
-(void) deactivate;
-(void) waitForDeactivate;
-(BOOL) isDeactivated;
-(void) destroy;
-(nullable ICEObjectPrx*) createProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error NS_SWIFT_NAME(createProxy(name:category:));
-(nullable ICEObjectPrx*) createDirectProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error NS_SWIFT_NAME(createDirectProxy(name:category:));
-(nullable ICEObjectPrx*) createIndirectProxy:(NSString*)name category:(NSString*)category error:(NSError* _Nullable * _Nullable)error NS_SWIFT_NAME(createIndirectProxy(name:category:));
-(void) setLocator:(ICEObjectPrx* _Nullable)locator;
-(nullable ICEObjectPrx*) getLocator;
-(NSArray<ICEEndpoint*>*) getEndpoints;
-(BOOL) refreshPublishedEndpoints:(NSError* _Nullable * _Nullable)error;
-(NSArray<ICEEndpoint*>*) getPublishedEndpoints;
-(BOOL) setPublishedEndpoints:(NSArray<ICEEndpoint*>*)newEndpoints error:(NSError* _Nullable * _Nullable)error;

-(void) registerDefaultServant:(id<ICEBlobjectFacade>)facade NS_SWIFT_NAME(registerDefaultServant(_:));
@end

#ifdef __cplusplus

@interface ICEObjectAdapter()
@property (nonatomic, readonly) std::shared_ptr<Ice::ObjectAdapter> objectAdapter;
-(nullable instancetype) initWithCppObjectAdapter:(std::shared_ptr<Ice::ObjectAdapter>)objectAdapter;
@end

#endif

NS_ASSUME_NONNULL_END
