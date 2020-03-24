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

        public CommunicatorDestroyedException(Exception innerException)
            : base("", innerException)
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

    /// <summary>This exception reports that a proxy's endpoints could not be resolved.</summary>
    [Serializable]
    public class NoEndpointException : Exception
    {
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
        // A plain TransportException should have a custom message or an inner exception (or both).
        protected TransportException()
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
    // TODO: eliminate this exception
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
            : base(innerException)
        {
        }

        protected SecurityException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports that data (bytes) received by Ice are not in an expected format.</summary>
    [Serializable]
    public class InvalidDataException : Exception
    {
        public InvalidDataException(string message)
            : base(message)
        {
        }

        public InvalidDataException(string message, Exception innerException)
            : base(message, innerException)
        {
        }

        protected InvalidDataException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports an error that occurred while marshaling data into a sequence of bytes.</summary>
    [Serializable]
    public class MarshalException : Exception
    {
        public MarshalException(string message)
            : base(message)
        {
        }

        public MarshalException(string message, Exception innerException)
            : base(message, innerException)
        {
        }

        protected MarshalException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    //
    // TODO: all remaining exceptions below need to be refactored
    //

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
    public class CloseConnectionException : Exception
    {
        public CloseConnectionException()
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

        public ConnectionManuallyClosedException(bool graceful) => Graceful = graceful;
    }
}
