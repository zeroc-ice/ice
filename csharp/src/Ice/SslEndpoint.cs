//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Net;

namespace ZeroC.Ice
{
    /// <summary> Endpoint represents a TLS layer over another endpoint.</summary>
    public class SslEndpoint : TcpEndpoint
    {
        public override bool IsSecure => true;
        public override EndpointType Type => EndpointType.SSL;

        private protected SslEngine SslEngine { get; }

        public override bool Equals(Endpoint? other)
        {
            if (ReferenceEquals(this, other))
            {
                return true;
            }
            return other is SslEndpoint && base.Equals(other);
        }

        public override bool Equivalent(Endpoint endpoint) => endpoint is SslEndpoint && base.Equivalent(endpoint);

        public override ITransceiver? GetTransceiver() => null;

        internal SslEndpoint(SslEngine engine, Communicator communicator, string host, int port, IPAddress? sourceAddress,
            int timeout, string connectionId, bool compressionFlag)
            : base(communicator, host, port, sourceAddress, timeout, connectionId, compressionFlag) =>
            SslEngine = engine;

        internal SslEndpoint(SslEngine engine, Communicator communicator, InputStream istr)
            : base(communicator, istr) => SslEngine = engine;

        internal SslEndpoint(SslEngine engine, Communicator communicator, string endpointString,
            Dictionary<string, string?> options, bool oaEndpoint)
            : base(communicator, endpointString, options, oaEndpoint) => SslEngine = engine;

        private protected override IPEndpoint CreateEndpoint(string host, int port, string connectionId,
            bool compressionFlag, int timeout) =>
            new SslEndpoint(SslEngine, Communicator, host, port, SourceAddress, timeout, connectionId,
                compressionFlag);

        internal override ITransceiver CreateTransceiver(string transport, StreamSocket socket, string? adapterName) =>
            new SslTransceiver(Communicator, SslEngine, base.CreateTransceiver(transport, socket, adapterName),
                adapterName ?? Host, adapterName != null);
    }

    internal sealed class SslEndpointFactory : IEndpointFactory
    {
        public EndpointType Type => EndpointType.SSL;
        public string Name => "ssl";

        private SslEngine SslEngine { get; }
        private Communicator Communicator { get; }

        public SslEndpointFactory(Communicator communicator, SslEngine engine)
        {
            Communicator = communicator;
            SslEngine = engine;
        }

        public Endpoint Create(string endpointString, Dictionary<string, string?> options, bool oaEndpoint) =>
            new SslEndpoint(SslEngine, Communicator, endpointString, options, oaEndpoint);
        public Endpoint Read(InputStream istr) => new SslEndpoint(SslEngine, Communicator, istr);
    }
}
