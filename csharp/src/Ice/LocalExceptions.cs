// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

// This file contains all the exception classes derived from LocalException.

// The 3 NotExist exceptions and the 3 Unknown exceptions are special local exceptions that can be marshaled in an Ice
// reply message. Other local exceptions can't be marshaled.

/// <summary>
/// The base exception for the 3 NotExist exceptions.
/// </summary>
public class RequestFailedException : LocalException
{
    public Identity id { get; set; }
    public string facet { get; set; }
    public string operation { get; set; }

    // We can't set the message in the constructor because id/facet/operation can be set after construction.
    public override string Message =>
        $"{base.Message} id = '{Util.identityToString(id)}', facet = '{facet}', operation = '{operation}'";

    protected RequestFailedException(Identity id, string facet, string operation)
    {
        this.id = id;
        this.facet = facet;
        this.operation = operation;
    }
}

/// <summary>
/// The dispatch could not find a servant for the identity carried by the request.
/// </summary>
public sealed class ObjectNotExistException : RequestFailedException
{
    public ObjectNotExistException()
        : base(new Identity(), "", "")
    {
    }

    public ObjectNotExistException(Identity id, string facet, string operation)
        : base(id, facet, operation)
    {
    }

    public override string ice_id() => "::Ice::ObjectNotExistException";
}

/// <summary>
/// The dispatch could not find a servant for the identity + facet carried by the request.
/// </summary>
public sealed class FacetNotExistException : RequestFailedException
{
    public FacetNotExistException()
        : base(new Identity(), "", "")
    {
    }

    public FacetNotExistException(Identity id, string facet, string operation)
        : base(id, facet, operation)
    {
    }

    public override string ice_id() => "::Ice::FacetNotExistException";
}

/// <summary>
/// The dispatch could not find the operation carried by the request on the target servant. This is typically due
/// to a mismatch in the Slice definitions, such as the client using Slice definitions newer than the server's.
/// </summary>
public sealed class OperationNotExistException : RequestFailedException
{
    public OperationNotExistException()
        : base(new Identity(), "", "")
    {
    }

    public OperationNotExistException(Identity id, string facet, string operation)
        : base(id, facet, operation)
    {
    }

    public override string ice_id() => "::Ice::OperationNotExistException";
}

/// <summary>
/// The dispatch failed with an exception that is not a <see cref="LocalException"/> or a <see cref="UserException"/>.
/// </summary>
public class UnknownException : LocalException
{
    public string unknown => Message;

    public UnknownException(string message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    public override string ice_id() => "::Ice::UnknownException";
}

/// <summary>
/// The dispatch failed with a <see cref="LocalException" /> that is not one of the special marshal-able local
/// exception.
/// </summary>
public sealed class UnknownLocalException : UnknownException
{
    public UnknownLocalException(string message, LocalException? innerException = null)
        : base(message, innerException)
    {
    }

    public override string ice_id() => "::Ice::UnknownLocalException";
}

/// <summary>
/// The dispatch returned a <see cref="UserException" /> that was not declared in the operation's exception
/// specification.
/// </summary>
public sealed class UnknownUserException : UnknownException
{
    public UnknownUserException(string message)
        : base(message, innerException: null)
    {
    }

    public override string ice_id() => "::Ice::UnknownUserException";
}

/// <summary>
/// This exception is raised if the Communicator has been destroyed.
/// </summary>
public sealed class CommunicatorDestroyedException : LocalException
{
    public CommunicatorDestroyedException()
        : base("Communicator destroyed.")
    {
    }

    public override string ice_id() => "::Ice::CommunicatorDestroyedException";
}

/// <summary>
/// This exception indicates that a connection was closed gracefully.
/// </summary>
public sealed class ConnectionClosedException : LocalException
{
    public bool closedByApplication { get; }

    public ConnectionClosedException(string message, bool closedByApplication)
        : base(message, innerException: null) =>
        this.closedByApplication = closedByApplication;

    public override string ice_id() => "::Ice::ConnectionClosedException";
}

/// <summary>
/// This exception indicates that a connection was aborted by the idle check.
/// </summary>
public sealed class ConnectionIdleException : LocalException
{
    public ConnectionIdleException(string message)
        : base(message, innerException: null)
    {
    }

    public override string ice_id() => "::Ice::ConnectionIdleException";
}

/// <summary>
/// This exception is raised if an unsupported feature is used.
/// </summary>
public sealed class FeatureNotSupportedException : LocalException
{
    public FeatureNotSupportedException(string message)
        : base(message)
    {
    }

