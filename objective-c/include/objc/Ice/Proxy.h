//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#import <objc/Ice/Config.h>

#import <objc/Ice/Format.h>  // For ICEFormatType
#import <objc/Ice/Current.h> // For ICEOperationMode
#import <objc/Ice/EndpointTypes.h> // For ICEEndpointSelectionType
#import <objc/Ice/EndpointF.h> // For ICEEndpointSeq

#import <Foundation/NSProxy.h>

//
// Forward declarations.
//
@class ICEObjectPrx;
@class ICEException;
@protocol ICECommunicator;
@protocol ICERouterPrx;
@protocol ICELocatorPrx;
@protocol ICEOutputStream;
@protocol ICEInputStream;

//
// Marshal/Unmarshall callbacks
//
typedef void (^ICEMarshalCB)(id<ICEOutputStream>);
typedef void (^ICEUnmarshalCB)(id<ICEInputStream>, BOOL);

ICE_API @protocol ICEAsyncResult <NSObject>

-(void) cancel;

-(id<ICECommunicator>) getCommunicator;
-(id<ICEConnection>) getConnection;
-(id<ICEObjectPrx>) getProxy;

-(BOOL) isCompleted;
-(void) waitForCompleted;

-(BOOL) isSent;
-(void) waitForSent;

-(void) throwLocalException;

-(BOOL) sentSynchronously;
-(NSString*) getOperation;
@end

ICE_API @protocol ICEObjectPrx <NSObject, NSCopying>

-(NSComparisonResult) compareIdentity:(id<ICEObjectPrx>)aProxy;
-(NSComparisonResult) compareIdentityAndFacet:(id<ICEObjectPrx>)aProxy;

