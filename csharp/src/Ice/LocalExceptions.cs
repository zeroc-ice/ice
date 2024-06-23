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
    public Identity id;
    public string facet;
    public string operation;

    protected RequestFailedException(
        Identity id,
        string facet,
        string operation,
        System.Exception? innerException = null)
        : base(message: null, innerException)
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
    public ObjectNotExistException(System.Exception? innerException = null)
        : base(new Identity(), "", "", innerException)
    {
    }

    public ObjectNotExistException(Identity id, string facet, string operation, System.Exception? innerException = null)
        : base(id, facet, operation, innerException)
    {
    }

    public override string ice_id() => "::Ice::ObjectNotExistException";
}

/// <summary>
/// The dispatch could not find a servant for the identity + facet carried by the request.
/// </summary>
public sealed class FacetNotExistException : RequestFailedException
{
    public FacetNotExistException(System.Exception? innerException = null)
        : base(new Identity(), "", "", innerException)
    {
    }

    public FacetNotExistException(Identity id, string facet, string operation, System.Exception? innerException = null)
        : base(id, facet, operation, innerException)
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
    public OperationNotExistException(System.Exception? innerException = null)
        : base(new Identity(), "", "", innerException)
    {
    }

    public OperationNotExistException(Identity id, string facet, string operation, System.Exception? innerException = null)
        : base(id, facet, operation, innerException)
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
/// This exception indicates that a connection was closed gracefully.
/// </summary>
public sealed class ConnectionClosedException : LocalException
{
    public ConnectionClosedException(string message)
        : base(message, innerException: null)
    {
    }

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
/// Reports a failure that occurred while parsing a string.
/// </summary>
public sealed class ParseException : LocalException
{
    public string unknown => Message;

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
