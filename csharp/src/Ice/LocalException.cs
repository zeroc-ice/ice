// Copyright (c) ZeroC, Inc.

namespace Ice;

/// <summary>
/// This exception is raised when a failure occurs during initialization.
/// </summary>
public class InitializationException : LocalException
{
    public string reason;

    private void _initDM()
    {
        this.reason = "";
    }

    public InitializationException()
    {
        _initDM();
    }

    public InitializationException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string reason)
    {
        this.reason = reason;
    }

    public InitializationException(string reason)
    {
        _initDM(reason);
    }

    public InitializationException(string reason, System.Exception ex) : base(ex)
    {
        _initDM(reason);
    }

    public override string ice_id()
    {
        return "::Ice::InitializationException";
    }
}

/// <summary>
/// This exception indicates that a failure occurred while initializing a plug-in.
/// </summary>
public class PluginInitializationException : LocalException
{
    public string reason;

    private void _initDM()
    {
        this.reason = "";
    }

    public PluginInitializationException()
    {
        _initDM();
    }

    public PluginInitializationException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string reason)
    {
        this.reason = reason;
    }

    public PluginInitializationException(string reason)
    {
        _initDM(reason);
    }

    public PluginInitializationException(string reason, System.Exception ex) : base(ex)
    {
        _initDM(reason);
    }

    public override string ice_id()
    {
        return "::Ice::PluginInitializationException";
    }
}

/// <summary>
/// An attempt was made to register something more than once with the Ice run time.
/// This exception is raised if an
///  attempt is made to register a servant, servant locator, facet, value factory, plug-in, object adapter, object, or
///  user exception factory more than once for the same ID.
/// </summary>
public class AlreadyRegisteredException : LocalException
{
    public string kindOfObject;
    public string id;

    private void _initDM()
    {
        this.kindOfObject = "";
        this.id = "";
    }

    public AlreadyRegisteredException()
    {
        _initDM();
    }

    public AlreadyRegisteredException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string kindOfObject, string id)
    {
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    public AlreadyRegisteredException(string kindOfObject, string id)
    {
        _initDM(kindOfObject, id);
    }

    [System.CodeDom.Compiler.GeneratedCodeAttribute("slice2cs", "3.7.10")]
    public AlreadyRegisteredException(string kindOfObject, string id, System.Exception ex) : base(ex)
    {
        _initDM(kindOfObject, id);
    }

    public override string ice_id()
    {
        return "::Ice::AlreadyRegisteredException";
    }
}

/// <summary>
/// An attempt was made to find or deregister something that is not registered with the Ice run time or Ice locator.
/// This exception is raised if an attempt is made to remove a servant, servant locator, facet, value factory, plug-in,
///  object adapter, object, or user exception factory that is not currently registered. It's also raised if the Ice
///  locator can't find an object or object adapter when resolving an indirect proxy or when an object adapter is
///  activated.
/// </summary>
public class NotRegisteredException : LocalException
{
    public string kindOfObject;
    public string id;

    private void _initDM()
    {
        this.kindOfObject = "";
        this.id = "";
    }

    public NotRegisteredException()
    {
        _initDM();
    }

    public NotRegisteredException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string kindOfObject, string id)
    {
        this.kindOfObject = kindOfObject;
        this.id = id;
    }

    public NotRegisteredException(string kindOfObject, string id)
    {
        _initDM(kindOfObject, id);
    }

    public NotRegisteredException(string kindOfObject, string id, System.Exception ex) : base(ex)
    {
        _initDM(kindOfObject, id);
    }
    public override string ice_id()
    {
        return "::Ice::NotRegisteredException";
    }
}

/// <summary>
/// The operation can only be invoked with a twoway request.
/// This exception is raised if an attempt is made to invoke
///  an operation with ice_oneway, ice_batchOneway, ice_datagram, or
///  ice_batchDatagram and the operation has a return value, out-parameters, or an exception specification.
/// </summary>
public class TwowayOnlyException : LocalException
{
    public string operation;

    private void _initDM()
    {
        this.operation = "";
    }

    public TwowayOnlyException()
    {
        _initDM();
    }

    public TwowayOnlyException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string operation)
    {
        this.operation = operation;
    }

    public TwowayOnlyException(string operation)
    {
        _initDM(operation);
    }

    public TwowayOnlyException(string operation, System.Exception ex) : base(ex)
    {
        _initDM(operation);
    }

    public override string ice_id()
    {
        return "::Ice::TwowayOnlyException";
    }
}

