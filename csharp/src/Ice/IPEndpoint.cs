//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Net;

namespace IceInternal
{
    public abstract class IPEndpoint : Endpoint
    {
        public IPEndpoint(TransportInstance instance, string host, int port, EndPoint? sourceAddr, string connectionId)
        {
            Instance = instance;
            Host = host;
            Port = port;
            SourceAddr = sourceAddr;
            ConnectionId_ = connectionId;
        }

        public IPEndpoint(TransportInstance instance)
        {
            Instance = instance;
            Host = null;
            Port = 0;
            SourceAddr = null;
            ConnectionId_ = "";
        }

        public IPEndpoint(TransportInstance instance, Ice.InputStream s)
        {
            Instance = instance;
            Host = s.ReadString();
            Port = s.ReadInt();
            SourceAddr = null;
            ConnectionId_ = "";
        }

        private sealed class Info : Ice.IPEndpointInfo
        {
            public Info(IPEndpoint e) => _endpoint = e;

            public override short Type() => _endpoint.Type();

            public override bool Datagram() => _endpoint.Datagram();

            public override bool Secure() => _endpoint.Secure();

            private readonly IPEndpoint _endpoint;
        }

        public override Ice.EndpointInfo GetInfo()
        {
            var info = new Info(this);
            FillEndpointInfo(info);
            return info;
        }

        public override short Type() => Instance.Type;

        public override string Transport() => Instance.Transport;

        public override bool Secure() => Instance.Secure;

        public override string ConnectionId() => ConnectionId_;

        public override Endpoint ConnectionId(string connectionId)
        {
            if (connectionId.Equals(ConnectionId_))
            {
                return this;
            }
            else
            {
                return CreateEndpoint(Host, Port, connectionId);
            }
        }

        public override void ConnectorsAsync(Ice.EndpointSelectionType selType, IEndpointConnectors callback) =>
            Instance.Resolve(Host!, Port, selType, this, callback);

        public override List<Endpoint> ExpandIfWildcard()
        {
            var endps = new List<Endpoint>();
            List<string> hosts = Network.GetHostsForEndpointExpand(Host!, Instance.IPVersion, false);
            if (hosts == null || hosts.Count == 0)
            {
                endps.Add(this);
            }
            else
            {
                foreach (string h in hosts)
                {
                    endps.Add(CreateEndpoint(h, Port, ConnectionId_));
                }
            }
            return endps;
        }

        public override List<Endpoint> ExpandHost(out Endpoint? publish)
        {
            //
            // If this endpoint has an empty host (wildcard address), don't expand, just return
            // this endpoint.
            //
            var endpoints = new List<Endpoint>();
            if (Host!.Length == 0)
            {
                publish = null;
                endpoints.Add(this);
                return endpoints;
            }

            //
            // If using a fixed port, this endpoint can be used as the published endpoint to
            // access the returned endpoints. Otherwise, we'll publish each individual expanded
            // endpoint.
            //
            publish = Port > 0 ? this : null;

            List<EndPoint> addresses = Network.GetAddresses(Host,
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
                foreach (EndPoint addr in addresses)
                {
                    endpoints.Add(CreateEndpoint(Network.EndpointAddressToString(addr),
                                                 Network.EndpointPort(addr),
                                                 ConnectionId_));
                }
            }
            return endpoints;
        }

        public override bool Equivalent(Endpoint endpoint)
        {
            if (!(endpoint is IPEndpoint))
            {
                return false;
            }
            var ipEndpointI = (IPEndpoint)endpoint;
            return ipEndpointI.Type() == Type() &&
                Equals(ipEndpointI.Host, Host) &&
                ipEndpointI.Port == Port &&
                Equals(ipEndpointI.SourceAddr, SourceAddr);
        }

        public virtual List<IConnector> Connectors(List<EndPoint> addresses, INetworkProxy? proxy)
        {
            var connectors = new List<IConnector>();
            foreach (EndPoint p in addresses)
            {
                connectors.Add(CreateConnector(p, proxy));
            }
            return connectors;
        }

        public override string Options()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
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

            s += " -p " + Port;

