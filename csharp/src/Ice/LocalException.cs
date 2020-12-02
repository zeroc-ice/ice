// Copyright (c) ZeroC, Inc. All rights reserved.

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
        /// <summary>Constructs a new instance of the <see cref="CommunicatorDisposedException"/> class.</summary>
        public CommunicatorDisposedException()
            : base($"{typeof(Communicator).FullName}")
        {
        }

        /// <summary>Constructs a new instance of the <see cref="CommunicatorDisposedException"/> class with a
        /// reference to the inner exception that is the cause of this exception.</summary>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        public CommunicatorDisposedException(Exception innerException)
            : base($"{typeof(Communicator).FullName}", innerException)
        {
        }
    }

    /// <summary>This exception reports that a proxy's endpoints could not be resolved.</summary>
    public class NoEndpointException : Exception
    {
        /// <summary>Constructs a new instance of the <see cref="NoEndpointException"/> class.</summary>
        public NoEndpointException()
        {
        }

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
        /// <summary>The exception retry policy.</summary>
        internal RetryPolicy RetryPolicy { get; }

        /// <summary>Constructs a new instance of the <see cref="TransportException"/> class. A plain
        /// TransportException should have a custom message or an inner exception (or both).</summary>
        /// <param name="retryPolicy">The exception retry policy.</param>
        protected TransportException(RetryPolicy retryPolicy = default) => RetryPolicy = retryPolicy;

        /// <summary>Constructs a new instance of the <see cref="TransportException"/> class with a specified error
        /// message.</summary>
        /// <param name="message">The message that describes the error.</param>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public TransportException(string message, RetryPolicy retryPolicy = default)
            : base(message) => RetryPolicy = retryPolicy;

        /// <summary>Constructs a new instance of the <see cref="TransportException"/> class with a reference to the
        /// inner exception that is the cause of this exception.</summary>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public TransportException(Exception innerException, RetryPolicy retryPolicy = default)
            : base("", innerException) => RetryPolicy = retryPolicy;

        /// <summary>Constructs a new instance of the <see cref="LoadException"/> class with a specified error message
        /// and a reference to the inner exception that is the cause of this exception.</summary>
        /// <param name="message">The message that describes the error.</param>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public TransportException(
            string message,
            Exception innerException,
            RetryPolicy retryPolicy = default)
            : base(message, innerException) => RetryPolicy = retryPolicy;
    }

    /// <summary>This exception reports a failed attempt to establish a connection.</summary>
    public class ConnectFailedException : TransportException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectFailedException"/> class.</summary>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public ConnectFailedException(RetryPolicy retryPolicy = default)
            : base(retryPolicy)
        {
        }

        /// <summary>Constructs a new instance of the <see cref="ConnectFailedException"/> class with a reference to
        /// the inner exception that is the cause of this exception.</summary>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public ConnectFailedException(Exception innerException, RetryPolicy retryPolicy = default)
            : base(innerException, retryPolicy)
        {
        }
    }

    /// <summary>This exception indicates a connection establishment timeout condition.</summary>
    public class ConnectTimeoutException : ConnectFailedException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectTimeoutException"/> class.</summary>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public ConnectTimeoutException(RetryPolicy retryPolicy = default)
            : base(retryPolicy)
        {
        }
    }

    /// <summary>This exception reports a connection refused error.</summary>
    public class ConnectionRefusedException : ConnectFailedException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectionRefusedException"/> class.</summary>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public ConnectionRefusedException(RetryPolicy retryPolicy = default)
            : base(retryPolicy)
        {
        }

        /// <summary>Constructs a new instance of the <see cref="ConnectionRefusedException"/> class with a reference
        /// to the inner exception that is the cause of this exception.</summary>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public ConnectionRefusedException(Exception innerException, RetryPolicy retryPolicy = default)
            : base(innerException, retryPolicy)
        {
        }
    }

    /// <summary>This exception reports that a previously established connection was lost.</summary>
    public class ConnectionLostException : TransportException
    {
        /// <summary>Constructs a new instance of the <see cref="ConnectionLostException"/> class.</summary>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public ConnectionLostException(RetryPolicy retryPolicy = default)
            : base(retryPolicy)
        {
        }

        /// <summary>Constructs a new instance of the <see cref="ConnectionLostException"/> class with a reference to
        /// the inner exception that is the cause of this exception.</summary>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public ConnectionLostException(Exception innerException, RetryPolicy retryPolicy = default)
            : base(innerException, retryPolicy)
        {
        }
    }

    /// <summary>This exception indicates that a previous established connection was closed.</summary>
    public class ConnectionClosedException : TransportException
    {
        /// <summary><c>true</c> if the connection closure originated from the peer, <c>false</c> otherwise.</summary>
        public bool IsClosedByPeer { get; }

        /// <summary>Constructs a new instance of the <see cref="ConnectionClosedException"/> class.</summary>
        /// <param name="isClosedByPeer"><c>true</c> if the connection closure originated from the peer, <c>false</c>
        /// otherwise</param>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public ConnectionClosedException(bool isClosedByPeer = false, RetryPolicy retryPolicy = default)
            : base("Cannot access closed connection.", retryPolicy) =>
            IsClosedByPeer = isClosedByPeer;

        /// <summary>Constructs a new instance of the <see cref="ConnectionClosedException"/> class with a specified
        /// error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        /// <param name="isClosedByPeer"><c>true</c> if the connection closure originated from the peer, <c>false</c>
        /// otherwise</param>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public ConnectionClosedException(
            string message,
            bool isClosedByPeer = false,
            RetryPolicy retryPolicy = default)
            : base(message, retryPolicy) => IsClosedByPeer = isClosedByPeer;
    }

    /// <summary>This exception reports a DNS error.</summary>
    public class DNSException : TransportException
    {
        /// <summary>Constructs a new instance of the <see cref="DNSException"/> class.</summary>
        /// <param name="host">The hostname that was the cause of the current exception.</param>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public DNSException(string host, RetryPolicy retryPolicy = default)
            : base($"failed to resolve hostname `{host}'", retryPolicy)
        {
        }

        /// <summary>Constructs a new instance of <see cref="DNSException"/> with a reference to the inner
        /// exception that is the cause of this exception.</summary>
        /// <param name="host">The hostname that was the cause of the current exception.</param>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        /// <param name="retryPolicy">The exception retry policy.</param>
        public DNSException(string host, Exception innerException, RetryPolicy retryPolicy = default)
            : base($"failed to resolve hostname `{host}'", innerException, retryPolicy)
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

    /// <summary>This exception indicates that a limit was exceeded.</summary>
    public class LimitExceededException : Exception
    {
        /// <summary>Constructs a new instance of the <see cref="LimitExceededException"/> class with a specified
        /// error message.</summary>
        /// <param name="message">The message that describes the error.</param>
        public LimitExceededException(string message)
            : base(message)
        {
        }

        /// <summary>Constructs a new instance of the <see cref="LimitExceededException"/> class with a specified error
        /// message and a reference to the inner exception that is the cause of this exception.</summary>
        /// <param name="message">The message that describes the error.</param>
        /// <param name="innerException">The exception that is the cause of the current exception.</param>
        public LimitExceededException(string message, Exception innerException)
            : base(message, innerException)
        {
        }
    }
}