/// <summary>
/// This exception is raised if an operation call on a server raises an unknown exception.
/// For example, for C++, this
///  exception is raised if the server throws a C++ exception that is not directly or indirectly derived from
///  Ice::LocalException or Ice::UserException.
/// </summary>
public class UnknownException : LocalException
{
    public string unknown;
    private void _initDM()
    {
        this.unknown = "";
    }

    public UnknownException()
    {
        _initDM();
    }

    public UnknownException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string unknown)
    {
        this.unknown = unknown;
    }

    public UnknownException(string unknown)
    {
        _initDM(unknown);
    }

    public UnknownException(string unknown, System.Exception ex) : base(ex)
    {
        _initDM(unknown);
    }

    public override string ice_id()
    {
        return "::Ice::UnknownException";
    }
}

/// <summary>
/// This exception is raised if an operation call on a server raises a  local exception.
/// Because local exceptions are
///  not transmitted by the Ice protocol, the client receives all local exceptions raised by the server as
///  UnknownLocalException. The only exception to this rule are all exceptions derived from
///  RequestFailedException, which are transmitted by the Ice protocol even though they are declared
///  local.
/// </summary>
public class UnknownLocalException : UnknownException
{
    public UnknownLocalException()
    {
    }

    public UnknownLocalException(System.Exception ex) : base(ex)
    {
    }

    public UnknownLocalException(string unknown) : base(unknown)
    {
    }

    public UnknownLocalException(string unknown, System.Exception ex) : base(unknown, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::UnknownLocalException";
    }
}

/// <summary>
/// An operation raised an incorrect user exception.
/// This exception is raised if an operation raises a user exception
///  that is not declared in the exception's throws clause. Such undeclared exceptions are not transmitted
///  from the server to the client by the Ice protocol, but instead the client just gets an UnknownUserException.
///  This is necessary in order to not violate the contract established by an operation's signature: Only local
///  exceptions and user exceptions declared in the throws clause can be raised.
/// </summary>
public class UnknownUserException : UnknownException
{
    public UnknownUserException()
    {
    }

    public UnknownUserException(System.Exception ex) : base(ex)
    {
    }

    public UnknownUserException(string unknown) : base(unknown)
    {
    }

    public UnknownUserException(string unknown, System.Exception ex) : base(unknown, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::UnknownUserException";
    }
}

/// <summary>
/// This exception is raised if the Communicator has been destroyed.
/// </summary>
public class CommunicatorDestroyedException : LocalException
{
    public CommunicatorDestroyedException()
    {
    }

    public CommunicatorDestroyedException(System.Exception ex) : base(ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::CommunicatorDestroyedException";
    }
}

/// <summary>
/// This exception is raised if an attempt is made to use a deactivated ObjectAdapter.
/// </summary>
public class ObjectAdapterDeactivatedException : LocalException
{
    public string name;

    private void _initDM()
    {
        this.name = "";
    }

    public ObjectAdapterDeactivatedException()
    {
        _initDM();
    }

    public ObjectAdapterDeactivatedException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string name)
    {
        this.name = name;
    }

    public ObjectAdapterDeactivatedException(string name)
    {
        _initDM(name);
    }

    public ObjectAdapterDeactivatedException(string name, System.Exception ex) : base(ex)
    {
        _initDM(name);
    }

    public override string ice_id()
    {
        return "::Ice::ObjectAdapterDeactivatedException";
    }
}

/// <summary>
/// This exception is raised if an ObjectAdapter cannot be activated.
/// This happens if the Locator
///  detects another active ObjectAdapter with the same adapter id.
/// </summary>
public class ObjectAdapterIdInUseException : LocalException
{
    public string id;

    private void _initDM()
    {
        this.id = "";
    }

    public ObjectAdapterIdInUseException()
    {
        _initDM();
    }

    public ObjectAdapterIdInUseException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string id)
    {
        this.id = id;
    }

    public ObjectAdapterIdInUseException(string id)
    {
        _initDM(id);
    }

    public ObjectAdapterIdInUseException(string id, System.Exception ex) : base(ex)
    {
        _initDM(id);
    }

    public override string ice_id()
    {
        return "::Ice::ObjectAdapterIdInUseException";
    }
}

/// <summary>
/// This exception is raised if no suitable endpoint is available.
/// </summary>
public class NoEndpointException : LocalException
{
    public string proxy;

    private void _initDM()
    {
        this.proxy = "";
    }

    public NoEndpointException()
    {
        _initDM();
    }

