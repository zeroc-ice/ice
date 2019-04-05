// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import IceObjc

private extension String {
    mutating func sep(_ str: String) {
        guard !str.isEmpty else {
            return
        }
        append(":\n\(str)")
    }

    mutating func nl(_ str: String) {
        guard !str.isEmpty else {
            return
        }
        append("\n\(str)")
    }

    mutating func failedRequest(_ ex: RequestFailedException) {
        let id: String = (try? identityToString(id: ex.id, mode: .Unicode)) ?? ""
        sep("identity: `\(id)'")
        nl("facet: \(ex.facet)")
        nl("operation: \(ex.operation)")
    }

    mutating func hex(_ value: UInt8) {
        append("0x")
        append(String(format: "%02X", value))
    }
}

private func socketError(_ error: Int32) -> String {
    if error == 0 {
        return "unknown error"
    }
    return ICEUtil.errorToString(error)
}

extension InitializationException {
    var _InitializationExceptionDescription: String {
        var s = String()

        s.sep("initialization exception")
        s.sep(reason)

        return s
    }
}

extension PluginInitializationException {
    var _PluginInitializationExceptionDescription: String {
        var s = String()

        s.sep("plug-in initialization failed")
        s.sep(reason)

        return s
    }
}
extension CollocationOptimizationException {
    var _CollocationOptimizationExceptionDescription: String {
        var s = String()

        s.sep("requested feature not available with collocation optimization")

        return s
    }
}
extension AlreadyRegisteredException {
    var _AlreadyRegisteredExceptionDescription: String {
        var s = String()

        s.sep("\(kindOfObject) with ice `\(id)' is already registered")

        return s
    }
}
extension NotRegisteredException {
    var _NotRegisteredExceptionDescription: String {
        var s = String()

        s.sep("no \(kindOfObject) with id `\(id)' is registered")

        return s
    }
}
extension TwowayOnlyException {
    var _TwowayOnlyExceptionDescription: String {
        var s = String()

        s.sep("operation `\(operation)' can only be invoked as a twoway request")

        return s
    }
}
extension CloneNotImplementedException {
    var _CloneNotImplementedExceptionDescription: String {
        var s = String()

        s.sep("ice_clone() must be implemented in classes derived from abstract base classes")

        return s
    }
}
extension UnknownException {
    var _UnknownExceptionDescription: String {
        var s = String()

        s.sep("unknown exception")
        s.sep(unknown)

        return s
    }
}
extension UnknownLocalException {
    var _UnknownLocalExceptionDescription: String {
        var s = String()

        s.sep("unknown local exception")
        s.sep(unknown)

        return s
    }
}
extension UnknownUserException {
    var _UnknownUserExceptionDescription: String {
        var s = String()

        s.sep("unknown user exception")
        s.sep(unknown)

        return s
    }
}
extension VersionMismatchException {
    var _VersionMismatchExceptionDescription: String {
        var s = String()

        s.sep("Ice library version mismatch")

        return s
    }
}
extension CommunicatorDestroyedException {
    var _CommunicatorDestroyedExceptionDescription: String {
        var s = String()

        s.sep("communicator object destroyed")

        return s
    }
}
extension ObjectAdapterDeactivatedException {
    var _ObjectAdapterDeactivatedExceptionDescription: String {
        var s = String()

        s.sep("object adapter `\(name)' deactivated")

        return s
    }
}
extension ObjectAdapterIdInUseException {
    var _ObjectAdapterIdInUseExceptionDescription: String {
        var s = String()

        s.sep("object adapter with id `\(id)' is already in use")

        return s
    }
}
extension NoEndpointException {
    var _NoEndpointExceptionDescription: String {
        var s = String()

        s.sep("no suitable endpoint available for proxy `\(proxy)'")

        return s
    }
}
extension EndpointParseException {
    var _EndpointParseExceptionDescription: String {
        var s = String()

        s.sep("error while parsing endpoint `\(str)'")

        return s
    }
}
extension EndpointSelectionTypeParseException {
    var _EndpointSelectionTypeParseExceptionDescription: String {
        var s = String()

        s.sep("error while parsing endpoint selection type `\(str)'")

        return s
    }
}
extension VersionParseException {
    var _VersionParseExceptionDescription: String {
        var s = String()

        s.sep("error while parsing version `\(str)'")

        return s
    }
}
extension IdentityParseException {
    var _IdentityParseExceptionDescription: String {
        var s = String()

        s.sep("error while parsing identity `\(str)'")

        return s
    }
}
extension ProxyParseException {
    var _ProxyParseExceptionDescription: String {
        var s = String()

        s.sep("error while parsing proxy `\(str)'")

        return s
    }
}
extension IllegalIdentityException {
    var _IllegalIdentityExceptionDescription: String {
        var s = String()
        let identity: String = (try? identityToString(id: id, mode: .Unicode)) ?? ""
        s.sep("llegal identity: `\(identity)'")

        return s
    }
}
extension IllegalServantException {
    var _IllegalServantExceptionDescription: String {
        var s = String()

        s.sep("illegal servant: `\(reason)'")

        return s
    }
}
extension RequestFailedException {
    var _RequestFailedExceptionDescription: String {
        var s = String()

        s.sep("request failed")
        s.failedRequest(self)

        return s
    }
}
extension ObjectNotExistException {
    var _ObjectNotExistExceptionDescription: String {
        var s = String()

        s.sep("object does not exist")
        s.failedRequest(self)

        return s
    }
}
extension FacetNotExistException {
    var _FacetNotExistExceptionDescription: String {
        var s = String()

        s.sep("facet does not exist")
        s.failedRequest(self)

        return s
    }
}
extension OperationNotExistException {
    var _OperationNotExistExceptionDescription: String {
        var s = String()

        s.sep("operation does not exist")
        s.failedRequest(self)

        return s
    }
}
extension SyscallException {
    var _SyscallExceptionDescription: String {
        var s = String()

        if error != 0 {
            s.sep("syscall exception: \(ICEUtil.errorToString(error))")
        }

        return s
    }
}
extension SocketException {
    var _SocketExceptionDescription: String {
        var s = String()

        s.sep("socket exception: \(socketError(error))")

        return s
    }
}
extension CFNetworkException {
    var _CFNetworkExceptionDescription: String {
        var s = String()

        s.sep("network exception: domain: `\(domain)' error: `\(error)'")

        return s
    }
}
extension FileException {
    var _FileExceptionDescription: String {
        var s = String()

        s.sep("file exception: ")
        if error == 0 {
            s.append("couldn't open file")
        } else {
            s.append(ICEUtil.errorToString(error))
        }
        if !path.isEmpty {
            s.nl("path: \(path)")
        }

        return s
    }
}
extension ConnectFailedException {
    var _ConnectFailedExceptionDescription: String {
        var s = String()

        s.sep("connect failed: \(socketError(error))")

        return s
    }
}
extension ConnectionRefusedException {
    var _ConnectionRefusedExceptionDescription: String {
        var s = String()

        s.sep("connection refused: \(socketError(error))")

        return s
    }
}
extension ConnectionLostException {
    var _ConnectionLostExceptionDescription: String {
        var s = String()

        s.sep("connection lost: ")
        if error == 0 {
            s.append("recv() returned zero")
        } else {
            s.append(socketError(error))
        }

        return s
    }
}
extension DNSException {
    var _DNSExceptionDescription: String {
        var s = String()

        s.sep("DNS error: \(ICEUtil.errorToStringDNS(error))")
        s.nl("host: \(host)")

        return s
    }
}
extension OperationInterruptedException {
    var _OperationInterruptedExceptionDescription: String {
        var s = String()

        s.sep("operation interrupted")

        return s
    }
}
extension TimeoutException {
    var _TimeoutExceptionDescription: String {
        var s = String()

        s.sep("timeout while sending or receiving data")

        return s
    }
}
extension ConnectTimeoutException {
    var _ConnectTimeoutExceptionDescription: String {
        var s = String()

        s.sep("timeout while establishing a connection")

        return s
    }
}
extension CloseTimeoutException {
    var _CloseTimeoutExceptionDescription: String {
        var s = String()

        s.sep("timeout while closing a connection")

        return s
    }
}
extension ConnectionTimeoutException {
    var _ConnectionTimeoutExceptionDescription: String {
        var s = String()

        s.sep("connection has timed out")

        return s
    }
}
extension InvocationTimeoutException {
    var _InvocationTimeoutExceptionDescription: String {
        var s = String()

        s.sep("invocation has timed out")

        return s
    }
}
extension InvocationCanceledException {
    var _InvocationCanceledExceptionDescription: String {
        var s = String()

        s.sep("invocation canceled")

        return s
    }
}
extension ProtocolException {
    var _ProtocolExceptionDescription: String {
        var s = String()

        s.sep("protocol exception")
        s.sep(reason)

        return s
    }
}
extension BadMagicException {
    var _BadMagicExceptionDescription: String {
        var s = String()

        s.sep("unknown magic number: ")

        s.hex(badMagic[0])
        s.append(", ")
        s.hex(badMagic[1])
        s.append(", ")
        s.hex(badMagic[2])
        s.append(", ")
        s.hex(badMagic[3])

        if !reason.isEmpty {
            s.nl(reason)
        }

        return s
    }
}
extension UnsupportedProtocolException {
    var _UnsupportedProtocolExceptionDescription: String {
        var s = String()

        s.sep("protocol error: unsupported protocol version: \(bad)")
        s.nl("(can only support protocols compatible with version \(supported))")

        return s
    }
}
extension UnsupportedEncodingException {
    var _UnsupportedEncodingExceptionDescription: String {
        var s = String()

        s.sep("encoding error: unsupported encoding version: \(bad)")
        s.nl("(can only support encodings compatible with version \(supported))")

        if !reason.isEmpty {
            s.nl(reason)
        }

        return s
    }
}
extension UnknownMessageException {
    var _UnknownMessageExceptionDescription: String {
        var s = String()

        s.sep("protocol error: unknown message type")
        s.sep(reason)

        return s
    }
}
extension ConnectionNotValidatedException {
    var _ConnectionNotValidatedExceptionDescription: String {
        var s = String()

        s.sep("protocol error: received message over unvalidated connection")
        s.sep(reason)

        return s
    }
}
extension UnknownRequestIdException {
    var _UnknownRequestIdExceptionDescription: String {
        var s = String()

        s.sep("protocol error: unknown request id")
        s.sep(reason)

        return s
    }
}
extension UnknownReplyStatusException {
    var _UnknownReplyStatusExceptionDescription: String {
        var s = String()

        s.sep("protocol error: unknown unknown reply status")
        s.sep(reason)

        return s
    }
}
extension CloseConnectionException {
    var _CloseConnectionExceptionDescription: String {
        var s = String()

        s.sep("protocol error: connection closed")
        s.sep(reason)

        return s
    }
}
extension ConnectionManuallyClosedException {
    var _ConnectionManuallyClosedExceptionDescription: String {
        var s = String()

        let type = graceful ? "gracefully" : "forcefully"
        s.sep("protocol error: connection manually closed (\(type))")

        return s
    }
}
extension IllegalMessageSizeException {
    var _IllegalMessageSizeExceptionDescription: String {
        var s = String()

        s.sep("protocol error: illegal message size")
        s.sep(reason)

        return s
    }
}
extension CompressionException {
    var _CompressionExceptionDescription: String {
        var s = String()

        s.sep("protocol error: failed to compress or uncompress data")
        s.sep(reason)

        return s
    }
}
extension DatagramLimitException {
    var _DatagramLimitExceptionDescription: String {
        var s = String()

        s.sep("protocol error: maximum datagram payload size exceeded")
        s.sep(reason)

        return s
    }
}
extension MarshalException {
    var _MarshalExceptionDescription: String {
        var s = String()

        s.sep("protocol error: error during marshaling or unmarshaling")
        s.sep(reason)

        return s
    }
}
extension ProxyUnmarshalException {
    var _ProxyUnmarshalExceptionDescription: String {
        var s = String()

        s.sep("protocol error: inconsistent proxy data during unmarshaling")
        s.sep(reason)

        return s
    }
}
extension UnmarshalOutOfBoundsException {
    var _UnmarshalOutOfBoundsExceptionDescription: String {
        var s = String()

        s.sep("protocol error: out of bounds during unmarshaling")
        s.sep(reason)

        return s
    }
}
extension NoValueFactoryException {
    var _NoValueFactoryExceptionDescription: String {
        var s = String()

        s.sep("protocol error: no suitable value factory found for `\(type)'")
        s.sep(reason)

        return s
    }
}
extension UnexpectedObjectException {
    var _UnexpectedObjectExceptionDescription: String {
        var s = String()

        s.sep("unexpected class instance of type `\(type)'; expected instance of type `\(expectedType)'")
        s.sep(reason)

        return s
    }
}
extension MemoryLimitException {
    var _MemoryLimitExceptionDescription: String {
        var s = String()

        s.sep("protocol error: memory limit exceeded")
        s.sep(reason)

        return s
    }
}
extension StringConversionException {
    var _StringConversionExceptionDescription: String {
        var s = String()

        s.sep("protocol error: string conversion failed")
        s.sep(reason)

        return s
    }
}
extension EncapsulationException {
    var _EncapsulationExceptionDescription: String {
        var s = String()

        s.sep("protocol error: illegal encapsulation")
        s.sep(reason)

        return s
    }
}
extension FeatureNotSupportedException {
    var _FeatureNotSupportedExceptionDescription: String {
        var s = String()

        s.sep("feature: `\(unsupportedFeature)' is not supported")

        return s
    }
}
extension SecurityException {
    var _SecurityExceptionDescription: String {
        var s = String()

        s.sep("security exception")
        s.sep(reason)

        return s
    }
}
extension FixedProxyException {
    var _FixedProxyExceptionDescription: String {
        var s = String()

        s.sep("fixed proxy exception")

        return s
    }
}
extension ResponseSentException {
    var _ResponseSentExceptionDescription: String {
        var s = String()

        s.sep("response sent exception")

        return s
    }
}
