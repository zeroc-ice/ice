// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice Touch is licensed to you under the terms described in the
// ICE_TOUCH_LICENSE file included in this distribution.
//
// **********************************************************************

#import <objc/Ice/Config.h>

#import <objc/Ice/Endpoint.h>

//
// Forward declarations.
//
@protocol ICEObjectPrx;
@protocol ICELocatorPrx;
@protocol ICECommunicator;
@class ICEIdentity;
@class ICEObject;

@protocol ICEObjectAdapter <NSObject>
-(NSString*) getName;
-(id<ICECommunicator>) getCommunicator;
-(void) activate;
-(void) hold;
-(void) waitForHold;
-(void) deactivate;
-(void) waitForDeactivate;
-(BOOL) isDeactivated;
-(void) destroy;
-(id<ICEObjectPrx>) add:(ICEObject*)servant identity:(ICEIdentity*)ident;
-(id<ICEObjectPrx>) addFacet:(ICEObject*)servant identity:(ICEIdentity*)ident facet:(NSString*)facet;
-(id<ICEObjectPrx>) addWithUUID:(ICEObject*)servant;
-(id<ICEObjectPrx>) addFacetWithUUID:(ICEObject*)servant facet:(NSString*)facet;
-(void) addDefaultServant:(ICEObject*)servant category:(NSString*)category;
-(ICEObject*) remove:(ICEIdentity*)ident;
-(ICEObject*) removeFacet:(ICEIdentity*)ident facet:(NSString*)facet;
-(NSDictionary*) removeAllFacets:(ICEIdentity*)ident;
-(ICEObject*) find:(ICEIdentity*)ident;
-(ICEObject*) findFacet:(ICEIdentity*)ident facet:(NSString*)facet;
-(NSDictionary*) findAllFacets:(ICEIdentity*)ident;
-(ICEObject*) findByProxy:(id<ICEObjectPrx>)proxy;
-(ICEObject*) findDefaultServant:(NSString*)category;
-(id<ICEObjectPrx>) createProxy:(ICEIdentity*)ident;
-(id<ICEObjectPrx>) createDirectProxy:(ICEIdentity*)ident;
-(id<ICEObjectPrx>) createIndirectProxy:(ICEIdentity*)ident;
-(void) setLocator:(id<ICELocatorPrx>)loc;
-(void) refreshPublishedEndpoints;
-(ICEEndpointSeq*) getEndpoints;
-(ICEEndpointSeq*) getPublishedEndpoints;
@end
