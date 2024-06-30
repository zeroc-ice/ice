// Copyright (c) ZeroC, Inc.

// This file contains all the exception classes derived from LocalException defined in the Ice assembly.

//
// The 6 (7 with the RequestFailedException base class) special local exceptions that can be marshaled in an Ice reply
// message. Other local exceptions can't be marshaled.
//

/// The base exception for the 3 NotExist exceptions.
public class RequestFailedException: LocalException {
    /// The identity of the Ice Object to which the request was sent.
    public let id: Identity
    /// The facet to which the request was sent.
    public let facet: String
    /// The operation name of the request.
    public let operation: String

    /// Creates a XxxNotExistException from an Ice C++ exception.
    /// - Parameters:
    ///   - id: The identity of the target Ice object carried by the request.
    ///   - facet: The facet of the target Ice object.
    ///   - operation: The operation name carried by the request.
    ///   - message: The exception message.
    ///   - cxxDescription: The C++ exception description.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public required init(
        id: Identity, facet: String, operation: String, message: String, cxxDescription: String, file: String,
        line: Int32
    ) {
        self.id = id
        self.facet = facet
        self.operation = operation
        super.init(message: message, cxxDescription: cxxDescription, file: file, line: line)
    }

    /// Internal initializer - don't use.
    public required init(message: String, cxxDescription: String, file: String, line: Int32) {
        fatalError("RequestFailedException must be initialized with an id, facet, and operation")
    }

    override public class func ice_staticId() -> String { "::Ice::RequestFailedException" }

    internal init(typeName: String, id: Identity, facet: String, operation: String, file: String, line: Int32) {
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

    internal class func makeMessage(typeName: String, id: Identity, facet: String, operation: String) -> String {
        "dispatch failed for \(typeName) { id = '\(identityToString(id: id))', facet = '\(facet)', operation = '\(operation)' }"
    }
}

/// The dispatch could not find a servant for the identity carried by the request.
public final class ObjectNotExistException: RequestFailedException {
    /// Creates an OperationNotExistException.
    /// - Parameters:
    ///   - id: The identity of the target Ice object carried by the request.
    ///   - facet: The facet of the target Ice object.
    ///   - operation: The operation name carried by the request.
    ///   - message: The exception message.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(id: Identity, facet: String, operation: String, file: String = #file, line: Int32 = #line) {
        self.init(
            typeName: "ObjectNotExistException", id: id, facet: facet, operation: operation, file: file, line: line)
    }

    /// Creates an OperationNotExistException. The request details (id, facet, operation) will be filled-in by the Ice
    /// runtime when the exception is marshaled.
    /// - Parameters:
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(file: String = #file, line: Int32 = #line) {
        self.init("dispatch failed with ObjectNotExistException", file: file, line: line)
    }

    override public class func ice_staticId() -> String { "::Ice::ObjectNotExistException" }
}

/// The dispatch could not find a servant for the identity + facet carried by the request.
public final class FacetNotExistException: RequestFailedException {
    /// Creates a FacetNotExistException.
    /// - Parameters:
    ///   - id: The identity of the target Ice object carried by the request.
    ///   - facet: The facet of the target Ice object.
    ///   - operation: The operation name carried by the request.
    ///   - message: The exception message.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(id: Identity, facet: String, operation: String, file: String = #file, line: Int32 = #line) {
        self.init(
            typeName: "FacetNotExistException", id: id, facet: facet, operation: operation, file: file, line: line)
    }

    /// Creates a FacetNotExistException. The request details (id, facet, operation) will be filled-in by the Ice
    /// runtime when the exception is marshaled.
    /// - Parameters:
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(file: String = #file, line: Int32 = #line) {
        self.init("dispatch failed with FacetNotExistException", file: file, line: line)
    }

    override public class func ice_staticId() -> String { "::Ice::FacetNotExistException" }
}

