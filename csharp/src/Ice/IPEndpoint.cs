//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Net;
using System.Text;

namespace Ice
{
    /// <summary>The base class for IP-based endpoints: TcpEndpoint, UdpEndpoint.</summary>
    public abstract class IPEndpoint : Endpoint
    {
        public override string ConnectionId { get; } = "";

        /// <summary>The hostname of this IP endpoint.</summary>
        // TODO: convert to a get-only property.
        public readonly string Host;
        public override bool IsSecure => Instance.Secure;

        /// <summary>The port number of this IP endpoint.</summary>
        // TODO: convert to a get-only property.
        public readonly int Port;

        /// <summary>The source address of this IP endpoint.</summary>
        public IPAddress? SourceAddress { get; }
        public override EndpointType Type => Instance.Type;

        protected readonly TransportInstance Instance;

        public override bool Equals(Endpoint? other)
        {
            if (other is IPEndpoint ipEndpoint)
            {
                if (Type != ipEndpoint.Type)
                {
                    return false;
                }
                if (Host != ipEndpoint.Host)
                {
                    return false;
                }
                if (Port != ipEndpoint.Port)
                {
                    return false;
                }
                if (!Equals(SourceAddress, ipEndpoint.SourceAddress))
                {
                    return false;
                }
                if (ConnectionId != ipEndpoint.ConnectionId)
                {
                    return false;
                }
                return true;
            }
            else
            {
                return false;
            }
        }

        public override int GetHashCode()
        {
            // The hash code is cached in the derived class, not this abstract base class.
            var hash = new HashCode();
            hash.Add(Type);
            hash.Add(Host);
            hash.Add(Port);
            if (SourceAddress != null)
            {
                hash.Add(SourceAddress);
            }
            hash.Add(ConnectionId);
            return hash.ToHashCode();
        }

        public override string OptionsToString()
        {
            var sb = new StringBuilder();

            if (Host.Length > 0)
            {
                sb.Append(" -h ");
                bool addQuote = Host.IndexOf(':') != -1;
                if (addQuote)
                {
                    sb.Append("\"");
                }
                sb.Append(Host);
                if (addQuote)
                {
                    sb.Append("\"");
                }
            }

            sb.Append(" -p ");
            sb.Append(Port.ToString(CultureInfo.InvariantCulture));

            if (SourceAddress != null)
            {
                string sourceAddr = SourceAddress.ToString();
                bool addQuote = sourceAddr.IndexOf(':') != -1;
                sb.Append(" --sourceAddress ");
                if (addQuote)
                {
                    sb.Append("\"");
                }
                sb.Append(sourceAddr);
                if (addQuote)
                {
                    sb.Append("\"");
                }
            }

            return sb.ToString();
        }

        public override bool Equivalent(Endpoint endpoint)
        {
            if (endpoint is IPEndpoint ipEndpoint)
            {
                // Same as Equals except we don't consider the connection ID

                if (Type != ipEndpoint.Type)
                {
                    return false;
                }
                if (Host != ipEndpoint.Host)
                {
                    return false;
                }
                if (Port != ipEndpoint.Port)
                {
                    return false;
                }
                if (!Equals(SourceAddress, ipEndpoint.SourceAddress))
                {
                    return false;
                }
                return true;
            }
            else
            {
                return false;
            }
        }

        public override void IceWritePayload(OutputStream ostr)
        {
            ostr.WriteString(Host);
            ostr.WriteInt(Port);
        }

        public override Endpoint NewConnectionId(string connectionId) =>
            connectionId == ConnectionId ? this : CreateEndpoint(Host, Port, connectionId);

        public virtual List<IConnector> Connectors(List<IPEndPoint> endpoints, INetworkProxy? proxy)
        {
            var connectors = new List<IConnector>();
            foreach (EndPoint p in endpoints)
            {
                connectors.Add(CreateConnector(p, proxy));
            }
            return connectors;
        }

        public override void ConnectorsAsync(EndpointSelectionType endpointSelection, IEndpointConnectors callback) =>
            Instance.Resolve(Host, Port, endpointSelection, this, callback);

