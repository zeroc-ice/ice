// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

class LocalExceptionFactory: ICELocalExceptionFactory {

    static func initializationException(_ reason: String) -> Error {
        return InitializationException(reason: reason)
    }

    static func pluginInitializationException(_ reason: String) -> Error {
        return PluginInitializationException(reason: reason)
    }

    static func collocationOptimizationException() -> Error {
        return CollocationOptimizationException()
    }

    static func alreadyRegisteredException(_ kindOfObject: String, id: String) -> Error {
        return AlreadyRegisteredException(kindOfObject: kindOfObject, id: id)
    }

    static func notRegisteredException(_ kindOfObject: String, id: String) -> Error {
        return NotRegisteredException(kindOfObject: kindOfObject, id: id)
    }

    static func twowayOnlyException(_ operation: String) -> Error {
        return TwowayOnlyException(operation: operation)
    }

    static func cloneNotImplementedException() -> Error {
        return CloneNotImplementedException()
    }

    static func versionMismatchException() -> Error {
        return VersionMismatchException()
    }

    static func communicatorDestroyedException() -> Error {
        return CommunicatorDestroyedException()
    }

    static func objectAdapterDeactivatedException(_ name: String) -> Error {
        return ObjectAdapterDeactivatedException(name: name)
    }

    static func objectAdapterIdInUseException(_ id: String) -> Error {
        return ObjectAdapterIdInUseException(id: id)
    }

    static func noEndpointException(_ proxy: String) -> Error {
        return NoEndpointException(proxy: proxy)
    }

    static func endpointParseException(_ str: String) -> Error {
        return EndpointParseException(str: str)
    }

    static func endpointSelectionTypeParseException(_ str: String) -> Error {
        return EndpointSelectionTypeParseException(str: str)
    }

    static func versionParseException(_ str: String) -> Error {
        return VersionParseException(str: str)
    }

    static func identityParseException(_ str: String) -> Error {
        return IdentityParseException(str: str)
    }

    static func proxyParseException(_ str: String) -> Error {
        return ProxyParseException(str: str)
    }

    static func illegalIdentityException(_ name: String, category: String) -> Error {
        return IllegalIdentityException(id: Identity(name: name, category: category))
    }

    static func illegalServantException(_ reason: String) -> Error {
        return IllegalServantException(reason: reason)
    }

    static func dNSException(_ error: Int32, host: String) -> Error {
        return DNSException(error: error, host: host)
    }

    static func operationInterruptedException() -> Error {
        return OperationInterruptedException()
    }

    static func invocationCanceledException() -> Error {
        return InvocationCanceledException()
    }

    static func featureNotSupportedException(_ unsupportedFeature: String) -> Error {
        return FeatureNotSupportedException(unsupportedFeature: unsupportedFeature)
    }

    static func fixedProxyException() -> Error {
        return FixedProxyException()
    }

    static func responseSentException() -> Error {
        return ResponseSentException()
    }

    static func securityException(_ reason: String) -> Error {
        return SecurityException(reason: reason)
    }

    static func localException() -> Error {
        return LocalException()
    }

    static func unknownLocalException(_ unknown: String) -> Error {
        return UnknownLocalException(unknown: unknown)
    }

    static func unknownUserException(_ unknown: String) -> Error {
        return UnknownUserException(unknown: unknown)
    }

    static func unknownException(_ unknown: String) -> Error {
        return UnknownException(unknown: unknown)
    }

    static func objectNotExistException(_ name: String, category: String, facet: String, operation: String) -> Error {
        return ObjectNotExistException(id: Identity(name: name, category: category), facet: facet, operation: operation)
    }

    static func facetNotExistException(_ name: String, category: String, facet: String, operation: String) -> Error {
        return FacetNotExistException(id: Identity(name: name, category: category), facet: facet, operation: operation)
    }

    static func operationNotExistException(_ name: String, category: String, facet: String, operation: String) -> Error {
        return OperationNotExistException(id: Identity(name: name, category: category), facet: facet, operation: operation)
    }

    static func requestFailedException(_ name: String, category: String, facet: String, operation: String) -> Error {
        return RequestFailedException(id: Identity(name: name, category: category), facet: facet, operation: operation)
    }

