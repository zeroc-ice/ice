//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Net;

namespace ZeroC.Ice
{
    /// <summary>Endpoint represents a TLS layer over another endpoint.</summary>
    // TODO: fix summary
    public class SslEndpoint : TcpEndpoint
    {
        public override bool IsSecure => true;
        public override Transport Transport => Transport.SSL;

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

        internal SslEndpoint(
            SslEngine engine,
            Communicator communicator,
            Protocol protocol,
            string host,
            int port,
            IPAddress? sourceAddress,
            int timeout,
            string connectionId,
            bool compressionFlag)
            : base(communicator, protocol, host, port, sourceAddress, timeout, connectionId, compressionFlag) =>
            SslEngine = engine;

        internal SslEndpoint(SslEngine engine, InputStream istr, Protocol protocol)
            : base(istr, protocol) => SslEngine = engine;

        internal SslEndpoint(
            SslEngine engine,
            Communicator communicator,
            Protocol protocol,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
            : base(communicator, protocol, options, oaEndpoint, endpointString) => SslEngine = engine;

        private protected override IPEndpoint CreateIPEndpoint(
            string host,
            int port,
            string connectionId,
            bool compressionFlag,
            int timeout) =>
            new SslEndpoint(SslEngine,
                            Communicator,
                            Protocol,
                            host,
                            port,
                            SourceAddress,
                            timeout,
                            connectionId,
                            compressionFlag);

        internal override ITransceiver CreateTransceiver(string transport, StreamSocket socket, string? adapterName) =>
            new SslTransceiver(Communicator,
                               SslEngine,
                               base.CreateTransceiver(transport, socket, adapterName),
                               adapterName ?? Host, adapterName != null);
    }

    internal sealed class SslEndpointFactory : IEndpointFactory
    {
        private SslEngine SslEngine { get; }
        private Communicator Communicator { get; }

        public SslEndpointFactory(Communicator communicator, SslEngine engine)
        {
            Communicator = communicator;
            SslEngine = engine;
        }

        public Endpoint Create(
            Transport transport,
            Protocol protocol,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
        {
            Debug.Assert(transport == Transport.SSL);
            return new SslEndpoint(SslEngine, Communicator, protocol, options, oaEndpoint, endpointString);
        }

        public Endpoint Read(InputStream istr, Protocol protocol) => new SslEndpoint(SslEngine, istr, protocol);
    }
}
