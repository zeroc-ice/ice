// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Linq;
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

        /// <summary>Gets the external "over the wire" representation of this endpoint. With ice2 (and up) this is the
        /// actual data structure sent and received over the wire for this endpoint. With ice1, it is a subset of this
        /// external representation.</summary>
        /// <remarks>The Options field of EndpointData is a writable array but should be treated as if it was read-only.
        /// Do not update the contents of this array.</remarks>
        public EndpointData Data { get; }

        /// <summary>The host name or address.</summary>
        public string Host => Data.Host;

        /// <summary>Indicates whether or not this endpoint's transport is always secure. Only applies to ice1.</summary>
        /// <value>True when this endpoint's transport is secure; otherwise, false.</value>
        public virtual bool IsAlwaysSecure => false;

        /// <summary>Indicates whether or not this endpoint's transport uses datagrams with no ordering or delivery
        /// guarantees.</summary>
        /// <value>True when this endpoint's transport is datagram-based; otherwise, false. There is currently a
        /// single datagram-based transport: UDP.</value>
        public virtual bool IsDatagram => false;

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
        public ushort Port => Data.Port;

        /// <summary>The Ice protocol of this endpoint.</summary>
        public Protocol Protocol { get; }

        /// <summary>The scheme for this endpoint. With ice1, it's the transport name (tcp, ssl etc.) or opaque. With
        /// ice2, it's ice+transport (ice+tcp, ice+quic etc.) or ice+universal.</summary>
        public virtual string Scheme => Protocol == Protocol.Ice1 ? TransportName : $"ice+{TransportName}";

        /// <summary>The <see cref="Ice.Transport"></see> of this endpoint.</summary>
        public Transport Transport => Data.Transport;

        /// <summary>The name of the endpoint's transport in lowercase.</summary>
        public virtual string TransportName => Transport.ToString().ToLowerInvariant();

        /// <summary>Gets the default port of this endpoint.</summary>
        protected internal abstract ushort DefaultPort { get; }

        /// <summary>Returns true when Host is a DNS name that needs to be resolved; otherwise, returns false.
        /// When a derived implementation returns true, it must override <see cref="ExpandHostAsync"/>.</summary>
        protected internal virtual bool HasDnsHost => false;

        /// <summary>Indicates whether or not this endpoint has options with non default values that ToString would
        /// print. Always true for ice1 endpoints.</summary>
        protected internal abstract bool HasOptions { get; }

        /// <summary>Creates a connection to this endpoint.</summary>
        /// <param name="preferNonSecure">Indicates under what conditions establishing a non-secure connection should
        /// be preferred.</param>
        /// <param name="label">The corresponding Connection property <see cref="Connection.Label"/> can be non-null
        /// only for outgoing connections.</param>
        /// <param name="cancel">A cancellation token that receives the cancellation requests.</param>
        /// <returns>The new established connection.</returns>
        protected internal abstract Task<Connection> ConnectAsync(
            NonSecure preferNonSecure,
            object? label,
            CancellationToken cancel);

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
                Data == endpoint.Data;

        /// <inheritdoc/>
        public override int GetHashCode() => HashCode.Combine(Communicator, Protocol, Data);

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
        /// the colocation optimization check to figure out whether or not a proxy endpoint points to a local adapter.
        /// </summary>
        /// <param name="endpoint">The other endpoint.</param>
        /// <returns><c>True</c> if the other endpoint point to the same local peer, <c>False</c> otherwise.</returns>
        public abstract bool IsLocal(Endpoint endpoint);

        /// <summary>Appends the options of this endpoint with non default values to the string builder.</summary>
        /// <param name="sb">The string builder.</param>
        /// <param name="optionSeparator">The character used to separate two options. This separator is not used for
        /// ice1 endpoints.</param>
        protected internal abstract void AppendOptions(StringBuilder sb, char optionSeparator);

        /// <summary>Provides the same hash code for two equivalent endpoints. See <see cref="IsEquivalent"/>.</summary>
        protected internal virtual int GetEquivalentHashCode() => GetHashCode();

        /// <summary>Two endpoints are considered equivalent if they are equal or their differences should not trigger
        /// the establishment of separate connections to those endpoints. For example, two tcp endpoints that are
        /// identical except for their ice1 HashCompressedFlag property are equivalent but are not equal.</summary>
        protected internal virtual bool IsEquivalent(Endpoint other) => Equals(other);

        /// <summary>Writes the options of this endpoint to the output stream. ice1-only.</summary>
        /// <param name="ostr">The output stream.</param>
        protected internal abstract void WriteOptions(OutputStream ostr);

        /// <summary>Returns an acceptor for this endpoint. An acceptor listens for connection establishment requests
        /// from clients and creates a new connection for each client. This is typically used to implement a
        /// stream-based transport. Datagram transports don't implement this method but instead implement the
        /// <see cref="CreateDatagramServerConnection"/> method.</summary>
        /// <param name="adapter">The object adapter associated to the acceptor.</param>
        /// <returns>An acceptor for this endpoint.</returns>
        public abstract IAcceptor Acceptor(ObjectAdapter adapter);

        /// <summary>Creates a datagram server side connection for this endpoint to receive datagrams from clients.
        /// Unlike stream-based transports, datagram endpoints don't support an acceptor responsible for accepting new
        /// connections but implement this method to provide a connection responsible for receiving datagrams from
        /// clients.</summary>
        /// <returns>The datagram server side connection.</returns>
        public abstract Connection CreateDatagramServerConnection(ObjectAdapter adapter);

        /// <summary>Expands endpoint into separate endpoints for each IP address returned by the DNS resolver.
        /// Precondition: <see cref="HasDnsHost"/> is true.</summary>
        /// <returns>A value task holding the expanded endpoints.</returns>
        protected internal virtual ValueTask<IEnumerable<Endpoint>> ExpandHostAsync(CancellationToken cancel) =>
            throw new NotImplementedException();

        /// <summary>Returns the published endpoint for this object adapter endpoint.</summary>
        /// <param name="serverName">The server name, to be used as the host of the published endpoint when the
        /// endpoint's type supports DNS resolution of its hosts. Otherwise, <c>serverName</c> is not used.</param>
        /// <returns>The published endpoint.</returns>
        protected internal abstract Endpoint GetPublishedEndpoint(string serverName);

        /// <summary>Constructs a new endpoint</summary>
        /// <param name="data">The <see cref="EndpointData"/> struct.</param>
        /// <param name="communicator">The endpoint's communicator.</param>
        /// <param name="protocol">The endpoint's protocol.</param>
        protected Endpoint(EndpointData data, Communicator communicator, Protocol protocol)
        {
            Communicator = communicator;
            Data = data;
            Protocol = protocol;
        }
    }

    public static class EndpointExtensions
    {
        /// <summary>Creates an endpoint from an <see cref="EndpointData"/> struct.</summary>
        /// <param name="data">The endpoint's data.</param>
        /// <param name="communicator">The communicator.</param>
        /// <param name="protocol">The endpoint's protocol. Must be ice2 or greater.</param>
        /// <returns>A new endpoint.</returns>
        public static Endpoint ToEndpoint(
            this EndpointData data,
            Communicator communicator,
            Protocol protocol = Protocol.Ice2)
        {
            if ((byte)protocol < (byte)Protocol.Ice2)
            {
                throw new ArgumentException("protocol must be ice2 or greater", nameof(protocol));
            }

            Ice2EndpointFactory? factory =
                    protocol == Protocol.Ice2 ? communicator.FindIce2EndpointFactory(data.Transport) : null;

            return factory?.Invoke(data, communicator) ?? UniversalEndpoint.Create(data, communicator, protocol);
        }

        /// <summary>Creates an endpoint data list from a sequence of endpoints.</summary>
        /// <param name="endpoints">The sequence of endpoints.</param>
        /// <returns>A new list of endpoint data.</returns>
        public static List<EndpointData> ToEndpointDataList(this IEnumerable<Endpoint> endpoints) =>
            endpoints.Select(e => e.Data).ToList();

        /// <summary>Creates an endpoint list from a sequence of <see cref="EndpointData"/> structs.</summary>
        /// <param name="dataSequence">The sequence of endpoint data.</param>
        /// <param name="communicator">The communicator.</param>
        /// <param name="protocol">The endpoint's protocol. Must be ice2 or greater.</param>
        /// <returns>A new list of endpoints.</returns>
        public static List<Endpoint> ToEndpointList(
            this IEnumerable<EndpointData> dataSequence,
            Communicator communicator,
            Protocol protocol = Protocol.Ice2) =>
            dataSequence.Select(data => data.ToEndpoint(communicator, protocol)).ToList();

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

        internal static StringBuilder AppendEndpointList(
            this StringBuilder sb,
            IReadOnlyList<Endpoint> endpoints)
        {
            Debug.Assert(endpoints.Count > 0);

            if (endpoints[0].Protocol == Protocol.Ice1)
            {
                sb.Append(string.Join(":", endpoints));
            }
            else
            {
                sb.AppendEndpoint(endpoints[0]);
                if (endpoints.Count > 1)
                {
                    Transport mainTransport = endpoints[0].Transport;
                    sb.Append("?alt-endpoint=");
                    for (int i = 1; i < endpoints.Count; ++i)
                    {
                        if (i > 1)
                        {
                            sb.Append(',');
                        }
                        sb.AppendEndpoint(endpoints[i], "", mainTransport != endpoints[i].Transport, '$');
                    }
                }
            }
            return sb;
        }
    }
}
