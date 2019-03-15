// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import "IceObjcLocalObject.h"

@class ICEObjectPrx;
@class ICEImplicitContext;
@class ICEProperties;
@protocol ICELoggerProtocol;

NS_ASSUME_NONNULL_BEGIN

@interface ICECommunicator : ICELocalObject
-(void) destroy;
-(void) shutdown;
-(void) waitForShutdown;
-(bool) isShutdown;
-(nullable id) stringToProxy:(NSString*)str error:(NSError**)error;
-(nullable NSString*) proxyToString:(ICEObjectPrx*)prx error:(NSError**)error;
-(nullable id) propertyToProxy:(NSString*)property error:(NSError**)error
NS_SWIFT_NAME(propertyToProxy(property:));
-(nullable NSDictionary<NSString*, NSString*>*) proxyToProperty:(ICEObjectPrx*)prx property:(NSString*)property error:(NSError**)error;
//-(nullable ObjectAdapterI*) createObjectAdapter:(NSString*)name error:(NSError**)error;
//-(nullable ObjectAdapterI*) createObjectAdapterWithEndpoints:(NSString*)name endpoints:(NSString*)endpoints error:(NSError**)error;
//-(nullable ObjectAdapterI*) createObjectAdapterWithRouter:(NSString*)name router:(ObjectPrxObjc*)router error:(NSError**)error;
-(ICEImplicitContext*) getImplicitContext;
-(id<ICELoggerProtocol>) getLogger;
-(nullable ICEObjectPrx*) getDefaultRouter;
-(BOOL) setDefaultRouter:(ICEObjectPrx* _Nullable)router error:(NSError**)error;
-(nullable ICEObjectPrx*) getDefaultLocator;
-(BOOL) setDefaultLocator:(ICEObjectPrx* _Nullable)locator error:(NSError**)error;
-(BOOL) flushBatchRequests:(uint8_t)compress error:(NSError**)error;

-(ICEProperties*) getProperties;

// DefaultsAndOverrides
-(void) getDefaultEncoding:(nonnull uint8_t*)major minor:(nonnull uint8_t*)minor
    NS_SWIFT_NAME(getDefaultEncoding(major:minor:));
-(uint8_t) getDefaultFormat;

@end

#ifdef __cplusplus

@interface ICECommunicator()
@property (nonatomic, readonly) std::shared_ptr<Ice::Communicator> communicator;
-(instancetype) initWithCppCommunicator:(std::shared_ptr<Ice::Communicator>)communicator;
@end

#endif

NS_ASSUME_NONNULL_END
