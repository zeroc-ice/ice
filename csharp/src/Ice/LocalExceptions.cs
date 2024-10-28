// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

// This file contains all the exception classes derived from LocalException defined in the Ice assembly.

//
// The 6 (7 with the RequestFailedException base class) special local exceptions that can be marshaled in an Ice reply
// message. Other local exceptions can't be marshaled.
//

/// <summary>
/// The base exception for the 3 NotExist exceptions.
/// </summary>
public class RequestFailedException : LocalException
{
    /// <summary>
    /// Gets the identity of the Ice Object to which the request was sent.
    /// </summary>
    public Identity id { get; }

    /// <summary>
    /// Gets the facet to which the request was sent.
    /// </summary>
    public string facet { get; }

    /// <summary>
    /// Gets the operation name of the request.
    /// </summary>
    public string operation { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="RequestFailedException" /> class.
    /// </summary>
    /// <param name="typeName">The type name of the exception used to construct the exception message.</param>
    /// <param name="id">The identity of the Ice Object to which the request was sent.</param>
    /// <param name="facet">The facet to which the request was sent.</param>
    /// <param name="operation">The operation name of the request.</param>
    protected RequestFailedException(string typeName, Identity id, string facet, string operation)
        : base(createMessage(typeName, id, facet, operation))
    {
        this.id = id;
        this.facet = facet;
        this.operation = operation;
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="RequestFailedException" /> class.
    /// </summary>
    /// <param name="typeName">The type name of the exception used to construct the exception message.</param>
    protected RequestFailedException(string typeName)
        : base($"Dispatch failed with {typeName}.")
    {
        id = new Identity();
        facet = "";
        operation = "";
    }

    internal static string createMessage(string typeName, Identity id, string facet, string operation) =>
        $"Dispatch failed with {typeName} {{ id = '{Util.identityToString(id)}', facet = '{facet}', operation = '{operation}' }}";
}

/// <summary>
/// The dispatch could not find a servant for the identity carried by the request.
/// </summary>
public sealed class ObjectNotExistException : RequestFailedException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ObjectNotExistException" /> class.
    /// </summary>
    public ObjectNotExistException()
        : base(nameof(ObjectNotExistException))
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="ObjectNotExistException" /> class.
    /// </summary>
    /// <param name="id">The identity of the Ice Object to which the request was sent.</param>
    /// <param name="facet">The facet to which the request was sent.</param>
    /// <param name="operation">The operation name of the request.</param>
    public ObjectNotExistException(Identity id, string facet, string operation)
        : base(nameof(ObjectNotExistException), id, facet, operation)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::ObjectNotExistException";
}

/// <summary>
/// The dispatch could not find a servant for the identity + facet carried by the request.
/// </summary>
public sealed class FacetNotExistException : RequestFailedException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="FacetNotExistException" /> class.
    /// </summary>
    public FacetNotExistException()
        : base(nameof(FacetNotExistException))
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="FacetNotExistException" /> class.
    /// </summary>
    /// <param name="id">The identity of the Ice Object to which the request was sent.</param>
    /// <param name="facet">The facet to which the request was sent.</param>
    /// <param name="operation">The operation name of the request.</param>
    public FacetNotExistException(Identity id, string facet, string operation)
        : base(nameof(FacetNotExistException), id, facet, operation)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::FacetNotExistException";
}

/// <summary>
/// The dispatch could not find the operation carried by the request on the target servant. This is typically due
/// to a mismatch in the Slice definitions, such as the client using Slice definitions newer than the server's.
/// </summary>
public sealed class OperationNotExistException : RequestFailedException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="OperationNotExistException" /> class.
    /// </summary>
    public OperationNotExistException()
        : base(nameof(OperationNotExistException))
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="OperationNotExistException" /> class.
    /// </summary>
    /// <param name="id">The identity of the Ice Object to which the request was sent.</param>
    /// <param name="facet">The facet to which the request was sent.</param>
    /// <param name="operation">The operation name of the request.</param>
    public OperationNotExistException(Identity id, string facet, string operation)
        : base(nameof(OperationNotExistException), id, facet, operation)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::OperationNotExistException";
}

/// <summary>
/// The dispatch failed with an exception that is not a <see cref="LocalException"/> or a <see cref="UserException"/>.
/// </summary>
public class UnknownException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="UnknownException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    public UnknownException(string message)
        : base(message)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::UnknownException";
}

/// <summary>
/// The dispatch failed with a <see cref="LocalException" /> that is not one of the special marshal-able local
/// exceptions.
/// </summary>
public sealed class UnknownLocalException : UnknownException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="UnknownLocalException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    public UnknownLocalException(string message)
        : base(message)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::UnknownLocalException";
}

