// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BRIDGING_HEADER_H
#define ICE_BRIDGING_HEADER_H

#import <Foundation/Foundation.h>

# define ICE_SWIFT_DESIGNATED_INITIALIZER NS_DESIGNATED_INITIALIZER
# define ICE_SWIFT_UNAVAILABLE(msg) __attribute__((unavailable(msg)))

NS_ASSUME_NONNULL_BEGIN

@class ICEObjectPrx;
@class ICEImplicitContext;
@class ICEProperties;
@class ICEEndpoint;
@class ICEConnection;
@class ICEInputStream;
@protocol ICELoggerProtocol;

//
// LocalObject
//
@interface ICELocalObject : NSObject
@property (weak, nonatomic, nullable) id swiftRef;
-(instancetype) init ICE_SWIFT_UNAVAILABLE("");
@end

//
// Communicator
//
@interface ICECommunicator : ICELocalObject
-(void) destroy;
-(void) shutdown;
-(void) waitForShutdown;
-(bool) isShutdown;
-(nullable id) stringToProxy:(NSString*)str error:(NSError**)error;
-(nullable NSString*) proxyToString:(ICEObjectPrx*)prx error:(NSError**)error;
-(nullable id) propertyToProxy:(NSString*)property error:(NSError**)error;
-(nullable NSDictionary<NSString*, NSString*>*) proxyToProperty:(ICEObjectPrx*)prx property:(NSString*)property error:(NSError**)error;
//-(nullable ObjectAdapterI*) createObjectAdapter:(NSString*)name error:(NSError**)error;
//-(nullable ObjectAdapterI*) createObjectAdapterWithEndpoints:(NSString*)name endpoints:(NSString*)endpoints error:(NSError**)error;
//-(nullable ObjectAdapterI*) createObjectAdapterWithRouter:(NSString*)name router:(ObjectPrxObjc*)router error:(NSError**)error;
-(ICEImplicitContext*) getImplicitContext;
-(id<ICELoggerProtocol>) getLogger;
@end

//
// Properties
//
@interface ICEProperties : ICELocalObject
-(nonnull NSString*) getProperty:(NSString*)key;
-(nonnull NSString*) getPropertyWithDefault:(NSString*)key value:(NSString*)value;
-(int32_t) getPropertyAsInt:(NSString*)key;
-(int32_t)  getPropertyAsIntWithDefault:(NSString*)key value:(int32_t)value;
-(nonnull NSArray<NSString*>*) getPropertyAsList:(NSString* _Nonnull)key;
-(nonnull NSArray<NSString*>*) getPropertyAsListWithDefault:(NSString* _Nonnull)key value:(NSArray<NSString*>* _Nonnull)value;
-(nonnull NSDictionary<NSString*, NSString*>*) getPropertiesForPrefix:(NSString* _Nonnull)prefix NS_SWIFT_NAME(getPropertiesForPrefix(_:));
-(BOOL) setProperty:(NSString*)key value:(NSString*)value error:(NSError**)error;
-(nonnull NSArray<NSString*>*) getCommandLineOptions;
-(nullable NSArray<NSString*>*) parseCommandLineOptions:(NSString*)prefix options:(NSArray<NSString*>*)options error:(NSError* _Nullable * _Nullable)error;
-(nullable NSArray<NSString*>*) parseIceCommandLineOptions:(NSArray<NSString*>*)options error:(NSError**)error;
-(BOOL) load:(NSString*)file error:(NSError* _Nullable * _Nullable)error;
-(nonnull ICEProperties*) clone;
@end

@protocol ICEOutputStreamHelper
-(void) copy:(const void*)start count:(NSNumber*)count;
@end