    static func connectionRefusedException(_ error: Int32) -> Error {
        return ConnectionRefusedException(error: error)
    }

    static func fileException(_ error: Int32, path: String) -> Error {
        return FileException(error: error, path: path)
    }

    static func connectFailedException(_ error: Int32) -> Error {
        return ConnectionRefusedException(error: error)
    }

    static func connectionLostException(_ error: Int32) -> Error {
        return ConnectionLostException(error: error)
    }

    static func socketException(_ error: Int32) -> Error {
        return SocketException(error: error)
    }

    static func syscallException(_ error: Int32) -> Error {
        return SyscallException(error: error)
    }

    static func connectTimeoutException() -> Error {
        return ConnectTimeoutException()
    }

    static func closeTimeoutException() -> Error {
        return CloseTimeoutException()
    }

    static func connectionTimeoutException() -> Error {
        return ConnectionTimeoutException()
    }

    static func invocationTimeoutException() -> Error {
        return InvocationTimeoutException()
    }

    static func timeoutException() -> Error {
        return TimeoutException()
    }

    static func badMagicException(_ reason: String, badMagic: [NSNumber]) -> Error {
        //TODO
        return BadMagicException(reason: reason, badMagic: badMagic as! [UInt8])
    }

    static func unsupportedProtocolException(_ reason: String, badMajor: UInt8, badMinor: UInt8, supportedMajor: UInt8, supportedMinor: UInt8) -> Error {
        return UnsupportedProtocolException(reason: reason,
                                            bad: ProtocolVersion(major: badMajor, minor: badMinor),
                                            supported: ProtocolVersion(major: supportedMajor, minor: supportedMinor))
    }

    static func unsupportedEncodingException(_ reason: String, badMajor: UInt8, badMinor: UInt8, supportedMajor: UInt8, supportedMinor: UInt8) -> Error {
        return UnsupportedEncodingException(reason: reason,
                                            bad: EncodingVersion(major: badMajor, minor: badMinor),
                                            supported: EncodingVersion(major: supportedMajor, minor: supportedMinor))
    }

    static func unknownMessageException(_ reason: String) -> Error {
        return UnknownMessageException(reason: reason)
    }

    static func connectionNotValidatedException(_ reason: String) -> Error {
        return ConnectionNotValidatedException(reason: reason)
    }

    static func unknownRequestIdException(_ reason: String) -> Error {
        return UnknownRequestIdException(reason: reason)
    }

    static func unknownReplyStatusException(_ reason: String) -> Error {
        return UnknownReplyStatusException(reason: reason)
    }

    static func closeConnectionException(_ reason: String) -> Error {
        return CloseConnectionException(reason: reason)
    }

    static func connectionManuallyClosedException(_ graceful: Bool) -> Error {
        return ConnectionManuallyClosedException(graceful: graceful)
    }

    static func illegalMessageSizeException(_ reason: String) -> Error {
        return IllegalMessageSizeException(reason: reason)
    }

    static func compressionException(_ reason: String) -> Error {
        return CompressionException(reason: reason)
    }

    static func datagramLimitException(_ reason: String) -> Error {
        return DatagramLimitException(reason: reason)
    }

    static func proxyUnmarshalException(_ reason: String) -> Error {
        return ProxyUnmarshalException(reason: reason)
    }

    static func unmarshalOutofBoundsException(_ reason: String) -> Error {
        return UnmarshalOutOfBoundsException(reason: reason)
    }

    static func noValueFactoryException(_ reason: String, type: String) -> Error {
        return NoValueFactoryException(reason: reason, type: type)
    }

    static func unexpectedObjectException(_ reason: String, type: String, expectedType: String) -> Error {
        return UnexpectedObjectException(reason: reason, type: type, expectedType: expectedType)
    }

    static func memoryLimitException(_ reason: String) -> Error {
        return MemoryLimitException(reason: reason)
    }

    static func stringConversionException(_ reason: String) -> Error {
        return StringConversionException(reason: reason)
    }

    static func encapsulationException(_ reason: String) -> Error {
        return EncapsulationException(reason: reason)
    }

    static func marshalException(_ reason: String) -> Error {
        return MarshalException(reason: reason)
    }

    static func protocolException(_ reason: String) -> Error {
        return ProtocolException(reason: reason)
    }

}
