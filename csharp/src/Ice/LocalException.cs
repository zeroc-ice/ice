//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Runtime.Serialization;

namespace ZeroC.Ice
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
        public CommunicatorDestroyedException()
            : base("")
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

    /// <summary>This exception indicates a connection establishment timeout condition.</summary>
    [Serializable]
    public class ConnectTimeoutException : ConnectFailedException
    {
        public ConnectTimeoutException()
        {
        }

        protected ConnectTimeoutException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports a connection refused error.</summary>
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

    /// <summary>This exception reports that a previously established connection timed out.</summary>
    [Serializable]
    public class ConnectionTimeoutException : TransportException
    {
        public ConnectionTimeoutException()
        {
        }

        protected ConnectionTimeoutException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception indicates that a previous established connection was closed.</summary>
    [Serializable]
    public class ConnectionClosedException : TransportException
    {
        protected ConnectionClosedException()
        {
        }

        protected ConnectionClosedException(string message)
            : base(message)
        {
        }

        protected ConnectionClosedException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception indicates that the connection was gracefully shut down by the server. A request
    /// that failed with this exception was not executed by the server. In most cases you will not get this
    /// exception because the client automatically retries the invocation in case the server shuts down the connection.
    /// However, if upon retry the server shuts down the connection again, and the retry limit is reached, this
    /// exception is propagated to the application code.</summary>
    [Serializable]
    public class ConnectionClosedByPeerException : ConnectionClosedException
    {
        public ConnectionClosedByPeerException()
        {
        }

        protected ConnectionClosedByPeerException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception indicates the application (client) closed the connection with Connection.Close.
    /// </summary>
    [Serializable]
    public class ConnectionClosedLocallyException : ConnectionClosedException
    {
        public ConnectionClosedLocallyException(string message)
            : base(message)
        {
        }

        protected ConnectionClosedLocallyException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception indicates the connection was closed by ACM because it was idle. </summary>
    [Serializable]
    public class ConnectionIdleException : TransportException
    {
        public ConnectionIdleException()
        {
        }

        protected ConnectionIdleException(SerializationInfo info, StreamingContext context)
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
    /// the maximum payload size of a UDP packet (65507 bytes).</summary>
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

    /// <summary>This is a purely Ice-internal exception used for retries.</summary>
    [Serializable]
    public class RetryException : Exception
    {
        internal RetryException(Exception innerException)
            : base("", innerException)
        {
        }

        protected RetryException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }
}
