//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Net;
using System.Text;

namespace ZeroC.Ice
{
    public sealed class WSEndpoint : TcpEndpoint
    {
        /// <summary>A URI specifying the resource associated with this endpoint. The value is passed as the target for
        /// GET in the WebSocket upgrade request.</summary>
        public string Resource { get; }
        public override EndpointType Type => EndpointType.WS;

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

        internal WSEndpoint(Communicator communicator, string host, int port, IPAddress? sourceAddress, int timeout,
            string connectionId, bool compressionFlag, string resource)
            : base(communicator, host, port, sourceAddress, timeout, connectionId, compressionFlag) =>
            Resource = resource;

        internal WSEndpoint(Communicator communicator, string endpointString, Dictionary<string, string?> options,
            bool oaEndpoint) : base(communicator, endpointString, options, oaEndpoint)
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

        internal WSEndpoint(Communicator communicator, InputStream istr) : base(communicator, istr) =>
            Resource = istr.ReadString();

        private protected override IPEndpoint CreateEndpoint(string host, int port, string connectionId,
            bool compressionFlag, int timeout) =>
            new WSEndpoint(Communicator, host, port, SourceAddress, timeout, connectionId, compressionFlag, Resource);

        internal override ITransceiver CreateTransceiver(
            string transport, StreamSocket socket, string? adapterName)
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

    public sealed class WSSEndpoint : SslEndpoint
    {
        /// <summary> The resource of the WebSocket endpoint.</summary>
        // TODO: better description
        public string Resource { get; }
        public override EndpointType Type => EndpointType.WSS;

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is WSSEndpoint wssEndpoint && Resource == wssEndpoint.Resource && base.Equals(wssEndpoint);
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

        public override bool Equivalent(Endpoint endpoint) => endpoint is WSSEndpoint && base.Equivalent(endpoint);

        public override void IceWritePayload(OutputStream ostr)
        {
            base.IceWritePayload(ostr);
            ostr.WriteString(Resource);
        }

        public override ITransceiver? GetTransceiver() => null;

        internal WSSEndpoint(SslEngine engine, Communicator communicator, string host, int port, IPAddress? sourceAddress, int timeout,
            string connectionId, bool compressionFlag, string resource)
            : base(engine, communicator, host, port, sourceAddress, timeout, connectionId, compressionFlag) =>
            Resource = resource;

        internal WSSEndpoint(SslEngine engine, Communicator communicator, string endpointString, Dictionary<string, string?> options,
            bool oaEndpoint) : base(engine, communicator, endpointString, options, oaEndpoint)
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

        internal WSSEndpoint(SslEngine engine, Communicator communicator, InputStream istr)
            : base(engine, communicator, istr) => Resource = istr.ReadString();

        private protected override IPEndpoint CreateEndpoint(string host, int port, string connectionId,
            bool compressionFlag, int timeout) =>
            new WSSEndpoint(SslEngine, Communicator, host, port, SourceAddress, timeout, connectionId,
                compressionFlag, Resource);

        internal override ITransceiver CreateTransceiver(
            string transport, StreamSocket socket, string? adapterName)
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
        internal WSEndpointFactory(Communicator communicator) => Communicator = communicator;

        public string Name => "ws";
        public EndpointType Type => EndpointType.WS;

        public Endpoint Create(string endpointString, Dictionary<string, string?> options, bool oaEndpoint) =>
            new WSEndpoint(Communicator, endpointString, options, oaEndpoint);
        public Endpoint Read(InputStream istr) => new WSEndpoint(Communicator, istr);
    }

    internal class WSSEndpointFactory : IEndpointFactory
    {
        private Communicator Communicator { get; }
        private SslEngine SslEngine { get; }
        internal WSSEndpointFactory(Communicator communicator, SslEngine engine)
        {
            Communicator = communicator;
            SslEngine = engine;
        }

        public string Name => "wss";
        public EndpointType Type => EndpointType.WSS;

        public Endpoint Create(string endpointString, Dictionary<string, string?> options, bool oaEndpoint) =>
            new WSSEndpoint(SslEngine, Communicator, endpointString, options, oaEndpoint);
        public Endpoint Read(InputStream istr) => new WSSEndpoint(SslEngine, Communicator, istr);
    }
}