/// <summary>
/// The dispatch returned a <see cref="UserException" /> that was not declared in the operation's exception
/// specification.
/// </summary>
public sealed class UnknownUserException : UnknownException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="UnknownUserException" /> class from a user exception type ID.
    /// </summary>
    /// <param name="typeId">The type ID of the user exception.</param>
    /// <returns>The new instance of the <see cref="UnknownUserException" /> class.</returns>
    public static UnknownUserException fromTypeId(string typeId) =>
        new($"The reply carries a user exception that does not conform to the operation's exception specification: {typeId}");

    /// <summary>
    /// Initializes a new instance of the <see cref="UnknownUserException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    public UnknownUserException(string message)
        : base(message)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::UnknownUserException";
}

//
// Protocol exceptions
//

/// <summary>
/// The base class for Ice protocol exceptions.
/// </summary>
public class ProtocolException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ProtocolException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    public ProtocolException(string message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::ProtocolException";
}

/// <summary>
/// This exception indicates that the connection has been gracefully closed by the server.
/// The operation call that caused this exception has not been executed by the server. In most cases you will not get
/// this exception because the client will automatically retry the operation call in case the server shut down the
/// connection. However, if upon retry the server shuts down the connection again, and the retry limit has been reached,
/// then this exception is propagated to the application code.
/// </summary>
public sealed class CloseConnectionException : ProtocolException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="CloseConnectionException" /> class.
    /// </summary>
    public CloseConnectionException()
        : base("Connection closed by the peer.")
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::CloseConnectionException";
}

/// <summary>
/// A datagram exceeds the configured size.
/// This exception is raised if a datagram exceeds the configured send or receive buffer size, or exceeds the maximum
/// payload size of a UDP packet (65507 bytes).
/// </summary>
public sealed class DatagramLimitException : ProtocolException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="DatagramLimitException" /> class.
    /// </summary>
    public DatagramLimitException()
        : base("Datagram limit exceeded.")
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::DatagramLimitException";
}

/// <summary>
/// This exception reports an error during marshaling or unmarshaling.
/// </summary>
public sealed class MarshalException : ProtocolException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="MarshalException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    public MarshalException(string message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::MarshalException";
}

//
// Timeout exceptions
//

/// <summary>This exception indicates a timeout condition.</summary>
public class TimeoutException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="TimeoutException" /> class.
    /// </summary>
    /// <param name="message">The exception message or null to use the default message.</param>
    public TimeoutException(string? message = null)
        : base(message ?? "Operation timed out.")
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::TimeoutException";
}

/// <summary>This exception indicates a connection closure timeout condition.</summary>
public sealed class CloseTimeoutException : TimeoutException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="CloseTimeoutException" /> class.
    /// </summary>
    public CloseTimeoutException()
        : base("Close timed out.")
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::CloseTimeoutException";
}

/// <summary>
/// This exception indicates a connection establishment timeout condition.
/// </summary>
public sealed class ConnectTimeoutException : TimeoutException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ConnectTimeoutException" /> class.
    /// </summary>
    public ConnectTimeoutException()
        : base("Connect timed out.")
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::ConnectTimeoutException";
}

/// <summary>
/// This exception indicates that an invocation failed because it timed out.
/// </summary>
public sealed class InvocationTimeoutException : TimeoutException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="InvocationTimeoutException" /> class.
    /// </summary>
    public InvocationTimeoutException()
        : base("Invocation timed out.")
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::InvocationTimeoutException";
}

//
// Syscall exceptions
//

/// <summary>
/// This exception is raised if a system error occurred in the server or client process.
/// </summary>
public class SyscallException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="SyscallException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    public SyscallException(string? message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="SyscallException" /> class.
    /// </summary>
    /// <param name="innerException">The inner exception.</param>
    public SyscallException(System.Exception innerException)
        : this(message: null, innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::SyscallException";
}

/// <summary>
/// This exception indicates a DNS problem.
/// </summary>
public sealed class DNSException : SyscallException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="DNSException" /> class.
    /// </summary>
    /// <param name="host">The host name that could not be resolved.</param>
    /// <param name="innerException">The inner exception.</param>
    public DNSException(string host, System.Exception? innerException = null)
        : base($"Cannot resolve host '{host}'", innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::DNSException";
}

/// <summary>This exception indicates a file error occurred.</summary>
public sealed class FileException : SyscallException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="FileException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    public FileException(string message, System.Exception innerException)
        : base(message, innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::FileException";
}

//
// Socket exceptions
//

/// <summary>
/// This exception indicates a socket error.
/// </summary>
public class SocketException : SyscallException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="SocketException" /> class.
    /// </summary>
    /// <param name="innerException">The inner exception.</param>
    public SocketException(System.Exception? innerException = null)
        : base(message: null, innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::SocketException";
}