@interface ICEInputStream : NSObject
-(instancetype) init ICE_SWIFT_UNAVAILABLE("ICEInputStream cannot be initialized from Swift");
-(void*) data;
-(size_t) size;
@end

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
-(nullable id) ice_getConnection:(NSError* _Nullable * _Nullable)error; //Either NSNull or ICEConnection
-(nullable ICEConnection*) ice_getCachedConnection;
-(BOOL) ice_flushBatchRequests:(NSError* _Nullable * _Nullable)error;

// Either ICEObjectPrx or NSNull
+(nullable id) iceRead:(void*)start size:(NSInteger)size communicator:(ICECommunicator*)communicator error:(NSError* _Nullable * _Nullable)error;
-(void) iceWrite:(id<ICEOutputStreamHelper>)os;

-(nullable ICEInputStream*) iceInvoke:(NSString* _Nonnull)op
                              mode:(NSInteger)mode
                          inParams:(void* _Null_unspecified)inParams
                            inSize:(NSInteger)inSize
                           context:(NSDictionary* _Nullable)context
                       returnValue:(bool*)returnValue
                             error:(NSError* _Nullable * _Nullable)error;
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

@interface ICEEndpointInfo: ICELocalObject
-(int16_t) getType;
-(BOOL) getDatagram;
-(BOOL) getSecure;
@end

@protocol ICEEndpointInfoFactory
+(id) createIPEndpointInfo:(ICEEndpointInfo*)handle
                underlying:(id)underlying
                   timeout:(int32_t)timeout
                  compress:(BOOL)compress
                      host:(NSString*)host
                      port:(int32_t)port
             sourceAddress:(NSString*)sourceAddress;

+(id) createTCPEndpointInfo:(ICEEndpointInfo*)handle
                 underlying:(id)underlying
                    timeout:(int32_t)timeout
                   compress:(BOOL)compress
                       host:(NSString*)host
                       port:(int32_t)port
              sourceAddress:(NSString*)sourceAddress;

+(id) createUDPEndpointInfo:(ICEEndpointInfo*)handle
                 underlying:(id)underlying
                    timeout:(int32_t)timeout
                   compress:(BOOL)compress
                       host:(NSString*)host
                       port:(int32_t)port
              sourceAddress:(NSString*)sourceAddress
             mcastInterface:(NSString*)mcastInterface
                   mcastTtl:(int32_t)mcastTtl;

+(id) createWSEndpointInfo:(ICEEndpointInfo*)handle
                underlying:(id)underlying
                   timeout:(int32_t)timeout
                  compress:(BOOL)compress
                  resource:(NSString*)resource;

+(id) createOpaqueEndpointInfo:(ICEEndpointInfo*)handle
                    underlying:(id)underlying
                       timeout:(int32_t)timeout
                      compress:(BOOL)compress
                 encodingMajor:(UInt8)encodingMajor
                 encodingMinor:(UInt8)encodingMinor
                       rawBytes:(NSArray<NSNumber*>*)byteSeq;

+(id) createSSLEndpointInfo:(ICEEndpointInfo*)handle
                underlying:(id)underlying
                   timeout:(int32_t)timeout
                   compress:(BOOL)compress;
@end

@interface ICEEndpoint: ICELocalObject
-(NSString*) toString;
-(id) getInfo;
@end

@interface ICEImplicitContext: ICELocalObject
-(NSDictionary<NSString*, NSString*>*) getContext;
-(void) setContext:(NSDictionary<NSString*, NSString*>*)context;
-(bool) containsKey:(NSString*)string;
-(NSString*) get:(NSString*)key;
-(NSString*) put:(NSString*)key value:(NSString*)value;
-(NSString*) remove:(NSString*)key;
@end

