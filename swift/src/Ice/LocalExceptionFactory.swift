//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

import IceImpl

class ExceptionFactory: ICEExceptionFactory {
    static func initializationException(_ reason: String, file: String, line: Int32) -> Error {
        return InitializationException(reason: reason, file: file, line: line)
    }

    static func pluginInitializationException(_ reason: String, file: String, line: Int32) -> Error {
        return PluginInitializationException(reason: reason, file: file, line: line)
    }

    static func alreadyRegisteredException(
        _ kindOfObject: String, id: String, file: String, line: Int32
    ) -> Error {
        return AlreadyRegisteredException(kindOfObject: kindOfObject, id: id, file: file, line: line)
    }

    static func notRegisteredException(_ kindOfObject: String, id: String, file: String, line: Int32)
        -> Error
    {
        return NotRegisteredException(kindOfObject: kindOfObject, id: id, file: file, line: line)
    }

    static func twowayOnlyException(_ operation: String, file: String, line: Int32) -> Error {
        return TwowayOnlyException(operation: operation, file: file, line: line)
    }

    static func cloneNotImplementedException(_ file: String, line: Int32) -> Error {
        return CloneNotImplementedException(file: file, line: line)
    }

    static func versionMismatchException(_ file: String, line: Int32) -> Error {
        return VersionMismatchException(file: file, line: line)
    }

    static func communicatorDestroyedException(_ file: String, line: Int32) -> Error {
        return CommunicatorDestroyedException(file: file, line: line)
    }

    static func objectAdapterDeactivatedException(_ name: String, file: String, line: Int32) -> Error {
        return ObjectAdapterDeactivatedException(name: name, file: file, line: line)
    }

    static func objectAdapterIdInUseException(_ id: String, file: String, line: Int32) -> Error {
        return ObjectAdapterIdInUseException(id: id, file: file, line: line)
    }

    static func noEndpointException(_ proxy: String, file: String, line: Int32) -> Error {
        return NoEndpointException(proxy: proxy, file: file, line: line)
    }

    static func endpointParseException(_ str: String, file: String, line: Int32) -> Error {
        return EndpointParseException(str: str, file: file, line: line)
    }

    static func endpointSelectionTypeParseException(_ str: String, file: String, line: Int32) -> Error {
        return EndpointSelectionTypeParseException(str: str, file: file, line: line)
    }

    static func versionParseException(_ str: String, file: String, line: Int32) -> Error {
        return VersionParseException(str: str, file: file, line: line)
    }

    static func identityParseException(_ str: String, file: String, line: Int32) -> Error {
        return IdentityParseException(str: str, file: file, line: line)
    }

    static func proxyParseException(_ str: String, file: String, line: Int32) -> Error {
        return ProxyParseException(str: str, file: file, line: line)
    }

    static func illegalIdentityException(_ file: String, line: Int32) -> Error {
        return IllegalIdentityException(file: file, line: line)
    }

    static func illegalServantException(_ reason: String, file: String, line: Int32) -> Error {
        return IllegalServantException(reason: reason, file: file, line: line)
    }

    static func dNSException(_ error: Int32, host: String, file: String, line: Int32) -> Error {
        return DNSException(error: error, host: host, file: file, line: line)
    }

    static func operationInterruptedException(_ file: String, line: Int32) -> Error {
        return OperationInterruptedException(file: file, line: line)
    }

    static func invocationCanceledException(_ file: String, line: Int32) -> Error {
        return InvocationCanceledException(file: file, line: line)
    }

    static func featureNotSupportedException(_ unsupportedFeature: String, file: String, line: Int32)
        -> Error
    {
        return FeatureNotSupportedException(
            unsupportedFeature: unsupportedFeature, file: file, line: line)
    }

    static func fixedProxyException(_ file: String, line: Int32) -> Error {
        return FixedProxyException(file: file, line: line)
    }

