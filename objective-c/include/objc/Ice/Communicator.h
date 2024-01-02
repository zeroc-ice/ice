//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `Communicator.ice'

#import <objc/Ice/Config.h>
#import <objc/Ice/Proxy.h>
#import <objc/Ice/Current.h>
#import <objc/Ice/Object.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>
#import <objc/Ice/Exception.h>
#import <objc/Ice/LoggerF.h>
#import <objc/Ice/InstrumentationF.h>
#import <objc/Ice/ObjectAdapterF.h>
#import <objc/Ice/ObjectFactory.h>
#import <objc/Ice/ValueFactory.h>
#import <objc/Ice/Router.h>
#import <objc/Ice/Locator.h>
#import <objc/Ice/PluginF.h>
#import <objc/Ice/ImplicitContextF.h>
#import <objc/Ice/Current.h>
#import <objc/Ice/Properties.h>
#import <objc/Ice/FacetMap.h>
#import <objc/Ice/Connection.h>

#ifndef ICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

@protocol ICECommunicator;

ICE_API @protocol ICECommunicator <NSObject>
-(void) destroy;
-(void) shutdown;
-(void) waitForShutdown;
-(BOOL) isShutdown;
-(id<ICEObjectPrx>) stringToProxy:(NSString*)str;
-(NSMutableString*) proxyToString:(id<ICEObjectPrx>)obj;
-(id<ICEObjectPrx>) propertyToProxy:(NSString*)property;
-(ICEMutablePropertyDict*) proxyToProperty:(id<ICEObjectPrx>)proxy property:(NSString*)property;
-(ICEIdentity*) stringToIdentity:(NSString*)str ICE_DEPRECATED_API("stringToIdentity() is deprecated, use the static stringToIdentity() method instead.");
-(NSMutableString*) identityToString:(ICEIdentity*)ident;
-(id<ICEObjectAdapter>) createObjectAdapter:(NSString*)name;
-(id<ICEObjectAdapter>) createObjectAdapterWithEndpoints:(NSString*)name endpoints:(NSString*)endpoints;
-(id<ICEObjectAdapter>) createObjectAdapterWithRouter:(NSString*)name router:(id<ICERouterPrx>)rtr;
-(void) addObjectFactory:(id<ICEObjectFactory>)factory sliceId:(NSString*)id_ ICE_DEPRECATED_API("addObjectFactory() is deprecated, use ValueFactoryManager::add() instead.");
-(id<ICEObjectFactory>) findObjectFactory:(NSString*)id_ ICE_DEPRECATED_API("findObjectFactory() is deprecated, use ValueFactoryManager::find() instead.");
-(id<ICEImplicitContext>) getImplicitContext;
-(id<ICEProperties>) getProperties;
-(id<ICELogger>) getLogger;
-(id<ICEINSTRUMENTATIONCommunicatorObserver>) getObserver;
-(id<ICERouterPrx>) getDefaultRouter;
-(void) setDefaultRouter:(id<ICERouterPrx>)rtr;
-(id<ICELocatorPrx>) getDefaultLocator;
-(void) setDefaultLocator:(id<ICELocatorPrx>)loc;
-(id<ICEPluginManager>) getPluginManager;
-(id<ICEValueFactoryManager>) getValueFactoryManager;
-(void) flushBatchRequests:(ICECompressBatch)compress;
-(id<ICEAsyncResult>) begin_flushBatchRequests:(ICECompressBatch)compress;
-(id<ICEAsyncResult>) begin_flushBatchRequests:(ICECompressBatch)compress exception:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_flushBatchRequests:(ICECompressBatch)compress exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(void) end_flushBatchRequests:(id<ICEAsyncResult>)result;
-(id<ICEObjectPrx>) createAdmin:(id<ICEObjectAdapter>)adminAdapter adminId:(ICEIdentity*)adminId;
-(id<ICEObjectPrx>) getAdmin;
-(void) addAdminFacet:(ICEObject*)servant facet:(NSString*)facet;
-(ICEObject*) removeAdminFacet:(NSString*)facet;
-(ICEObject*) findAdminFacet:(NSString*)facet;
-(ICEMutableFacetMap*) findAllAdminFacets;
@end

typedef enum : ICEInt
{
    ICEUnicode,
    ICEASCII,
    ICECompat
} ICEToStringMode;