    public NoEndpointException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string proxy)
    {
        this.proxy = proxy;
    }

    public NoEndpointException(string proxy)
    {
        _initDM(proxy);
    }

    public NoEndpointException(string proxy, System.Exception ex) : base(ex)
    {
        _initDM(proxy);
    }

    public override string ice_id()
    {
        return "::Ice::NoEndpointException";
    }
}

/// <summary>
/// This exception is raised if there was an error while parsing an endpoint.
/// </summary>
public class EndpointParseException : LocalException
{
    public string str;

    private void _initDM()
    {
        this.str = "";
    }

    public EndpointParseException()
    {
        _initDM();
    }

    public EndpointParseException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string str)
    {
        this.str = str;
    }

    public EndpointParseException(string str)
    {
        _initDM(str);
    }

    public EndpointParseException(string str, System.Exception ex) : base(ex)
    {
        _initDM(str);
    }

    public override string ice_id()
    {
        return "::Ice::EndpointParseException";
    }
}

/// <summary>
/// This exception is raised if there was an error while parsing an endpoint selection type.
/// </summary>
public class EndpointSelectionTypeParseException : LocalException
{
    public string str;

    private void _initDM()
    {
        this.str = "";
    }

    public EndpointSelectionTypeParseException()
    {
        _initDM();
    }

    public EndpointSelectionTypeParseException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string str)
    {
        this.str = str;
    }

    public EndpointSelectionTypeParseException(string str)
    {
        _initDM(str);
    }

    public EndpointSelectionTypeParseException(string str, System.Exception ex) : base(ex)
    {
        _initDM(str);
    }

    public override string ice_id()
    {
        return "::Ice::EndpointSelectionTypeParseException";
    }
}

/// <summary>
/// This exception is raised if there was an error while parsing a version.
/// </summary>
public class VersionParseException : LocalException
{
    public string str;

    private void _initDM()
    {
        this.str = "";
    }

    public VersionParseException()
    {
        _initDM();
    }

    public VersionParseException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string str)
    {
        this.str = str;
    }

    public VersionParseException(string str)
    {
        _initDM(str);
    }

    public VersionParseException(string str, System.Exception ex) : base(ex)
    {
        _initDM(str);
    }

    public override string ice_id()
    {
        return "::Ice::VersionParseException";
    }
}

/// <summary>
/// This exception is raised if there was an error while parsing a stringified identity.
/// </summary>
public class IdentityParseException : LocalException
{
    public string str;

    private void _initDM()
    {
        this.str = "";
    }

    public IdentityParseException()
    {
        _initDM();
    }

    public IdentityParseException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string str)
    {
        this.str = str;
    }

    public IdentityParseException(string str)
    {
        _initDM(str);
    }

    public IdentityParseException(string str, System.Exception ex) : base(ex)
    {
        _initDM(str);
    }

    public override string ice_id()
    {
        return "::Ice::IdentityParseException";
    }
}

/// <summary>
/// This exception is raised if there was an error while parsing a stringified proxy.
/// </summary>
public class ProxyParseException : LocalException
{
    public string str;

    private void _initDM()
    {
        this.str = "";
    }

    public ProxyParseException()
    {
        _initDM();
    }

    public ProxyParseException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string str)
    {
        this.str = str;
    }

    public ProxyParseException(string str)
    {
        _initDM(str);
    }

    public ProxyParseException(string str, System.Exception ex) : base(ex)
    {
        _initDM(str);
    }

    public override string ice_id()
    {
        return "::Ice::ProxyParseException";
    }
}

/// <summary>
/// This exception is raised if an illegal identity is encountered.
/// </summary>
public class IllegalIdentityException : LocalException
{
    public Identity id;

    private void _initDM()
    {
        this.id = new Identity();
    }

    public IllegalIdentityException()
    {
        _initDM();
    }

    public IllegalIdentityException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(Identity id)
    {
        this.id = id;
    }

    public IllegalIdentityException(Identity id)
    {
        _initDM(id);
    }

    public IllegalIdentityException(Identity id, System.Exception ex) : base(ex)
    {
        _initDM(id);
    }

    public override string ice_id()
    {
        return "::Ice::IllegalIdentityException";
    }
}

/// <summary>
/// This exception is raised to reject an illegal servant (typically a null servant).
/// </summary>
public class IllegalServantException : LocalException
{
    public string reason;

    private void _initDM()
    {
        this.reason = "";
    }

    public IllegalServantException()
    {
        _initDM();
    }