    static func securityException(_ reason: String, file: String, line: Int32) -> Error {
        return SecurityException(reason: reason, file: file, line: line)
    }

    static func localException(_ file: String, line: Int32) -> Error {
        return LocalException(file: file, line: line)
    }

    static func unknownLocalException(_ unknown: String, file: String, line: Int32) -> Error {
        return UnknownLocalException(unknown: unknown, file: file, line: line)
    }

    static func unknownUserException(_ unknown: String, file: String, line: Int32) -> Error {
        return UnknownUserException(unknown: unknown, file: file, line: line)
    }

    static func unknownException(_ unknown: String, file: String, line: Int32) -> Error {
        return UnknownException(unknown: unknown, file: file, line: line)
    }

    static func objectNotExistException(
        _ name: String, category: String, facet: String, operation: String,
        file: String, line: Int32
    ) -> Error {
        return ObjectNotExistException(
            id: Identity(name: name, category: category), facet: facet, operation: operation,
            file: file, line: line)
    }

    static func facetNotExistException(
        _ name: String, category: String, facet: String, operation: String,
        file: String, line: Int32
    ) -> Error {
        return FacetNotExistException(
            id: Identity(name: name, category: category), facet: facet, operation: operation,
            file: file, line: line)
    }

    static func operationNotExistException(
        _ name: String,
        category: String,
        facet: String,
        operation: String, file: String, line: Int32
    ) -> Error {
        return OperationNotExistException(
            id: Identity(
                name: name,
                category: category),
            facet: facet,
            operation: operation, file: file, line: line)
    }

    static func requestFailedException(
        _ name: String, category: String, facet: String, operation: String,
        file: String, line: Int32
    ) -> Error {
        return RequestFailedException(
            id: Identity(name: name, category: category), facet: facet, operation: operation,
            file: file, line: line)
    }

    static func connectionRefusedException(_ error: Int32, file: String, line: Int32) -> Error {
        return ConnectionRefusedException(error: error, file: file, line: line)
    }

    static func fileException(_ error: Int32, path: String, file: String, line: Int32) -> Error {
        return FileException(error: error, path: path, file: file, line: line)
    }

    static func connectFailedException(_ error: Int32, file: String, line: Int32) -> Error {
        return ConnectionRefusedException(error: error, file: file, line: line)
    }

    static func connectionLostException(_ error: Int32, file: String, line: Int32) -> Error {
        return ConnectionLostException(error: error, file: file, line: line)
    }

    static func socketException(_ error: Int32, file: String, line: Int32) -> Error {
        return SocketException(error: error, file: file, line: line)
    }

    static func syscallException(_ error: Int32, file: String, line: Int32) -> Error {
        return SyscallException(error: error, file: file, line: line)
    }

    static func connectionIdleException(_ file: String, line: Int32) -> Error {
        return ConnectionIdleException(file: file, line: line)
    }

    static func connectTimeoutException(_ file: String, line: Int32) -> Error {
        return ConnectTimeoutException(file: file, line: line)
    }

    static func closeTimeoutException(_ file: String, line: Int32) -> Error {
        return CloseTimeoutException(file: file, line: line)
    }

    static func invocationTimeoutException(_ file: String, line: Int32) -> Error {
        return InvocationTimeoutException(file: file, line: line)
    }

    static func timeoutException(_ file: String, line: Int32) -> Error {
        return TimeoutException(file: file, line: line)
    }

    static func badMagicException(_ reason: String, badMagic: Data, file: String, line: Int32)
        -> Error
    {
        return BadMagicException(reason: reason, badMagic: badMagic, file: file, line: line)
    }

    static func unsupportedProtocolException(
        _ reason: String,
        badMajor: UInt8,
        badMinor: UInt8,
        supportedMajor: UInt8,
        supportedMinor: UInt8, file: String, line: Int32
    ) -> Error {
        return UnsupportedProtocolException(
            reason: reason,
            bad: ProtocolVersion(major: badMajor, minor: badMinor),
            supported: ProtocolVersion(major: supportedMajor, minor: supportedMinor),
            file: file, line: line)
    }

