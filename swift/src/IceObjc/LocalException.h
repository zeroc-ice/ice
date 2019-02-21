// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

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

NS_ASSUME_NONNULL_END