@protocol ICELocalExceptionFactory
+(NSError*) initializationException:(NSString*)reason;
+(NSError*) pluginInitializationException:(NSString*)reason;
+(NSError*) collocationOptimizationException;
+(NSError*) alreadyRegisteredException:(NSString*)kindOfObject id:(NSString*)id;
+(NSError*) notRegisteredException:(NSString*)kindOfObject id:(NSString*)id;
+(NSError*) twowayOnlyException:(NSString*)operation;
+(NSError*) cloneNotImplementedException;
+(NSError*) versionMismatchException;
+(NSError*) communicatorDestroyedException;
+(NSError*) objectAdapterDeactivatedException:(NSString*)name;
+(NSError*) objectAdapterIdInUseException:(NSString*)id NS_SWIFT_NAME(objectAdapterIdInUseException(_:));
+(NSError*) noEndpointException:(NSString*)proxy;
+(NSError*) endpointParseException:(NSString*)str;
+(NSError*) endpointSelectionTypeParseException:(NSString*)str;
+(NSError*) versionParseException:(NSString*)str;
+(NSError*) identityParseException:(NSString*)str;
+(NSError*) proxyParseException:(NSString*)str;
+(NSError*) illegalIdentityException:(NSString*)name category:(NSString*)category;
+(NSError*) illegalServantException:(NSString*)reason;
+(NSError*) dNSException:(int32_t)error host:(NSString*)host;
+(NSError*) operationInterruptedException;
+(NSError*) invocationCanceledException;
+(NSError*) featureNotSupportedException:(NSString*)unsupportedFeature;
+(NSError*) fixedProxyException;
+(NSError*) responseSentException;
+(NSError*) securityException:(NSString*)reason;
+(NSError*) localException;

// UnknownException
+(NSError*) unknownLocalException:(NSString*)unknown;
+(NSError*) unknownUserException:(NSString*)unknown;
+(NSError*) unknownException:(NSString*)unknown;

// RequestFailedException
+(NSError*) objectNotExistException:(NSString*)name category:(NSString*)category facet:(NSString*)facet operation:(NSString*)operation;
+(NSError*) facetNotExistException:(NSString*)name category:(NSString*)category facet:(NSString*)facet operation:(NSString*)operation;
+(NSError*) operationNotExistException:(NSString*)name category:(NSString*)category facet:(NSString*)facet operation:(NSString*)operation;
+(NSError*) requestFailedException:(NSString*)name category:(NSString*)category facet:(NSString*)facet operation:(NSString*)operation;

// SyscallException
+(NSError*) connectionRefusedException:(int32_t)error; // ConnectFailedException: SyscallException
+(NSError*) fileException:(int32_t)error path:(NSString*)path;
+(NSError*) connectFailedException:(int32_t)error;
+(NSError*) connectionLostException:(int32_t)error;
+(NSError*) socketException:(int32_t)error;
+(NSError*) syscallException:(int32_t)error;

// TimeoutException
+(NSError*) connectTimeoutException;
+(NSError*) closeTimeoutException;
+(NSError*) connectionTimeoutException;
+(NSError*) invocationTimeoutException;
+(NSError*) timeoutException;

// ProtocolException
+(NSError*) badMagicException:(NSString*)reason badMagic:(NSArray<NSNumber*>*)badMagic;
+(NSError*) unsupportedProtocolException:(NSString*)reason badMajor:(uint8_t)badMajor badMinor:(uint8_t)badMinor supportedMajor:(uint8_t)supportedMajor supportedMinor:(uint8_t)supportedMinor;
+(NSError*) unsupportedEncodingException:(NSString*)reason badMajor:(uint8_t)badMajor badMinor:(uint8_t)badMinor supportedMajor:(uint8_t)supportedMajor supportedMinor:(uint8_t)supportedMinor;
+(NSError*) unknownMessageException:(NSString*)reason;
+(NSError*) connectionNotValidatedException:(NSString*)reason;
+(NSError*) unknownRequestIdException:(NSString*)reason;
+(NSError*) unknownReplyStatusException:(NSString*)reason;
+(NSError*) closeConnectionException:(NSString*)reason;
+(NSError*) connectionManuallyClosedException:(BOOL)graceful;
+(NSError*) illegalMessageSizeException:(NSString*)reason;
+(NSError*) compressionException:(NSString*)reason;
+(NSError*) datagramLimitException:(NSString*)reason;
// ProtocolException/MarshalException
+(NSError*) proxyUnmarshalException:(NSString*)reason;
+(NSError*) unmarshalOutOfBoundsException:(NSString*)reason NS_SWIFT_NAME(unmarshalOutofBoundsException(_:));
+(NSError*) noValueFactoryException:(NSString*)reason type:(NSString*)type;
+(NSError*) unexpectedObjectException:(NSString*)reason type:(NSString*)type expectedType:(NSString*)expectedType;
+(NSError*) memoryLimitException:(NSString*)reason;
+(NSError*) stringConversionException:(NSString*)reason;
+(NSError*) encapsulationException:(NSString*)reason;
+(NSError*) marshalException:(NSString*)reason;
+(NSError*) protocolException:(NSString*)reason;
@end