    static func unsupportedEncodingException(
        _ reason: String,
        badMajor: UInt8,
        badMinor: UInt8,
        supportedMajor: UInt8,
        supportedMinor: UInt8, file: String, line: Int32
    ) -> Error {
        return UnsupportedEncodingException(
            reason: reason,
            bad: EncodingVersion(major: badMajor, minor: badMinor),
            supported: EncodingVersion(major: supportedMajor, minor: supportedMinor),
            file: file, line: line)
    }

    static func unknownMessageException(_ reason: String, file: String, line: Int32) -> Error {
        return UnknownMessageException(reason: reason, file: file, line: line)
    }

    static func connectionNotValidatedException(_ reason: String, file: String, line: Int32) -> Error {
        return ConnectionNotValidatedException(reason: reason, file: file, line: line)
    }

    static func unknownRequestIdException(_ reason: String, file: String, line: Int32) -> Error {
        return UnknownRequestIdException(reason: reason, file: file, line: line)
    }

    static func unknownReplyStatusException(_ reason: String, file: String, line: Int32) -> Error {
        return UnknownReplyStatusException(reason: reason, file: file, line: line)
    }

    static func closeConnectionException(_ reason: String, file: String, line: Int32) -> Error {
        return CloseConnectionException(reason: reason, file: file, line: line)
    }

    static func connectionManuallyClosedException(_ graceful: Bool, file: String, line: Int32)
        -> Error
    {
        return ConnectionManuallyClosedException(graceful: graceful, file: file, line: line)
    }

    static func illegalMessageSizeException(_ reason: String, file: String, line: Int32) -> Error {
        return IllegalMessageSizeException(reason: reason, file: file, line: line)
    }

    static func compressionException(_ reason: String, file: String, line: Int32) -> Error {
        return CompressionException(reason: reason, file: file, line: line)
    }

    static func datagramLimitException(_ reason: String, file: String, line: Int32) -> Error {
        return DatagramLimitException(reason: reason, file: file, line: line)
    }

    static func proxyUnmarshalException(_ reason: String, file: String, line: Int32) -> Error {
        return ProxyUnmarshalException(reason: reason, file: file, line: line)
    }

    static func unmarshalOutofBoundsException(_ reason: String, file: String, line: Int32) -> Error {
        return UnmarshalOutOfBoundsException(reason: reason, file: file, line: line)
    }

    static func noValueFactoryException(_ reason: String, type: String, file: String, line: Int32)
        -> Error
    {
        return NoValueFactoryException(reason: reason, type: type, file: file, line: line)
    }

    static func unexpectedObjectException(
        _ reason: String, type: String, expectedType: String,
        file: String, line: Int32
    ) -> Error {
        return UnexpectedObjectException(
            reason: reason, type: type, expectedType: expectedType,
            file: file, line: line)
    }

    static func memoryLimitException(_ reason: String, file: String, line: Int32) -> Error {
        return MemoryLimitException(reason: reason, file: file, line: line)
    }

    static func stringConversionException(_ reason: String, file: String, line: Int32) -> Error {
        return StringConversionException(reason: reason, file: file, line: line)
    }

    static func encapsulationException(_ reason: String, file: String, line: Int32) -> Error {
        return EncapsulationException(reason: reason, file: file, line: line)
    }

    static func marshalException(_ reason: String, file: String, line: Int32) -> Error {
        return MarshalException(reason: reason, file: file, line: line)
    }

    static func protocolException(_ reason: String, file: String, line: Int32) -> Error {
        return ProtocolException(reason: reason, file: file, line: line)
    }

    static func runtimeError(_ message: String) -> Error {
        return RuntimeError(message)
    }
}