-(id<ICECommunicator>) ice_getCommunicator;
-(NSMutableString*) ice_toString;
-(BOOL) ice_isA:(NSString*)typeId;
-(BOOL) ice_isA:(NSString*)typeId context:(ICEContext*)context;
-(id<ICEAsyncResult>) begin_ice_isA:(NSString*)typeId;
-(id<ICEAsyncResult>) begin_ice_isA:(NSString*)typeId context:(ICEContext*)context;
-(id<ICEAsyncResult>) begin_ice_isA:(NSString*)typeId response:(void(^)(BOOL))response exception:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_ice_isA:(NSString*)typeId context:(ICEContext*)context response:(void(^)(BOOL))response exception:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_ice_isA:(NSString*)typeId response:(void(^)(BOOL))response exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(id<ICEAsyncResult>) begin_ice_isA:(NSString*)typeId context:(ICEContext*)context response:(void(^)(BOOL))response exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(BOOL) end_ice_isA:(id<ICEAsyncResult>)result;
-(void) ice_ping;
-(void) ice_ping:(ICEContext*)context;
-(id<ICEAsyncResult>) begin_ice_ping;
-(id<ICEAsyncResult>) begin_ice_ping:(ICEContext*)context;
-(id<ICEAsyncResult>) begin_ice_ping:(void(^)(void))response exception:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_ice_ping:(ICEContext*)context response:(void(^)(void))response exception:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_ice_ping:(void(^)(void))response exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(id<ICEAsyncResult>) begin_ice_ping:(ICEContext*)context response:(void(^)(void))response exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(void) end_ice_ping:(id<ICEAsyncResult>)result;
-(NSMutableArray*) ice_ids;
-(NSMutableArray*) ice_ids:(ICEContext*)context;
-(id<ICEAsyncResult>) begin_ice_ids;
-(id<ICEAsyncResult>) begin_ice_ids:(ICEContext*)context;
-(id<ICEAsyncResult>) begin_ice_ids:(void(^)(NSArray*))response exception:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_ice_ids:(ICEContext*)context response:(void(^)(NSArray*))response exception:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_ice_ids:(void(^)(NSArray*))response exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(id<ICEAsyncResult>) begin_ice_ids:(ICEContext*)context response:(void(^)(NSArray*))response exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(NSMutableArray*) end_ice_ids:(id<ICEAsyncResult>)result;
-(NSMutableString*) ice_id;
-(NSMutableString*) ice_id:(ICEContext*)context;
-(id<ICEAsyncResult>) begin_ice_id;
-(id<ICEAsyncResult>) begin_ice_id:(ICEContext*)context;
-(id<ICEAsyncResult>) begin_ice_id:(void(^)(NSString*))response exception:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_ice_id:(ICEContext*)context response:(void(^)(NSString*))response exception:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_ice_id:(void(^)(NSString*))response exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(id<ICEAsyncResult>) begin_ice_id:(ICEContext*)context response:(void(^)(NSString*))response exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(NSMutableString*) end_ice_id:(id<ICEAsyncResult>)result;
-(BOOL) ice_invoke:(NSString*)operation mode:(ICEOperationMode)mode inEncaps:(NSData*)inEncaps outEncaps:(NSMutableData**)outEncaps;
-(BOOL) ice_invoke:(NSString*)operation mode:(ICEOperationMode)mode inEncaps:(NSData*)inEncaps outEncaps:(NSMutableData**)outEncaps context:(ICEContext*)context;
-(id<ICEAsyncResult>) begin_ice_invoke:(NSString*)operation mode:(ICEOperationMode)mode inEncaps:(NSData*)inEncaps;
-(id<ICEAsyncResult>) begin_ice_invoke:(NSString*)operation mode:(ICEOperationMode)mode inEncaps:(NSData*)inEncaps context :(ICEContext*)context;
-(id<ICEAsyncResult>) begin_ice_invoke:(NSString*)operation mode:(ICEOperationMode)mode inEncaps:(NSData*)inEncaps response:(void(^)(BOOL, NSMutableData*))response exception:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_ice_invoke:(NSString*)operation mode:(ICEOperationMode)mode inEncaps:(NSData*)inEncaps  context:(ICEContext*)context response:(void(^)(BOOL, NSMutableData*))response exception:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_ice_invoke:(NSString*)operation mode:(ICEOperationMode)mode inEncaps:(NSData*)inEncaps response:(void(^)(BOOL, NSMutableData*))response exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(id<ICEAsyncResult>) begin_ice_invoke:(NSString*)operation mode:(ICEOperationMode)mode inEncaps:(NSData*)inEncaps  context:(ICEContext*)context response:(void(^)(BOOL, NSMutableData*))response exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(BOOL) end_ice_invoke:(NSMutableData**)outEncaps result:(id<ICEAsyncResult>)result;
-(ICEIdentity*) ice_getIdentity;
-(id) ice_identity:(ICEIdentity*)identity;
-(ICEMutableContext*) ice_getContext;
-(id) ice_context:(ICEContext*)context;
-(NSMutableString*) ice_getFacet;
-(id) ice_facet:(NSString*)facet;
-(NSMutableString*) ice_getAdapterId;
-(id) ice_adapterId:(NSString*)adapterId;
-(ICEMutableEndpointSeq*) ice_getEndpoints;
-(id) ice_endpoints:(ICEEndpointSeq*)endpoints;
-(ICEInt) ice_getLocatorCacheTimeout;
-(id) ice_locatorCacheTimeout:(ICEInt)timeout;
-(BOOL) ice_isConnectionCached;
-(id) ice_connectionCached:(BOOL)cached;
-(ICEEndpointSelectionType) ice_getEndpointSelection;
-(id) ice_endpointSelection:(ICEEndpointSelectionType)type;
-(BOOL) ice_isSecure;
-(id) ice_secure:(BOOL)secure;
-(ICEEncodingVersion*) ice_getEncodingVersion;
-(id) ice_encodingVersion:(ICEEncodingVersion*)encoding;
-(BOOL) ice_isPreferSecure;
-(id) ice_preferSecure:(BOOL)preferSecure;
-(id<ICERouterPrx>) ice_getRouter;
-(id) ice_router:(id<ICERouterPrx>)router;
-(id<ICELocatorPrx>) ice_getLocator;
-(id) ice_locator:(id<ICELocatorPrx>)locator;
-(BOOL) ice_isCollocationOptimized;
-(id) ice_collocationOptimized:(BOOL)collocOptimized;
-(ICEInt) ice_getInvocationTimeout;
-(id) ice_invocationTimeout:(ICEInt)timeout;
-(id) ice_twoway;
-(BOOL) ice_isTwoway;
-(id) ice_oneway;
-(BOOL) ice_isOneway;
-(id) ice_batchOneway;
-(BOOL) ice_isBatchOneway;
-(id) ice_datagram;
-(BOOL) ice_isDatagram;
-(id) ice_batchDatagram;
-(BOOL) ice_isBatchDatagram;
-(id) ice_compress:(BOOL)compress;
-(id) ice_getCompress;
-(id) ice_timeout:(int)timeout;
-(id) ice_getTimeout;
-(id) ice_fixed:(id<ICEConnection>)connection;
-(id) ice_connectionId:(NSString*)connectionId;
-(id<ICEConnection>) ice_getConnection;
-(id<ICEAsyncResult>) begin_ice_getConnection;
-(id<ICEAsyncResult>) begin_ice_getConnection:(void(^)(id<ICEConnection>))response exception:(void(^)(ICEException*))exception;
-(id<ICEConnection>) end_ice_getConnection:(id<ICEAsyncResult>)result;
-(id<ICEConnection>) ice_getCachedConnection;
-(void) ice_flushBatchRequests;
-(id<ICEAsyncResult>) begin_ice_flushBatchRequests;
-(id<ICEAsyncResult>) begin_ice_flushBatchRequests:(void(^)(ICEException*))exception;
-(id<ICEAsyncResult>) begin_ice_flushBatchRequests:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent;
-(void) end_ice_flushBatchRequests:(id<ICEAsyncResult>)result;
@end