    public IllegalServantException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string reason)
    {
        this.reason = reason;
    }

    public IllegalServantException(string reason)
    {
        _initDM(reason);
    }

    public IllegalServantException(string reason, System.Exception ex) : base(ex)
    {
        _initDM(reason);
    }

    public override string ice_id()
    {
        return "::Ice::IllegalServantException";
    }
}

/// <summary>
/// This exception is raised if a request failed.
/// This exception, and all exceptions derived from
///  RequestFailedException, are transmitted by the Ice protocol, even though they are declared
///  local.
/// </summary>
public class RequestFailedException : LocalException
{
    public Identity id;
    public string facet;
    public string operation;

    private void _initDM()
    {
        this.id = new Identity();
        this.facet = "";
        this.operation = "";
    }

    public RequestFailedException()
    {
        _initDM();
    }

    public RequestFailedException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(Identity id, string facet, string operation)
    {
        this.id = id;
        this.facet = facet;
        this.operation = operation;
    }

    public RequestFailedException(Identity id, string facet, string operation)
    {
        _initDM(id, facet, operation);
    }

    public RequestFailedException(Identity id, string facet, string operation, System.Exception ex) : base(ex)
    {
        _initDM(id, facet, operation);
    }

    public override string ice_id()
    {
        return "::Ice::RequestFailedException";
    }
}

/// <summary>
/// This exception is raised if an object does not exist on the server, that is, if no facets with the given identity
///  exist.
/// </summary>
public class ObjectNotExistException : RequestFailedException
{
    public ObjectNotExistException()
    {
    }

    public ObjectNotExistException(System.Exception ex) : base(ex)
    {
    }

    public ObjectNotExistException(Identity id, string facet, string operation) : base(id, facet, operation)
    {
    }

    public ObjectNotExistException(Identity id, string facet, string operation, System.Exception ex)
        : base(id, facet, operation, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::ObjectNotExistException";
    }
}

/// <summary>
/// This exception is raised if no facet with the given name exists, but at least one facet with the given identity
///  exists.
/// </summary>
public class FacetNotExistException : RequestFailedException
{
    public FacetNotExistException()
    {
    }

    public FacetNotExistException(System.Exception ex) : base(ex)
    {
    }

    public FacetNotExistException(Identity id, string facet, string operation) : base(id, facet, operation)
    {
    }

    public FacetNotExistException(Identity id, string facet, string operation, System.Exception ex)
        : base(id, facet, operation, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::FacetNotExistException";
    }
}

/// <summary>
/// This exception is raised if an operation for a given object does not exist on the server.
/// Typically this is caused
///  by either the client or the server using an outdated Slice specification.
/// </summary>
public class OperationNotExistException : RequestFailedException
{
    public OperationNotExistException()
    {
    }

    public OperationNotExistException(System.Exception ex) : base(ex)
    {
    }

    public OperationNotExistException(Identity id, string facet, string operation) : base(id, facet, operation)
    {
    }

    public OperationNotExistException(Identity id, string facet, string operation, System.Exception ex)
        : base(id, facet, operation, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::OperationNotExistException";
    }
}

/// <summary>
/// This exception is raised if a system error occurred in the server or client process.
/// There are many possible causes
///  for such a system exception. For details on the cause, SyscallException.error should be inspected.
/// </summary>
public class SyscallException : LocalException
{
    public int error;

    private void _initDM()
    {
        this.error = 0;
    }

    public SyscallException()
    {
        _initDM();
    }

    public SyscallException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(int error)
    {
        this.error = error;
    }

    public SyscallException(int error)
    {
        _initDM(error);
    }

    public SyscallException(int error, System.Exception ex) : base(ex)
    {
        _initDM(error);
    }

    public override string ice_id()
    {
        return "::Ice::SyscallException";
    }
}

/// <summary>
/// This exception indicates socket errors.
/// </summary>
public class SocketException : SyscallException
{
    public SocketException()
    {
    }

    public SocketException(System.Exception ex) : base(ex)
    {
    }

    public SocketException(int error) : base(error)
    {
    }

    public SocketException(int error, System.Exception ex) : base(error, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::SocketException";
    }
}

/// <summary>This exception indicates file errors.</summary>
public class FileException : SyscallException
{
    public string path;

    private void _initDM()
    {
        this.path = "";
    }

    public FileException()
    {
        _initDM();
    }

    public FileException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string path)
    {
        this.path = path;
    }

    public FileException(int error, string path) : base(error)
    {
        _initDM(path);
    }

    public FileException(int error, string path, System.Exception ex) : base(error, ex)
    {
        _initDM(path);
    }

    public override string ice_id()
    {
        return "::Ice::FileException";
    }
}