        public override IEnumerable<Endpoint> ExpandHost(out Endpoint? publish)
        {
            publish = null;
            // If this endpoint has an empty host (wildcard address), don't expand, just return this endpoint.
            if (Host.Length == 0)
            {
                return new Endpoint[] { this };
            }

            // If using a fixed port, this endpoint can be used as the published endpoint to access the returned
            // endpoints. Otherwise, we'll publish each individual expanded endpoint.
            publish = Port > 0 ? this : null;

            List<IPEndPoint> addresses = Network.GetAddresses(Host,
                                                              Port,
                                                              Instance.IPVersion,
                                                              EndpointSelectionType.Ordered,
                                                              Instance.PreferIPv6,
                                                              true);

            if (addresses.Count == 1)
            {
                return new Endpoint[] { this };
            }
            else
            {
                return addresses.Select(address => CreateEndpoint(Network.EndpointAddressToString(address),
                                                                  Network.EndpointPort(address),
                                                                  ConnectionId));
            }
        }

        public override IEnumerable<Endpoint> ExpandIfWildcard()
        {
            List<string> hosts = Network.GetHostsForEndpointExpand(Host, Instance.IPVersion, false);
            if (hosts.Count == 0)
            {
                return new Endpoint[] { this };
            }
            else
            {
                return hosts.Select(host => CreateEndpoint(host, Port, ConnectionId));
            }
        }

        private protected IPEndpoint(TransportInstance instance, string host, int port, IPAddress? sourceAddress,
            string connectionId)
        {
            Instance = instance;
            Host = host;
            Port = port;
            SourceAddress = sourceAddress;
            ConnectionId = connectionId;
        }

        private protected IPEndpoint(TransportInstance instance, InputStream s)
        {
            Instance = instance;
            Host = s.ReadString();
            Port = s.ReadInt();
        }

        private protected IPEndpoint(TransportInstance instance, string endpointString,
                                     Dictionary<string, string?> options, bool oaEndpoint)
        {
            Instance = instance;

            if (options.TryGetValue("-h", out string? argument))
            {
                Host = argument ??
                    throw new FormatException($"no argument provided for -h option in endpoint `{endpointString}'");

                if (Host == "*")
                {
                    Host = oaEndpoint ? "" :
                        throw new FormatException($"`-h *' not valid for proxy endpoint `{endpointString}'");
                }
                options.Remove("-h");
            }
            else
            {
                Host = Instance.DefaultHost;
            }

            if (options.TryGetValue("-p", out argument))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for -p option in endpoint `{endpointString}'");
                }

                try
                {
                    Port = int.Parse(argument, CultureInfo.InvariantCulture);
                }
                catch (FormatException ex)
                {
                    throw new FormatException($"invalid port value `{argument}' in endpoint `{endpointString}'", ex);
                }

                if (Port < 0 || Port > 65535)
                {
                    throw new FormatException($"port value `{argument}' out of range in endpoint `{endpointString}'");
                }
                options.Remove("-p");
            }
            // else Port remains 0

            if (options.TryGetValue("--sourceAddress", out argument))
            {
                if (oaEndpoint)
                {
                    throw new FormatException(
                        $"`--sourceAddress' not valid for object adapter endpoint `{endpointString}'");
                }
                if (argument == null)
                {
                    throw new FormatException(
                        $"no argument provided for --sourceAddress option in endpoint `{endpointString}'");
                }
                SourceAddress = Network.GetNumericAddress(argument);
                if (SourceAddress == null)
                {
                    throw new FormatException(
                        $"invalid IP address provided for --sourceAddress option in endpoint `{endpointString}'");
                }
                options.Remove("--sourceAddress");
            }
            // else SourceAddress remains null
        }

        private protected abstract IConnector CreateConnector(EndPoint addr, INetworkProxy? proxy);
        private protected abstract IPEndpoint CreateEndpoint(string host, int port, string connectionId);
    }
}
