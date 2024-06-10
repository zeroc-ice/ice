// Copyright (c) ZeroC, Inc.

import Foundation

/// This exception is raised when a failure occurs during initialization.
open class InitializationException: LocalException {
    /// The reason for the failure.
    public var reason: String = ""

    public required init() {
        super.init()
    }

    public init(reason: String, file: String = #file, line: Int32 = #line) {
        self.reason = reason
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::InitializationException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _InitializationExceptionDescription
    }
}

/// This exception indicates that a failure occurred while initializing a plug-in.
open class PluginInitializationException: LocalException {
    /// The reason for the failure.
    public var reason: String = ""

    public required init() {
        super.init()
    }

    public init(reason: String, file: String = #file, line: Int32 = #line) {
        self.reason = reason
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::PluginInitializationException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _PluginInitializationExceptionDescription
    }
}

/// An attempt was made to register something more than once with the Ice run time. This exception is raised if an
/// attempt is made to register a servant, servant locator, facet, value factory, plug-in, object adapter, object, or
/// user exception factory more than once for the same ID.
open class AlreadyRegisteredException: LocalException {
    /// The kind of object that could not be removed: "servant", "facet", "object", "default servant",
    /// "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
    public var kindOfObject: String = ""
    /// The ID (or name) of the object that is registered already.
    public var id: String = ""

    public required init() {
        super.init()
    }

    public init(
        kindOfObject: String, id: String, file: String = #file,
        line: Int32 = #line
    ) {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::AlreadyRegisteredException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _AlreadyRegisteredExceptionDescription
    }
}

/// An attempt was made to find or deregister something that is not registered with the Ice run time or Ice locator.
/// This exception is raised if an attempt is made to remove a servant, servant locator, facet, value factory, plug-in,
/// object adapter, object, or user exception factory that is not currently registered. It's also raised if the Ice
/// locator can't find an object or object adapter when resolving an indirect proxy or when an object adapter is
/// activated.
open class NotRegisteredException: LocalException {
    /// The kind of object that could not be removed: "servant", "facet", "object", "default servant",
    /// "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
    public var kindOfObject: String = ""
    /// The ID (or name) of the object that could not be removed.
    public var id: String = ""

    public required init() {
        super.init()
    }

    public init(
        kindOfObject: String, id: String, file: String = #file,
        line: Int32 = #line
    ) {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::NotRegisteredException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _NotRegisteredExceptionDescription
    }
}

/// The operation can only be invoked with a twoway request. This exception is raised if an attempt is made to invoke
/// an operation with ice_oneway, ice_batchOneway, ice_datagram, or
/// ice_batchDatagram and the operation has a return value, out-parameters, or an exception specification.
open class TwowayOnlyException: LocalException {
    /// The name of the operation that was invoked.
    public var operation: String = ""

    public required init() {
        super.init()
    }

    public init(operation: String, file: String = #file, line: Int32 = #line) {
        self.operation = operation
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::TwowayOnlyException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _TwowayOnlyExceptionDescription
    }
}

/// An attempt was made to clone a class that does not support cloning. This exception is raised if
/// ice_clone is called on a class that is derived from an abstract Slice class (that is, a class
/// containing operations), and the derived class does not provide an implementation of the ice_clone
/// operation (C++ only).
open class CloneNotImplementedException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::CloneNotImplementedException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _CloneNotImplementedExceptionDescription
    }
}

/// This exception is raised if an operation call on a server raises an unknown exception. For example, for C++, this
/// exception is raised if the server throws a C++ exception that is not directly or indirectly derived from
/// Ice::LocalException or Ice::UserException.
open class UnknownException: LocalException {
    /// This field is set to the textual representation of the unknown exception if available.
    public var unknown: String = ""

    public required init() {
        super.init()
    }

    public init(unknown: String, file: String = #file, line: Int32 = #line) {
        self.unknown = unknown
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::UnknownException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _UnknownExceptionDescription
    }
}

/// This exception is raised if an operation call on a server raises a  local exception. Because local exceptions are
/// not transmitted by the Ice protocol, the client receives all local exceptions raised by the server as
/// UnknownLocalException. The only exception to this rule are all exceptions derived from
/// RequestFailedException, which are transmitted by the Ice protocol even though they are declared
/// local.
open class UnknownLocalException: UnknownException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::UnknownLocalException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _UnknownLocalExceptionDescription
    }
}

