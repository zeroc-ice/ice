//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Runtime.Serialization;

namespace Ice
{
    /// <summary>This exception reports incorrect or missing Ice configuration.</summary>
    [Serializable]
    public class InvalidConfigurationException : Exception
    {
        public InvalidConfigurationException(string message)
            : base(message)
        {
        }

        public InvalidConfigurationException(string message, Exception innerException)
            : base(message, innerException)
        {
        }

        protected InvalidConfigurationException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception provides context for an exception thrown while attempting to load a class or create a
    /// class instance at runtime.</summary>
    [Serializable]
    public class LoadException : Exception
    {
        public LoadException(string message)
            : base(message)
        {
        }

        public LoadException(string message, Exception innerException)
            : base(message, innerException)
        {
        }

        protected LoadException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports an attempt to use a destroyed communicator.</summary>
    [Serializable]
    public class CommunicatorDestroyedException : ObjectDisposedException
    {
        public CommunicatorDestroyedException() :
            base("")
        {
        }

        public CommunicatorDestroyedException(Exception ex)
            : base("", ex)
        {
        }

        protected CommunicatorDestroyedException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports an attempt to use a deactivated object adapter.</summary>
    [Serializable]
    public class ObjectAdapterDeactivatedException : ObjectDisposedException
    {
        public ObjectAdapterDeactivatedException(string objectName)
            : base(objectName)
        {
        }

        protected ObjectAdapterDeactivatedException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports that a proxy's endpoints could not be resolved./// </summary>
    [Serializable]
    public class NoEndpointException : Exception
    {
        public NoEndpointException() // TODO: remove this constructor
        {
        }

        public NoEndpointException(string stringifiedProxy)
            : base($"could not find the endpoints for proxy `{stringifiedProxy}'")
        {
        }

        protected NoEndpointException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports an error from the transport layer.</summary>
    [Serializable]
    public class TransportException : Exception
    {
        public TransportException()
        {
        }

        public TransportException(string message)
            : base(message)
        {
        }

        public TransportException(Exception innerException)
            : base("", innerException)
        {
        }

        public TransportException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
        protected TransportException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports a failed attempt to establish a connection.</summary>
    [Serializable]
    public class ConnectFailedException : TransportException
    {
        public ConnectFailedException()
        {
        }

        public ConnectFailedException(Exception innerException)
            : base(innerException)
        {
        }

        protected ConnectFailedException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports connection refused error.</summary>
    [Serializable]
    public class ConnectionRefusedException : ConnectFailedException
    {
        public ConnectionRefusedException()
        {
        }

        public ConnectionRefusedException(Exception innerException)
            : base(innerException)
        {
        }

        protected ConnectionRefusedException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports that a previously established connection was lost.</summary>
    [Serializable]
    public class ConnectionLostException : TransportException
    {
        public ConnectionLostException()
        {
        }

        public ConnectionLostException(Exception innerException)
            : base(innerException)
        {
        }

        protected ConnectionLostException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports a DNS error.</summary>
    [Serializable]
    public class DNSException : TransportException
    {
        public DNSException()
        {
        }

        public DNSException(string host)
            : base($"failed to resolve hostname `{host}'")
        {
        }

        public DNSException(string host, Exception innerException)
            : base($"failed to resolve hostname `{host}'", innerException)
        {
        }

        protected DNSException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports that a datagram exceeds the configured send or receive buffer size, or exceeds
    ///  the maximum payload size of a UDP packet (65507 bytes).</summary>
    [Serializable]
    public class DatagramLimitException : TransportException
    {
        public DatagramLimitException(string message)
            : base(message)
        {
        }

        protected DatagramLimitException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports a failure in the security subsystem.</summary>
    [Serializable]
    public class SecurityException : TransportException
    {
        public SecurityException(string message)
            : base(message)
        {
        }

        public SecurityException(Exception innerException)
            : base("", innerException)
        {
        }

        protected SecurityException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>
    /// This exception indicates a timeout condition.
    /// </summary>
    public class TimeoutException : Exception
    {
        public TimeoutException()
        {
        }

        public TimeoutException(Exception ex) : base("", ex)
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

        public ConnectTimeoutException(Exception ex) : base(ex)
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

        public CloseTimeoutException(Exception ex) : base(ex)
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

        public ConnectionTimeoutException(Exception ex) : base(ex)
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

        public InvocationTimeoutException(Exception ex) : base(ex)
        {
        }
    }

    /// <summary>
    /// This exception indicates that an asynchronous invocation failed
    /// because it was canceled explicitly by the user.
    /// </summary>
    public class InvocationCanceledException : Exception
    {
        public InvocationCanceledException()
        {
        }

        public InvocationCanceledException(Exception ex) : base("", ex)
        {
        }
    }

    /// <summary>
    /// A generic exception base for all kinds of protocol error
    /// conditions.
    /// </summary>
    public class ProtocolException : Exception
    {
        public string Reason;

        public ProtocolException() => Reason = "";

        public ProtocolException(Exception ex) : base("", ex) => Reason = "";

        public ProtocolException(string reason) => Reason = reason;

        public ProtocolException(string reason, Exception ex) : base("", ex) => Reason = reason;
    }

