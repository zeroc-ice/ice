// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Config.h>

//
// Forward declarations
//
@protocol ICEObjectPrx;
@protocol ICERouterPrx;
@protocol ICELocatorPrx;
@protocol ICELogger;
@protocol ICEProperties;
@protocol ICEObjectAdapter;
@protocol ICEObjectFactory;
@protocol ICEAsyncResult;
@protocol ICEImplicitContext;
@protocol ICEObject;
@class ICEIdentity;
@class ICEException;

@protocol ICECommunicator <NSObject>
-(void) destroy;
-(void) shutdown;
-(void) waitForShutdown;
-(BOOL) isShutdown;
-(id<ICEObjectPrx>) stringToProxy:(NSString*)str;
-(NSString*)proxyToString:(id<ICEObjectPrx>)obj;
-(id<ICEObjectPrx>)propertyToProxy:(NSString*)property;
-(NSMutableDictionary*)proxyToProperty:(id<ICEObjectPrx>)prx property:(NSString*)property;
-(ICEIdentity*) stringToIdentity:(NSString*)str;
-(NSString*) identityToString:(ICEIdentity*)ident;
-(id<ICEObjectAdapter>) createObjectAdapter:(NSString*)name;
-(id<ICEObjectAdapter>) createObjectAdapterWithEndpoints:(NSString*)name endpoints:(NSString*)endpoints;
-(id<ICEObjectAdapter>) createObjectAdapterWithRouter:(NSString*)name router:(id<ICERouterPrx>)rtr;
-(void) addObjectFactory:(id<ICEObjectFactory>)factory sliceId:(NSString*)sliceId;
-(id<ICEObjectFactory>) findObjectFactory:(NSString*)sliceId;
-(id<ICEImplicitContext>) getImplicitContext;
-(id<ICEProperties>) getProperties;
-(id<ICELogger>) getLogger;
//-(ICEStats*) getStats;
-(id<ICERouterPrx>) getDefaultRouter;
-(void) setDefaultRouter:(id<ICERouterPrx>)rtr;
-(id<ICELocatorPrx>) getDefaultLocator;
-(void) setDefaultLocator:(id<ICELocatorPrx>)loc;
//-(PluginManager*) getPluginManager;
-(void) flushBatchRequests;
-(id<ICEAsyncResult>) begin_flushBatchRequests;
-(id<ICEAsyncResult>) begin_flushBatchRequests:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_flushBatchRequests:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(void) end_flushBatchRequests:(id<ICEAsyncResult>)result;
-(id<ICEObjectPrx>) getAdmin;
//void addAdminFacet(Object servant, NSString* facet);
//Object removeAdminFacet(NSString* facet);
-(id<ICEObject>) findAdminFacet:(NSString*)facet;

@end
