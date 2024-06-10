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
+ (NSError*)endpointParseException:(NSString*)str file:(NSString*)file line:(int32_t)line;
+ (NSError*)endpointSelectionTypeParseException:(NSString*)str file:(NSString*)file line:(int32_t)line;
+ (NSError*)versionParseException:(NSString*)str file:(NSString*)file line:(int32_t)line;
+ (NSError*)identityParseException:(NSString*)str file:(NSString*)file line:(int32_t)line;
+ (NSError*)proxyParseException:(NSString*)str file:(NSString*)file line:(int32_t)line;
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
+ (NSError*)badMagicException:(NSString*)reason badMagic:(NSData*)badMagic file:(NSString*)file line:(int32_t)line;
+ (NSError*)unsupportedProtocolException:(NSString*)reason
                                badMajor:(uint8_t)badMajor
                                badMinor:(uint8_t)badMinor
                          supportedMajor:(uint8_t)supportedMajor
                          supportedMinor:(uint8_t)supportedMinor
                                    file:(NSString*)file
                                    line:(int32_t)line;
+ (NSError*)unsupportedEncodingException:(NSString*)reason
                                badMajor:(uint8_t)badMajor
                                badMinor:(uint8_t)badMinor
                          supportedMajor:(uint8_t)supportedMajor
                          supportedMinor:(uint8_t)supportedMinor
                                    file:(NSString*)file
                                    line:(int32_t)line;
+ (NSError*)unknownMessageException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)connectionNotValidatedException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)unknownReplyStatusException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)closeConnectionException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)connectionManuallyClosedException:(BOOL)graceful file:(NSString*)file line:(int32_t)line;
+ (NSError*)illegalMessageSizeException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)compressionException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)datagramLimitException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
// ProtocolException/MarshalException
+ (NSError*)proxyUnmarshalException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)unmarshalOutOfBoundsException:(NSString*)reason
                                     file:(NSString*)file
                                     line:(int32_t)line NS_SWIFT_NAME(unmarshalOutOfBoundsException(_:file:line:));
+ (NSError*)noValueFactoryException:(NSString*)reason type:(NSString*)type file:(NSString*)file line:(int32_t)line;
+ (NSError*)unexpectedObjectException:(NSString*)reason
                                 type:(NSString*)type
                         expectedType:(NSString*)expectedType
                                 file:(NSString*)file
                                 line:(int32_t)line;
+ (NSError*)memoryLimitException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)encapsulationException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)marshalException:(NSString*)reason file:(NSString*)file line:(int32_t)line;
+ (NSError*)protocolException:(NSString*)reason file:(NSString*)file line:(int32_t)line;

// For generic std::exception
+ (NSError*)runtimeError:(NSString*)message;
@end

NS_ASSUME_NONNULL_END