/// The dispatch could not find the operation carried by the request on the target servant. This is typically due
/// to a mismatch in the Slice definitions, such as the client using Slice definitions newer than the server's.
public final class OperationNotExistException: RequestFailedException {
    /// Creates an OperationNotExistException.
    /// - Parameters:
    ///   - id: The identity of the target Ice object carried by the request.
    ///   - facet: The facet of the target Ice object.
    ///   - operation: The operation name carried by the request.
    ///   - message: The exception message.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(id: Identity, facet: String, operation: String, file: String = #file, line: Int32 = #line) {
        self.init(
            typeName: "OperationNotExistException", id: id, facet: facet, operation: operation, file: file, line: line)
    }

    /// Creates an OperationNotExistException. The request details (id, facet, operation) will be filled-in by the Ice
    /// runtime when the exception is marshaled.
    /// - Parameters:
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(file: String = #file, line: Int32 = #line) {
        self.init("dispatch failed with OperationNotExistException", file: file, line: line)
    }

    override public class func ice_staticId() -> String { "::Ice::OperationNotExistException" }
}

/// The dispatch failed with an exception that is not an `Ice.LocalException` or an `Ice.UserException`.
public class UnknownException: LocalException {
    @available(*, deprecated, renamed: "message")
    public var reason: String { message }

    override public class func ice_staticId() -> String { "::Ice::UnknownException" }
}

/// The dispatch failed with an `Ice.LocalException` that is not one of the special marshal-able local exceptions.
public final class UnknownLocalException: UnknownException {
    override public class func ice_staticId() -> String { "::Ice::UnknownLocalException" }
}

/// The dispatch returned an `Ice.UserException` that was not declared in the operation's exception specification.
public final class UnknownUserException: UnknownException {
    /// Creates an UnknownUserException.
    /// - Parameters:
    ///   - badTypeId: The type ID of the user exception carried by the reply.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(badTypeId: String, file: String = #file, line: Int32 = #line) {
        self.init(
            "the user exception carried by the reply does not conform to the operation's exception specification: \(badTypeId)",
            file: file, line: line)
    }

    override public class func ice_staticId() -> String { "::Ice::UnknownUserException" }
}

//
// Protocol exceptions
//

/// The base class for Ice protocol exceptions.
public class ProtocolException: LocalException {
    override public class func ice_staticId() -> String { "::Ice::ProtocolException" }
}

/// This exception indicates that the connection has been gracefully shut down by the server. The operation call that
/// caused this exception has not been executed by the server. In most cases you will not get this exception, because
/// the client will automatically retry the operation call in case the server shut down the connection. However, if
/// upon retry the server shuts down the connection again, and the retry limit has been reached, then this exception is
/// propagated to the application code.
public final class CloseConnectionException: ProtocolException {
    override public class func ice_staticId() -> String { "::Ice::CloseConnectionException" }
}

/// A datagram exceeds the configured size. This exception is raised if a datagram exceeds the configured send or
/// receive buffer size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
public final class DatagramLimitException: ProtocolException {
    override public class func ice_staticId() -> String { "::Ice::DatagramLimitException" }
}

/// This exception is raised for errors during marshaling or unmarshaling data.
public final class MarshalException: ProtocolException {
    override public class func ice_staticId() -> String { "::Ice::MarshalException" }
}

//
// Timeout exceptions
//

/// This exception indicates a timeout condition.
public class TimeoutException: LocalException {
    override public class func ice_staticId() -> String { "::Ice::TimeoutException" }
}

/// This exception indicates a connection establishment timeout condition.
public final class ConnectTimeoutException: TimeoutException {
    override public class func ice_staticId() -> String { "::Ice::ConnectTimeoutException" }
}

/// This exception indicates a connection closure timeout condition.
public final class CloseTimeoutException: TimeoutException {
    override public class func ice_staticId() -> String { "::Ice::CloseTimeoutException" }
}

/// This exception indicates that an invocation failed because it timed out.
public final class InvocationTimeoutException: TimeoutException {
    override public class func ice_staticId() -> String { "::Ice::InvocationTimeoutException" }
}

//
// Syscall exceptions
//

/// This exception is raised if a system error occurred in the server or client process.
public class SyscallException: LocalException {
    override public class func ice_staticId() -> String { "::Ice::SyscallException" }
}

/// This exception indicates a DNS problem.
public final class DNSException: SyscallException {
    override public class func ice_staticId() -> String { "::Ice::DNSException" }
}

//
// Socket exceptions
//

/// This exception indicates a socket error.
public class SocketException: SyscallException {
    override public class func ice_staticId() -> String { "::Ice::SocketException" }
}

