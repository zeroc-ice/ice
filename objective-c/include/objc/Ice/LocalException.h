//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// Ice version 3.7.10
// Generated from file `LocalException.ice'

#import <objc/Ice/Config.h>
#import <objc/Ice/Stream.h>
#import <objc/Ice/LocalObject.h>
#import <objc/Ice/Exception.h>
#import <objc/Ice/Identity.h>
#import <objc/Ice/Version.h>
#import <objc/Ice/BuiltinSequences.h>

#ifndef ICE_API
#   if defined(ICE_STATIC_LIBS)
#       define ICE_API /**/
#   elif defined(ICE_API_EXPORTS)
#       define ICE_API ICE_DECLSPEC_EXPORT
#   else
#       define ICE_API ICE_DECLSPEC_IMPORT
#   endif
#endif

ICE_API @interface ICEInitializationException : ICELocalException
{
    NSString *reason_;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *reason_;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line reason:(NSString*)reason;
+(id) initializationException:(const char*)file line:(int)line;
+(id) initializationException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEPluginInitializationException : ICELocalException
{
    NSString *reason_;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *reason_;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line reason:(NSString*)reason;
+(id) pluginInitializationException:(const char*)file line:(int)line;
+(id) pluginInitializationException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICECollocationOptimizationException : ICELocalException
-(NSString *) ice_id;
+(id) collocationOptimizationException:(const char*)file line:(int)line;
@end

ICE_API @interface ICEAlreadyRegisteredException : ICELocalException
{
    NSString *kindOfObject;
    NSString *id_;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *kindOfObject;
@property(nonatomic, ICE_STRONG_ATTR) NSString *id_;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line kindOfObject:(NSString*)kindOfObject id:(NSString*)id_;
+(id) alreadyRegisteredException:(const char*)file line:(int)line;
+(id) alreadyRegisteredException:(const char*)file line:(int)line kindOfObject:(NSString*)kindOfObject id:(NSString*)id_;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICENotRegisteredException : ICELocalException
{
    NSString *kindOfObject;
    NSString *id_;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *kindOfObject;
@property(nonatomic, ICE_STRONG_ATTR) NSString *id_;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line kindOfObject:(NSString*)kindOfObject id:(NSString*)id_;
+(id) notRegisteredException:(const char*)file line:(int)line;
+(id) notRegisteredException:(const char*)file line:(int)line kindOfObject:(NSString*)kindOfObject id:(NSString*)id_;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICETwowayOnlyException : ICELocalException
{
    NSString *operation;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *operation;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line operation:(NSString*)operation;
+(id) twowayOnlyException:(const char*)file line:(int)line;
+(id) twowayOnlyException:(const char*)file line:(int)line operation:(NSString*)operation;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICECloneNotImplementedException : ICELocalException
-(NSString *) ice_id;
+(id) cloneNotImplementedException:(const char*)file line:(int)line;
@end

ICE_API @interface ICEUnknownException : ICELocalException
{
    NSString *unknown;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *unknown;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line unknown:(NSString*)unknown;
+(id) unknownException:(const char*)file line:(int)line;
+(id) unknownException:(const char*)file line:(int)line unknown:(NSString*)unknown;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEUnknownLocalException : ICEUnknownException
-(NSString *) ice_id;
+(id) unknownLocalException:(const char*)file line:(int)line;
+(id) unknownLocalException:(const char*)file line:(int)line unknown:(NSString*)unknown;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEUnknownUserException : ICEUnknownException
-(NSString *) ice_id;
+(id) unknownUserException:(const char*)file line:(int)line;
+(id) unknownUserException:(const char*)file line:(int)line unknown:(NSString*)unknown;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEVersionMismatchException : ICELocalException
-(NSString *) ice_id;
+(id) versionMismatchException:(const char*)file line:(int)line;
@end

ICE_API @interface ICECommunicatorDestroyedException : ICELocalException
-(NSString *) ice_id;
+(id) communicatorDestroyedException:(const char*)file line:(int)line;
@end

ICE_API @interface ICEObjectAdapterDeactivatedException : ICELocalException
{
    NSString *name_;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *name_;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line name:(NSString*)name;
+(id) objectAdapterDeactivatedException:(const char*)file line:(int)line;
+(id) objectAdapterDeactivatedException:(const char*)file line:(int)line name:(NSString*)name;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEObjectAdapterIdInUseException : ICELocalException
{
    NSString *id_;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *id_;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line id:(NSString*)id_;
+(id) objectAdapterIdInUseException:(const char*)file line:(int)line;
+(id) objectAdapterIdInUseException:(const char*)file line:(int)line id:(NSString*)id_;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICENoEndpointException : ICELocalException
{
    NSString *proxy;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *proxy;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line proxy:(NSString*)proxy;
+(id) noEndpointException:(const char*)file line:(int)line;
+(id) noEndpointException:(const char*)file line:(int)line proxy:(NSString*)proxy;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEEndpointParseException : ICELocalException
{
    NSString *str;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *str;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line str:(NSString*)str;
+(id) endpointParseException:(const char*)file line:(int)line;
+(id) endpointParseException:(const char*)file line:(int)line str:(NSString*)str;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEEndpointSelectionTypeParseException : ICELocalException
{
    NSString *str;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *str;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line str:(NSString*)str;
+(id) endpointSelectionTypeParseException:(const char*)file line:(int)line;
+(id) endpointSelectionTypeParseException:(const char*)file line:(int)line str:(NSString*)str;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEVersionParseException : ICELocalException
{
    NSString *str;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *str;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line str:(NSString*)str;
+(id) versionParseException:(const char*)file line:(int)line;
+(id) versionParseException:(const char*)file line:(int)line str:(NSString*)str;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEIdentityParseException : ICELocalException
{
    NSString *str;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *str;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line str:(NSString*)str;
+(id) identityParseException:(const char*)file line:(int)line;
+(id) identityParseException:(const char*)file line:(int)line str:(NSString*)str;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEProxyParseException : ICELocalException
{
    NSString *str;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *str;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line str:(NSString*)str;
+(id) proxyParseException:(const char*)file line:(int)line;
+(id) proxyParseException:(const char*)file line:(int)line str:(NSString*)str;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEIllegalIdentityException : ICELocalException
{
    ICEIdentity *id_;
}

@property(nonatomic, ICE_STRONG_ATTR) ICEIdentity *id_;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line id:(ICEIdentity*)id_;
+(id) illegalIdentityException:(const char*)file line:(int)line;
+(id) illegalIdentityException:(const char*)file line:(int)line id:(ICEIdentity*)id_;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEIllegalServantException : ICELocalException
{
    NSString *reason_;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *reason_;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line reason:(NSString*)reason;
+(id) illegalServantException:(const char*)file line:(int)line;
+(id) illegalServantException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICERequestFailedException : ICELocalException
{
    ICEIdentity *id_;
    NSString *facet;
    NSString *operation;
}

@property(nonatomic, ICE_STRONG_ATTR) ICEIdentity *id_;
@property(nonatomic, ICE_STRONG_ATTR) NSString *facet;
@property(nonatomic, ICE_STRONG_ATTR) NSString *operation;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line id:(ICEIdentity*)id_ facet:(NSString*)facet operation:(NSString*)operation;
+(id) requestFailedException:(const char*)file line:(int)line;
+(id) requestFailedException:(const char*)file line:(int)line id:(ICEIdentity*)id_ facet:(NSString*)facet operation:(NSString*)operation;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEObjectNotExistException : ICERequestFailedException
-(NSString *) ice_id;
+(id) objectNotExistException:(const char*)file line:(int)line;
+(id) objectNotExistException:(const char*)file line:(int)line id:(ICEIdentity*)id_ facet:(NSString*)facet operation:(NSString*)operation;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEFacetNotExistException : ICERequestFailedException
-(NSString *) ice_id;
+(id) facetNotExistException:(const char*)file line:(int)line;
+(id) facetNotExistException:(const char*)file line:(int)line id:(ICEIdentity*)id_ facet:(NSString*)facet operation:(NSString*)operation;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEOperationNotExistException : ICERequestFailedException
-(NSString *) ice_id;
+(id) operationNotExistException:(const char*)file line:(int)line;
+(id) operationNotExistException:(const char*)file line:(int)line id:(ICEIdentity*)id_ facet:(NSString*)facet operation:(NSString*)operation;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICESyscallException : ICELocalException
{
    ICEInt error;
}

@property(nonatomic, assign) ICEInt error;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line error:(ICEInt)error;
+(id) syscallException:(const char*)file line:(int)line;
+(id) syscallException:(const char*)file line:(int)line error:(ICEInt)error;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICESocketException : ICESyscallException
-(NSString *) ice_id;
+(id) socketException:(const char*)file line:(int)line;
+(id) socketException:(const char*)file line:(int)line error:(ICEInt)error;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICECFNetworkException : ICESocketException
{
    NSString *domain;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *domain;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line error:(ICEInt)error domain:(NSString*)domain;
+(id) cfNetworkException:(const char*)file line:(int)line;
+(id) cFNetworkException:(const char*)file line:(int)line ICE_DEPRECATED_API("use cfNetworkException instead");
+(id) cfNetworkException:(const char*)file line:(int)line error:(ICEInt)error domain:(NSString*)domain;
+(id) cFNetworkException:(const char*)file line:(int)line error:(ICEInt)error domain:(NSString*)domain ICE_DEPRECATED_API("use cfNetworkException instead");
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEFileException : ICESyscallException
{
    NSString *path;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *path;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line error:(ICEInt)error path:(NSString*)path;
+(id) fileException:(const char*)file line:(int)line;
+(id) fileException:(const char*)file line:(int)line error:(ICEInt)error path:(NSString*)path;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEConnectFailedException : ICESocketException
-(NSString *) ice_id;
+(id) connectFailedException:(const char*)file line:(int)line;
+(id) connectFailedException:(const char*)file line:(int)line error:(ICEInt)error;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEConnectionRefusedException : ICEConnectFailedException
-(NSString *) ice_id;
+(id) connectionRefusedException:(const char*)file line:(int)line;
+(id) connectionRefusedException:(const char*)file line:(int)line error:(ICEInt)error;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEConnectionLostException : ICESocketException
-(NSString *) ice_id;
+(id) connectionLostException:(const char*)file line:(int)line;
+(id) connectionLostException:(const char*)file line:(int)line error:(ICEInt)error;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEDNSException : ICELocalException
{
    ICEInt error;
    NSString *host;
}

@property(nonatomic, assign) ICEInt error;
@property(nonatomic, ICE_STRONG_ATTR) NSString *host;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line error:(ICEInt)error host:(NSString*)host;
+(id) dnsException:(const char*)file line:(int)line;
+(id) dNSException:(const char*)file line:(int)line ICE_DEPRECATED_API("use dnsException instead");
+(id) dnsException:(const char*)file line:(int)line error:(ICEInt)error host:(NSString*)host;
+(id) dNSException:(const char*)file line:(int)line error:(ICEInt)error host:(NSString*)host ICE_DEPRECATED_API("use dnsException instead");
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEOperationInterruptedException : ICELocalException
-(NSString *) ice_id;
+(id) operationInterruptedException:(const char*)file line:(int)line;
@end

ICE_API @interface ICETimeoutException : ICELocalException
-(NSString *) ice_id;
+(id) timeoutException:(const char*)file line:(int)line;
@end

ICE_API @interface ICEConnectTimeoutException : ICETimeoutException
-(NSString *) ice_id;
+(id) connectTimeoutException:(const char*)file line:(int)line;
@end

ICE_API @interface ICECloseTimeoutException : ICETimeoutException
-(NSString *) ice_id;
+(id) closeTimeoutException:(const char*)file line:(int)line;
@end

ICE_API @interface ICEConnectionTimeoutException : ICETimeoutException
-(NSString *) ice_id;
+(id) connectionTimeoutException:(const char*)file line:(int)line;
@end

ICE_API @interface ICEInvocationTimeoutException : ICETimeoutException
-(NSString *) ice_id;
+(id) invocationTimeoutException:(const char*)file line:(int)line;
@end

ICE_API @interface ICEInvocationCanceledException : ICELocalException
-(NSString *) ice_id;
+(id) invocationCanceledException:(const char*)file line:(int)line;
@end

ICE_API @interface ICEProtocolException : ICELocalException
{
    NSString *reason_;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *reason_;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line reason:(NSString*)reason;
+(id) protocolException:(const char*)file line:(int)line;
+(id) protocolException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEBadMagicException : ICEProtocolException
{
    ICEByteSeq *badMagic;
}

@property(nonatomic, ICE_STRONG_ATTR) ICEByteSeq *badMagic;

-(NSString *) ice_id;

-(id) init:(const char*)file line:(int)line reason:(NSString*)reason badMagic:(ICEByteSeq*)badMagic;
+(id) badMagicException:(const char*)file line:(int)line;
+(id) badMagicException:(const char*)file line:(int)line reason:(NSString*)reason badMagic:(ICEByteSeq*)badMagic;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEUnsupportedProtocolException : ICEProtocolException
{
    ICEProtocolVersion *bad;
    ICEProtocolVersion *supported;
}

@property(nonatomic, ICE_STRONG_ATTR) ICEProtocolVersion *bad;
@property(nonatomic, ICE_STRONG_ATTR) ICEProtocolVersion *supported;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line reason:(NSString*)reason bad:(ICEProtocolVersion*)bad supported:(ICEProtocolVersion*)supported;
+(id) unsupportedProtocolException:(const char*)file line:(int)line;
+(id) unsupportedProtocolException:(const char*)file line:(int)line reason:(NSString*)reason bad:(ICEProtocolVersion*)bad supported:(ICEProtocolVersion*)supported;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEUnsupportedEncodingException : ICEProtocolException
{
    ICEEncodingVersion *bad;
    ICEEncodingVersion *supported;
}

@property(nonatomic, ICE_STRONG_ATTR) ICEEncodingVersion *bad;
@property(nonatomic, ICE_STRONG_ATTR) ICEEncodingVersion *supported;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line reason:(NSString*)reason bad:(ICEEncodingVersion*)bad supported:(ICEEncodingVersion*)supported;
+(id) unsupportedEncodingException:(const char*)file line:(int)line;
+(id) unsupportedEncodingException:(const char*)file line:(int)line reason:(NSString*)reason bad:(ICEEncodingVersion*)bad supported:(ICEEncodingVersion*)supported;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEUnknownMessageException : ICEProtocolException
-(NSString *) ice_id;
+(id) unknownMessageException:(const char*)file line:(int)line;
+(id) unknownMessageException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEConnectionNotValidatedException : ICEProtocolException
-(NSString *) ice_id;
+(id) connectionNotValidatedException:(const char*)file line:(int)line;
+(id) connectionNotValidatedException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEUnknownRequestIdException : ICEProtocolException
-(NSString *) ice_id;
+(id) unknownRequestIdException:(const char*)file line:(int)line;
+(id) unknownRequestIdException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEUnknownReplyStatusException : ICEProtocolException
-(NSString *) ice_id;
+(id) unknownReplyStatusException:(const char*)file line:(int)line;
+(id) unknownReplyStatusException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICECloseConnectionException : ICEProtocolException
-(NSString *) ice_id;
+(id) closeConnectionException:(const char*)file line:(int)line;
+(id) closeConnectionException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEConnectionManuallyClosedException : ICELocalException
{
    BOOL graceful;
}

@property(nonatomic, assign) BOOL graceful;

-(NSString *) ice_id;

-(id) init:(const char*)file line:(int)line graceful:(BOOL)graceful;
+(id) connectionManuallyClosedException:(const char*)file line:(int)line;
+(id) connectionManuallyClosedException:(const char*)file line:(int)line graceful:(BOOL)graceful;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEIllegalMessageSizeException : ICEProtocolException
-(NSString *) ice_id;
+(id) illegalMessageSizeException:(const char*)file line:(int)line;
+(id) illegalMessageSizeException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICECompressionException : ICEProtocolException
-(NSString *) ice_id;
+(id) compressionException:(const char*)file line:(int)line;
+(id) compressionException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEDatagramLimitException : ICEProtocolException
-(NSString *) ice_id;
+(id) datagramLimitException:(const char*)file line:(int)line;
+(id) datagramLimitException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEMarshalException : ICEProtocolException
-(NSString *) ice_id;
+(id) marshalException:(const char*)file line:(int)line;
+(id) marshalException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEProxyUnmarshalException : ICEMarshalException
-(NSString *) ice_id;
+(id) proxyUnmarshalException:(const char*)file line:(int)line;
+(id) proxyUnmarshalException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEUnmarshalOutOfBoundsException : ICEMarshalException
-(NSString *) ice_id;
+(id) unmarshalOutOfBoundsException:(const char*)file line:(int)line;
+(id) unmarshalOutOfBoundsException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICENoValueFactoryException : ICEMarshalException
{
    NSString *type;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *type;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line reason:(NSString*)reason type:(NSString*)type;
+(id) noValueFactoryException:(const char*)file line:(int)line;
+(id) noValueFactoryException:(const char*)file line:(int)line reason:(NSString*)reason type:(NSString*)type;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEUnexpectedObjectException : ICEMarshalException
{
    NSString *type;
    NSString *expectedType;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *type;
@property(nonatomic, ICE_STRONG_ATTR) NSString *expectedType;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line reason:(NSString*)reason type:(NSString*)type expectedType:(NSString*)expectedType;
+(id) unexpectedObjectException:(const char*)file line:(int)line;
+(id) unexpectedObjectException:(const char*)file line:(int)line reason:(NSString*)reason type:(NSString*)type expectedType:(NSString*)expectedType;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEMemoryLimitException : ICEMarshalException
-(NSString *) ice_id;
+(id) memoryLimitException:(const char*)file line:(int)line;
+(id) memoryLimitException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEStringConversionException : ICEMarshalException
-(NSString *) ice_id;
+(id) stringConversionException:(const char*)file line:(int)line;
+(id) stringConversionException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEEncapsulationException : ICEMarshalException
-(NSString *) ice_id;
+(id) encapsulationException:(const char*)file line:(int)line;
+(id) encapsulationException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
@end

ICE_API @interface ICEFeatureNotSupportedException : ICELocalException
{
    NSString *unsupportedFeature;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *unsupportedFeature;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line unsupportedFeature:(NSString*)unsupportedFeature;
+(id) featureNotSupportedException:(const char*)file line:(int)line;
+(id) featureNotSupportedException:(const char*)file line:(int)line unsupportedFeature:(NSString*)unsupportedFeature;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICESecurityException : ICELocalException
{
    NSString *reason_;
}

@property(nonatomic, ICE_STRONG_ATTR) NSString *reason_;

-(NSString *) ice_id;

-(id) init;
-(id) init:(const char*)file line:(int)line reason:(NSString*)reason;
+(id) securityException:(const char*)file line:(int)line;
+(id) securityException:(const char*)file line:(int)line reason:(NSString*)reason;
// This class also overrides copyWithZone:
// This class also overrides dealloc
@end

ICE_API @interface ICEFixedProxyException : ICELocalException
-(NSString *) ice_id;
+(id) fixedProxyException:(const char*)file line:(int)line;
@end

ICE_API @interface ICEResponseSentException : ICELocalException
-(NSString *) ice_id;
+(id) responseSentException:(const char*)file line:(int)line;
@end
