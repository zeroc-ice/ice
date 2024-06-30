// Copyright (c) ZeroC, Inc.

import Foundation

/// This exception is raised when a failure occurs during initialization.
public final class InitializationException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::InitializationException"
    }
}

/// This exception indicates that a failure occurred while initializing a plug-in.
public final class PluginInitializationException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::PluginInitializationException"
    }
}

/// An attempt was made to register something more than once with the Ice run time. This exception is raised if an
/// attempt is made to register a servant, servant locator, facet, value factory, plug-in, object adapter, object, or
/// user exception factory more than once for the same ID.
public final class AlreadyRegisteredException: LocalException {
    /// The kind of object that could not be removed: "servant", "facet", "object", "default servant",
    /// "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
    public let kindOfObject: String
    /// The ID (or name) of the object that is registered already.
    public let id: String

    public init(kindOfObject: String, id: String, file: String = #file, line: Int32 = #line) {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init("another \(kindOfObject) is already registered with ID '\(id)'", file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::AlreadyRegisteredException"
    }

    // Initializer for C++ exceptions
    internal init(kindOfObject: String, id: String, message: String, cxxDescription: String, file: String, line: Int32)
    {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init(message: message, cxxDescription: cxxDescription, file: file, line: line)
    }

    public required init(message: String, cxxDescription: String, file: String, line: Int32) {
        fatalError("AlreadyRegisteredException must be initialized with a kindOfObject and id")
    }
}

/// An attempt was made to find or deregister something that is not registered with the Ice run time or Ice locator.
/// This exception is raised if an attempt is made to remove a servant, servant locator, facet, value factory, plug-in,
/// object adapter, object, or user exception factory that is not currently registered. It's also raised if the Ice
/// locator can't find an object or object adapter when resolving an indirect proxy or when an object adapter is
/// activated.
public final class NotRegisteredException: LocalException {
    /// The kind of object that could not be removed: "servant", "facet", "object", "default servant",
    /// "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
    public let kindOfObject: String
    /// The ID (or name) of the object that could not be removed.
    public let id: String

    public init(kindOfObject: String, id: String, file: String = #file, line: Int32 = #line) {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init("no \(kindOfObject) is registered with ID '\(id)'", file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::NotRegisteredException"
    }

    // Initializer for C++ exceptions
    internal init(kindOfObject: String, id: String, message: String, cxxDescription: String, file: String, line: Int32)
    {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init(message: message, cxxDescription: cxxDescription, file: file, line: line)
    }

    public required init(message: String, cxxDescription: String, file: String, line: Int32) {
        fatalError("NotRegisteredException must be initialized with a kindOfObject and id")
    }
}

/// The operation can only be invoked with a twoway request. This exception is raised if an attempt is made to invoke
/// an operation with ice_oneway, ice_batchOneway, ice_datagram, or
/// ice_batchDatagram and the operation has a return value, out-parameters, or an exception specification.
public final class TwowayOnlyException: LocalException {
    public convenience init(operation: String, file: String = #file, line: Int32 = #line) {
        self.init(
            "cannot invoke operation '\(operation)' with a oneway, batchOneway, datagram, or batchDatagram proxy",
            file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::TwowayOnlyException"
    }
}

/// This exception is raised if an operation call on a server raises an unknown exception. For example, for C++, this
/// exception is raised if the server throws a C++ exception that is not directly or indirectly derived from
/// Ice::LocalException or Ice::UserException.
public class UnknownException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::UnknownException"
    }
}

/// This exception is raised if an operation call on a server raises a  local exception. Because local exceptions are
/// not transmitted by the Ice protocol, the client receives all local exceptions raised by the server as
/// UnknownLocalException. The only exception to this rule are all exceptions derived from
/// RequestFailedException, which are transmitted by the Ice protocol even though they are declared
/// local.
public final class UnknownLocalException: UnknownException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::UnknownLocalException"
    }
}

/// An operation raised an incorrect user exception. This exception is raised if an operation raises a user exception
/// that is not declared in the exception's throws clause. Such undeclared exceptions are not transmitted
/// from the server to the client by the Ice protocol, but instead the client just gets an UnknownUserException.
/// This is necessary in order to not violate the contract established by an operation's signature: Only local
/// exceptions and user exceptions declared in the throws clause can be raised.
public final class UnknownUserException: UnknownException {
    public convenience init(badTypeId: String, file: String = #file, line: Int32 = #line) {
        self.init(
            "the user exception carried by the reply does not conform to the operation's exception specification: \(badTypeId)",
            file: file, line: line)
    }
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::UnknownUserException"
    }
}

