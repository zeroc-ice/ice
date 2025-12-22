// Copyright (c) ZeroC, Inc.

// This file contains all the exception classes derived from LocalException defined in the Ice module.

//
// Dispatch exceptions
//

/// The exception that is thrown when a dispatch failed. This is the base class for local exceptions that can be
/// marshaled and transmitted "over the wire".
/// You can throw this exception in the implementation of an operation, or in a middleware.
/// The Ice runtime then logically rethrows this exception to the client.
public class DispatchException: LocalException, @unchecked Sendable {
    public let replyStatus: UInt8

    /// Creates a DispatchException.
    ///
    /// - Parameters:
    ///   - replyStatus: The reply status raw value. It may not correspond to a valid ``ReplyStatus`` enum value.
    ///   - message: The exception message.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public init(
        replyStatus: UInt8, message: String? = nil, file: String = #fileID, line: Int32 = #line
    ) {
        precondition(
            replyStatus > ReplyStatus.userException.rawValue, "replyStatus must be greater than .userException")
        self.replyStatus = replyStatus
        super.init(Self.makeDispatchFailedMessage(replyStatus: replyStatus, message: message), file: file, line: line)
    }

    // Don't use.
    internal required init(_ message: String, file: String, line: Int32) {
        fatalError("DispatchException must be initialized with a reply status")
    }

    private class func makeDispatchFailedMessage(replyStatus: UInt8, message: String?) -> String {
        if let message {
            return message
        } else {
            if let replyStatusEnum = ReplyStatus(rawValue: replyStatus) {
                return "The dispatch failed with reply status \(replyStatusEnum)."
            } else {
                return "The dispatch failed with reply status \(replyStatus)."
            }
        }
    }
}

/// The base exception for the 3 NotExist exceptions.
public class RequestFailedException: DispatchException, @unchecked Sendable {
    /// The identity of the Ice Object to which the request was sent.
    public let id: Identity

    /// The facet to which the request was sent.
    public let facet: String

    /// The operation name of the request.
    public let operation: String

    internal init(replyStatus: ReplyStatus, id: Identity, facet: String, operation: String, file: String, line: Int32) {
        self.id = id
        self.facet = facet
        self.operation = operation
        super.init(
            replyStatus: replyStatus.rawValue,
            message: Self.makeMessage(replyStatus: replyStatus, id: id, facet: facet, operation: operation), file: file,
            line: line)
    }

    internal convenience init(replyStatus: ReplyStatus, file: String, line: Int32) {
        // The request details (id, facet, operation) will be filled-in by the Ice runtime when the exception is marshaled,
        // while the message is computed by DispatchException.
        self.init(replyStatus: replyStatus, id: Identity(), facet: "", operation: "", file: file, line: line)
    }

    internal required init(_ message: String, file: String, line: Int32) {
        fatalError("RequestFailedException cannot be initialized with a message")
    }

    private class func makeMessage(replyStatus: ReplyStatus, id: Identity, facet: String, operation: String) -> String?
    {
        id.name.isEmpty
            ? nil
            : "Dispatch failed with \(replyStatus) { id = '\(identityToString(id: id))', facet = '\(facet)', operation = '\(operation)' }"
    }
}

/// The exception that is thrown when a dispatch could not find a servant for the identity carried by the request.
public final class ObjectNotExistException: RequestFailedException, @unchecked Sendable {
    /// Creates an ObjectNotExistException.
    ///
    /// - Parameters:
    ///   - id: The identity of the target Ice object carried by the request.
    ///   - facet: The facet of the target Ice object.
    ///   - operation: The operation name carried by the request.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(
        id: Identity, facet: String, operation: String, file: String = #fileID, line: Int32 = #line
    ) {
        self.init(
            replyStatus: .objectNotExist, id: id, facet: facet, operation: operation, file: file, line: line)
    }

    /// Creates an ObjectNotExistException. The request details (id, facet, operation) will be filled-in by the Ice
    /// runtime when the exception is marshaled.
    ///
    /// - Parameters:
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(file: String = #fileID, line: Int32 = #line) {
        self.init(replyStatus: .objectNotExist, file: file, line: line)
    }
}

