// Copyright (c) ZeroC, Inc.

namespace Ice;

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
