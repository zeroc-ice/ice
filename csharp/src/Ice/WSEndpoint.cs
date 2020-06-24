//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Net;
using System.Text;

namespace ZeroC.Ice
{
    public sealed class WSEndpoint : TcpEndpoint
    {
        public override bool IsSecure => Transport == Transport.WSS;

        /// <summary>A URI specifying the resource associated with this endpoint. The value is passed as the target for
        /// GET in the WebSocket upgrade request.</summary>
        public string Resource { get; }

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is WSEndpoint wsEndpoint && Resource == wsEndpoint.Resource && base.Equals(wsEndpoint);
        }

        public override int GetHashCode() => HashCode.Combine(base.GetHashCode(), Resource);

        public override string OptionsToString()
        {
            var sb = new StringBuilder(base.OptionsToString());
            if (Resource.Length > 0)
            {
                sb.Append(" -r ");
                bool addQuote = Resource.IndexOf(':') != -1;
                if (addQuote)
                {
                    sb.Append("\"");
                }
                sb.Append(Resource);
                if (addQuote)
                {
                    sb.Append("\"");
                }
            }
            return sb.ToString();
        }

        public override bool Equivalent(Endpoint endpoint) => endpoint is WSEndpoint && base.Equivalent(endpoint);

        public override void IceWritePayload(OutputStream ostr)
        {
            base.IceWritePayload(ostr);
            ostr.WriteString(Resource);
        }

        public override ITransceiver? GetTransceiver() => null;

        internal WSEndpoint(
            Communicator communicator,
            Protocol protocol,
            Transport transport,
            string host,
            int port,
            IPAddress? sourceAddress,
            int timeout,
            string connectionId,
            bool compressionFlag,
            string resource)
            : base(communicator,
                   protocol,
                   transport,
                   host,
                   port,
                   sourceAddress,
                   timeout,
                   connectionId,
                   compressionFlag) =>
            Resource = resource;

        internal WSEndpoint(
            Communicator communicator,
            Protocol protocol,
            Transport transport,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
            : base(communicator, protocol, transport, options, oaEndpoint, endpointString)
        {
            if (options.TryGetValue("-r", out string? argument))
            {
                Resource = argument ?? throw new FormatException(
                        $"no argument provided for -r option in endpoint `{endpointString}'");

                options.Remove("-r");
            }
            else
            {
                Resource = "/";
            }
        }

        internal WSEndpoint(InputStream istr, Protocol protocol, Transport transport)
            : base(istr, protocol, transport) =>
            Resource = istr.ReadString();

        private protected override IPEndpoint CreateIPEndpoint(
            string host,
            int port,
            string connectionId,
            bool compressionFlag,
            int timeout) =>
            new WSEndpoint(Communicator,
                           Protocol,
                           Transport,
                           host,
                           port,
                           SourceAddress,
                           timeout,
                           connectionId,
                           compressionFlag,
                           Resource);

        internal override ITransceiver CreateTransceiver(string transport, StreamSocket socket, string? adapterName)
        {
            if (adapterName != null)
            {
                return new WSTransceiver(Communicator, base.CreateTransceiver(transport, socket, adapterName));
            }
            else
            {
                return new WSTransceiver(Communicator, base.CreateTransceiver(transport, socket, adapterName),
                    Host, Resource);
            }
        }
    }

    internal class WSEndpointFactory : IEndpointFactory
    {
        private Communicator Communicator { get; }

        public Endpoint Create(
            Transport transport,
            Protocol protocol,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
        {
            Debug.Assert(transport == Transport.WS || transport == Transport.WSS);
            return new WSEndpoint(Communicator, protocol, transport, options, oaEndpoint, endpointString);
        }

        public Endpoint Read(InputStream istr, Protocol protocol, Transport transport) =>
            new WSEndpoint(istr, protocol, transport);

        internal WSEndpointFactory(Communicator communicator) => Communicator = communicator;
    }
}