/// This exception indicates a connection failure.
public class ConnectFailedException: SocketException {
    override public class func ice_staticId() -> String { "::Ice::ConnectFailedException" }
}

/// This exception indicates a connection failure for which the server host actively refuses a connection.
public final class ConnectionRefusedException: ConnectFailedException {
    override public class func ice_staticId() -> String { "::Ice::ConnectionRefusedException" }
}

/// This exception indicates a lost connection.
public final class ConnectionLostException: SocketException {
    override public class func ice_staticId() -> String { "::Ice::ConnectionLostException" }
}

//
// Other leaf local exceptions in alphabetical order.
//

/// An attempt was made to register something more than once with the Ice run time. This exception is raised if an
/// attempt is made to register a servant, servant locator, facet, value factory, plug-in, object adapter, object, or
/// user exception factory more than once for the same ID.
public final class AlreadyRegisteredException: LocalException {
    /// The kind of object that could not be removed: "servant", "facet", "object", "default servant",
    /// "servant locator", "value factory", "plugin", "object adapter", "object adapter with router", "replica group".
    public let kindOfObject: String

    /// The ID (or name) of the object that is registered already.
    public let id: String

    /// Creates an AlreadyRegisteredException.
    /// - Parameters:
    ///   - kindOfObject: The kind of object that is already registered.
    ///   - id: The ID (or name) of the object that is already registered.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public init(kindOfObject: String, id: String, file: String = #file, line: Int32 = #line) {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init("another \(kindOfObject) is already registered with ID '\(id)'", file: file, line: line)
    }

    /// Internal initializer - don't use.
    public required init(message: String, cxxDescription: String, file: String, line: Int32) {
        fatalError("AlreadyRegisteredException must be initialized with a kindOfObject and id")
    }

    override public class func ice_staticId() -> String { "::Ice::AlreadyRegisteredException" }

    // Initializer for C++ exceptions
    internal init(kindOfObject: String, id: String, message: String, cxxDescription: String, file: String, line: Int32)
    {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init(message: message, cxxDescription: cxxDescription, file: file, line: line)
    }
}

/// This exception is raised if the Communicator has been destroyed.
public final class CommunicatorDestroyedException: LocalException {
    override public class func ice_staticId() -> String { "::Ice::CommunicatorDestroyedException" }
}

/// This exception indicates that a connection was aborted by the idle check.
public class ConnectionIdleException: LocalException {
    override public class func ice_staticId() -> String { "::Ice::ConnectionIdleException" }
}

/// To be removed
public final class ConnectionManuallyClosedException: LocalException {
    public var graceful: Bool

    public init(graceful: Bool, file: String = #file, line: Int32 = #line) {
        self.graceful = graceful
        super.init("connection was manually closed", file: file, line: line)
    }

    public required init(message: String, cxxDescription: String, file: String, line: Int32) {
        fatalError("ConnectionManuallyClosedException must be initialized with a graceful flag")
    }

    override public class func ice_staticId() -> String { "::Ice::ConnectionManuallyClosedException" }

    internal init(graceful: Bool, message: String, cxxDescription: String, file: String, line: Int32) {
        self.graceful = graceful
        super.init(message: message, cxxDescription: cxxDescription, file: file, line: line)
    }
}

/// Represents a C++ local exception or a std::exception without its own corresponding Swift class.
internal final class CxxLocalException: LocalException {
    private let typeId: String

    override public func ice_id() -> String { typeId }

    internal init(typeId: String, message: String, cxxDescription: String, file: String, line: Int32) {
        self.typeId = typeId
        super.init(message: message, cxxDescription: cxxDescription, file: file, line: line)
    }

    internal required init(message: String, cxxDescription: String, file: String, line: Int32) {
        fatalError("CxxLocalException must be initialized with a typeId")
    }
}

/// This exception is raised if an unsupported feature is used.
public final class FeatureNotSupportedException: LocalException {
    override public class func ice_staticId() -> String { "::Ice::FeatureNotSupportedException" }
}

/// This exception indicates that an attempt has been made to change the connection properties of a fixed proxy.
public final class FixedProxyException: LocalException {
    override public class func ice_staticId() -> String { "::Ice::FixedProxyException" }
}

