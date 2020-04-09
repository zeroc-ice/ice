//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using IceInternal;

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Net;

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
            string s = "";

            if (Host != null && Host.Length > 0)
            {
                s += " -h ";
                bool addQuote = Host.IndexOf(':') != -1;
                if (addQuote)
                {
                    s += "\"";
                }
                s += Host;
                if (addQuote)
                {
                    s += "\"";
                }
            }

            s += $" -p {Port.ToString(CultureInfo.InvariantCulture)}";

            if (SourceAddress != null)
            {
                string sourceAddr = SourceAddress.ToString();
                bool addQuote = sourceAddr.IndexOf(':') != -1;
                s += " --sourceAddress ";
                if (addQuote)
                {
                    s += "\"";
                }
                s += sourceAddr;
                if (addQuote)
                {
                    s += "\"";
                }
            }

            return s;
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

        public override void IceWriteImpl(Ice.OutputStream s)
        {
            s.WriteString(Host);
            s.WriteInt(Port);
        }

        public override Endpoint NewConnectionId(string connectionId)
            => connectionId == ConnectionId ? this : CreateEndpoint(Host, Port, connectionId);

        public virtual List<IConnector> Connectors(List<IPEndPoint> addresses, INetworkProxy? proxy)
        {
            var connectors = new List<IConnector>();
            foreach (EndPoint p in addresses)
            {
                connectors.Add(CreateConnector(p, proxy));
            }
            return connectors;
        }

        public override void ConnectorsAsync(EndpointSelectionType endpointSelection, IEndpointConnectors callback) =>
            Instance.Resolve(Host, Port, endpointSelection, this, callback);

        public override List<Endpoint> ExpandHost(out Endpoint? publish)
        {
            // If this endpoint has an empty host (wildcard address), don't expand, just return this endpoint.
            var endpoints = new List<Endpoint>();
            if (Host.Length == 0)
            {
                publish = null;
                endpoints.Add(this);
                return endpoints;
            }

            // If using a fixed port, this endpoint can be used as the published endpoint to access the returned
            // endpoints. Otherwise, we'll publish each individual expanded endpoint.
            publish = Port > 0 ? this : null;

            List<IPEndPoint> addresses = Network.GetAddresses(Host,
                                                              Port,
                                                              Instance.IPVersion,
                                                              Ice.EndpointSelectionType.Ordered,
                                                              Instance.PreferIPv6,
                                                              true);

            if (addresses.Count == 1)
            {
                endpoints.Add(this);
            }
            else
            {
                foreach (EndPoint address in addresses)
                {
                    endpoints.Add(CreateEndpoint(Network.EndpointAddressToString(address),
                                                 Network.EndpointPort(address),
                                                 ConnectionId));
                }
            }
            return endpoints;
        }

        public override List<Endpoint> ExpandIfWildcard()
        {
            var endpoints = new List<Endpoint>();
            List<string> hosts = Network.GetHostsForEndpointExpand(Host, Instance.IPVersion, false);
            if (hosts.Count == 0)
            {
                endpoints.Add(this);
            }
            else
            {
                foreach (string host in hosts)
                {
                    endpoints.Add(CreateEndpoint(host, Port, ConnectionId));
                }
            }
            return endpoints;
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

        private protected IPEndpoint(TransportInstance instance, Ice.InputStream s)
        {
            Instance = instance;
            Host = s.ReadString();
            Port = s.ReadInt();
        }

        private protected IPEndpoint(TransportInstance instance, string endpointString,
                                     Dictionary<string, string?> options, bool oaEndpoint)
        {
            Instance = instance;
            string? argument = null;

            if (options.TryGetValue("-h", out argument))
            {
                Host = argument ??
                    throw new FormatException($"no argument provided for -h option in endpoint `{endpointString}'");

                if (Host == "*")
                {
                    if (oaEndpoint)
                    {
                        Host = "";
                    }
                    else
                    {
                        throw new FormatException($"`-h *' not valid for proxy endpoint `{endpointString}'");
                    }
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