/// The dispatch could not find a servant for the identity + facet carried by the request.
public final class FacetNotExistException: RequestFailedException, @unchecked Sendable {
    /// Creates a FacetNotExistException.
    ///
    /// - Parameters:
    ///   - id: The identity of the target Ice object carried by the request.
    ///   - facet: The facet of the target Ice object.
    ///   - operation: The operation name carried by the request.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(
        id: Identity, facet: String, operation: String, file: String = #fileID, line: Int32 = #line
    ) {
        self.init(
            replyStatus: .facetNotExist, id: id, facet: facet, operation: operation, file: file, line: line)
    }

    /// Creates a FacetNotExistException. The request details (id, facet, operation) will be filled-in by the Ice
    /// runtime when the exception is marshaled.
    ///
    /// - Parameters:
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(file: String = #fileID, line: Int32 = #line) {
        self.init(replyStatus: .facetNotExist, file: file, line: line)
    }
}

/// The dispatch could not find the operation carried by the request on the target servant. This is typically due
/// to a mismatch in the Slice definitions, such as the client using Slice definitions newer than the server's.
public final class OperationNotExistException: RequestFailedException, @unchecked Sendable {
    /// Creates an OperationNotExistException.
    ///
    /// - Parameters:
    ///   - id: The identity of the target Ice object carried by the request.
    ///   - facet: The facet of the target Ice object.
    ///   - operation: The operation name carried by the request.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(
        id: Identity, facet: String, operation: String, file: String = #fileID, line: Int32 = #line
    ) {
        self.init(
            replyStatus: .operationNotExist, id: id, facet: facet, operation: operation, file: file, line: line)
    }

    /// Creates an OperationNotExistException. The request details (id, facet, operation) will be filled-in by the Ice
    /// runtime when the exception is marshaled.
    ///
    /// - Parameters:
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(file: String = #fileID, line: Int32 = #line) {
        self.init(replyStatus: .operationNotExist, file: file, line: line)
    }
}

/// The exception that is thrown when a dispatch failed with an exception that is not a `LocalException` or a
/// `UserException`.
public class UnknownException: DispatchException, @unchecked Sendable {
    @available(*, deprecated, renamed: "message")
    public var reason: String { message }

    public required init(_ message: String, file: String = #fileID, line: Int32 = #line) {
        super.init(replyStatus: ReplyStatus.unknownException.rawValue, message: message, file: file, line: line)
    }

    internal init(replyStatus: ReplyStatus, message: String, file: String, line: Int32) {
        super.init(replyStatus: replyStatus.rawValue, message: message, file: file, line: line)
    }
}

/// The exception that is thrown when a dispatch failed with a `LocalException` that is not a `DispatchException`.
public final class UnknownLocalException: UnknownException, @unchecked Sendable {
    public required init(_ message: String, file: String = #fileID, line: Int32 = #line) {
        super.init(replyStatus: .unknownLocalException, message: message, file: file, line: line)
    }
}

/// The exception that is thrown when a client receives a `UserException` that was not declared in the operation's
/// exception specification.
public final class UnknownUserException: UnknownException, @unchecked Sendable {
    public required init(_ message: String, file: String = #fileID, line: Int32 = #line) {
        super.init(replyStatus: .unknownUserException, message: message, file: file, line: line)
    }

    /// Creates an UnknownUserException.
    ///
    /// - Parameters:
    ///   - badTypeId: The type ID of the user exception carried by the reply.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(badTypeId: String, file: String = #fileID, line: Int32 = #line) {
        self.init(
            "the user exception carried by the reply does not conform to the operation's exception specification: \(badTypeId)",
            file: file, line: line)
    }
}

//
// Protocol exceptions
//

/// The base class for exceptions related to the Ice protocol.
public class ProtocolException: LocalException, @unchecked Sendable {}

/// The exception that is thrown when the connection has been gracefully shut down by the server. The request that
/// returned this exception has not been executed by the server. In most cases you will not get this exception,
/// because the client will automatically retry the invocation. However, if upon retry the server shuts down the
/// connection again and the retry limit has been reached, this exception is propagated to the application code.
public final class CloseConnectionException: ProtocolException, @unchecked Sendable {}

/// The exception that is thrown when a datagram exceeds the configured send or receive buffer size, or exceeds the
/// maximum payload size of a UDP packet (65507 bytes).
public final class DatagramLimitException: ProtocolException, @unchecked Sendable {}

/// The exception that is thrown when an error occurs during marshaling or unmarshaling.
public final class MarshalException: ProtocolException, @unchecked Sendable {}

//
// Timeout exceptions
//