/// An operation raised an incorrect user exception. This exception is raised if an operation raises a user exception
/// that is not declared in the exception's throws clause. Such undeclared exceptions are not transmitted
/// from the server to the client by the Ice protocol, but instead the client just gets an UnknownUserException.
/// This is necessary in order to not violate the contract established by an operation's signature: Only local
/// exceptions and user exceptions declared in the throws clause can be raised.
open class UnknownUserException: UnknownException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::UnknownUserException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _UnknownUserExceptionDescription
    }
}

/// This exception is raised if the Communicator has been destroyed.
open class CommunicatorDestroyedException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::CommunicatorDestroyedException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _CommunicatorDestroyedExceptionDescription
    }
}

/// This exception is raised if an attempt is made to use a deactivated ObjectAdapter.
open class ObjectAdapterDeactivatedException: LocalException {
    /// Name of the adapter.
    public var name: String = ""

    public required init() {
        super.init()
    }

    public init(name: String, file: String = #file, line: Int32 = #line) {
        self.name = name
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::ObjectAdapterDeactivatedException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _ObjectAdapterDeactivatedExceptionDescription
    }
}

/// This exception is raised if an ObjectAdapter cannot be activated. This happens if the Locator
/// detects another active ObjectAdapter with the same adapter id.
open class ObjectAdapterIdInUseException: LocalException {
    /// Adapter ID.
    public var id: String = ""

    public required init() {
        super.init()
    }

    public init(id: String, file: String = #file, line: Int32 = #line) {
        self.id = id
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::ObjectAdapterIdInUseException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _ObjectAdapterIdInUseExceptionDescription
    }
}

/// This exception is raised if no suitable endpoint is available.
open class NoEndpointException: LocalException {
    /// The stringified proxy for which no suitable endpoint is available.
    public var proxy: String = ""

    public required init() {
        super.init()
    }

    public init(proxy: String, file: String = #file, line: Int32 = #line) {
        self.proxy = proxy
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::NoEndpointException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _NoEndpointExceptionDescription
    }
}

/// This exception is raised if there was an error while parsing an endpoint.
open class EndpointParseException: LocalException {
    /// Describes the failure and includes the string that could not be parsed.
    public var str: String = ""

    public required init() {
        super.init()
    }

    public init(str: String, file: String = #file, line: Int32 = #line) {
        self.str = str
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::EndpointParseException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _EndpointParseExceptionDescription
    }
}

/// This exception is raised if there was an error while parsing an endpoint selection type.
open class EndpointSelectionTypeParseException: LocalException {
    /// Describes the failure and includes the string that could not be parsed.
    public var str: String = ""

    public required init() {
        super.init()
    }

    public init(str: String, file: String = #file, line: Int32 = #line) {
        self.str = str
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::EndpointSelectionTypeParseException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _EndpointSelectionTypeParseExceptionDescription
    }
}

/// This exception is raised if there was an error while parsing a version.
open class VersionParseException: LocalException {
    /// Describes the failure and includes the string that could not be parsed.
    public var str: String = ""

    public required init() {
        super.init()
    }

    public init(str: String, file: String = #file, line: Int32 = #line) {
        self.str = str
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::VersionParseException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _VersionParseExceptionDescription
    }
}

/// This exception is raised if there was an error while parsing a stringified identity.
open class IdentityParseException: LocalException {
    /// Describes the failure and includes the string that could not be parsed.
    public var str: String = ""

    public required init() {
        super.init()
    }

    public init(str: String, file: String = #file, line: Int32 = #line) {
        self.str = str
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::IdentityParseException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _IdentityParseExceptionDescription
    }
}

/// This exception is raised if there was an error while parsing a stringified proxy.
open class ProxyParseException: LocalException {
    /// Describes the failure and includes the string that could not be parsed.
    public var str: String = ""

    public required init() {
        super.init()
    }

    public init(str: String, file: String = #file, line: Int32 = #line) {
        self.str = str
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::ProxyParseException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _ProxyParseExceptionDescription
    }
}

/// This exception is raised if an identity with an empty name is encountered.
open class IllegalIdentityException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::IllegalIdentityException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _IllegalIdentityExceptionDescription
    }
}

/// This exception is raised to reject an illegal servant (typically a null servant).
open class IllegalServantException: LocalException {
    /// Describes why this servant is illegal.
    public var reason: String = ""

    public required init() {
        super.init()
    }

    public init(reason: String, file: String = #file, line: Int32 = #line) {
        self.reason = reason
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::IllegalServantException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _IllegalServantExceptionDescription
    }
}