/// This exception is raised if the Communicator has been destroyed.
public final class CommunicatorDestroyedException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::CommunicatorDestroyedException"
    }
}

/// This exception is raised if an attempt is made to use a deactivated ObjectAdapter.
public final class ObjectAdapterDeactivatedException: LocalException {
    public convenience init(name: String, file: String = #file, line: Int32 = #line) {
        self.init("object adapter '\(name)' is deactivated", file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::ObjectAdapterDeactivatedException"
    }
}

/// This exception is raised if an ObjectAdapter cannot be activated. This happens if the Locator
/// detects another active ObjectAdapter with the same adapter id.
public final class ObjectAdapterIdInUseException: LocalException {
    public convenience init(id: String, file: String = #file, line: Int32 = #line) {
        self.init("an object adapter with adapter ID'\(id)' is already active", file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::ObjectAdapterIdInUseException"
    }
}

/// This exception is raised if no suitable endpoint is available.
public final class NoEndpointException: LocalException {
    public convenience init(proxy: ObjectPrx, file: String = #file, line: Int32 = #line) {
        self.init("no suitable endpoint available for proxy '\(proxy)'", file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::NoEndpointException"
    }
}

/// This exception is raised if there was an error while parsing a string.
public final class ParseException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::ParseException"
    }
}

/// This exception is raised if a request failed. This exception, and all exceptions derived from
/// RequestFailedException, are transmitted by the Ice protocol, even though they are declared
/// local.
public class RequestFailedException: LocalException {
    /// The identity of the Ice Object to which the request was sent.
    public let id: Identity
    /// The facet to which the request was sent.
    public let facet: String
    /// The operation name of the request.
    public let operation: String

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::RequestFailedException"
    }

    internal init(
        typeName: String,
        id: Identity,
        facet: String,
        operation: String,
        file: String,
        line: Int32
    ) {
        self.id = id
        self.facet = facet
        self.operation = operation
        super.init(
            Self.makeMessage(typeName: typeName, id: id, facet: facet, operation: operation), file: file, line: line)
    }

    override internal init(_ message: String, file: String, line: Int32) {
        self.id = Identity()
        self.facet = ""
        self.operation = ""
        super.init(message, file: file, line: line)
    }

    public required init(
        id: Identity, facet: String, operation: String, message: String, cxxDescription: String, file: String,
        line: Int32
    ) {
        self.id = id
        self.facet = facet
        self.operation = operation
        super.init(message: message, cxxDescription: cxxDescription, file: file, line: line)
    }

    public required init(message: String, cxxDescription: String, file: String, line: Int32) {
        fatalError("RequestFailedException must be initialized with an id, facet, and operation")
    }

    internal class func makeMessage(typeName: String, id: Identity, facet: String, operation: String) -> String {
        "dispatch failed for \(typeName) { id = '\(identityToString(id: id))', facet = '\(facet)', operation = '\(operation)' }"
    }
}

/// This exception is raised if an object does not exist on the server, that is, if no facets with the given identity
/// exist.
public final class ObjectNotExistException: RequestFailedException {
    public convenience init(id: Identity, facet: String, operation: String, file: String = #file, line: Int32 = #line) {
        self.init(
            typeName: "ObjectNotExistException", id: id, facet: facet, operation: operation, file: file, line: line)
    }

    public convenience init(file: String = #file, line: Int32 = #line) {
        self.init("dispatch failed with ObjectNotExistException", file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::ObjectNotExistException"
    }
}

/// This exception is raised if no facet with the given name exists, but at least one facet with the given identity
/// exists.
public final class FacetNotExistException: RequestFailedException {
    public convenience init(id: Identity, facet: String, operation: String, file: String = #file, line: Int32 = #line) {
        self.init(
            typeName: "FacetNotExistException", id: id, facet: facet, operation: operation, file: file, line: line)
    }

    public convenience init(file: String = #file, line: Int32 = #line) {
        self.init("dispatch failed with FacetNotExistException", file: file, line: line)
    }
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::FacetNotExistException"
    }
}