/// The exception that is thrown when a timeout occurs. This is the base class for all timeout exceptions.
public class TimeoutException: LocalException, @unchecked Sendable {}

/// The exception that is thrown when a connection establishment times out.
public final class ConnectTimeoutException: TimeoutException, @unchecked Sendable {}

/// The exception that is thrown when a graceful connection closure times out.
public final class CloseTimeoutException: TimeoutException, @unchecked Sendable {}

/// The exception that is thrown when an invocation times out.
public final class InvocationTimeoutException: TimeoutException, @unchecked Sendable {}

//
// Syscall exceptions
//

/// The exception that is thrown to report the failure of a system call.
public class SyscallException: LocalException, @unchecked Sendable {}

/// The exception that is thrown to report a DNS resolution failure.
public final class DNSException: SyscallException, @unchecked Sendable {}

//
// Socket exceptions
//

/// The exception that is thrown to report a socket error.
public class SocketException: SyscallException, @unchecked Sendable {}

/// The exception that is thrown when a connection establishment fails.
public class ConnectFailedException: SocketException, @unchecked Sendable {}

/// The exception that is thrown when the server host actively refuses a connection.
public final class ConnectionRefusedException: ConnectFailedException, @unchecked Sendable {}

/// This exception indicates a lost connection.
public final class ConnectionLostException: SocketException, @unchecked Sendable {}

//
// Other leaf local exceptions in alphabetical order.
//

/// An attempt was made to register something more than once with the Ice run time. This exception is thrown if an
/// attempt is made to register a servant, servant locator, facet,  plug-in, or object adapter more than once for the
// same ID.
public final class AlreadyRegisteredException: LocalException, @unchecked Sendable {
    /// The kind of object that could not be removed: "servant", "facet", "object", "default servant",
    /// "servant locator", "plugin", "object adapter", "object adapter with router", "replica group".
    public let kindOfObject: String

    /// The ID (or name) of the object that is registered already.
    public let id: String

    /// Creates an AlreadyRegisteredException.
    ///
    /// - Parameters:
    ///   - kindOfObject: The kind of object that is already registered.
    ///   - id: The ID (or name) of the object that is already registered.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public init(kindOfObject: String, id: String, file: String = #fileID, line: Int32 = #line) {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init(
            "another \(kindOfObject) is already registered with ID '\(id)'", file: file, line: line)
    }

    // Initializer for C++ exceptions
    internal init(kindOfObject: String, id: String, message: String, file: String, line: Int32) {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init(message, file: file, line: line)
    }

    // Don't use.
    internal required init(_ message: String, file: String, line: Int32) {
        fatalError("AlreadyRegisteredException must be initialized with a kindOfObject and id")
    }
}

/// This exception is raised if the Communicator has been destroyed.
public final class CommunicatorDestroyedException: LocalException, @unchecked Sendable {}

/// This exception indicates that a connection was aborted by the idle check.
public class ConnectionIdleException: LocalException, @unchecked Sendable {}

/// This exception indicates the connection was closed forcefully.
public final class ConnectionAbortedException: LocalException, @unchecked Sendable {
    /// When true, the connection was aborted by the application. When false, the connection was aborted by the Ice
    /// runtime.
    public let closedByApplication: Bool

    internal init(closedByApplication: Bool, message: String, file: String, line: Int32) {
        self.closedByApplication = closedByApplication
        super.init(message, file: file, line: line)
    }

    // Don't use.
    internal required init(_ message: String, file: String, line: Int32) {
        fatalError("ConnectionAbortedException must be initialized with a closedByApplication flag")
    }
}

/// This exception indicates the connection was closed gracefully.
public final class ConnectionClosedException: LocalException, @unchecked Sendable {
    /// When true, the connection was aborted by the application. When false, the connection was aborted by the Ice
    /// runtime.
    public let closedByApplication: Bool

    internal init(closedByApplication: Bool, message: String, file: String, line: Int32) {
        self.closedByApplication = closedByApplication
        super.init(message, file: file, line: line)
    }

    // Don't use.
    internal required init(_ message: String, file: String, line: Int32) {
        fatalError("ConnectionClosedException must be initialized with a closedByApplication flag")
    }
}

/// Represents a C++ local exception or a std::exception without its own corresponding Swift class.
internal final class CxxLocalException: LocalException, @unchecked Sendable {
    private let typeId: String

    override public func ice_id() -> String { typeId }

