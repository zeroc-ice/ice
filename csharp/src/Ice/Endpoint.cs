//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>An endpoint describes a server-side network sink for Ice requests: an object adapter listens on one or
    /// more endpoints and a client establishes a connection to a given object adapter endpoint. Its properties are
    /// a network transport protocol such as TCP or Bluetooth RFCOMM, a host or address, a port number, and
    /// transport-specific options.</summary>
    public abstract class Endpoint : IEquatable<Endpoint>
    {
        /// <summary>Gets the communicator that created this endpoint.</summary>
        public Communicator Communicator { get; }

        /// <summary>The host name or address.</summary>
        public abstract string Host { get; }

        /// <summary>Indicates whether or not this endpoint's transport uses datagrams with no ordering or delivery
        /// guarantees.</summary>
        /// <value>True when this endpoint's transport is datagram-based; otherwise, false. There is currently a
        /// single datagram-based transport: UDP.</value>
        public virtual bool IsDatagram => false;

        /// <summary>Indicates whether or not this endpoint's transport is secure. Only applies to ice1.</summary>
        /// <value>True when this endpoint's transport is secure; otherwise, false.</value>
        public virtual bool IsSecure => false;

        /// <summary>Gets an option of the endpoint.</summary>
        /// <param name="option">The name of the option to retrieve.</param>
        /// <value>The value of this option, or null if this option is unknown, not set or set to its default value.
        /// </value>
        public virtual string? this[string option]
        {
            get
            {
                if (Protocol == Protocol.Ice1)
                {
                    return option switch
                    {
                        "host" => Host.Length > 0 ? Host : null,
                        "port" => Port != DefaultPort ? Port.ToString(CultureInfo.InvariantCulture) : null,
                        _ => null,
                    };
                }
                else
                {
                    return null;
                }
            }
        }

        /// <summary>The port number.</summary>
        public virtual ushort Port => DefaultPort;

        /// <summary>The Ice protocol of this endpoint.</summary>
        public Protocol Protocol { get; }

        /// <summary>The scheme for this endpoint. With ice1, it's the transport name (tcp, ssl etc.) or opaque. With
        /// ice2, it's ice+transport (ice+tcp, ice+quic etc.) or ice+universal.</summary>
        public virtual string Scheme => Protocol == Protocol.Ice1 ? TransportName : $"ice+{TransportName}";

        /// <summary>The <see cref="ZeroC.Ice.Transport"></see> of this endpoint.</summary>
        public abstract Transport Transport { get; }

        /// <summary>The name of the endpoint's transport in lowercase.</summary>
        public virtual string TransportName => Transport.ToString().ToLowerInvariant();

        /// <summary>Gets the default port of this endpoint.</summary>
        protected internal abstract ushort DefaultPort { get; }

        /// <summary>Indicates whether or not this endpoint has options with non default values that ToString would
        /// print. Always true for ice1 endpoints.</summary>
        protected internal abstract bool HasOptions { get; }

        /// <summary>The equality operator == returns true if its operands are equal, false otherwise.</summary>
        /// <param name="lhs">The left hand side operand.</param>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>true</c> if the operands are equal, otherwise <c>false</c>.</returns>
        public static bool operator ==(Endpoint? lhs, Endpoint? rhs)
        {
            if (ReferenceEquals(lhs, rhs))
            {
                return true;
            }

            if (lhs is null || rhs is null)
            {
                return false;
            }
            return rhs.Equals(lhs);
        }

        /// <summary>The inequality operator != returns true if its operands are not equal, false otherwise.</summary>
        /// <param name="lhs">The left hand side operand.</param>
        /// <param name="rhs">The right hand side operand.</param>
        /// <returns><c>true</c> if the operands are not equal, otherwise <c>false</c>.</returns>
        public static bool operator !=(Endpoint? lhs, Endpoint? rhs) => !(lhs == rhs);

        /// <inheritdoc/>
        public override bool Equals(object? obj) => obj is Endpoint other && Equals(other);

        /// <inheritdoc/>
        public virtual bool Equals(Endpoint? other) =>
            other is Endpoint endpoint &&
                Communicator == endpoint.Communicator &&
                Protocol == endpoint.Protocol &&
                Transport == endpoint.Transport &&
                Host == endpoint.Host &&
                Port == endpoint.Port;

        /// <inheritdoc/>
        public override int GetHashCode()
        {
            var hash = new HashCode();
            hash.Add(Communicator);
            hash.Add(Protocol);
            hash.Add(Transport);
            hash.Add(Host);
            hash.Add(Port);
            return hash.ToHashCode();
        }

        /// <summary>Converts the endpoint into a string. The format of this string depends on the protocol: either
        /// ice1 format (for ice1) or URI format (for ice2 and up).</summary>
        public override string ToString()
        {
            if (Protocol == Protocol.Ice1)
            {
                var sb = new StringBuilder(Scheme);
                AppendOptions(sb, ' '); // option separator is not used with ice1
                return sb.ToString();
            }
            else
            {
                var sb = new StringBuilder();
                sb.AppendEndpoint(this);
                return sb.ToString();
            }
        }

        /// <summary>Checks whether this endpoint and the given endpoint point to the same local peer. This is used for
        /// the collocation optimization check to figure out whether or not a proxy endpoint points to a local adapter.
        /// </summary>
        /// <param name="endpoint">The other endpoint</param>
        /// <returns><c>True</c> if the other endpoint point to the same local peer, <c>False</c> otherwise.</returns>
        public abstract bool IsLocal(Endpoint endpoint);

        /// <summary>Appends the options of this endpoint with non default values to the string builder.</summary>
        /// <param name="sb">The string builder.</param>
        /// <param name="optionSeparator">The character used to separate two options. This separator is not used for
        /// ice1 endpoints.</param>
        protected internal abstract void AppendOptions(StringBuilder sb, char optionSeparator);

        /// <summary>Writes the options of this endpoint to the output stream. With ice1, the options typically
        /// include the host and port; with ice2, the host and port are not considered options.</summary>
        /// <param name="ostr">The output stream.</param>
        protected internal abstract void WriteOptions(OutputStream ostr);

        /// <summary>Returns a connector for this endpoint, or empty list if no connector is available.</summary>
        /// <param name="endpointSelection">The endpoint selection type used when expanding the endpoint address.
        /// </param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>A collector of connectors for this endpoint</returns>
        public abstract ValueTask<IEnumerable<IConnector>> ConnectorsAsync(
            EndpointSelectionType endpointSelection,
            CancellationToken cancel);

        /// <summary>Creates a new connection to the given endpoint.</summary>
        /// <param name="manager">The connection manager which owns the connection.</param>
        /// <param name="transceiver">The transceiver to use for the connection.</param>
        /// <param name="connector">The connector associated with the new connection, this is always null for incoming
        /// connections.</param>
        /// <param name="connectionId">The connection ID associated with the new connection. This is always an empty
        /// string for incoming connections.</param>
        /// <param name="adapter">The adapter associated with the new connection, this is always null for outgoing
        /// connections.</param>
        /// <returns>A new connection to the given endpoint.</returns>
        public abstract Connection CreateConnection(
            IConnectionManager manager,
            ITransceiver transceiver,
            IConnector? connector,
            string connectionId,
            ObjectAdapter? adapter);

        /// <summary>Expands endpoint out in to separate endpoints for each local host if listening on INADDR_ANY on
        /// server side or if no host was specified on client side.</summary>
        /// <returns>The collection containing the expanded endpoints.</returns>
        public abstract IEnumerable<Endpoint> ExpandIfWildcard();

        /// <summary>Expands endpoint out into separate endpoints for each IP address returned by the DNS resolver.
        /// Also returns the endpoint which can be used to connect to the returned endpoints or null if no specific
        /// endpoint can be used to connect to these endpoints (e.g.: with the IP endpoint, it returns this endpoint if
        /// it uses a fixed port, null otherwise).
        /// </summary>
        /// <param name="publishedEndpoint">TODO???</param>
        /// <returns>The collection containing the expanded endpoints.</returns>
        public abstract IEnumerable<Endpoint> ExpandHost(out Endpoint? publishedEndpoint);

        /// <summary>Return a server side transceiver for this endpoint and the transceiver bound endpoint.</summary>
        /// <returns>The server side transceiver and the bound endpoint</returns>
        public abstract (ITransceiver, Endpoint) GetTransceiver();

        /// <summary>Constructs a new endpoint</summary>
        /// <param name="communicator">The endpoint's communicator.</param>
        /// <param name="protocol">The endpoint's protocol.</param>
        protected Endpoint(Communicator communicator, Protocol protocol)
        {
            Communicator = communicator;
            Protocol = protocol;
        }

        /// <summary>Skip unknown endpoints options during unmarshal.</summary>
        /// <param name="istr">The <see cref="InputStream"/> being used to unmarshal the endpoint.</param>
        /// <param name="count">The number of options to skip.</param>
        protected void SkipUnknownOptions(InputStream istr, int count)
        {
            Debug.Assert(count == 0); // TODO: temporary, remove before release
            while (count > 0)
            {
                istr.Skip(istr.ReadSize());
                count--;
            }
        }
    }

    internal static class EndpointExtensions
    {
        /// <summary>Appends the endpoint and all its options (if any) to this string builder, when using the URI
        /// format.</summary>
        /// <param name="sb">The string builder.</param>
        /// <param name="endpoint">The endpoint to append.</param>
        /// <param name="path">The path of the endpoint URI. Use this parameter to start building a proxy URI.</param>
        /// <param name="includeScheme">When true, first appends the endpoint's scheme followed by ://.</param>
        /// <param name="optionSeparator">The character that separates options in the query component of the URI.
        /// </param>
        /// <returns>The string builder parameter.</returns>
        internal static StringBuilder AppendEndpoint(
            this StringBuilder sb,
            Endpoint endpoint,
            string path = "",
            bool includeScheme = true,
            char optionSeparator = '&')
        {
            Debug.Assert(endpoint.Protocol != Protocol.Ice1); // we never generate URIs for the ice1 protocol

            if (includeScheme)
            {
                sb.Append(endpoint.Scheme);
                sb.Append("://");
            }

            if (endpoint.Host.Contains(':'))
            {
                sb.Append('[');
                sb.Append(endpoint.Host);
                sb.Append(']');
            }
            else
            {
                sb.Append(endpoint.Host);
            }

            if (endpoint.Port != endpoint.DefaultPort)
            {
                sb.Append(':');
                sb.Append(endpoint.Port.ToString(CultureInfo.InvariantCulture));
            }

            if (path.Length > 0)
            {
                sb.Append('/');
                sb.Append(path);
            }

            if (endpoint.HasOptions)
            {
                sb.Append('?');
                endpoint.AppendOptions(sb, optionSeparator);
            }
            return sb;
        }
    }
}