/// <summary>
/// This exception indicates connection failures.
/// </summary>
public class ConnectFailedException : SocketException
{
    public ConnectFailedException()
    {
    }

    public ConnectFailedException(System.Exception ex) : base(ex)
    {
    }

    public ConnectFailedException(int error) : base(error)
    {
    }

    public ConnectFailedException(int error, System.Exception ex) : base(error, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::ConnectFailedException";
    }
}

/// <summary>
/// This exception indicates a connection failure for which the server host actively refuses a connection.
/// </summary>
public class ConnectionRefusedException : ConnectFailedException
{
    public ConnectionRefusedException()
    {
    }

    public ConnectionRefusedException(System.Exception ex) : base(ex)
    {
    }

    public ConnectionRefusedException(int error) : base(error)
    {
    }

    public ConnectionRefusedException(int error, System.Exception ex) : base(error, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::ConnectionRefusedException";
    }
}

/// <summary>
/// This exception indicates a lost connection.
/// </summary>
public class ConnectionLostException : SocketException
{
    public ConnectionLostException()
    {
    }

    public ConnectionLostException(System.Exception ex) : base(ex)
    {
    }

    public ConnectionLostException(int error) : base(error)
    {
    }

    public ConnectionLostException(int error, System.Exception ex) : base(error, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::ConnectionLostException";
    }
}

/// <summary>
/// This exception indicates a DNS problem.
/// For details on the cause, DNSException.error should be inspected.
/// </summary>
public class DNSException : LocalException
{
    public int error;
    public string host;

    private void _initDM()
    {
        this.error = 0;
        this.host = "";
    }

    public DNSException()
    {
        _initDM();
    }

    public DNSException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(int error, string host)
    {
        this.error = error;
        this.host = host;
    }

    public DNSException(int error, string host)
    {
        _initDM(error, host);
    }

    public DNSException(int error, string host, System.Exception ex) : base(ex)
    {
        _initDM(error, host);
    }

    public override string ice_id()
    {
        return "::Ice::DNSException";
    }
}

/// <summary>
/// This exception indicates that a connection was aborted by the idle check.
/// </summary>
public class ConnectionIdleException : LocalException
{
    public ConnectionIdleException()
    {
    }

    public override string ice_id() => "::Ice::ConnectionIdleException";
}

/// <summary>This exception indicates a timeout condition.</summary>
public class TimeoutException : LocalException
{
    public TimeoutException()
    {
    }

    public TimeoutException(System.Exception ex) : base(ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::TimeoutException";
    }
}

/// <summary>
/// This exception indicates a connection establishment timeout condition.
/// </summary>
public class ConnectTimeoutException : TimeoutException
{
    public ConnectTimeoutException()
    {
    }

    public ConnectTimeoutException(System.Exception ex) : base(ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::ConnectTimeoutException";
    }
}

/// <summary>This exception indicates a connection closure timeout condition.</summary>
public class CloseTimeoutException : TimeoutException
{
    public CloseTimeoutException()
    {
    }

    public CloseTimeoutException(System.Exception ex) : base(ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::CloseTimeoutException";
    }
}

/// <summary>
/// This exception indicates that an invocation failed because it timed out.
/// </summary>
public class InvocationTimeoutException : TimeoutException
{
    public InvocationTimeoutException()
    {
    }

    public InvocationTimeoutException(System.Exception ex) : base(ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::InvocationTimeoutException";
    }
}

/// <summary>
/// This exception indicates that an asynchronous invocation failed because it was canceled explicitly by the user.
/// </summary>
public class InvocationCanceledException : LocalException
{
    public InvocationCanceledException()
    {
    }

    public InvocationCanceledException(System.Exception ex) : base(ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::InvocationCanceledException";
    }
}

/// <summary>
/// A generic exception base for all kinds of protocol error conditions.
/// </summary>
public class ProtocolException : LocalException
{
    public string reason;

    private void _initDM()
    {
        this.reason = "";
    }

    public ProtocolException()
    {
        _initDM();
    }

    public ProtocolException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string reason)
    {
        this.reason = reason;
    }

    public ProtocolException(string reason)
    {
        _initDM(reason);
    }

    public ProtocolException(string reason, System.Exception ex) : base(ex)
    {
        _initDM(reason);
    }

    public override string ice_id()
    {
        return "::Ice::ProtocolException";
    }
}