    internal init(typeId: String, message: String, file: String, line: Int32) {
        self.typeId = typeId
        super.init(message, file: file, line: line)
    }

    // Don't use.
    internal required init(_ message: String, file: String, line: Int32) {
        fatalError("CxxLocalException must be initialized with a typeId")
    }
}

/// This exception is raised if an unsupported feature is used.
public final class FeatureNotSupportedException: LocalException, @unchecked Sendable {}

/// This exception indicates that an attempt has been made to change the connection properties of a fixed proxy.
public final class FixedProxyException: LocalException, @unchecked Sendable {}

/// This exception is raised when a failure occurs during initialization.
public final class InitializationException: LocalException, @unchecked Sendable {}

/// This exception is raised if no suitable endpoint is available.
public final class NoEndpointException: LocalException, @unchecked Sendable {
    /// Creates a NoEndpointException.
    ///
    /// - Parameters:
    ///   - proxy: The proxy that carries the endpoints.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(proxy: ObjectPrx, file: String = #fileID, line: Int32 = #line) {
        self.init("no suitable endpoint available for proxy '\(proxy)'", file: file, line: line)
    }
}

/// An attempt was made to find or deregister something that is not registered with Ice.
public final class NotRegisteredException: LocalException, @unchecked Sendable {
    /// The kind of object that is not registered.
    public let kindOfObject: String

    /// The ID (or name) of the object that is not registered.
    public let id: String

    /// Creates a NotRegisteredException.
    ///
    /// - Parameters:
    ///   - kindOfObject: The kind of object that is not registered.
    ///   - id: The ID (or name) of the object that is not registered.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public init(kindOfObject: String, id: String, file: String = #fileID, line: Int32 = #line) {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init("no \(kindOfObject) is registered with ID '\(id)'", file: file, line: line)
    }

    // Initializer for C++ exceptions
    internal init(kindOfObject: String, id: String, message: String, file: String, line: Int32) {
        self.kindOfObject = kindOfObject
        self.id = id
        super.init(message, file: file, line: line)
    }

    // Don't use.
    internal required init(_ message: String, file: String, line: Int32) {
        fatalError("NotRegisteredException must be initialized with a kindOfObject and id")
    }
}

/// This exception is raised if an attempt is made to use a deactivated ObjectAdapter.
public final class ObjectAdapterDeactivatedException: LocalException, @unchecked Sendable {}

/// This exception is raised if an attempt is made to use a destroyed ObjectAdapter.
public final class ObjectAdapterDestroyedException: LocalException, @unchecked Sendable {}

/// This exception is raised if an ObjectAdapter cannot be activated. This happens if the Locator
/// detects another active ObjectAdapter with the same adapter id.
public final class ObjectAdapterIdInUseException: LocalException, @unchecked Sendable {
    /// Creates an ObjectAdapterIdInUseException.
    ///
    /// - Parameters:
    ///   - id: The adapter ID that is already active in the Locator.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(id: String, file: String = #fileID, line: Int32 = #line) {
        self.init("an object adapter with adapter ID'\(id)' is already active", file: file, line: line)
    }
}

/// This exception is raised if there was an error while parsing a string.
public final class ParseException: LocalException, @unchecked Sendable {}

/// This exception indicates that a failure occurred while initializing a plug-in.
public final class PluginInitializationException: LocalException, @unchecked Sendable {}

/// This exception indicates a failure in a security subsystem, such as the IceSSL plug-in.
public final class SecurityException: LocalException, @unchecked Sendable {}

/// The operation can only be invoked with a twoway request. This exception is raised if an attempt is made to invoke
/// an operation with ice_oneway, ice_batchOneway, ice_datagram, or
/// ice_batchDatagram and the operation has a return value, out-parameters, or an exception specification.
public final class TwowayOnlyException: LocalException, @unchecked Sendable {
    /// Creates a TwowayOnlyException.
    ///
    /// - Parameters:
    ///   - operation: The name of the two-way only operation.
    ///   - file: The file where the exception was thrown.
    ///   - line: The line where the exception was thrown.
    public convenience init(operation: String, file: String = #fileID, line: Int32 = #line) {
        self.init(
            "cannot invoke operation '\(operation)' with a oneway, batchOneway, datagram, or batchDatagram proxy",
            file: file, line: line)
    }
}

/// This exception is raised when there is an error while getting or setting a property.
public final class PropertyException: LocalException, @unchecked Sendable {}