ICE_API @interface ICEObjectPrx : NSObject<ICEObjectPrx>
{
    void* objectPrx_;
    id<ICECommunicator> communicator_;
}
+(id) uncheckedCast:(id<ICEObjectPrx>)proxy;
+(id) uncheckedCast:(id<ICEObjectPrx>)proxy facet:(NSString*)facet;
+(id) checkedCast:(id<ICEObjectPrx>)proxy;
+(id) checkedCast:(id<ICEObjectPrx>)proxy facet:(NSString*)facet;
+(id) checkedCast:(id<ICEObjectPrx>)proxy context:(ICEContext*)context;
+(id) checkedCast:(id<ICEObjectPrx>)proxy facet:(NSString*)facet context:(ICEContext*)context;
+(NSString*) ice_staticId;

+(Protocol*) iceProtocol;
-(id<ICEOutputStream>) iceCreateOutputStream;
-(void) iceInvoke:(NSString*)operation mode:(ICEOperationMode)mode format:(ICEFormatType)format marshal:(ICEMarshalCB)marshal
        unmarshal:(ICEUnmarshalCB)unmarshal context:(ICEContext*)context;
-(id<ICEAsyncResult>) iceI_begin_invoke:(NSString*)operation mode:(ICEOperationMode)mode format:(ICEFormatType)format marshal:(ICEMarshalCB)marshal
                            returnsData:(BOOL)returnsData context:(ICEContext*)context;
-(id<ICEAsyncResult>) iceI_begin_invoke:(NSString*)operation mode:(ICEOperationMode)mode format:(ICEFormatType)format marshal:(ICEMarshalCB)marshal
                               response:(void(^)(void))response
                              exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent
                          context:(ICEContext*)context;
-(id<ICEAsyncResult>) iceI_begin_invoke:(NSString*)operation mode:(ICEOperationMode)mode format:(ICEFormatType)format marshal:(ICEMarshalCB)marshal
                              completed:(void(^)(id<ICEInputStream>, BOOL))completed
                               response:(BOOL)response exception:(void(^)(ICEException*))exception sent:(void(^)(BOOL))sent
                                context:(ICEContext*)context;
-(void)iceI_end_invoke:(NSString*)operation unmarshal:(ICEUnmarshalCB)unmarshal result:(id<ICEAsyncResult>)result;
@end
