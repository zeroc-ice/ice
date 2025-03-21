// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

internal class EndpointFactory : Ice.Internal.EndpointFactory
{
    internal EndpointFactory(Ice.Internal.EndpointFactory factory)
    {
        _factory = factory;
    }

    public void initialize()
    {
    }

    public short type()
    {
        return (short)(EndpointI.TYPE_BASE + _factory.type());
    }

    public string protocol()
    {
        return "test-" + _factory.protocol();
    }

    public Ice.Internal.EndpointI create(List<string> args, bool server)
    {
        return new EndpointI(_factory.create(args, server));
    }

    public Ice.Internal.EndpointI read(Ice.InputStream s)
    {
        short type = s.readShort();
        Debug.Assert(type == _factory.type());

        s.startEncapsulation();
        Ice.Internal.EndpointI endpoint = new EndpointI(_factory.read(s));
        s.endEncapsulation();
        return endpoint;
    }

    public Ice.Internal.EndpointFactory clone(Ice.Internal.ProtocolInstance instance)
    {
        return this;
    }

    private readonly Ice.Internal.EndpointFactory _factory;
}