    /// <summary>
    /// This exception indicates that a message did not start with the expected
    /// magic number ('I', 'c', 'e', 'P').
    /// </summary>
    public class BadMagicException : ProtocolException
    {
        public byte[] BadMagic;

        public BadMagicException() => BadMagic = System.Array.Empty<byte>();

        public BadMagicException(Exception ex) : base(ex) => BadMagic = System.Array.Empty<byte>();

        public BadMagicException(string reason, byte[] badMagic) : base(reason) => BadMagic = badMagic;

        public BadMagicException(string reason, byte[] badMagic, Exception ex) : base(reason, ex) => BadMagic = badMagic;
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

        public UnsupportedProtocolException(string reason, Protocol bad, Protocol supported, Exception ex) : base(reason, ex)
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

        public UnsupportedEncodingException(Exception ex) : base(ex)
        {
            Bad = default;
            Supported = default;
        }

        public UnsupportedEncodingException(string reason, Encoding bad, Encoding supported) : base(reason)
        {
            Bad = bad;
            Supported = supported;
        }

        public UnsupportedEncodingException(string reason, Encoding bad, Encoding supported, Exception ex) : base(reason, ex)
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

        public UnknownMessageException(Exception ex) : base(ex)
        {
        }

        public UnknownMessageException(string reason) : base(reason)
        {
        }

        public UnknownMessageException(string reason, Exception ex) : base(reason, ex)
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

        public ConnectionNotValidatedException(Exception ex) : base(ex)
        {
        }

        public ConnectionNotValidatedException(string reason) : base(reason)
        {
        }

        public ConnectionNotValidatedException(string reason, Exception ex) : base(reason, ex)
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

        public UnknownRequestIdException(Exception ex) : base(ex)
        {
        }

        public UnknownRequestIdException(string reason) : base(reason)
        {
        }

        public UnknownRequestIdException(string reason, Exception ex) : base(reason, ex)
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

        public UnknownReplyStatusException(Exception ex) : base(ex)
        {
        }

        public UnknownReplyStatusException(string reason) : base(reason)
        {
        }

        public UnknownReplyStatusException(string reason, Exception ex) : base(reason, ex)
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

        public CloseConnectionException(Exception ex) : base(ex)
        {
        }

        public CloseConnectionException(string reason) : base(reason)
        {
        }

        public CloseConnectionException(string reason, Exception ex) : base(reason, ex)
        {
        }
    }

    /// <summary>
    /// This exception is raised by an operation call if the application
    /// closes the connection locally using Connection.close.
    /// </summary>
    public class ConnectionManuallyClosedException : Exception
    {
        public bool Graceful;

        public ConnectionManuallyClosedException()
        {
        }

        public ConnectionManuallyClosedException(Exception ex) : base("", ex)
        {
        }

        public ConnectionManuallyClosedException(bool graceful) => Graceful = graceful;

        public ConnectionManuallyClosedException(bool graceful, Exception ex) : base("", ex) => Graceful = graceful;
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

        public IllegalMessageSizeException(Exception ex) : base(ex)
        {
        }

        public IllegalMessageSizeException(string reason) : base(reason)
        {
        }

        public IllegalMessageSizeException(string reason, Exception ex) : base(reason, ex)
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

        public CompressionException(Exception ex) : base(ex)
        {
        }

        public CompressionException(string reason) : base(reason)
        {
        }

        public CompressionException(string reason, Exception ex) : base(reason, ex)
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

        public MarshalException(Exception ex) : base(ex)
        {
        }

        public MarshalException(string reason) : base(reason)
        {
        }

        public MarshalException(string reason, Exception ex) : base(reason, ex)
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

        public ProxyUnmarshalException(Exception ex) : base(ex)
        {
        }

        public ProxyUnmarshalException(string reason) : base(reason)
        {
        }

        public ProxyUnmarshalException(string reason, Exception ex) : base(reason, ex)
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

        public UnmarshalOutOfBoundsException(Exception ex) : base(ex)
        {
        }

        public UnmarshalOutOfBoundsException(string reason) : base(reason)
        {
        }

        public UnmarshalOutOfBoundsException(string reason, Exception ex) : base(reason, ex)
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

        public NoClassFactoryException(Exception ex) : base(ex) => Type = "";

        public NoClassFactoryException(string reason, string type) : base(reason) => Type = type;

        public NoClassFactoryException(string reason, string type, Exception ex) : base(reason, ex) => Type = type;
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

        public UnexpectedObjectException(Exception ex) : base(ex)
        {
            Type = "";
            ExpectedType = "";
        }

        public UnexpectedObjectException(string reason, string type, string expectedType) : base(reason)
        {
            Type = type;
            ExpectedType = expectedType;
        }

        public UnexpectedObjectException(string reason, string type, string expectedType, Exception ex) : base(reason, ex)
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

        public MemoryLimitException(Exception ex) : base(ex)
        {
        }

        public MemoryLimitException(string reason) : base(reason)
        {
        }

        public MemoryLimitException(string reason, Exception ex) : base(reason, ex)
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

        public EncapsulationException(Exception ex) : base(ex)
        {
        }

        public EncapsulationException(string reason) : base(reason)
        {
        }

        public EncapsulationException(string reason, Exception ex) : base(reason, ex)
        {
        }
    }
}
