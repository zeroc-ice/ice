//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.Text;
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

        /// <summary>Indicates whether or not this endpoint's compression flag is set. When the compression flag is
        /// set, a request sent to this endpoint using the ice1 protocol is automatically compressed using bzip2 if
        /// the request's uncompressed size is greater than 100 bytes. Only applies to ice1.</summary>
        /// <value>True when the compression flag is set; otherwise, false.</value>
        public virtual bool HasCompressionFlag => false;

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
                        "compress" => HasCompressionFlag ? "true" : null,
                        "host" => Host.Length > 0 ? Host : null,
                        "port" => Port != DefaultPort ? Port.ToString(CultureInfo.InvariantCulture) : null,
                        "timeout" => Timeout != DefaultTimeout ?
                            Timeout.TotalMilliseconds.ToString(CultureInfo.InvariantCulture) : null,
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

        /// <summary>The timeout for the endpoint. This timeout is no longer used since Ice 4.0.</summary>
        public virtual TimeSpan Timeout => DefaultTimeout;

        /// <summary>The <see cref="ZeroC.Ice.Transport"></see> of this endpoint.</summary>
        public abstract Transport Transport { get; }

        /// <summary>The name of the endpoint's transport in lowercase.</summary>
        public virtual string TransportName => Transport.ToString().ToLowerInvariant();

        /// <summary>Gets the default port of this endpoint.</summary>
        protected internal abstract ushort DefaultPort { get; }

        /// <summary>Indicates whether or not this endpoint has options with non default values that ToString would
        /// print. Always true for ice1 endpoints.</summary>
        protected internal abstract bool HasOptions { get; }

        /// <summary>The default timeout for ice1 endpoints.</summary>
        protected static readonly TimeSpan DefaultTimeout = TimeSpan.FromSeconds(60);

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

        public static bool operator !=(Endpoint? lhs, Endpoint? rhs) => !(lhs == rhs);

        public override bool Equals(object? obj) => obj is Endpoint other && Equals(other);

        public virtual bool Equals(Endpoint? other) =>
            other is Endpoint endpoint &&
                Communicator == endpoint.Communicator &&
                Protocol == endpoint.Protocol &&
                Transport == endpoint.Transport &&
                Host == endpoint.Host &&
                Port == endpoint.Port;

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

        // Checks whether this endpoint and the given endpoint point to the same local peer. This is used for the
        // collocation optimization check to figure out whether or not a proxy endpoint points to a local adapter.
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

        // Returns a new endpoint with a different timeout value, provided that timeouts are supported by the endpoint.
        // Otherwise the same endpoint is returned.
        public abstract Endpoint NewTimeout(TimeSpan t);

        // Returns a new endpoint with a different compression flag, provided that compression is supported by the
        // endpoint. Otherwise the same endpoint is returned.
        public abstract Endpoint NewCompressionFlag(bool compressionFlag);

        // Returns a connector for this endpoint, or empty list if no connector is available.
        public abstract ValueTask<IEnumerable<IConnector>> ConnectorsAsync(EndpointSelectionType endpointSelection);

        // Expands endpoint out in to separate endpoints for each local host if listening on INADDR_ANY on server side
        // or if no host was specified on client side.
        public abstract IEnumerable<Endpoint> ExpandIfWildcard();

        // Expands endpoint out into separate endpoints for each IP address returned by the DNS resolver. Also returns
        // the endpoint which can be used to connect to the returned endpoints or null if no specific endpoint can be
        // used to connect to these endpoints (e.g.: with the IP endpoint, it returns this endpoint if it uses a fixed
        // port, null otherwise).
        public abstract IEnumerable<Endpoint> ExpandHost(out Endpoint? publishedEndpoint);

        // Returns an acceptor for this endpoint, or null if no acceptor is available.
        public abstract IAcceptor? GetAcceptor(string adapterName);

        // Return a server side transceiver for this endpoint, or null if a transceiver can only be created by an
        // acceptor.
        public abstract ITransceiver? GetTransceiver();

        protected Endpoint(Communicator communicator, Protocol protocol)
        {
            Communicator = communicator;
            Protocol = protocol;
        }

        protected void SkipUnknownOptions(InputStream istr, int count)
        {
            Debug.Assert(count == 0); // TODO: temporary, remove before release
            while (count > 0)
            {
                istr.Skip(istr.ReadSize());
                count--;
            }
        }

        /// <summary>Creates an endpoint from a string.</summary>
        /// <param name="endpointString">The string parsed by this method.</param>
        /// <param name="protocol">The Ice protocol of the enclosing proxy.</param>
        /// <param name="communicator">The communicator of the enclosing proxy or object adapter.</param>
        /// <param name="oaEndpoint">When true, endpointString represents an object adapter's endpoint configuration;
        /// when false, endpointString represents a proxy endpoint.</param>
        /// <returns>The new endpoint.</returns>
        /// <exception cref="FormatException">Thrown when endpointString cannot be parsed.</exception>
        /// <exception cref="NotSupportedException">Thrown when the transport specified in endpointString does not
        /// support the specified protocol.</exception>
        // TODO: revise description of protocol when oaEndpoint = true.
        internal static Endpoint Parse(
            string endpointString,
            Protocol protocol,
            Communicator communicator,
            bool oaEndpoint)
        {
            string[]? args = StringUtil.SplitString(endpointString, " \t\r\n");
            if (args == null)
            {
                throw new FormatException($"mismatched quote in endpoint `{endpointString}'");
            }

            if (args.Length == 0)
            {
                throw new FormatException("no non-whitespace character in endpoint string");
            }

            string transportName = args[0];
            if (transportName == "default")
            {
                transportName = communicator.DefaultTransport;
            }

            var options = new Dictionary<string, string?>();

            // Parse args into options (and skip transportName at args[0])
            for (int n = 1; n < args.Length; ++n)
            {
                // Any option with < 2 characters or that does not start with - is illegal
                string option = args[n];
                if (option.Length < 2 || option[0] != '-')
                {
                    throw new FormatException($"invalid option `{option}' in endpoint `{endpointString}'");
                }

                // Extract the argument given to the current option, if any
                string? argument = null;
                if (n + 1 < args.Length && args[n + 1][0] != '-')
                {
                    argument = args[++n];
                }

                try
                {
                    options.Add(option, argument);
                }
                catch (ArgumentException)
                {
                    throw new FormatException($"duplicate option `{option}' in endpoint `{endpointString}'");
                }
            }

            if (communicator.FindEndpointFactory(transportName) is (IEndpointFactory factory, Transport transport))
            {
                Endpoint endpoint = factory.Create(transport, protocol, options, oaEndpoint, endpointString);
                if (options.Count > 0)
                {
                    throw new FormatException(
                        $"unrecognized option(s) `{ToString(options)}' in endpoint `{endpointString}'");
                }
                return endpoint;
            }

            // If the stringified endpoint is opaque, create an unknown endpoint, then see whether the type matches one
            // of the known endpoints.
            if (!oaEndpoint && transportName == "opaque")
            {
                var opaqueEndpoint = new OpaqueEndpoint(communicator, options, endpointString);
                if (options.Count > 0)
                {
                    throw new FormatException(
                        $"unrecognized option(s) `{ToString(options)}' in endpoint `{endpointString}'");
                }

                if (opaqueEndpoint.ValueEncoding.IsSupported &&
                    communicator.IceFindEndpointFactory(opaqueEndpoint.Transport) != null)
                {
                    // We may be able to unmarshal this endpoint, so we first marshal it into a byte buffer and then
                    // unmarshal it from this buffer.
                    var bufferList = new List<ArraySegment<byte>>
                    {
                        // 8 = size of short + size of encapsulation header with 1.1 encoding
                        new byte[8 + opaqueEndpoint.Value.Length]
                    };

                    var ostr = new OutputStream(Ice1Definitions.Encoding, bufferList);
                    ostr.WriteEndpoint(opaqueEndpoint);
                    OutputStream.Position tail = ostr.Save();
                    Debug.Assert(bufferList.Count == 1);
                    Debug.Assert(tail.Segment == 0 && tail.Offset == 8 + opaqueEndpoint.Value.Length);

                    return
                        new InputStream(Ice1Definitions.Encoding, bufferList[0]).ReadEndpoint(protocol, communicator);
                }
                else
                {
                    return opaqueEndpoint;
                }
            }

            throw new FormatException($"unknown transport `{transportName}' in endpoint `{endpointString}'");
        }

        // Stringify the options of an endpoint
        private static string ToString(Dictionary<string, string?> options)
        {
            var sb = new StringBuilder();
            foreach ((string option, string? argument) in options)
            {
                if (sb.Length > 0)
                {
                    sb.Append(" ");
                }
                sb.Append(option);
                if (argument != null)
                {
                    sb.Append(" ");
                    sb.Append(argument);
                }
            }
            return sb.ToString();
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
