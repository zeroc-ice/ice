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

        /// <summary>Construct a new instance of the <see cref="InvalidConfigurationException"/> class with serialized
        /// data.</summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
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

        /// <summary>Construct a new instance of the <see cref="LoadException"/> class with serialized data.
        /// </summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected LoadException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports an attempt to use a destroyed communicator.</summary>
    [Serializable]
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

        /// <summary>Construct a new instance of the <see cref="CommunicatorDisposedException"/> class with serialized
        /// data.</summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected CommunicatorDisposedException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports that a proxy's endpoints could not be resolved.</summary>
    [Serializable]
    public class NoEndpointException : Exception
    {
        /// <summary>Constructs a new instance of the <see cref="NoEndpointException"/> class.</summary>
        /// <param name="stringifiedProxy">The stringified proxy that was the cause of this exception.</param>
        public NoEndpointException(string stringifiedProxy)
            : base($"could not find the endpoints for proxy `{stringifiedProxy}'")
        {
        }

        /// <summary>Construct a new instance of the <see cref="NoEndpointException"/> class with serialized data.
        /// </summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected NoEndpointException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports an error from the transport layer.</summary>
    [Serializable]
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

        /// <summary>Construct a new instance of the <see cref="TransportException"/> class with serialized data.
        /// </summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected TransportException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports a failed attempt to establish a connection.</summary>
    [Serializable]
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

        /// <summary>Construct a new instance of the <see cref="ConnectFailedException"/> class with serialized data.
        /// </summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected ConnectFailedException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception indicates a connection establishment timeout condition.</summary>
    [Serializable]
    public class ConnectTimeoutException : ConnectFailedException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectTimeoutException"/> class.</summary>
        public ConnectTimeoutException()
        {
        }

        /// <summary>Construct a new instance of the <see cref="ConnectTimeoutException"/> class with serialized data.
        /// </summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected ConnectTimeoutException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports a connection refused error.</summary>
    [Serializable]
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

        /// <summary>Construct a new instance of the <see cref="ConnectionRefusedException"/> class with serialized
        /// data.</summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected ConnectionRefusedException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports that a previously established connection was lost.</summary>
    [Serializable]
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

        /// <summary>Construct a new instance of the <see cref="ConnectionLostException"/> class with serialized data.
        /// </summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected ConnectionLostException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports that a previously established connection timed out.</summary>
    [Serializable]
    public class ConnectionTimeoutException : TransportException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectionTimeoutException"/> class.</summary>
        public ConnectionTimeoutException()
        {
        }

        /// <summary>Construct a new instance of the <see cref="ConnectionTimeoutException"/> class with serialized
        /// data.</summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected ConnectionTimeoutException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception indicates that a previous established connection was closed.</summary>
    [Serializable]
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

        /// <summary>Construct a new instance of the <see cref="ConnectionClosedException"/> class with serialized
        /// data.</summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
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
        /// <summary>Constructs a new instance of the <see cref="ConnectionClosedByPeerException"/> class with a
        /// specified error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        public ConnectionClosedByPeerException(string message)
            : base(message)
        {
        }

        /// <summary>Construct a new instance of the <see cref="ConnectionClosedByPeerException"/> class with
        /// serialized data.</summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
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
        /// <summary>Constructs a new instance of the <see cref="ConnectionClosedLocallyException"/> class with a
        /// specified error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        public ConnectionClosedLocallyException(string message)
            : base(message)
        {
        }

        /// <summary>Construct a new instance of the <see cref="ConnectionClosedLocallyException"/> class with
        /// serialized data.</summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected ConnectionClosedLocallyException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception indicates the connection was closed by ACM because it was idle. </summary>
    [Serializable]
    public class ConnectionIdleException : TransportException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectionIdleException"/> class.</summary>
        public ConnectionIdleException()
        {
        }

        /// <summary>Construct a new instance of the <see cref="ConnectionIdleException"/> class with
        /// serialized data.</summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected ConnectionIdleException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports a DNS error.</summary>
    [Serializable]
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

        /// <summary>Construct a new instance of the <see cref="DNSException"/> class with serialized data.</summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
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
        /// <summary>Constructs a new instance of the <see cref="DatagramLimitException"/> class with a specified
        /// error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        public DatagramLimitException(string message)
            : base(message)
        {
        }

        /// <summary>Construct a new instance of the <see cref="DatagramLimitException"/> class with serialized data.
        /// </summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected DatagramLimitException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This exception reports that data (bytes) received by Ice are not in an expected format.</summary>
    [Serializable]
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

        /// <summary>Construct a new instance of the <see cref="InvalidDataException"/> class with serialized data.
        /// </summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected InvalidDataException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }

    /// <summary>This is a purely Ice-internal exception used for retries.</summary>
    [Serializable]
    public class RetryException : Exception
    {
        /// <summary>Constructs a new instance of the <see cref="RetryException"/> class with a reference to the inner
        /// exception that is the cause of this exception.</summary>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        internal RetryException(Exception innerException)
            : base("", innerException)
        {
        }

        /// <summary>Construct a new instance of the <see cref="RetryException"/> class with serialized data.
        /// </summary>
        /// <param name="info">The <see cref="SerializationInfo"/> that holds the serialized object data about the
        /// exception being thrown.</param>
        /// <param name="context">The <see cref="StreamingContext"/> that contains contextual information about the
        /// source or destination.</param>
        protected RetryException(SerializationInfo info, StreamingContext context)
            : base(info, context)
        {
        }
    }
}