/// <summary>
/// This exception indicates that a message did not start with the expected magic number ('I', 'c', 'e', 'P').
/// </summary>
public class BadMagicException : ProtocolException
{
    public byte[] badMagic;

    public BadMagicException()
    {
    }

    public BadMagicException(System.Exception ex) : base(ex)
    {
    }

    private void _initDM(byte[] badMagic)
    {
        this.badMagic = badMagic;
    }

    public BadMagicException(string reason, byte[] badMagic) : base(reason)
    {
        _initDM(badMagic);
    }

    public BadMagicException(string reason, byte[] badMagic, System.Exception ex) : base(reason, ex)
    {
        _initDM(badMagic);
    }

    public override string ice_id()
    {
        return "::Ice::BadMagicException";
    }
}

/// <summary>
/// This exception indicates an unsupported protocol version.
/// </summary>
public class UnsupportedProtocolException : ProtocolException
{
    public ProtocolVersion bad;
    public ProtocolVersion supported;

    private void _initDM()
    {
        this.bad = new ProtocolVersion();
        this.supported = new ProtocolVersion();
    }

    public UnsupportedProtocolException()
    {
        _initDM();
    }

    public UnsupportedProtocolException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(ProtocolVersion bad, ProtocolVersion supported)
    {
        this.bad = bad;
        this.supported = supported;
    }

    public UnsupportedProtocolException(string reason, ProtocolVersion bad, ProtocolVersion supported) : base(reason)
    {
        _initDM(bad, supported);
    }

    public UnsupportedProtocolException(string reason, ProtocolVersion bad, ProtocolVersion supported, System.Exception ex) : base(reason, ex)
    {
        _initDM(bad, supported);
    }

    public override string ice_id()
    {
        return "::Ice::UnsupportedProtocolException";
    }
}

/// <summary>
/// This exception indicates an unsupported data encoding version.
/// </summary>
public class UnsupportedEncodingException : ProtocolException
{
    public EncodingVersion bad;
    public EncodingVersion supported;

    private void _initDM()
    {
        this.bad = new EncodingVersion();
        this.supported = new EncodingVersion();
    }

    public UnsupportedEncodingException()
    {
        _initDM();
    }

    public UnsupportedEncodingException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(EncodingVersion bad, EncodingVersion supported)
    {
        this.bad = bad;
        this.supported = supported;
    }

    public UnsupportedEncodingException(string reason, EncodingVersion bad, EncodingVersion supported) : base(reason)
    {
        _initDM(bad, supported);
    }

    public UnsupportedEncodingException(string reason, EncodingVersion bad, EncodingVersion supported, System.Exception ex) : base(reason, ex)
    {
        _initDM(bad, supported);
    }
    public override string ice_id()
    {
        return "::Ice::UnsupportedEncodingException";
    }
}

/// <summary>
/// This exception indicates that an unknown protocol message has been received.
/// </summary>
public class UnknownMessageException : ProtocolException
{
    public UnknownMessageException()
    {
    }

    public UnknownMessageException(System.Exception ex) : base(ex)
    {
    }

    public UnknownMessageException(string reason) : base(reason)
    {
    }

    public UnknownMessageException(string reason, System.Exception ex) : base(reason, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::UnknownMessageException";
    }
}

/// <summary>
/// This exception is raised if a message is received over a connection that is not yet validated.
/// </summary>
public class ConnectionNotValidatedException : ProtocolException
{
    public ConnectionNotValidatedException()
    {
    }

    public ConnectionNotValidatedException(System.Exception ex) : base(ex)
    {
    }

    public ConnectionNotValidatedException(string reason) : base(reason)
    {
    }

    public ConnectionNotValidatedException(string reason, System.Exception ex) : base(reason, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::ConnectionNotValidatedException";
    }
}

/// <summary>
/// This exception indicates that an unknown reply status has been received.
/// </summary>
public class UnknownReplyStatusException : ProtocolException
{
    public UnknownReplyStatusException()
    {
    }

    public UnknownReplyStatusException(System.Exception ex) : base(ex)
    {
    }

    public UnknownReplyStatusException(string reason) : base(reason)
    {
    }

    public UnknownReplyStatusException(string reason, System.Exception ex) : base(reason, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::UnknownReplyStatusException";
    }
}

/// <summary>
/// This exception indicates that the connection has been gracefully shut down by the server.
/// The operation call that
///  caused this exception has not been executed by the server. In most cases you will not get this exception, because
///  the client will automatically retry the operation call in case the server shut down the connection. However, if
///  upon retry the server shuts down the connection again, and the retry limit has been reached, then this exception is
///  propagated to the application code.
/// </summary>
public class CloseConnectionException : ProtocolException
{
    public CloseConnectionException()
    {
    }