/// This exception is raised if an operation for a given object does not exist on the server. Typically this is caused
/// by either the client or the server using an outdated Slice specification.
public final class OperationNotExistException: RequestFailedException {
    public convenience init(id: Identity, facet: String, operation: String, file: String = #file, line: Int32 = #line) {
        self.init(
            typeName: "OperationNotExistException", id: id, facet: facet, operation: operation, file: file, line: line)
    }

    public convenience init(file: String = #file, line: Int32 = #line) {
        self.init("dispatch failed with OperationNotExistException", file: file, line: line)
    }

    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::OperationNotExistException"
    }
}

/// This exception is raised if a system error occurred in the server or client process. There are many possible causes
/// for such a system exception. For details on the cause, SyscallException.error should be inspected.
public class SyscallException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::SyscallException"
    }
}

/// This exception indicates socket errors.
public class SocketException: SyscallException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::SocketException"
    }
}

/// This exception indicates connection failures.
public class ConnectFailedException: SocketException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::ConnectFailedException"
    }
}

/// This exception indicates a connection failure for which the server host actively refuses a connection.
public final class ConnectionRefusedException: ConnectFailedException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::ConnectionRefusedException"
    }
}

/// This exception indicates a lost connection.
public final class ConnectionLostException: SocketException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::ConnectionLostException"
    }
}

/// This exception indicates that a connection was aborted by the idle check.
public class ConnectionIdleException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::ConnectionIdleException"
    }
}

/// This exception indicates a timeout condition.
public class TimeoutException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::TimeoutException"
    }
}

/// This exception indicates a connection establishment timeout condition.
public final class ConnectTimeoutException: TimeoutException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::ConnectTimeoutException"
    }
}

/// This exception indicates a connection closure timeout condition.
public final class CloseTimeoutException: TimeoutException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::CloseTimeoutException"
    }
}

/// This exception indicates that an invocation failed because it timed out.
public final class InvocationTimeoutException: TimeoutException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::InvocationTimeoutException"
    }
}

/// This exception indicates that an asynchronous invocation failed because it was canceled explicitly by the user.
public final class InvocationCanceledException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::InvocationCanceledException"
    }
}

/// A generic exception base for all kinds of protocol error conditions.
public class ProtocolException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::ProtocolException"
    }
}

/// This exception indicates that the connection has been gracefully shut down by the server. The operation call that
/// caused this exception has not been executed by the server. In most cases you will not get this exception, because
/// the client will automatically retry the operation call in case the server shut down the connection. However, if
/// upon retry the server shuts down the connection again, and the retry limit has been reached, then this exception is
/// propagated to the application code.
public final class CloseConnectionException: ProtocolException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::CloseConnectionException"
    }
}

/// A datagram exceeds the configured size. This exception is raised if a datagram exceeds the configured send or
/// receive buffer size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
public final class DatagramLimitException: ProtocolException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::DatagramLimitException"
    }
}

/// This exception is raised for errors during marshaling or unmarshaling data.
public final class MarshalException: ProtocolException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::MarshalException"
    }
}

/// This exception is raised if an unsupported feature is used.
public final class FeatureNotSupportedException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::FeatureNotSupportedException"
    }
}

/// This exception indicates a failure in a security subsystem, such as the IceSSL plug-in.
public final class SecurityException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::SecurityException"
    }
}

/// This exception indicates that an attempt has been made to change the connection properties of a fixed proxy.
public final class FixedProxyException: LocalException {
    /// Returns the Slice type ID of this exception.
    ///
    /// - returns: `String` - the Slice type ID of this exception.
    override public class func ice_staticId() -> String {
        return "::Ice::FixedProxyException"
    }
}

public final class DNSException: SyscallException {
    override public class func ice_staticId() -> String {
        return "::Ice::DNSException"
    }
}

public final class ConnectionManuallyClosedException: LocalException {
    public var graceful: Bool

    public init(graceful: Bool, file: String = #file, line: Int32 = #line) {
        self.graceful = graceful
        super.init("connection was manually closed", file: file, line: line)
    }

    override public class func ice_staticId() -> String {
        return "::Ice::ConnectionManuallyClosedException"
    }

    internal init(graceful: Bool, message: String, cxxDescription: String, file: String, line: Int32) {
        self.graceful = graceful
        super.init(message: message, cxxDescription: cxxDescription, file: file, line: line)
    }

    public required init(message: String, cxxDescription: String, file: String, line: Int32) {
        fatalError("ConnectionManuallyClosedException must be initialized with a graceful flag")
    }
}
