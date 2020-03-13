//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    /// <summary>
    /// This exception is raised when a failure occurs during initialization.
    /// </summary>
    public class InitializationException : System.Exception
    {
        public string Reason;

        public InitializationException() => Reason = "";

        public InitializationException(System.Exception ex) : base("", ex) => Reason = "";
        public InitializationException(string reason) => Reason = reason;

        public InitializationException(string reason, System.Exception ex) : base("", ex) => Reason = reason;
    }

    /// <summary>
    /// An attempt was made to find or deregister something that is not
    /// registered with the Ice run time or Ice locator.
    /// This exception is raised if an attempt is made to remove a servant,
    /// servant locator, facet, class factory, plug-in, object adapter,
    /// object, or user exception factory that is not currently registered.
    ///
    /// It's also raised if the Ice locator can't find an object or object
    /// adapter when resolving an indirect proxy or when an object adapter
    /// is activated.
    /// </summary>
    public class NotRegisteredException : System.Exception
    {
        public string KindOfObject;
        public string Id;
        public NotRegisteredException()
        {
            KindOfObject = "";
            Id = "";
        }

        public NotRegisteredException(System.Exception ex) : base("", ex)
        {
            KindOfObject = "";
            Id = "";
        }

        public NotRegisteredException(string kindOfObject, string id)
        {
            KindOfObject = kindOfObject;
            Id = id;
        }

        public NotRegisteredException(string kindOfObject, string id, System.Exception ex) : base("", ex)
        {
            KindOfObject = kindOfObject;
            Id = id;
        }
    }

    /// <summary>
    /// This exception is raised if the Communicator has been destroyed.
    /// </summary>
    public class CommunicatorDestroyedException : System.Exception
    {
        public CommunicatorDestroyedException()
        {
        }

        public CommunicatorDestroyedException(System.Exception ex) : base("", ex)
        {
        }
    }

    /// <summary>
    /// This exception is raised if an attempt is made to use a deactivated
    /// ObjectAdapter.
    /// </summary>
    public class ObjectAdapterDeactivatedException : System.Exception
    {
        public string Name;

        public ObjectAdapterDeactivatedException() => Name = "";

        public ObjectAdapterDeactivatedException(System.Exception ex) : base("", ex) => Name = "";

        public ObjectAdapterDeactivatedException(string name) => Name = name;

        public ObjectAdapterDeactivatedException(string name, System.Exception ex) : base("", ex) => Name = name;

    }

    /// <summary>
    /// This exception is raised if an ObjectAdapter cannot be activated.
    /// This happens if the Locator detects another active ObjectAdapter with
    /// the same adapter id.
    /// </summary>
    public class ObjectAdapterIdInUseException : System.Exception
    {
        public string Id;

        public ObjectAdapterIdInUseException() => Id = "";

        public ObjectAdapterIdInUseException(System.Exception ex) : base("", ex) => Id = "";

        public ObjectAdapterIdInUseException(string id) => Id = id;

        public ObjectAdapterIdInUseException(string id, System.Exception ex) : base("", ex) => Id = id;
    }

    /// <summary>
    /// This exception is raised if no suitable endpoint is available.
    /// </summary>
    public class NoEndpointException : System.Exception
    {
        public string Proxy;

        public NoEndpointException() => Proxy = "";

        public NoEndpointException(System.Exception ex) : base("", ex) => Proxy = "";

        public NoEndpointException(string proxy) => Proxy = proxy;

        public NoEndpointException(string proxy, System.Exception ex) : base("", ex) => Proxy = proxy;

    }

    /// <summary>
    /// This exception is raised if a system error occurred in the server
    /// or client process.
    /// There are many possible causes for such a system
    /// exception. For details on the cause, SyscallException.error
    /// should be inspected.
    /// </summary>
    public class SyscallException : System.Exception
    {
        public int Error;

        public SyscallException() => Error = 0;

        public SyscallException(System.Exception ex) : base("", ex) => Error = 0;

        public SyscallException(int error) => Error = error;

        public SyscallException(int error, System.Exception ex) : base("", ex) => Error = error;

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
    }

    /// <summary>
    /// This exception indicates file errors.
    /// </summary>
    public class FileException : SyscallException
    {
        public string Path;

        public FileException() => Path = "";

        public FileException(System.Exception ex) : base(ex) => Path = "";

        public FileException(int error, string path) : base(error) => Path = path;

        public FileException(int error, string path, System.Exception ex) : base(error, ex) => Path = path;

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
    }

    /// <summary>
    /// This exception indicates a connection failure for which
    /// the server host actively refuses a connection.
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
    }

    /// <summary>
    /// This exception indicates a DNS problem.
    /// For details on the cause,
    /// DNSException.error should be inspected.
    /// </summary>
    public class DNSException : System.Exception
    {
        public int Error;
        public string Host;

        public DNSException()
        {
            Error = 0;
            Host = "";
        }

        public DNSException(System.Exception ex) : base("", ex)
        {
            Error = 0;
            Host = "";
        }

        public DNSException(int error, string host)
        {
            Error = error;
            Host = host;
        }

        public DNSException(int error, string host, System.Exception ex) : base("", ex)
        {
            Error = error;
            Host = host;
        }
    }

    /// <summary>
    /// This exception indicates a timeout condition.
    /// </summary>
    public class TimeoutException : System.Exception
    {
        public TimeoutException()
        {
        }

        public TimeoutException(System.Exception ex) : base("", ex)
        {
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
    }

    /// <summary>
    /// This exception indicates a connection closure timeout condition.
    /// </summary>
    public class CloseTimeoutException : TimeoutException
    {

        public CloseTimeoutException()
        {
        }

        public CloseTimeoutException(System.Exception ex) : base(ex)
        {
        }
    }

    /// <summary>
    /// This exception indicates that a connection has been shut down because it has been
    /// idle for some time.
    /// </summary>
    public class ConnectionTimeoutException : TimeoutException
    {

        public ConnectionTimeoutException()
        {
        }

        public ConnectionTimeoutException(System.Exception ex) : base(ex)
        {
        }
    }

    /// <summary>
    /// This exception indicates that an invocation failed because it timed
    /// out.
    /// </summary>
    public class InvocationTimeoutException : TimeoutException
    {
        public InvocationTimeoutException()
        {
        }

        public InvocationTimeoutException(System.Exception ex) : base(ex)
        {
        }
    }

    /// <summary>
    /// This exception indicates that an asynchronous invocation failed
    /// because it was canceled explicitly by the user.
    /// </summary>
    public class InvocationCanceledException : System.Exception
    {
        public InvocationCanceledException()
        {
        }

        public InvocationCanceledException(System.Exception ex) : base("", ex)
        {
        }
    }

    /// <summary>
    /// A generic exception base for all kinds of protocol error
    /// conditions.
    /// </summary>
    public class ProtocolException : System.Exception
    {
        public string Reason;

        public ProtocolException() => Reason = "";

        public ProtocolException(System.Exception ex) : base("", ex) => Reason = "";

        public ProtocolException(string reason) => Reason = reason;

        public ProtocolException(string reason, System.Exception ex) : base("", ex) => Reason = reason;
    }

    /// <summary>
    /// This exception indicates that a message did not start with the expected
    /// magic number ('I', 'c', 'e', 'P').
    /// </summary>
    public class BadMagicException : ProtocolException
    {
        public byte[] BadMagic;

        public BadMagicException() => BadMagic = System.Array.Empty<byte>();

        public BadMagicException(System.Exception ex) : base(ex) => BadMagic = System.Array.Empty<byte>();

        public BadMagicException(string reason, byte[] badMagic) : base(reason) => BadMagic = badMagic;

        public BadMagicException(string reason, byte[] badMagic, System.Exception ex) : base(reason, ex) => BadMagic = badMagic;
    }

    /// <summary>
    /// This exception indicates an unsupported protocol version.
    /// </summary>
    public class UnsupportedProtocolException : ProtocolException
    {

        public Protocol Bad;
        public Protocol Supported;

        public UnsupportedProtocolException(string reason, Protocol bad, Protocol supported) : base(reason)
        {
            Bad = bad;
            Supported = supported;
        }

        public UnsupportedProtocolException(string reason, Protocol bad, Protocol supported, System.Exception ex) : base(reason, ex)
        {
            Bad = bad;
            Supported = supported;
        }
    }

    /// <summary>
    /// This exception indicates an unsupported data encoding version.
    /// </summary>
    public class UnsupportedEncodingException : ProtocolException
    {
        public Encoding Bad;
        public Encoding Supported;

        public UnsupportedEncodingException()
        {
            Bad = default;
            Supported = default;
        }

        public UnsupportedEncodingException(System.Exception ex) : base(ex)
        {
            Bad = default;
            Supported = default;
        }

        public UnsupportedEncodingException(string reason, Encoding bad, Encoding supported) : base(reason)
        {
            Bad = bad;
            Supported = supported;
        }

        public UnsupportedEncodingException(string reason, Encoding bad, Encoding supported, System.Exception ex) : base(reason, ex)
        {
            Bad = bad;
            Supported = supported;
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
    }

    /// <summary>
    /// This exception is raised if a message is received over a connection
    /// that is not yet validated.
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
    }

    /// <summary>
    /// This exception indicates that a response for an unknown request ID has been
    /// received.
    /// </summary>
    public class UnknownRequestIdException : ProtocolException
    {
        public UnknownRequestIdException()
        {
        }

        public UnknownRequestIdException(System.Exception ex) : base(ex)
        {
        }

        public UnknownRequestIdException(string reason) : base(reason)
        {
        }

        public UnknownRequestIdException(string reason, System.Exception ex) : base(reason, ex)
        {
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
    }

    /// <summary>
    /// This exception indicates that the connection has been gracefully shut down by the
    /// server.
    /// The operation call that caused this exception has not been
    /// executed by the server. In most cases you will not get this
    /// exception, because the client will automatically retry the
    /// operation call in case the server shut down the connection. However,
    /// if upon retry the server shuts down the connection again, and the
    /// retry limit has been reached, then this exception is propagated to
    /// the application code.
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
    }

    /// <summary>
    /// This exception is raised by an operation call if the application
    /// closes the connection locally using Connection.close.
    /// </summary>
    public class ConnectionManuallyClosedException : System.Exception
    {
        public bool Graceful;

        public ConnectionManuallyClosedException()
        {
        }

        public ConnectionManuallyClosedException(System.Exception ex) : base("", ex)
        {
        }

        public ConnectionManuallyClosedException(bool graceful) => Graceful = graceful;

        public ConnectionManuallyClosedException(bool graceful, System.Exception ex) : base("", ex) => Graceful = graceful;
    }

    /// <summary>
    /// This exception indicates that a message size is less
    /// than the minimum required size.
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
    }

    /// <summary>
    /// A datagram exceeds the configured size.
    /// This exception is raised if a datagram exceeds the configured send or receive buffer
    /// size, or exceeds the maximum payload size of a UDP packet (65507 bytes).
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
    }

    /// <summary>
    /// This exception is raised if no suitable class factory was found during
    /// unmarshaling of a Slice class instance.
    /// </summary>
    public class NoClassFactoryException : MarshalException
    {
        public string Type;

        public NoClassFactoryException() => Type = "";

        public NoClassFactoryException(System.Exception ex) : base(ex) => Type = "";

        public NoClassFactoryException(string reason, string type) : base(reason) => Type = type;

        public NoClassFactoryException(string reason, string type, System.Exception ex) : base(reason, ex) => Type = type;
    }

    /// <summary>
    /// This exception is raised if the type of an unmarshaled Slice class instance does
    /// not match its expected type.
    /// This can happen if client and server are compiled with mismatched Slice
    /// definitions or if a class of the wrong type is passed as a parameter
    /// or return value using dynamic invocation. This exception can also be
    /// raised if IceStorm is used to send Slice class instances and
    /// an operation is subscribed to the wrong topic.
    /// </summary>
    public class UnexpectedObjectException : MarshalException
    {
        public string Type;
        public string ExpectedType;

        public UnexpectedObjectException()
        {
            Type = "";
            ExpectedType = "";
        }

        public UnexpectedObjectException(System.Exception ex) : base(ex)
        {
            Type = "";
            ExpectedType = "";
        }

        public UnexpectedObjectException(string reason, string type, string expectedType) : base(reason)
        {
            Type = type;
            ExpectedType = expectedType;
        }

        public UnexpectedObjectException(string reason, string type, string expectedType, System.Exception ex) : base(reason, ex)
        {
            Type = type;
            ExpectedType = expectedType;
        }
    }

    /// <summary>
    /// This exception is raised when Ice receives a request or reply
    /// message whose size exceeds the limit specified by the
    /// Ice.MessageSizeMax property.
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
    }

    /// <summary>
    /// This exception is raised if an unsupported feature is used.
    /// The
    /// unsupported feature string contains the name of the unsupported
    /// feature
    /// </summary>
    public class FeatureNotSupportedException : System.Exception
    {
        public string UnsupportedFeature;

        public FeatureNotSupportedException() => UnsupportedFeature = "";

        public FeatureNotSupportedException(System.Exception ex) : base("", ex) => UnsupportedFeature = "";

        public FeatureNotSupportedException(string unsupportedFeature) => UnsupportedFeature = unsupportedFeature;

        public FeatureNotSupportedException(string unsupportedFeature, System.Exception ex) : base("", ex) =>
            UnsupportedFeature = unsupportedFeature;
    }

    /// <summary>
    /// This exception indicates a failure in a security subsystem,
    /// such as the IceSSL plug-in.
    /// </summary>
    public class SecurityException : System.Exception
    {
        public string Reason;

        public SecurityException() => Reason = "";

        public SecurityException(System.Exception ex) : base("", ex) => Reason = "";

        public SecurityException(string reason) => Reason = reason;

        public SecurityException(string reason, System.Exception ex) : base("", ex) => Reason = reason;
    }
}
