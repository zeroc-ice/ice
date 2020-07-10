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

        // Parsing of old format
        public Endpoint Create(
            Transport transport,
            Protocol protocol,
            Dictionary<string, string?> options,
            bool oaEndpoint,
            string endpointString)
        {
            switch (transport)
            {
                case Transport.TCP:
                case Transport.SSL:
                    return new TcpEndpoint(_communicator, transport, protocol, options, oaEndpoint, endpointString);
                case Transport.WS:
                case Transport.WSS:
                    return new WSEndpoint(_communicator, transport, protocol, options, oaEndpoint, endpointString);
                case Transport.UDP:
                    return new UdpEndpoint(_communicator, protocol, options, oaEndpoint, endpointString);
                default:
                    Debug.Assert(false);
                    throw new NotSupportedException($"the transport `{transport}' is not supported");
            }
        }

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
                    new TcpEndpoint(_communicator, transport, protocol, host, port, options, oaEndpoint, endpointString),
                var tv when tv == Transport.WS || tv == Transport.WSS =>
                    new WSEndpoint(_communicator, transport, protocol, host, port, options, oaEndpoint, endpointString),
                Transport.UDP =>
                    new UdpEndpoint(_communicator, protocol, host, port, options, oaEndpoint, endpointString),
                _ => throw new NotSupportedException($"the transport `{transport}' is not supported"),
            };

        public Endpoint Read(InputStream istr, Transport transport, Protocol protocol)
        {
            switch (transport)
            {
                case Transport.TCP:
                case Transport.SSL:
                    return new TcpEndpoint(istr, _communicator, transport, protocol);
                case Transport.WS:
                case Transport.WSS:
                    return new WSEndpoint(istr, _communicator, transport, protocol);
                case Transport.UDP:
                    return new UdpEndpoint(istr, _communicator, protocol);
                default:
                    Debug.Assert(false);
                    throw new NotSupportedException($"the transport `{transport}' is not supported");
            }
        }

        internal EndpointFactory(Communicator communicator) => _communicator = communicator;
    }
}