/// <summary>
/// This exception indicates a connection failure.
/// </summary>
public class ConnectFailedException : SocketException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ConnectFailedException" /> class.
    /// </summary>
    /// <param name="innerException">The inner exception.</param>
    public ConnectFailedException(System.Exception? innerException = null)
        : base(innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::ConnectFailedException";
}

/// <summary>
/// This exception indicates a lost connection.
/// </summary>
public sealed class ConnectionLostException : SocketException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ConnectionLostException" /> class.
    /// </summary>
    /// <param name="innerException">The inner exception.</param>
    public ConnectionLostException(System.Exception? innerException = null)
        : base(innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::ConnectionLostException";
}

/// <summary>
/// This exception indicates a connection failure for which the server host actively refuses a connection.
/// </summary>
public sealed class ConnectionRefusedException : ConnectFailedException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ConnectionRefusedException" /> class.
    /// </summary>
    /// <param name="innerException">The inner exception.</param>
    public ConnectionRefusedException(System.Exception? innerException = null)
        : base(innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::ConnectionRefusedException";
}

//
// Other leaf local exceptions in alphabetical order.
//

/// <summary>
/// An attempt was made to register something more than once with the Ice run time.
/// This exception is raised if an attempt is made to register a servant, servant locator, facet, value factory,
/// plug-in, object adapter (etc.) more than once for the same ID.
/// </summary>
public sealed class AlreadyRegisteredException : LocalException
{
    /// <summary>
    /// Gets the kind of object that was already registered.
    /// </summary>
    public string kindOfObject { get; }

    /// <summary>
    /// Gets the ID or name of the object that was already registered.
    /// </summary>
    public string id { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="AlreadyRegisteredException" /> class.
    /// </summary>
    /// <param name="kindOfObject">The kind of object that was already registered.</param>
    /// <param name="id">The ID or name of the object that was already registered.</param>
    public AlreadyRegisteredException(string kindOfObject, string id)
        : base($"Another {kindOfObject} is already registered with ID '{id}'.")
    {
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::AlreadyRegisteredException";
}

/// <summary>
/// This exception is raised if the Communicator has been destroyed.
/// </summary>
public sealed class CommunicatorDestroyedException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="CommunicatorDestroyedException" /> class.
    /// </summary>
    public CommunicatorDestroyedException()
        : base("Communicator destroyed.")
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::CommunicatorDestroyedException";
}

/// <summary>
/// This exception indicates that a connection was closed forcefully.
/// </summary>
public sealed class ConnectionAbortedException : LocalException
{
    /// <summary>
    /// Gets a value indicating whether the connection was aborted by the application.
    /// </summary>
    /// <value><c>true</c> if the connection was aborted by the application; otherwise, <c>false</c>.</value>
    public bool closedByApplication { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="ConnectionAbortedException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="closedByApplication">A value indicating whether the connection was aborted by the application.
    /// </param>
    public ConnectionAbortedException(string message, bool closedByApplication)
        : base(message) =>
        this.closedByApplication = closedByApplication;

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::ConnectionAbortedException";
}

/// <summary>
/// This exception indicates that a connection was closed gracefully.
/// </summary>
public sealed class ConnectionClosedException : LocalException
{
    /// <summary>
    /// Gets a value indicating whether the connection was closed by the application.
    /// </summary>
    /// <value><c>true</c> if the connection was closed by the application; otherwise, <c>false</c>.</value>
    public bool closedByApplication { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="ConnectionClosedException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="closedByApplication">A value indicating whether the connection was closed by the application.
    /// </param>
    public ConnectionClosedException(string message, bool closedByApplication)
        : base(message) =>
        this.closedByApplication = closedByApplication;

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::ConnectionClosedException";
}

/// <summary>
/// This exception is raised if an unsupported feature is used.
/// </summary>
public sealed class FeatureNotSupportedException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="FeatureNotSupportedException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    public FeatureNotSupportedException(string message)
        : base(message)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::FeatureNotSupportedException";
}

/// <summary>
/// This exception indicates that an attempt has been made to change the connection properties of a fixed proxy.
/// </summary>
public sealed class FixedProxyException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="FixedProxyException" /> class.
    /// </summary>
    public FixedProxyException()
        : base("Cannot change the connection properties of a fixed proxy.")
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::FixedProxyException";
}

/// <summary>
/// This exception is raised when a failure occurs during initialization.
/// </summary>
public sealed class InitializationException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="InitializationException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    public InitializationException(string message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::InitializationException";
}

/// <summary>
/// This exception indicates that an asynchronous invocation failed because it was canceled explicitly by the user.
/// </summary>
public sealed class InvocationCanceledException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="InvocationCanceledException" /> class.
    /// </summary>
    public InvocationCanceledException()
        : base("Invocation canceled.")
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::InvocationCanceledException";
}

