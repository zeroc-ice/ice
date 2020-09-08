//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice
{
    /// <summary>This exception reports incorrect or missing Ice configuration.</summary>
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
    }

    /// <summary>This exception provides context for an exception thrown while attempting to load a class or create a
    /// class instance at runtime.</summary>
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
    }

    /// <summary>This exception reports an attempt to use a destroyed communicator.</summary>
    public class CommunicatorDisposedException : ObjectDisposedException
    {
        public CommunicatorDisposedException()
            : base("")
        {
        }

        public CommunicatorDisposedException(Exception innerException)
            : base("", innerException)
        {
        }
    }

    /// <summary>This exception reports that a proxy's endpoints could not be resolved.</summary>
    public class NoEndpointException : Exception
    {
        public NoEndpointException(string stringifiedProxy)
            : base($"could not find the endpoints for proxy `{stringifiedProxy}'")
        {
        }
    }

    /// <summary>This exception reports an error from the transport layer.</summary>
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
    }

    /// <summary>This exception reports a failed attempt to establish a connection.</summary>
    public class ConnectFailedException : TransportException
    {
        public ConnectFailedException()
        {
        }

        public ConnectFailedException(Exception innerException)
            : base(innerException)
        {
        }
    }

    /// <summary>This exception indicates a connection establishment timeout condition.</summary>
    public class ConnectTimeoutException : ConnectFailedException
    {
        public ConnectTimeoutException()
        {
        }
    }

    /// <summary>This exception reports a connection refused error.</summary>
    public class ConnectionRefusedException : ConnectFailedException
    {
        public ConnectionRefusedException()
        {
        }

        public ConnectionRefusedException(Exception innerException)
            : base(innerException)
        {
        }
    }

    /// <summary>This exception reports that a previously established connection was lost.</summary>
    public class ConnectionLostException : TransportException
    {
        public ConnectionLostException()
        {
        }

        public ConnectionLostException(Exception innerException)
            : base(innerException)
        {
        }
    }

    /// <summary>This exception reports that a previously established connection timed out.</summary>
    public class ConnectionTimeoutException : TransportException
    {
        public ConnectionTimeoutException()
        {
        }
    }

    /// <summary>This exception indicates that a previous established connection was closed.</summary>
    public class ConnectionClosedException : TransportException
    {
        protected ConnectionClosedException()
        {
        }

        protected ConnectionClosedException(string message)
            : base(message)
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
        public ConnectionClosedByPeerException(string message)
            : base(message)
        {
        }
    }

    /// <summary>This exception indicates the application (client) closed the connection with Connection.Close.
    /// </summary>
    public class ConnectionClosedLocallyException : ConnectionClosedException
    {
        public ConnectionClosedLocallyException(string message)
            : base(message)
        {
        }
    }

    /// <summary>This exception indicates the connection was closed by ACM because it was idle. </summary>
    public class ConnectionIdleException : TransportException
    {
        public ConnectionIdleException()
        {
        }
    }

    /// <summary>This exception reports a DNS error.</summary>
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
    }

    /// <summary>This exception reports that a datagram exceeds the configured send or receive buffer size, or exceeds
    /// the maximum payload size of a UDP packet (65507 bytes).</summary>
    // TODO: eliminate this exception
    public class DatagramLimitException : TransportException
    {
        public DatagramLimitException(string message)
            : base(message)
        {
        }
    }

    /// <summary>This exception reports that data (bytes) received by Ice are not in an expected format.</summary>
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
    }

    /// <summary>This is a purely Ice-internal exception used for retries.</summary>
    public class RetryException : Exception
    {
        internal RetryException(Exception innerException)
            : base("", innerException)
        {
        }
    }
}
