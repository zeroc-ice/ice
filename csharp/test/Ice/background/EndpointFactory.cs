//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;

using System.Collections.Generic;
using System.Diagnostics;

internal class EndpointFactory : IceInternal.IEndpointFactory
{
    internal EndpointFactory(IceInternal.IEndpointFactory factory)
    {
        _factory = factory;
    }

    public void Initialize()
    {
    }

    public EndpointType Type()
    {
        return (EndpointType)(Endpoint.TYPE_BASE + (short)_factory.Type());
    }

    public string Transport()
    {
        return "test-" + _factory.Transport();
    }

    public Ice.Endpoint Create(string endpointString, Dictionary<string, string?> options, bool server)
    {
        return new Endpoint(_factory.Create(endpointString, options, server));
    }

    public Ice.Endpoint Read(Ice.InputStream s)
    {
        Ice.EndpointType type = (Ice.EndpointType)s.ReadShort();
        Debug.Assert(type == _factory.Type());
        Ice.Endpoint endpoint = new Endpoint(_factory.Read(s));
        return endpoint;
    }

    public void Destroy()
    {
    }

    public IceInternal.IEndpointFactory Clone(IceInternal.TransportInstance instance)
    {
        return this;
    }

    private IceInternal.IEndpointFactory _factory;
}
