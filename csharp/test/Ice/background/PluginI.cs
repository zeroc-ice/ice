// Copyright (c) ZeroC, Inc.

internal class PluginI : Ice.Plugin
{
    internal PluginI(Ice.Communicator communicator) => _communicator = communicator;

    public void initialize()
    {
        Ice.Internal.ProtocolPluginFacade facade = Ice.Internal.Util.getProtocolPluginFacade(_communicator);
        for (short s = 0; s < 100; ++s)
        {
            Ice.Internal.EndpointFactory factory = facade.getEndpointFactory(s);
            if (factory != null)
            {
                facade.addEndpointFactory(new EndpointFactory(factory));
            }
        }
    }

    public void destroy()
    {
    }

    private readonly Ice.Communicator _communicator;
}
