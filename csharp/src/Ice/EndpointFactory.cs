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

        // ice1 with old string format
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

        // URI parsing or ice2 unmarshaling
        public Endpoint Create(
            Transport transport,
            Protocol protocol,
            string host,
            ushort port,
            Dictionary<string, string> options,
            bool oaEndpoint,
            string? endpointString) =>
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

        // Only for ice1
        public Endpoint Read(InputStream istr, Transport transport)
        {
            switch (transport)
            {
                case Transport.TCP:
                case Transport.SSL:
                    return new TcpEndpoint(istr, _communicator, transport);
                case Transport.WS:
                case Transport.WSS:
                    return new WSEndpoint(istr, _communicator, transport);
                case Transport.UDP:
                    return new UdpEndpoint(istr, _communicator);
                default:
                    Debug.Assert(false);
                    throw new NotSupportedException($"the transport `{transport}' is not supported");
            }
        }

        internal EndpointFactory(Communicator communicator) => _communicator = communicator;
    }
}