@protocol ICELoggerProtocol
-(void) print:(NSString*)message NS_SWIFT_NAME(print(message:));
-(void) trace:(NSString*)category message:(NSString*)message NS_SWIFT_NAME(trace(category:message:));
-(void) warning:(NSString*)message NS_SWIFT_NAME(warning(message:));
-(void) error:(NSString*)message NS_SWIFT_NAME(error(message:));
-(NSString*) getPrefix;
-(id) cloneWithPrefix:(NSString*)prefix NS_SWIFT_NAME(cloneWithPrefix(prefix:));
@end

@interface ICELogger: ICELocalObject<ICELoggerProtocol>
-(void) print:(NSString*)message;
-(void) trace:(NSString*)category message:(NSString*)message;
-(void) warning:(NSString*)message;
-(void) error:(NSString*)message;
-(NSString*) getPrefix;
-(id) cloneWithPrefix:(NSString*)prefix;
@end

//
// Static utility methods
//
@interface ICEUtil: NSObject
@property (class, nonatomic, readonly) Class<ICELocalExceptionFactory> localExceptionFactory;
@property (class, nonatomic, readonly) Class<ICEConnectionInfoFactory> connectionInfoFactory;
@property (class, nonatomic, readonly) Class<ICEEndpointInfoFactory> endpointInfoFactory;

//This method should only be called once to guarenteed thread safety
+(BOOL) registerFactories:(Class<ICELocalExceptionFactory>)localException
             connectionInfo:(Class<ICEConnectionInfoFactory>)connectionInfo
             endpointInfo:(Class<ICEEndpointInfoFactory>)endpointInfo
NS_SWIFT_NAME(registerFactories(localException:connectionInfo:endpointInfo:));

+(nullable ICECommunicator*) initialize:(NSArray*)swiftArgs
                             properties:(ICEProperties* _Null_unspecified)properties
                          logger:(id<ICELoggerProtocol>)logger
                                  error:(NSError* _Nullable * _Nullable)error;

+(nullable ICEProperties*) createProperties:(NSArray* _Nullable)swiftArgs
                                 defaults:(ICEProperties* _Nullable)defaults
                                  remArgs:(NSArray* _Null_unspecified * _Null_unspecified)remArgs
                                    error:(NSError* _Nullable * _Nullable)error;
+(BOOL) stringToIdentity:(NSString*)str
                    name:(NSString* __strong _Nonnull *  _Nonnull)name
                category:(NSString* __strong  _Nonnull *  _Nonnull)category
                   error:(NSError* _Nullable * _Nullable)error NS_SWIFT_NAME(stringToIdentity(str:name:category:));

+(nullable NSString*) identityToString:(NSString*)name
                              category:(NSString*)category
                                 error:(NSError* _Nullable * _Nullable)error NS_SWIFT_NAME(identityToString(name:category:));

+(void) currentEncoding:(UInt8*)major minor:(UInt8*)minor NS_SWIFT_NAME(currentEncoding(major:minor:));

@end

NS_ASSUME_NONNULL_END

#endif
