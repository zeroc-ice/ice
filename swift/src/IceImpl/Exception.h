// Copyright (c) ZeroC, Inc.
#import "Config.h"
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

ICEIMPL_API @protocol ICEExceptionFactory
+ (NSError*)initializationException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)pluginInitializationException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)alreadyRegisteredException:(NSString*)kindOfObject id:(NSString*)id file:(NSString*)file line:(int32_t)line;
+ (NSError*)notRegisteredException:(NSString*)kindOfObject id:(NSString*)id file:(NSString*)file line:(int32_t)line;
+ (NSError*)twowayOnlyException:(NSString*)operation file:(NSString*)file line:(int32_t)line;
+ (NSError*)communicatorDestroyedException:(NSString*)file line:(int32_t)line;
+ (NSError*)objectAdapterDeactivatedException:(NSString*)name file:(NSString*)file line:(int32_t)line;
+ (NSError*)objectAdapterIdInUseException:(NSString*)id
                                     file:(NSString*)file
                                     line:(int32_t)line NS_SWIFT_NAME(objectAdapterIdInUseException(_:file:line:));
+ (NSError*)noEndpointException:(NSString*)proxy file:(NSString*)file line:(int32_t)line;
+ (NSError*)parseException:(NSString*)str file:(NSString*)file line:(int32_t)line;
+ (NSError*)illegalIdentityException:(NSString*)file line:(int32_t)line;
+ (NSError*)illegalServantException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)dNSException:(int32_t)error host:(NSString*)host file:(NSString*)file line:(int32_t)line;
+ (NSError*)invocationCanceledException:(NSString*)file line:(int32_t)line;
+ (NSError*)featureNotSupportedException:(NSString*)unsupportedFeature file:(NSString*)file line:(int32_t)line;
+ (NSError*)fixedProxyException:(NSString*)file line:(int32_t)line;
+ (NSError*)securityException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)localException:(NSString*)file line:(int32_t)line;

// UnknownException
+ (NSError*)unknownLocalException:(NSString*)unknown file:(NSString*)file line:(int32_t)line;
+ (NSError*)unknownUserException:(NSString*)unknown file:(NSString*)file line:(int32_t)line;
+ (NSError*)unknownException:(NSString*)unknown file:(NSString*)file line:(int32_t)line;

// RequestFailedException
+ (NSError*)objectNotExistException:(NSString*)name
                           category:(NSString*)category
                              facet:(NSString*)facet
                          operation:(NSString*)operation
                               file:(NSString*)file
                               line:(int32_t)line;
+ (NSError*)facetNotExistException:(NSString*)name
                          category:(NSString*)category
                             facet:(NSString*)facet
                         operation:(NSString*)operation
                              file:(NSString*)file
                              line:(int32_t)line;
+ (NSError*)operationNotExistException:(NSString*)name
                              category:(NSString*)category
                                 facet:(NSString*)facet
                             operation:(NSString*)operation
                                  file:(NSString*)file
                                  line:(int32_t)line;
+ (NSError*)requestFailedException:(NSString*)name
                          category:(NSString*)category
                             facet:(NSString*)facet
                         operation:(NSString*)operation
                              file:(NSString*)file
                              line:(int32_t)line;

// SyscallException
+ (NSError*)connectionRefusedException:(int32_t)error
                                  file:(NSString*)file
                                  line:(int32_t)line; // ConnectFailedException: SyscallException
+ (NSError*)fileException:(int32_t)error path:(NSString*)path file:(NSString*)file line:(int32_t)line;
+ (NSError*)connectFailedException:(int32_t)error file:(NSString*)file line:(int32_t)line;
+ (NSError*)connectionLostException:(int32_t)error file:(NSString*)file line:(int32_t)line;
+ (NSError*)socketException:(int32_t)error file:(NSString*)file line:(int32_t)line;
+ (NSError*)syscallException:(int32_t)error file:(NSString*)file line:(int32_t)line;

// ConnectionIdleException
+ (NSError*)connectionIdleException:(NSString*)file line:(int32_t)line;

// TimeoutException
+ (NSError*)connectTimeoutException:(NSString*)file line:(int32_t)line;
+ (NSError*)closeTimeoutException:(NSString*)file line:(int32_t)line;
+ (NSError*)invocationTimeoutException:(NSString*)file line:(int32_t)line;
+ (NSError*)timeoutException:(NSString*)file line:(int32_t)line;

// ProtocolException
+ (NSError*)closeConnectionException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)connectionManuallyClosedException:(BOOL)graceful file:(NSString*)file line:(int32_t)line; // not a protocol exception
+ (NSError*)datagramLimitException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
// ProtocolException/MarshalException
+ (NSError*)marshalException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)protocolException:(NSString*)reason file:(NSString*)file line:(int32_t)line;

// For generic std::exception
+ (NSError*)runtimeError:(NSString*)message;
@end

NS_ASSUME_NONNULL_END