/// This exception is raised when a failure occurs during initialization.
public final class InitializationException: LocalException {
    override public class func ice_staticId() -> String { "::Ice::InitializationException" }
}

/// This exception indicates that an asynchronous invocation failed because it was canceled explicitly by the user.
public final class InvocationCanceledException: LocalException {
    override public class func ice_staticId() -> String { "::Ice::InvocationCanceledException" }
}

/// This exception is raised if no suitable endpoint is available.
public final class NoEndpointException: LocalException {
    /// Creates a NoEndpointException.
    /// - Parameters:
    ///   - proxy: The proxy that carries the endpoints.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(proxy: ObjectPrx, file: String = #file, line: Int32 = #line) {
        self.init("no suitable endpoint available for proxy '\(proxy)'", file: file, line: line)
    }

    override public class func ice_staticId() -> String { "::Ice::NoEndpointException" }
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

    /// Creates a NotRegisteredException.
    /// - Parameters:
    ///   - kindOfObject: The kind of object that is not registered.
    ///   - id: The ID (or name) of the object that is not registered.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public init(kindOfObject: String, id: String, file: String = #file, line: Int32 = #line) {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init("no \(kindOfObject) is registered with ID '\(id)'", file: file, line: line)
    }

    /// Internal initializer - don't use.
    public required init(message: String, cxxDescription: String, file: String, line: Int32) {
        fatalError("NotRegisteredException must be initialized with a kindOfObject and id")
    }

    override public class func ice_staticId() -> String { "::Ice::NotRegisteredException" }

    // Initializer for C++ exceptions
    internal init(kindOfObject: String, id: String, message: String, cxxDescription: String, file: String, line: Int32)
    {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init(message: message, cxxDescription: cxxDescription, file: file, line: line)
    }
}

/// This exception is raised if an attempt is made to use a deactivated ObjectAdapter.
public final class ObjectAdapterDeactivatedException: LocalException {
    /// Creates an ObjectAdapterDeactivatedException.
    /// - Parameters:
    ///   - name: The name of the object adapter.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(name: String, file: String = #file, line: Int32 = #line) {
        self.init("object adapter '\(name)' is deactivated", file: file, line: line)
    }

    override public class func ice_staticId() -> String { "::Ice::ObjectAdapterDeactivatedException" }
}

/// This exception is raised if an ObjectAdapter cannot be activated. This happens if the Locator
/// detects another active ObjectAdapter with the same adapter id.
public final class ObjectAdapterIdInUseException: LocalException {
    /// Creates an ObjectAdapterIdInUseException.
    /// - Parameters:
    ///   - id: The adapter ID that is already active in the Locator.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(id: String, file: String = #file, line: Int32 = #line) {
        self.init("an object adapter with adapter ID'\(id)' is already active", file: file, line: line)
    }

    override public class func ice_staticId() -> String { "::Ice::ObjectAdapterIdInUseException" }
}

/// This exception is raised if there was an error while parsing a string.
public final class ParseException: LocalException {
    override public class func ice_staticId() -> String { "::Ice::ParseException" }
}

/// This exception indicates that a failure occurred while initializing a plug-in.
public final class PluginInitializationException: LocalException {
    override public class func ice_staticId() -> String { "::Ice::PluginInitializationException" }
}

/// This exception indicates a failure in a security subsystem, such as the IceSSL plug-in.
public final class SecurityException: LocalException {
    override public class func ice_staticId() -> String { "::Ice::SecurityException" }
}

/// The operation can only be invoked with a twoway request. This exception is raised if an attempt is made to invoke
/// an operation with ice_oneway, ice_batchOneway, ice_datagram, or
/// ice_batchDatagram and the operation has a return value, out-parameters, or an exception specification.
public final class TwowayOnlyException: LocalException {
    /// Creates a TwowayOnlyException.
    /// - Parameters:
    ///   - operation: The name of the two-way only operation.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(operation: String, file: String = #file, line: Int32 = #line) {
        self.init(
            "cannot invoke operation '\(operation)' with a oneway, batchOneway, datagram, or batchDatagram proxy",
            file: file, line: line)
    }

    override public class func ice_staticId() -> String { "::Ice::TwowayOnlyException" }
}
