//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace ZeroC.Ice
{
    internal class EndpointFactory : IEndpointFactory
    {
        private readonly Communicator _communicator;

        // ice1 format parsing
        public Endpoint Create(
            Transport transport,
            Protocol protocol,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString) =>
            transport switch
            {
                var tv when tv == Transport.TCP || tv == Transport.SSL =>
                    new TcpEndpoint(_communicator, transport, protocol, options, oaEndpoint, endpointString),
                var tv when tv == Transport.WS || tv == Transport.WSS =>
                    new WSEndpoint(_communicator, transport, protocol, options, oaEndpoint, endpointString),
                Transport.UDP =>
                    new UdpEndpoint(_communicator, protocol, options, oaEndpoint, endpointString),
                _ => throw new NotSupportedException(
                    $"cannot create endpoint for transport `{transport.ToString().ToLowerInvariant()}'"),
            };

        // URI parsing
        public Endpoint Create(
            Transport transport,
            Protocol protocol,
            string host,
            ushort port,
            Dictionary<string, string> options,
            bool oaEndpoint,
            string endpointString) =>
            transport switch
            {
                var tv when tv == Transport.TCP || tv == Transport.SSL =>
                    new TcpEndpoint(_communicator,
                                    transport,
                                    protocol,
                                    host,
                                    port,
                                    options,
                                    oaEndpoint,
                                    endpointString),
                var tv when tv == Transport.WS || tv == Transport.WSS =>
                    new WSEndpoint(_communicator, transport, protocol, host, port, options, oaEndpoint, endpointString),
                Transport.UDP =>
                    new UdpEndpoint(_communicator, protocol, host, port, options, oaEndpoint, endpointString),
                _ => throw new NotSupportedException(
                    $"cannot create endpoint for transport `{transport.ToString().ToLowerInvariant()}'"),
            };

        public Endpoint Read(InputStream istr, Transport transport, Protocol protocol) =>
            transport switch
            {
                var tv when tv == Transport.TCP || tv == Transport.SSL =>
                    new TcpEndpoint(istr, _communicator, transport, protocol),
                var tv when tv == Transport.WS || tv == Transport.WSS =>
                    new WSEndpoint(istr, _communicator, transport, protocol),
                Transport.UDP =>
                    new UdpEndpoint(istr, _communicator, protocol),
                _ => throw new NotSupportedException(
                    $"cannot read endpoint for transport `{transport.ToString().ToLowerInvariant()}'"),
            };

        internal EndpointFactory(Communicator communicator) => _communicator = communicator;
    }
}