/// This exception is raised if a request failed. This exception, and all exceptions derived from
/// RequestFailedException, are transmitted by the Ice protocol, even though they are declared
/// local.
open class RequestFailedException: LocalException {
    /// The identity of the Ice Object to which the request was sent.
    public var id: Identity = .init()
    /// The facet to which the request was sent.
    public var facet: String = ""
    /// The operation name of the request.
    public var operation: String = ""

    public required init() {
        super.init()
    }

    public init(
        id: Identity,
        facet: String,
        operation: String,
        file: String = #file,
        line: Int32 = #line
    ) {
        self.id = id
        self.facet = facet
        self.operation = operation
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::RequestFailedException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _RequestFailedExceptionDescription
    }
}

/// This exception is raised if an object does not exist on the server, that is, if no facets with the given identity
/// exist.
open class ObjectNotExistException: RequestFailedException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::ObjectNotExistException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _ObjectNotExistExceptionDescription
    }
}

/// This exception is raised if no facet with the given name exists, but at least one facet with the given identity
/// exists.
open class FacetNotExistException: RequestFailedException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::FacetNotExistException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _FacetNotExistExceptionDescription
    }
}

/// This exception is raised if an operation for a given object does not exist on the server. Typically this is caused
/// by either the client or the server using an outdated Slice specification.
open class OperationNotExistException: RequestFailedException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::OperationNotExistException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _OperationNotExistExceptionDescription
    }
}

/// This exception is raised if a system error occurred in the server or client process. There are many possible causes
/// for such a system exception. For details on the cause, SyscallException.error should be inspected.
open class SyscallException: LocalException {
    /// The error number describing the system exception. For C++ and Unix, this is equivalent to errno.
    /// For C++ and Windows, this is the value returned by GetLastError() or
    /// WSAGetLastError().
    public var error: Int32 = 0

    public required init() {
        super.init()
    }

    public init(error: Int32, file: String = #file, line: Int32 = #line) {
        self.error = error
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::SyscallException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _SyscallExceptionDescription
    }
}

/// This exception indicates socket errors.
open class SocketException: SyscallException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::SocketException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _SocketExceptionDescription
    }
}

/// This exception indicates CFNetwork errors.
open class CFNetworkException: SocketException {
    /// The domain of the error.
    public var domain: String = ""

    public required init() {
        super.init()
    }

    public init(
        error: Int32, domain: String, file: String = #file, line: Int32 = #line
    ) {
        self.domain = domain
        super.init(error: error, file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::CFNetworkException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _CFNetworkExceptionDescription
    }
}

/// This exception indicates file errors.
open class FileException: SyscallException {
    /// The path of the file responsible for the error.
    public var path: String = ""

    public required init() {
        super.init()
    }

    public init(
        error: Int32, path: String, file: String = #file, line: Int32 = #line
    ) {
        self.path = path
        super.init(error: error, file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::FileException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _FileExceptionDescription
    }
}

/// This exception indicates connection failures.
open class ConnectFailedException: SocketException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::ConnectFailedException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _ConnectFailedExceptionDescription
    }
}

/// This exception indicates a connection failure for which the server host actively refuses a connection.
open class ConnectionRefusedException: ConnectFailedException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::ConnectionRefusedException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _ConnectionRefusedExceptionDescription
    }
}

/// This exception indicates a lost connection.
open class ConnectionLostException: SocketException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::ConnectionLostException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _ConnectionLostExceptionDescription
    }
}

/// This exception indicates a DNS problem. For details on the cause, DNSException.error should be inspected.
open class DNSException: LocalException {
    /// The error number describing the DNS problem. For C++ and Unix, this is equivalent to h_errno. For
    /// C++ and Windows, this is the value returned by WSAGetLastError().
    public var error: Int32 = 0
    /// The host name that could not be resolved.
    public var host: String = ""

    public required init() {
        super.init()
    }

    public init(
        error: Int32, host: String, file: String = #file, line: Int32 = #line
    ) {
        self.error = error
        self.host = host
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::DNSException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _DNSExceptionDescription
    }
}

/// This exception indicates that a connection was aborted by the idle check.
open class ConnectionIdleException: TimeoutException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::ConnectionIdleException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _ConnectionIdleExceptionDescription
    }
}

/// This exception indicates a timeout condition.
open class TimeoutException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::TimeoutException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _TimeoutExceptionDescription
    }
}

