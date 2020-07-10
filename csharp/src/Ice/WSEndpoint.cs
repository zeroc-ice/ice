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
    internal sealed class WSEndpoint : TcpEndpoint
    {
        public override bool IsSecure => Transport == Transport.WSS;

        public override string? this[string option] => option == "resource" ? Resource : base[option];

        /// <summary>A URI specifying the resource associated with this endpoint. The value is passed as the target for
        /// GET in the WebSocket upgrade request.</summary>
        internal string Resource { get; } = "/";

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is WSEndpoint wsEndpoint && Resource == wsEndpoint.Resource && base.Equals(wsEndpoint);
        }

        // TODO: why no hashcode caching?
        public override int GetHashCode() => HashCode.Combine(base.GetHashCode(), Resource);

        public override string OptionsToString()
        {
            var sb = new StringBuilder(base.OptionsToString());
            if (Resource != "/")
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

        public override bool IsLocal(Endpoint endpoint) => endpoint is WSEndpoint && base.IsLocal(endpoint);

        protected internal override void WriteOptions(OutputStream ostr)
        {
            Debug.Assert(Protocol != Protocol.Ice1);
            if (Resource != "/")
            {
                ostr.WriteSize(1);
                ostr.WriteString(Resource);
            }
            else
            {
                ostr.WriteSize(0);
            }
        }

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
            ushort port,
            IPAddress? sourceAddress,
            int timeout,
            bool compressionFlag,
            string resource)
            : base(communicator,
                   transport,
                   protocol,
                   host,
                   port,
                   sourceAddress,
                   timeout,
                   compressionFlag) =>
            Resource = resource;

        // Constructor for parsing with the old format.
        // TODO: remove protocol, as it should be ice1-only.
        internal WSEndpoint(
            Communicator communicator,
            Transport transport,
            Protocol protocol,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
            : base(communicator, transport, protocol, options, oaEndpoint, endpointString)
        {
            if (options.TryGetValue("-r", out string? argument))
            {
                Resource = argument ?? throw new FormatException(
                        $"no argument provided for -r option in endpoint `{endpointString}'");

                options.Remove("-r");
            }
        }

        // Constructor for unmarshaling.
        internal WSEndpoint(InputStream istr, Communicator communicator, Transport transport, Protocol protocol)
            : base(istr, communicator, transport, protocol, mostDerived: false)
        {
            if (protocol == Protocol.Ice1)
            {
                Resource = istr.ReadString();
            }
            else
            {
                int optionCount = istr.ReadSize();
                if (optionCount > 0)
                {
                    Resource = istr.ReadString();
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
            bool oaEndpoint,
            string endpointString)
            : base(communicator, transport, protocol, host, port, options, oaEndpoint, endpointString)
        {
            if (options.TryGetValue("resource", out string? value))
            {
                Resource = value;
                options.Remove("resource");
            }
        }

        private protected override IPEndpoint CreateIPEndpoint(
            string host,
            ushort port,
            bool compressionFlag,
            int timeout) =>
            new WSEndpoint(Communicator,
                           Protocol,
                           Transport,
                           host,
                           port,
                           SourceAddress,
                           timeout,
                           compressionFlag,
                           Resource);

        internal override ITransceiver CreateTransceiver(StreamSocket socket, string? adapterName)
        {
            if (adapterName != null)
            {
                return new WSTransceiver(Communicator, base.CreateTransceiver(socket, adapterName));
            }
            else
            {
                return new WSTransceiver(Communicator, base.CreateTransceiver(socket, adapterName),
                    Host, Resource);
            }
        }
    }
}
