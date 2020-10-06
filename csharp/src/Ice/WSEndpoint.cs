// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace ZeroC.Ice
{
    internal sealed class WSEndpoint : TcpEndpoint
    {
        public override bool IsSecure => Transport == Transport.WSS;

        public override string? this[string option] => option == "resource" ? _resource : base[option];

        /// <summary>A URI specifying the resource associated with this endpoint. The value is passed as the target for
        /// GET in the WebSocket upgrade request.</summary>
        private readonly string _resource = "/";

        protected internal override bool HasOptions => _resource != "/" || base.HasOptions;

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is WSEndpoint wsEndpoint && _resource == wsEndpoint._resource && base.Equals(wsEndpoint);
        }

        // TODO: why no hashcode caching?
        public override int GetHashCode() => HashCode.Combine(base.GetHashCode(), _resource);

        public override bool IsLocal(Endpoint endpoint) => endpoint is WSEndpoint && base.IsLocal(endpoint);

        protected internal override void WriteOptions(OutputStream ostr)
        {
            if (Protocol == Protocol.Ice1)
            {
                base.WriteOptions(ostr);
                ostr.WriteString(_resource);
            }
            else
            {
                if (_resource != "/")
                {
                    ostr.WriteSize(1);
                    ostr.WriteString(_resource);
                }
                else
                {
                    ostr.WriteSize(0); // empty sequence of options
                }
            }
        }

        protected internal override void AppendOptions(StringBuilder sb, char optionSeparator)
        {
            base.AppendOptions(sb, optionSeparator);
            if (_resource != "/")
            {
                if (Protocol == Protocol.Ice1)
                {
                    sb.Append(" -r ");
                    bool addQuote = _resource.IndexOf(':') != -1;
                    if (addQuote)
                    {
                        sb.Append('"');
                    }
                    sb.Append(_resource);
                    if (addQuote)
                    {
                        sb.Append('"');
                    }
                }
                else
                {
                    if (base.HasOptions)
                    {
                        sb.Append(optionSeparator);
                    }
                    sb.Append("resource=");
                    // _resource must be in a URI-compatible format, with for example spaces escaped as %20.
                    sb.Append(_resource);
                }
            }
        }

        // Constructor for ice1 endpoint parsing.
        internal WSEndpoint(
            Communicator communicator,
            Transport transport,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
            : base(communicator, transport, options, oaEndpoint, endpointString)
        {
            if (options.TryGetValue("-r", out string? argument))
            {
                _resource = argument ?? throw new FormatException(
                        $"no argument provided for -r option in endpoint `{endpointString}'");

                options.Remove("-r");
            }
        }

        // Constructor for unmarshaling.
        internal WSEndpoint(InputStream istr, Transport transport, Protocol protocol)
            : base(istr, transport, protocol, mostDerived: false)
        {
            if (protocol == Protocol.Ice1)
            {
                _resource = istr.ReadString();
            }
            else
            {
                int optionCount = istr.ReadSize();
                if (optionCount > 0)
                {
                    _resource = istr.ReadString();
                    optionCount--;
                    SkipUnknownOptions(istr, optionCount);
                }
            }
        }

        // Constructor used for URI parsing.
        internal WSEndpoint(
            Communicator communicator,
            Transport transport,
            Protocol protocol,
            string host,
            ushort port,
            Dictionary<string, string> options,
            bool oaEndpoint)
            : base(communicator, transport, protocol, host, port, options, oaEndpoint)
        {
            if (options.TryGetValue("resource", out string? value))
            {
                // The resource value (as supplied in a URI string) must be percent-escaped with '/' separators
                // We keep it as-is, and will marshal it as-is.
                _resource = value;
                options.Remove("resource");
            }
            else if (options.TryGetValue("option", out value))
            {
                // We are parsing a ice+universal endpoint
                if (value.Contains(','))
                {
                    throw new FormatException("a WS endpoint accepts at most one marshaled option (resource)");
                }
                // Each option of a universal endpoint needs to be unescaped
                _resource = Uri.UnescapeDataString(value);
                options.Remove("option");
            }
        }

        internal override Connection CreateConnection(
            IConnectionManager manager,
            MultiStreamTransceiverWithUnderlyingTransceiver transceiver,
            IConnector? connector,
            string connectionId,
            ObjectAdapter? adapter) =>
            new WSConnection(manager, this, transceiver, connector, connectionId, adapter);

        // Clone constructor
        private WSEndpoint(WSEndpoint endpoint, string host, ushort port)
            : base(endpoint, host, port) => _resource = endpoint._resource;

        private protected override IPEndpoint Clone(string host, ushort port) =>
            new WSEndpoint(this, host, port);

        internal override ITransceiver CreateTransceiver(EndPoint addr, INetworkProxy? proxy) =>
            new WSTransceiver(Communicator, base.CreateTransceiver(addr, proxy), Host, _resource);

        internal override ITransceiver CreateTransceiver(Socket socket, string adapterName) =>
            new WSTransceiver(Communicator, base.CreateTransceiver(socket, adapterName));
    }
}