/// This exception indicates a connection establishment timeout condition.
open class ConnectTimeoutException: TimeoutException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::ConnectTimeoutException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _ConnectTimeoutExceptionDescription
    }
}

/// This exception indicates a connection closure timeout condition.
open class CloseTimeoutException: TimeoutException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::CloseTimeoutException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _CloseTimeoutExceptionDescription
    }
}

/// This exception indicates that an invocation failed because it timed out.
open class InvocationTimeoutException: TimeoutException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::InvocationTimeoutException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _InvocationTimeoutExceptionDescription
    }
}

/// This exception indicates that an asynchronous invocation failed because it was canceled explicitly by the user.
open class InvocationCanceledException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::InvocationCanceledException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _InvocationCanceledExceptionDescription
    }
}

/// A generic exception base for all kinds of protocol error conditions.
open class ProtocolException: LocalException {
    /// The reason for the failure.
    public var reason: String = ""

    public required init() {
        super.init()
    }

    public init(reason: String, file: String = #file, line: Int32 = #line) {
        self.reason = reason
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::ProtocolException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _ProtocolExceptionDescription
    }
}

/// This exception indicates that a message did not start with the expected magic number ('I', 'c', 'e', 'P').
open class BadMagicException: ProtocolException {
    /// A sequence containing the first four bytes of the incorrect message.
    public var badMagic: ByteSeq = .init()

    public required init() {
        super.init()
    }

    public init(
        reason: String, badMagic: ByteSeq, file: String = #file, line: Int32 = #line
    ) {
        self.badMagic = badMagic
        super.init(reason: reason, file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::BadMagicException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _BadMagicExceptionDescription
    }
}

/// This exception indicates an unsupported protocol version.
open class UnsupportedProtocolException: ProtocolException {
    /// The version of the unsupported protocol.
    public var bad: ProtocolVersion = .init()
    /// The version of the protocol that is supported.
    public var supported: ProtocolVersion = .init()

    public required init() {
        super.init()
    }

    public init(
        reason: String,
        bad: ProtocolVersion,
        supported: ProtocolVersion,
        file: String = #file,
        line: Int32 = #line
    ) {
        self.bad = bad
        self.supported = supported
        super.init(reason: reason, file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::UnsupportedProtocolException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _UnsupportedProtocolExceptionDescription
    }
}

/// This exception indicates an unsupported data encoding version.
open class UnsupportedEncodingException: ProtocolException {
    /// The version of the unsupported encoding.
    public var bad: EncodingVersion = .init()
    /// The version of the encoding that is supported.
    public var supported: EncodingVersion = .init()

    public required init() {
        super.init()
    }

    public init(
        reason: String,
        bad: EncodingVersion,
        supported: EncodingVersion,
        file: String = #file,
        line: Int32 = #line
    ) {
        self.bad = bad
        self.supported = supported
        super.init(reason: reason, file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::UnsupportedEncodingException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _UnsupportedEncodingExceptionDescription
    }
}

/// This exception indicates that an unknown protocol message has been received.
open class UnknownMessageException: ProtocolException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::UnknownMessageException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _UnknownMessageExceptionDescription
    }
}

/// This exception is raised if a message is received over a connection that is not yet validated.
open class ConnectionNotValidatedException: ProtocolException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::ConnectionNotValidatedException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _ConnectionNotValidatedExceptionDescription
    }
}

/// This exception indicates that an unknown reply status has been received.
open class UnknownReplyStatusException: ProtocolException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::UnknownReplyStatusException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _UnknownReplyStatusExceptionDescription
    }
}

/// This exception indicates that the connection has been gracefully shut down by the server. The operation call that
/// caused this exception has not been executed by the server. In most cases you will not get this exception, because
/// the client will automatically retry the operation call in case the server shut down the connection. However, if
/// upon retry the server shuts down the connection again, and the retry limit has been reached, then this exception is
/// propagated to the application code.
open class CloseConnectionException: ProtocolException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::CloseConnectionException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _CloseConnectionExceptionDescription
    }
}

/// This exception is raised by an operation call if the application closes the connection locally using
/// Connection.close.
open class ConnectionManuallyClosedException: LocalException {
    /// True if the connection was closed gracefully, false otherwise.
    public var graceful: Bool = false

    public required init() {
        super.init()
    }

    public init(graceful: Bool, file: String = #file, line: Int32 = #line) {
        self.graceful = graceful
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::ConnectionManuallyClosedException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _ConnectionManuallyClosedExceptionDescription
    }
}

