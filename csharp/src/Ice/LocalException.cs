//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice
{
    /// <summary>This exception reports incorrect or missing Ice configuration.</summary>
    public class InvalidConfigurationException : Exception
    {
        /// <summary>Constructs a new instance of the <see cref="InvalidConfigurationException"/> class with a
        /// specified error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        public InvalidConfigurationException(string message)
            : base(message)
        {
        }

        /// <summary>Constructs a new instance of the <see cref="InvalidConfigurationException"/> class with a
        /// specified error message and a reference to the inner exception that is the cause of this exception.
        /// </summary>
        /// <param name="message">The message that describes the error.</param>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        public InvalidConfigurationException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }

    /// <summary>This exception provides context for an exception thrown while attempting to load a class or create a
    /// class instance at runtime.</summary>
    public class LoadException : Exception
    {
        /// <summary>Constructs a new instance of the <see cref="LoadException"/> class with a specified error message.
        /// </summary>
        /// <param name="message">The message that describes the error.</param>
        public LoadException(string message)
            : base(message)
        {
        }

        /// <summary>Constructs a new instance of the <see cref="LoadException"/> class with a specified error message
        /// and a reference to the inner exception that is the cause of this exception.</summary>
        /// <param name="message">The message that describes the error.</param>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        public LoadException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }

    /// <summary>This exception reports an attempt to use a destroyed communicator.</summary>
    public class CommunicatorDisposedException : ObjectDisposedException
    {
        /// <summary>Constructs a new instance of the <see cref="CommunicatorDisposedException"/> class with a specified
        /// error message.</summary>
        public CommunicatorDisposedException()
            : base("")
        {
        }

        /// <summary>Constructs a new instance of the <see cref="CommunicatorDisposedException"/> class with a
        /// reference to the inner exception that is the cause of this exception.</summary>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        public CommunicatorDisposedException(Exception innerException)
            : base("", innerException)
        {
        }
    }

    /// <summary>This exception reports that a proxy's endpoints could not be resolved.</summary>
    public class NoEndpointException : Exception
    {
        /// <summary>Constructs a new instance of the <see cref="NoEndpointException"/> class.</summary>
        /// <param name="stringifiedProxy">The stringified proxy that was the cause of this exception.</param>
        public NoEndpointException(string stringifiedProxy)
            : base($"could not find the endpoints for proxy `{stringifiedProxy}'")
        {
        }
    }

    /// <summary>This exception reports an error from the transport layer.</summary>
    public class TransportException : Exception
    {
        /// <summary>Constructs a new instance of the <see cref="TransportException"/> class. A plain
        /// TransportException should have a custom message or an inner exception (or both).</summary>
        protected TransportException()
        {
        }

        /// <summary>Constructs a new instance of the <see cref="TransportException"/> class with a specified error
        /// message.</summary>
        /// <param name="message">The message that describes the error.</param>
        public TransportException(string message)
            : base(message)
        {
        }

        /// <summary>Constructs a new instance of the <see cref="TransportException"/> class with a reference to the
        /// inner exception that is the cause of this exception.</summary>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        public TransportException(Exception innerException)
            : base("", innerException)
        {
        }

        /// <summary>Constructs a new instance of the <see cref="LoadException"/> ckass with a specified error message
        /// and a reference to the inner exception that is the cause of this exception.</summary>
        /// <param name="message">The message that describes the error.</param>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        public TransportException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }

    /// <summary>This exception reports a failed attempt to establish a connection.</summary>
    public class ConnectFailedException : TransportException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectFailedException"/> class.</summary>
        public ConnectFailedException()
        {
        }

        /// <summary>Constructs a new instance of the <see cref="ConnectFailedException"/> class with a reference to
        /// the inner exception that is the cause of this exception.</summary>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        public ConnectFailedException(Exception innerException)
            : base(innerException)
        {
        }
    }

    /// <summary>This exception indicates a connection establishment timeout condition.</summary>
    public class ConnectTimeoutException : ConnectFailedException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectTimeoutException"/> class.</summary>
        public ConnectTimeoutException()
        {
        }
    }

    /// <summary>This exception reports a connection refused error.</summary>
    public class ConnectionRefusedException : ConnectFailedException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectionRefusedException"/> class.</summary>
        public ConnectionRefusedException()
        {
        }

        /// <summary>Constructs a new instance of the <see cref="ConnectionRefusedException"/> class with a reference
        /// to the inner exception that is the cause of this exception.</summary>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        public ConnectionRefusedException(Exception innerException)
            : base(innerException)
        {
        }
    }

    /// <summary>This exception reports that a previously established connection was lost.</summary>
    public class ConnectionLostException : TransportException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectionLostException"/> class.</summary>
        public ConnectionLostException()
        {
        }

        /// <summary>Constructs a new instance of the <see cref="ConnectionLostException"/> class with a reference to
        /// the inner exception that is the cause of this exception.</summary>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        public ConnectionLostException(Exception innerException)
            : base(innerException)
        {
        }
    }

    /// <summary>This exception reports that a previously established connection timed out.</summary>
    public class ConnectionTimeoutException : TransportException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectionTimeoutException"/> class.</summary>
        public ConnectionTimeoutException()
        {
        }
    }

    /// <summary>This exception indicates that a previous established connection was closed.</summary>
    public class ConnectionClosedException : TransportException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectionClosedException"/> class.</summary>
        protected ConnectionClosedException()
        {
        }

        /// <summary>Constructs a new instance of the <see cref="ConnectionClosedException"/> class with a specified
        /// error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        protected ConnectionClosedException(string message)
            : base(message)
        {
        }
    }

    /// <summary>This exception indicates that the stream was closed by the peer.</summary>
    [Serializable]
    public class StreamClosedByPeerException : TransportException
    {
        public StreamClosedByPeerException(string message)
            : base(message)
        {
        }

        protected StreamClosedByPeerException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception indicates that the connection was gracefully shut down by the server. A request
    /// that failed with this exception was not executed by the server. In most cases you will not get this
    /// exception because the client automatically retries the invocation in case the server shuts down the connection.
    /// However, if upon retry the server shuts down the connection again, and the retry limit is reached, this
    /// exception is propagated to the application code.</summary>
    public class ConnectionClosedByPeerException : ConnectionClosedException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectionClosedByPeerException"/> class with a
        /// specified error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        public ConnectionClosedByPeerException(string message)
            : base(message)
        {
        }
    }

    /// <summary>This exception indicates the application (client) closed the connection with Connection.Close.
    /// </summary>
    public class ConnectionClosedLocallyException : ConnectionClosedException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectionClosedLocallyException"/> class with a
        /// specified error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        public ConnectionClosedLocallyException(string message)
            : base(message)
        {
        }
    }

    /// <summary>This exception indicates the connection was closed by ACM because it was idle. </summary>
    public class ConnectionIdleException : TransportException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectionIdleException"/> class.</summary>
        public ConnectionIdleException()
        {
        }
    }

    /// <summary>This exception reports a DNS error.</summary>
    public class DNSException : TransportException
    {
        /// <summary>Constructs a new instance of the <see cref="DNSException"/> class.</summary>
        /// <param name="host">The hostname that was the cause of the current exception.</param>
        public DNSException(string host)
            : base($"failed to resolve hostname `{host}'")
        {
        }

        /// <summary>Constructs a new instance of <see cref="DNSException"/> with a reference to the inner
        /// exception that is the cause of this exception.</summary>
        /// <param name="host">The hostname that was the cause of the current exception.</param>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        public DNSException(string host, Exception innerException)
            : base($"failed to resolve hostname `{host}'", innerException)
        {
        }
    }

    /// <summary>This exception reports that a datagram exceeds the configured send or receive buffer size, or exceeds
    /// the maximum payload size of a UDP packet (65507 bytes).</summary>
    // TODO: eliminate this exception
    public class DatagramLimitException : TransportException
    {
        /// <summary>Constructs a new instance of the <see cref="DatagramLimitException"/> class with a specified
        /// error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        public DatagramLimitException(string message)
            : base(message)
        {
        }
    }

    /// <summary>This exception reports that data (bytes) received by Ice are not in an expected format.</summary>
    public class InvalidDataException : Exception
    {
        /// <summary>Constructs a new instance of the <see cref="InvalidDataException"/> class with a specified error message.
        /// </summary>
        /// <param name="message">The message that describes the error.</param>
        public InvalidDataException(string message)
            : base(message)
        {
        }

        /// <summary>Constructs a new instance of the <see cref="InvalidDataException"/> class with a specified error
        /// message and a reference to the inner exception that is the cause of this exception.</summary>
        /// <param name="message">The message that describes the error.</param>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        public InvalidDataException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }

    /// <summary>This is a purely Ice-internal exception used for retries.</summary>
    public class RetryException : Exception
    {
        /// <summary>Constructs a new instance of the <see cref="RetryException"/> class with a reference to the inner
        /// exception that is the cause of this exception.</summary>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        internal RetryException(Exception innerException)
            : base("", innerException)
        {
        }
    }
}