    public CloseConnectionException(System.Exception ex) : base(ex)
    {
    }

    public CloseConnectionException(string reason) : base(reason)
    {
    }

    public CloseConnectionException(string reason, System.Exception ex) : base(reason, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::CloseConnectionException";
    }
}

/// <summary>
/// This exception is raised by an operation call if the application closes the connection locally using
///  Connection.close.
/// </summary>
public class ConnectionManuallyClosedException : LocalException
{
    public bool graceful;

    private void _initDM()
    {
    }

    public ConnectionManuallyClosedException()
    {
        _initDM();
    }

    public ConnectionManuallyClosedException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(bool graceful)
    {
        this.graceful = graceful;
    }

    public ConnectionManuallyClosedException(bool graceful)
    {
        _initDM(graceful);
    }

    public ConnectionManuallyClosedException(bool graceful, System.Exception ex) : base(ex)
    {
        _initDM(graceful);
    }

    public override string ice_id()
    {
        return "::Ice::ConnectionManuallyClosedException";
    }
}

/// <summary>
/// This exception indicates that a message size is less than the minimum required size.
/// </summary>
public class IllegalMessageSizeException : ProtocolException
{
    public IllegalMessageSizeException()
    {
    }

    public IllegalMessageSizeException(System.Exception ex) : base(ex)
    {
    }

    public IllegalMessageSizeException(string reason) : base(reason)
    {
    }

    public IllegalMessageSizeException(string reason, System.Exception ex) : base(reason, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::IllegalMessageSizeException";
    }
}

/// <summary>
/// This exception indicates a problem with compressing or uncompressing data.
/// </summary>
public class CompressionException : ProtocolException
{
    public CompressionException()
    {
    }

    public CompressionException(System.Exception ex) : base(ex)
    {
    }

    public CompressionException(string reason) : base(reason)
    {
    }

    public CompressionException(string reason, System.Exception ex) : base(reason, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::CompressionException";
    }
}

/// <summary>
/// A datagram exceeds the configured size.
/// This exception is raised if a datagram exceeds the configured send or
///  receive buffer size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
/// </summary>
public class DatagramLimitException : ProtocolException
{
    public DatagramLimitException()
    {
    }

    public DatagramLimitException(System.Exception ex) : base(ex)
    {
    }

    public DatagramLimitException(string reason) : base(reason)
    {
    }

    public DatagramLimitException(string reason, System.Exception ex) : base(reason, ex)
    {
    }
    public override string ice_id()
    {
        return "::Ice::DatagramLimitException";
    }
}

/// <summary>
/// This exception is raised for errors during marshaling or unmarshaling data.
/// </summary>
public class MarshalException : ProtocolException
{
    public MarshalException()
    {
    }

    public MarshalException(System.Exception ex) : base(ex)
    {
    }

    public MarshalException(string reason) : base(reason)
    {
    }

    public MarshalException(string reason, System.Exception ex) : base(reason, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::MarshalException";
    }
}

/// <summary>
/// This exception is raised if inconsistent data is received while unmarshaling a proxy.
/// </summary>
public class ProxyUnmarshalException : MarshalException
{
    public ProxyUnmarshalException()
    {
    }

    public ProxyUnmarshalException(System.Exception ex) : base(ex)
    {
    }

    public ProxyUnmarshalException(string reason) : base(reason)
    {
    }

    public ProxyUnmarshalException(string reason, System.Exception ex) : base(reason, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::ProxyUnmarshalException";
    }
}

/// <summary>
/// This exception is raised if an out-of-bounds condition occurs during unmarshaling.
/// </summary>
public class UnmarshalOutOfBoundsException : MarshalException
{
    public UnmarshalOutOfBoundsException()
    {
    }

    public UnmarshalOutOfBoundsException(System.Exception ex) : base(ex)
    {
    }

    public UnmarshalOutOfBoundsException(string reason) : base(reason)
    {
    }

    public UnmarshalOutOfBoundsException(string reason, System.Exception ex) : base(reason, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::UnmarshalOutOfBoundsException";
    }
}

/// <summary>
/// This exception is raised if no suitable value factory was found during unmarshaling of a Slice class instance.
/// </summary>
public class NoValueFactoryException : MarshalException
{
    public string type;

    private void _initDM()
    {
        this.type = "";
    }

    public NoValueFactoryException()
    {
        _initDM();
    }

