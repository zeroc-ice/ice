//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using ZeroC.Ice;

using System.Collections.Generic;
using Test;

internal class EndpointFactory : IEndpointFactory
{
    internal EndpointFactory(IEndpointFactory factory) => _factory = factory;

    public EndpointType Type => (EndpointType)(Endpoint.TYPE_BASE + (short)_factory.Type);

    public string Transport => "test-" + _factory.Transport;

    public ZeroC.Ice.Endpoint Create(string endpointString, Dictionary<string, string?> options, bool server) =>
        new Endpoint(_factory.Create(endpointString, options, server)!);

    public ZeroC.Ice.Endpoint Read(InputStream s)
    {
        var type = (EndpointType)s.ReadShort();
        TestHelper.Assert(type == _factory.Type);
        ZeroC.Ice.Endpoint endpoint = new Endpoint(_factory.Read(s)!);
        return endpoint;
    }

    private readonly IEndpointFactory _factory;
}