/// <summary>
/// This exception is raised if no suitable endpoint is available.
/// </summary>
public sealed class NoEndpointException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="NoEndpointException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    public NoEndpointException(string message)
        : base(message)
    {
    }

    /// <summary>
    /// Initializes a new instance of the <see cref="NoEndpointException" /> class.
    /// </summary>
    /// <param name="proxy">The proxy for which no suitable endpoint is available.</param>
    public NoEndpointException(ObjectPrx proxy)
        : base($"No suitable endpoint available for proxy '{proxy}'.")
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::NoEndpointException";
}

/// <summary>
/// An attempt was made to find or deregister something that is not registered with the Ice run time or Ice locator.
/// This exception is raised if an attempt is made to remove a servant, servant locator, facet, value factory, plug-in,
/// object adapter (etc.) that is not currently registered. It's also raised if the Ice locator can't find an object or
/// object adapter when resolving an indirect proxy or when an object adapter is activated.
/// </summary>
public sealed class NotRegisteredException : LocalException
{
    /// <summary>
    /// Gets the kind of object that was not registered.
    /// </summary>
    public string kindOfObject { get; }

    /// <summary>
    /// Gets the ID or name of the object that was not registered.
    /// </summary>
    public string id { get; }

    /// <summary>
    /// Initializes a new instance of the <see cref="NotRegisteredException" /> class.
    /// </summary>
    /// <param name="kindOfObject">The kind of object that was not registered.</param>
    /// <param name="id">The ID or name of the object that was not registered.</param>
    public NotRegisteredException(string kindOfObject, string id)
       : base($"No {kindOfObject} is registered with ID '{id}'.")
    {
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::NotRegisteredException";
}

/// <summary>
/// This exception is raised if an attempt is made to use a deactivated ObjectAdapter.
/// </summary>
public sealed class ObjectAdapterDeactivatedException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ObjectAdapterDeactivatedException" /> class.
    /// </summary>
    /// <param name="name">The name of the object adapter that is deactivated.</param>
    public ObjectAdapterDeactivatedException(string name)
        : base($"Object adapter '{name}' is deactivated.")
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::ObjectAdapterDeactivatedException";
}

/// <summary>
/// This exception is raised if an attempt is made to use a destroyed ObjectAdapter.
/// </summary>
public sealed class ObjectAdapterDestroyedException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ObjectAdapterDestroyedException" /> class.
    /// </summary>
    /// <param name="name">The name of the object adapter that is destroyed.</param>
    public ObjectAdapterDestroyedException(string name)
        : base($"Object adapter '{name}' is destroyed.")
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::ObjectAdapterDestroyedException";
}

/// <summary>
/// This exception is raised if an ObjectAdapter cannot be activated.
/// This happens if the Locator detects another active ObjectAdapter with the same adapter ID.
/// </summary>
public sealed class ObjectAdapterIdInUseException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ObjectAdapterIdInUseException" /> class.
    /// </summary>
    /// <param name="adapterId">The adapter ID that is already in use.</param>
    public ObjectAdapterIdInUseException(string adapterId)
        : base($"An object adapter with adapter ID '{adapterId}' is already active.")
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::ObjectAdapterIdInUseException";
}

/// <summary>
/// Reports a failure that occurred while parsing a string.
/// </summary>
public sealed class ParseException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="ParseException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    public ParseException(string message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::ParseException";
}

/// <summary>
/// This exception indicates that a failure occurred while initializing a plug-in.
/// </summary>
public sealed class PluginInitializationException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="PluginInitializationException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    public PluginInitializationException(string message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::PluginInitializationException";
}

/// <summary>
/// This exception indicates a failure in a security subsystem.
/// </summary>
public sealed class SecurityException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="SecurityException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    public SecurityException(string message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::SecurityException";
}

/// <summary>
/// The operation can only be invoked with a two-way request.
/// This exception is raised if an attempt is made to invoke an operation with ice_oneway, ice_batchOneway,
/// ice_datagram, or ice_batchDatagram and the operation has a return value, out-parameters, or an exception
/// specification.
/// </summary>
public sealed class TwowayOnlyException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="TwowayOnlyException" /> class.
    /// </summary>
    /// <param name="operation">The operation name.</param>
    public TwowayOnlyException(string operation)
        : base($"Cannot invoke operation '{operation}' with a oneway, batchOneway, datagram, or batchDatagram proxy.")
    {
    }

    public override string ice_id() => "::Ice::TwowayOnlyException";
}

/// <summary>
/// This exception is raised when there is an error while getting or setting a property. For example, when
/// trying to set an unknown Ice property.
/// </summary>
public sealed class PropertyException : LocalException
{
    /// <summary>
    /// Initializes a new instance of the <see cref="PropertyException" /> class.
    /// </summary>
    /// <param name="message">The exception message.</param>
    /// <param name="innerException">The inner exception.</param>
    public PropertyException(string message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    /// <inheritdoc/>
    public override string ice_id() => "::Ice::PropertyException";
}
