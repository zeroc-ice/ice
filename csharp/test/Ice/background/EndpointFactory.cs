//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using Test;

namespace ZeroC.Ice.Test.Background
{
    internal class EndpointFactory : IEndpointFactory
    {
        public Transport Transport => (Transport)(Endpoint.TransportBase + (short)_underlyingTransport);

        public string TransportName => "test-" + _underlyingTransport.ToString().ToLowerInvariant();

        private readonly IEndpointFactory _underlyingFactory;
        private readonly Transport _underlyingTransport;

        internal EndpointFactory(IEndpointFactory underlyingFactory, Transport underlyingTransport)
        {
            _underlyingFactory = underlyingFactory;
            _underlyingTransport = underlyingTransport;
        }

        public ZeroC.Ice.Endpoint Create(
            Transport transport,
            Protocol protocol,
            Dictionary<string, string?> options,
            bool server,
            string endpointString)
        {
            Debug.Assert(transport == Transport);

            return new Endpoint(_underlyingFactory.Create(
                _underlyingTransport, protocol, options, server, endpointString));
        }

        public ZeroC.Ice.Endpoint Read(InputStream istr, Protocol protocol)
        {
            var transport = (Transport)istr.ReadShort();
            TestHelper.Assert(transport == Transport);
            return new Endpoint(_underlyingFactory.Read(istr, protocol));
        }
    }
}