    public override string ice_id() => "::Ice::FeatureNotSupportedException";
}

/// <summary>
/// This exception indicates that an attempt has been made to change the connection properties of a fixed proxy.
/// </summary>
public sealed class FixedProxyException : LocalException
{
    public FixedProxyException()
        : base("Cannot change the connection properties of a fixed proxy.")
    {
    }

    public override string ice_id() => "::Ice::FixedProxyException";
}

/// <summary>
/// This exception is raised when a failure occurs during initialization.
/// </summary>
public sealed class InitializationException : LocalException
{
    public InitializationException(string message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    public override string ice_id() => "::Ice::InitializationException";
}

/// <summary>
/// This exception indicates that an asynchronous invocation failed because it was canceled explicitly by the user.
/// </summary>
public sealed class InvocationCanceledException : LocalException
{
    public InvocationCanceledException()
        : base("Invocation canceled.")
    {
    }

    public override string ice_id() => "::Ice::InvocationCanceledException";
}

/// <summary>
/// This exception is raised if no suitable endpoint is available.
/// </summary>
public sealed class NoEndpointException : LocalException
{
    public NoEndpointException(string message)
        : base(message)
    {
    }

    public NoEndpointException(ObjectPrx proxy)
        : base($"No suitable endpoint available for proxy '{proxy}'.")
    {
    }

    public override string ice_id() => "::Ice::NoEndpointException";
}

/// <summary>
/// This exception is raised if an attempt is made to use a deactivated ObjectAdapter.
/// </summary>
public sealed class ObjectAdapterDeactivatedException : LocalException
{
    public ObjectAdapterDeactivatedException(string name)
        : base($"Object adapter '{name}' is deactivated.")
    {
    }

    public override string ice_id() => "::Ice::ObjectAdapterDeactivatedException";
}

/// <summary>
/// Reports a failure that occurred while parsing a string.
/// </summary>
public sealed class ParseException : LocalException
{
    public ParseException(string message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    public override string ice_id() => "::Ice::ParseException";
}

/// <summary>
/// This exception indicates that a failure occurred while initializing a plug-in.
/// </summary>
public sealed class PluginInitializationException : LocalException
{
    public PluginInitializationException(string message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    public override string ice_id() => "::Ice::PluginInitializationException";
}

/// <summary>
/// This exception indicates a failure in a security subsystem.
/// </summary>
public sealed class SecurityException : LocalException
{
    public SecurityException(string message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

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
    public TwowayOnlyException(string operation)
        : base($"Cannot invoke operation '{operation}' with a oneway, batchOneway, datagram, or batchDatagram proxy.")
    {
    }

    public override string ice_id() => "::Ice::TwowayOnlyException";
}

/// <summary>
/// The base class for Ice protocol exceptions.
/// </summary>
public class ProtocolException : LocalException
{
    public ProtocolException(string? message = null, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

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
    public CloseConnectionException()
        : base(message: "Connection closed by the peer.", innerException: null)
    {
    }

    public override string ice_id() => "::Ice::CloseConnectionException";
}

/// <summary>
/// A datagram exceeds the configured size.
/// This exception is raised if a datagram exceeds the configured send or receive buffer size, or exceeds the maximum
/// payload size of a UDP packet (65507 bytes).
/// </summary>
public sealed class DatagramLimitException : ProtocolException
{
    public DatagramLimitException()
        : base(message: "Datagram limit exceeded.", innerException: null)
    {
    }

    public override string ice_id() => "::Ice::DatagramLimitException";
}

/// <summary>
/// This exception reports an error during marshaling or unmarshaling.
/// </summary>
public sealed class MarshalException : ProtocolException
{
    public MarshalException(string message, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    public override string ice_id() => "::Ice::MarshalException";
}

/// <summary>This exception indicates a timeout condition.</summary>
public class TimeoutException : LocalException
{
    public TimeoutException(string? message = null, System.Exception? innerException = null)
        : base(message ?? "Operation timed out.", innerException)
    {
    }

    public override string ice_id() => "::Ice::TimeoutException";
}

/// <summary>
/// This exception indicates a connection establishment timeout condition.
/// </summary>
public sealed class ConnectTimeoutException : TimeoutException
{
    public ConnectTimeoutException()
        : base("Connect timed out.")
    {
    }