    public NoValueFactoryException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string type)
    {
        this.type = type;
    }

    public NoValueFactoryException(string reason, string type) : base(reason)
    {
        _initDM(type);
    }

    public NoValueFactoryException(string reason, string type, System.Exception ex) : base(reason, ex)
    {
        _initDM(type);
    }

    public override string ice_id()
    {
        return "::Ice::NoValueFactoryException";
    }
}

/// <summary>
/// This exception is raised if the type of an unmarshaled Slice class instance does not match its expected type.
/// This
///  can happen if client and server are compiled with mismatched Slice definitions or if a class of the wrong type is
///  passed as a parameter or return value using dynamic invocation. This exception can also be raised if IceStorm is
///  used to send Slice class instances and an operation is subscribed to the wrong topic.
/// </summary>
public class UnexpectedObjectException : MarshalException
{
    public string type;
    public string expectedType;

    private void _initDM()
    {
        this.type = "";
        this.expectedType = "";
    }

    public UnexpectedObjectException()
    {
        _initDM();
    }

    public UnexpectedObjectException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string type, string expectedType)
    {
        this.type = type;
        this.expectedType = expectedType;
    }

    public UnexpectedObjectException(string reason, string type, string expectedType) : base(reason)
    {
        _initDM(type, expectedType);
    }

    public UnexpectedObjectException(string reason, string type, string expectedType, System.Exception ex) : base(reason, ex)
    {
        _initDM(type, expectedType);
    }

    public override string ice_id()
    {
        return "::Ice::UnexpectedObjectException";
    }
}

/// <summary>
/// This exception is raised when Ice receives a request or reply message whose size exceeds the limit specified by the
///  Ice.MessageSizeMax property.
/// </summary>
public class MemoryLimitException : MarshalException
{
    public MemoryLimitException()
    {
    }

    public MemoryLimitException(System.Exception ex) : base(ex)
    {
    }

    public MemoryLimitException(string reason) : base(reason)
    {
    }

    public MemoryLimitException(string reason, System.Exception ex) : base(reason, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::MemoryLimitException";
    }
}

/// <summary>
/// This exception indicates a malformed data encapsulation.
/// </summary>
public class EncapsulationException : MarshalException
{
    public EncapsulationException()
    {
    }

    public EncapsulationException(System.Exception ex) : base(ex)
    {
    }

    public EncapsulationException(string reason) : base(reason)
    {
    }

    public EncapsulationException(string reason, System.Exception ex) : base(reason, ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::EncapsulationException";
    }
}

/// <summary>
/// This exception is raised if an unsupported feature is used.
/// The unsupported feature string contains the name of the
///  unsupported feature.
/// </summary>
public class FeatureNotSupportedException : LocalException
{
    public string unsupportedFeature;

    private void _initDM()
    {
        this.unsupportedFeature = "";
    }

    public FeatureNotSupportedException()
    {
        _initDM();
    }

    public FeatureNotSupportedException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string unsupportedFeature)
    {
        this.unsupportedFeature = unsupportedFeature;
    }

    public FeatureNotSupportedException(string unsupportedFeature)
    {
        _initDM(unsupportedFeature);
    }

    public FeatureNotSupportedException(string unsupportedFeature, System.Exception ex) : base(ex)
    {
        _initDM(unsupportedFeature);
    }

    public override string ice_id()
    {
        return "::Ice::FeatureNotSupportedException";
    }
}

/// <summary>
/// This exception indicates a failure in a security subsystem, such as the IceSSL plug-in.
/// </summary>
public class SecurityException : LocalException
{
    public string reason;

    private void _initDM()
    {
        this.reason = "";
    }

    public SecurityException()
    {
        _initDM();
    }

    public SecurityException(System.Exception ex) : base(ex)
    {
        _initDM();
    }

    private void _initDM(string reason)
    {
        this.reason = reason;
    }

    public SecurityException(string reason)
    {
        _initDM(reason);
    }

    public SecurityException(string reason, System.Exception ex) : base(ex)
    {
        _initDM(reason);
    }

    public override string ice_id()
    {
        return "::Ice::SecurityException";
    }
}

/// <summary>
/// This exception indicates that an attempt has been made to change the connection properties of a fixed proxy.
/// </summary>
public class FixedProxyException : LocalException
{
    public FixedProxyException()
    {
    }

    public FixedProxyException(System.Exception ex) : base(ex)
    {
    }

    public override string ice_id()
    {
        return "::Ice::FixedProxyException";
    }
}
