//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `ObjectAdapter.ice'

#import <objc/Ice/Config.h>
#import <objc/Ice/Proxy.h>
#import <objc/Ice/Current.h>
#import <objc/Ice/Object.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>
#import <objc/Ice/Exception.h>
#import <objc/Ice/CommunicatorF.h>
#import <objc/Ice/ServantLocatorF.h>
#import <objc/Ice/Locator.h>
#import <objc/Ice/FacetMap.h>
#import <objc/Ice/Endpoint.h>

#ifndef ICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

@protocol ICEObjectAdapter;

ICE_API @protocol ICEObjectAdapter <NSObject>
-(NSMutableString*) getName;
-(id<ICECommunicator>) getCommunicator;
-(void) activate;
-(void) hold;
-(void) waitForHold;
-(void) deactivate;
-(void) waitForDeactivate;
-(BOOL) isDeactivated;
-(void) destroy;
-(id<ICEObjectPrx>) add:(ICEObject*)servant identity:(ICEIdentity*)id_;
-(id<ICEObjectPrx>) addFacet:(ICEObject*)servant identity:(ICEIdentity*)id_ facet:(NSString*)facet;
-(id<ICEObjectPrx>) addWithUUID:(ICEObject*)servant;
-(id<ICEObjectPrx>) addFacetWithUUID:(ICEObject*)servant facet:(NSString*)facet;
-(void) addDefaultServant:(ICEObject*)servant category:(NSString*)category;
-(ICEObject*) remove:(ICEIdentity*)id_;
-(ICEObject*) removeFacet:(ICEIdentity*)id_ facet:(NSString*)facet;
-(ICEMutableFacetMap*) removeAllFacets:(ICEIdentity*)id_;
-(ICEObject*) removeDefaultServant:(NSString*)category;
-(ICEObject*) find:(ICEIdentity*)id_;
-(ICEObject*) findFacet:(ICEIdentity*)id_ facet:(NSString*)facet;
-(ICEMutableFacetMap*) findAllFacets:(ICEIdentity*)id_;
-(ICEObject*) findByProxy:(id<ICEObjectPrx>)proxy;
-(void) addServantLocator:(id<ICEServantLocator>)locator category:(NSString*)category;
-(id<ICEServantLocator>) removeServantLocator:(NSString*)category;
-(id<ICEServantLocator>) findServantLocator:(NSString*)category;
-(ICEObject*) findDefaultServant:(NSString*)category;
-(id<ICEObjectPrx>) createProxy:(ICEIdentity*)id_;
-(id<ICEObjectPrx>) createDirectProxy:(ICEIdentity*)id_;
-(id<ICEObjectPrx>) createIndirectProxy:(ICEIdentity*)id_;
-(void) setLocator:(id<ICELocatorPrx>)loc;
-(id<ICELocatorPrx>) getLocator;
-(ICEMutableEndpointSeq*) getEndpoints;
-(void) refreshPublishedEndpoints;
-(ICEMutableEndpointSeq*) getPublishedEndpoints;
-(void) setPublishedEndpoints:(ICEEndpointSeq*)newEndpoints;
@end