            if (SourceAddr != null)
            {
                string sourceAddr = Network.EndpointAddressToString(SourceAddr);
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

        public override int GetHashCode()
        {
            // This code is thread safe because reading/writing _hashCode (an int) is atomic.
            if (_hashCode != 0)
            {
                // Return cached value
                return _hashCode;
            }
            else
            {
                var hash = new HashCode();
                hash.Add(Type());
                HashInit(ref hash);
                int hashCode = hash.ToHashCode();
                if (hashCode == 0) // 0 is not a valid value as it means "not initialized"
                {
                    hashCode = 1;
                }
                _hashCode = hashCode;
                return _hashCode;
            }
        }

        public override int CompareTo(Endpoint obj)
        {
            if (!(obj is IPEndpoint))
            {
                return Type() < obj.Type() ? -1 : 1;
            }

            var p = (IPEndpoint)obj;
            if (this == p)
            {
                return 0;
            }

            int v = string.Compare(Host, p.Host, StringComparison.Ordinal);
            if (v != 0)
            {
                return v;
            }

            if (Port < p.Port)
            {
                return -1;
            }
            else if (p.Port < Port)
            {
                return 1;
            }

            int rc = string.Compare(Network.EndpointAddressToString(SourceAddr),
                                    Network.EndpointAddressToString(p.SourceAddr), StringComparison.Ordinal);
            if (rc != 0)
            {
                return rc;
            }

            return string.Compare(ConnectionId_, p.ConnectionId_, StringComparison.Ordinal);
        }

        public override void StreamWriteImpl(Ice.OutputStream s)
        {
            s.WriteString(Host!);
            s.WriteInt(Port);
        }

        public virtual void HashInit(ref HashCode hash)
        {
            hash.Add(Host!);
            hash.Add(Port);
            if (SourceAddr != null)
            {
                hash.Add(SourceAddr);
            }
            hash.Add(ConnectionId_);
        }

        public virtual void FillEndpointInfo(Ice.IPEndpointInfo info)
        {
            info.Host = Host!;
            info.Port = Port;
            info.SourceAddress = Network.EndpointAddressToString(SourceAddr);
        }

        public virtual void InitWithOptions(List<string> args, bool oaEndpoint)
        {
            base.InitWithOptions(args);

            if (Host == null || Host.Length == 0)
            {
                Host = Instance.DefaultHost;
            }
            else if (Host.Equals("*"))
            {
                if (oaEndpoint)
                {
                    Host = "";
                }
                else
                {
                    throw new FormatException($"`-h *' not valid for proxy endpoint `{this}'");
                }
            }

            if (Host == null)
            {
                Host = "";
            }

            if (SourceAddr != null)
            {
                if (oaEndpoint)
                {
                    throw new FormatException($"`--sourceAddress' not valid for object adapter endpoint `{this}'");
                }
            }
            else if (!oaEndpoint)
            {
                SourceAddr = Instance.DefaultSourceAddress;
            }
        }

        protected override bool CheckOption(string option, string? argument, string endpoint)
        {
            if (option.Equals("-h"))
            {
                Host = argument ?? throw new FormatException($"no argument provided for -h option in endpoint {endpoint}");
            }
            else if (option.Equals("-p"))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for -p option in endpoint {endpoint}");
                }

                try
                {
                    Port = int.Parse(argument, CultureInfo.InvariantCulture);
                }
                catch (FormatException ex)
                {
                    throw new FormatException($"invalid port value `{argument}' in endpoint {endpoint}", ex);
                }

                if (Port < 0 || Port > 65535)
                {
                    throw new FormatException($"port value `{argument}' out of range in endpoint {endpoint}");
                }
            }
            else if (option.Equals("--sourceAddress"))
            {
                if (argument == null)
                {
                    throw new FormatException($"no argument provided for --sourceAddress option in endpoint {endpoint}");
                }
                SourceAddr = Network.GetNumericAddress(argument);
                if (SourceAddr == null)
                {
                    throw new FormatException(
                        $"invalid IP address provided for --sourceAddress option in endpoint {endpoint}");
                }
            }
            else
            {
                return false;
            }
            return true;
        }

        protected abstract IConnector CreateConnector(EndPoint addr, INetworkProxy? proxy);
        protected abstract IPEndpoint CreateEndpoint(string? host, int port, string connectionId);

        protected TransportInstance Instance;
        protected string? Host;
        protected int Port;
        protected EndPoint? SourceAddr;
        protected string ConnectionId_;

        private int _hashCode = 0; // 0 is a special value that means not initialized.
    }

}