    public override string ice_id() => "::Ice::ConnectTimeoutException";
}

/// <summary>This exception indicates a connection closure timeout condition.</summary>
public sealed class CloseTimeoutException : TimeoutException
{
    public CloseTimeoutException()
        : base("Close timed out.")
    {
    }

    public override string ice_id() => "::Ice::CloseTimeoutException";
}

/// <summary>
/// This exception indicates that an invocation failed because it timed out.
/// </summary>
public sealed class InvocationTimeoutException : TimeoutException
{
    public InvocationTimeoutException()
        : base("Invocation timed out.")
    {
    }

    public override string ice_id() => "::Ice::InvocationTimeoutException";
}

/// <summary>
/// This exception is raised if a system error occurred in the server or client process.
/// </summary>
public class SyscallException : LocalException
{
    public SyscallException(string? message = null, System.Exception? innerException = null)
        : base(message, innerException)
    {
    }

    public SyscallException(System.Exception innerException)
        : base(innerException)
    {
    }

    public override string ice_id() => "::Ice::SyscallException";
}

/// <summary>
/// This exception indicates a DNS problem.
/// </summary>
public sealed class DNSException : SyscallException
{
    public DNSException(string host, System.Exception? innerException = null)
        : base($"Cannot resolve host '{host}'", innerException)
    {
    }

    public override string ice_id() => "::Ice::DNSException";
}

/// <summary>
/// This exception indicates socket errors.
/// </summary>
public class SocketException : SyscallException
{
    public SocketException(System.Exception? innerException = null)
        : base(message: null, innerException)
    {
    }

    public override string ice_id() => "::Ice::SocketException";
}

/// <summary>This exception indicates file errors.</summary>
public sealed class FileException : SyscallException
{
    public FileException(string message, System.Exception innerException)
        : base(message, innerException)
    {
    }

    public override string ice_id() => "::Ice::FileException";
}

/// <summary>
/// This exception indicates connection failures.
/// </summary>
public class ConnectFailedException : SocketException
{
    public ConnectFailedException(System.Exception? innerException = null)
        : base(innerException)
    {
    }

    public override string ice_id() => "::Ice::ConnectFailedException";
}

/// <summary>
/// This exception indicates a connection failure for which the server host actively refuses a connection.
/// </summary>
public sealed class ConnectionRefusedException : ConnectFailedException
{
    public ConnectionRefusedException(System.Exception? innerException = null)
        : base(innerException)
    {
    }

    public override string ice_id() => "::Ice::ConnectionRefusedException";
}

/// <summary>
/// This exception indicates a lost connection.
/// </summary>
public sealed class ConnectionLostException : SocketException
{
    public ConnectionLostException(System.Exception? innerException = null)
        : base(innerException)
    {
    }

    public override string ice_id() => "::Ice::ConnectionLostException";
}

/// <summary>
/// This exception is raised if an ObjectAdapter cannot be activated.
/// This happens if the Locator detects another active ObjectAdapter with the same adapter ID.
/// </summary>
public sealed class ObjectAdapterIdInUseException : LocalException
{
    public ObjectAdapterIdInUseException(string adapterId)
        : base($"An object adapter with adapter ID '{adapterId}' is already active.")
    {
    }

    public override string ice_id() => "::Ice::ObjectAdapterIdInUseException";
}

/// <summary>
/// An attempt was made to register something more than once with the Ice run time.
/// This exception is raised if an attempt is made to register a servant, servant locator, facet, value factory,
/// plug-in, object adapter (etc.) more than once for the same ID.
/// </summary>
public sealed class AlreadyRegisteredException : LocalException
{
    public string kindOfObject { get; }
    public string id { get; }

    public AlreadyRegisteredException(string kindOfObject, string id)
        : base($"Another {kindOfObject} is already registered with ID '{id}'.")
    {
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    public override string ice_id() => "::Ice::AlreadyRegisteredException";
}

/// <summary>
/// An attempt was made to find or deregister something that is not registered with the Ice run time or Ice locator.
/// This exception is raised if an attempt is made to remove a servant, servant locator, facet, value factory, plug-in,
/// object adapter (etc.) that is not currently registered. It's also raised if the Ice locator can't find an object or
/// object adapter when resolving an indirect proxy or when an object adapter is activated.
/// </summary>
public sealed class NotRegisteredException : LocalException
{
    public string kindOfObject { get; }
    public string id { get; }

     public NotRegisteredException(string kindOfObject, string id)
        : base($"No {kindOfObject} is registered with ID '{id}'.")
    {
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    public override string ice_id() => "::Ice::NotRegisteredException";
}
