// Copyright (c) ZeroC, Inc.

import IceImpl

extension String {
    fileprivate mutating func sep(_ str: String) {
        guard !str.isEmpty else {
            return
        }
        append(":\n\(str)")
    }

    fileprivate mutating func nl(_ str: String) {
        guard !str.isEmpty else {
            return
        }
        append("\n\(str)")
    }

    fileprivate mutating func failedRequest(_ ex: RequestFailedException) {
        let id = ex.id.name.isEmpty ? "" : identityToString(id: ex.id, mode: .Unicode)
        sep("identity: `\(id)'")
        nl("facet: \(ex.facet)")
        nl("operation: \(ex.operation)")
    }

    fileprivate mutating func hex(_ value: UInt8) {
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

extension ParseException {
    var _ParseExceptionDescription: String {
        var s = String()

        s.sep("error while parsing: `\(str)'")

        return s
    }
}

extension IllegalIdentityException {
    var _IllegalIdentityExceptionDescription: String {
        var s = String()

        s.sep("an identity with an empty name is not allowed")

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

extension ConnectionIdleException {
    var _ConnectionIdleExceptionDescription: String {
        var s = String()

        s.sep("connection aborted by the idle check")

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