/// This exception indicates that a message size is less than the minimum required size.
open class IllegalMessageSizeException: ProtocolException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::IllegalMessageSizeException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _IllegalMessageSizeExceptionDescription
    }
}

/// This exception indicates a problem with compressing or uncompressing data.
open class CompressionException: ProtocolException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::CompressionException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _CompressionExceptionDescription
    }
}

/// A datagram exceeds the configured size. This exception is raised if a datagram exceeds the configured send or
/// receive buffer size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
open class DatagramLimitException: ProtocolException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::DatagramLimitException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _DatagramLimitExceptionDescription
    }
}

/// This exception is raised for errors during marshaling or unmarshaling data.
open class MarshalException: ProtocolException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::MarshalException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _MarshalExceptionDescription
    }
}

/// This exception is raised if inconsistent data is received while unmarshaling a proxy.
open class ProxyUnmarshalException: MarshalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::ProxyUnmarshalException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _ProxyUnmarshalExceptionDescription
    }
}

/// This exception is raised if an out-of-bounds condition occurs during unmarshaling.
open class UnmarshalOutOfBoundsException: MarshalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::UnmarshalOutOfBoundsException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _UnmarshalOutOfBoundsExceptionDescription
    }
}

/// This exception is raised if no suitable value factory was found during unmarshaling of a Slice class instance.
open class NoValueFactoryException: MarshalException {
    /// The Slice type ID of the class instance for which no factory could be found.
    public var type: String = ""

    public required init() {
        super.init()
    }

    public init(
        reason: String, type: String, file: String = #file, line: Int32 = #line
    ) {
        self.type = type
        super.init(reason: reason, file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::NoValueFactoryException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _NoValueFactoryExceptionDescription
    }
}

/// This exception is raised if the type of an unmarshaled Slice class instance does not match its expected type. This
/// can happen if client and server are compiled with mismatched Slice definitions or if a class of the wrong type is
/// passed as a parameter or return value using dynamic invocation. This exception can also be raised if IceStorm is
/// used to send Slice class instances and an operation is subscribed to the wrong topic.
open class UnexpectedObjectException: MarshalException {
    /// The Slice type ID of the class instance that was unmarshaled.
    public var type: String = ""
    /// The Slice type ID that was expected by the receiving operation.
    public var expectedType: String = ""

    public required init() {
        super.init()
    }

    public init(
        reason: String,
        type: String,
        expectedType: String,
        file: String = #file,
        line: Int32 = #line
    ) {
        self.type = type
        self.expectedType = expectedType
        super.init(reason: reason, file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::UnexpectedObjectException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _UnexpectedObjectExceptionDescription
    }
}

/// This exception is raised when Ice receives a request or reply message whose size exceeds the limit specified by the
/// Ice.MessageSizeMax property.
open class MemoryLimitException: MarshalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::MemoryLimitException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _MemoryLimitExceptionDescription
    }
}

/// This exception indicates a malformed data encapsulation.
open class EncapsulationException: MarshalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::EncapsulationException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _EncapsulationExceptionDescription
    }
}

/// This exception is raised if an unsupported feature is used. The unsupported feature string contains the name of the
/// unsupported feature.
open class FeatureNotSupportedException: LocalException {
    /// The name of the unsupported feature.
    public var unsupportedFeature: String = ""

    public required init() {
        super.init()
    }

    public init(unsupportedFeature: String, file: String = #file, line: Int32 = #line) {
        self.unsupportedFeature = unsupportedFeature
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::FeatureNotSupportedException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _FeatureNotSupportedExceptionDescription
    }
}

/// This exception indicates a failure in a security subsystem, such as the IceSSL plug-in.
open class SecurityException: LocalException {
    /// The reason for the failure.
    public var reason: String = ""

    public required init() {
        super.init()
    }

    public init(reason: String, file: String = #file, line: Int32 = #line) {
        self.reason = reason
        super.init(file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::SecurityException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _SecurityExceptionDescription
    }
}

/// This exception indicates that an attempt has been made to change the connection properties of a fixed proxy.
open class FixedProxyException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override open class func ice_staticId() -> String {
        return "::Ice::FixedProxyException"
    }

    /// Returns a string representation of this exception
    ///
    /// - returns: `String` - The string representaton of this exception.
    override open func ice_print() -> String {
        return _FixedProxyExceptionDescription
    }
}
